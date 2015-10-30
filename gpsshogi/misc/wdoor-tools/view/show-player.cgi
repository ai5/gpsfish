#!/usr/bin/perl
# -w
use strict;

use CGI;
use File::Basename;
use URI::Escape;
use FileHandle;
use YAML::Tiny;
use Date::Calc qw( Delta_Days );

my $url_base = "http://".$ENV{HTTP_HOST}.$ENV{REQUEST_URI};
$url_base =~ s|view/.*||;
my $file_base = "/var/www/shogi/";
my $event = "LATEST";
my $filter = undef;
my $user = undef;
my $user_plus = undef;
my $user_at = undef;
my $show_self_play = 0;
my $view = "/shogi/view";
my $table = "/shogi/view/latest-table.cgi?";
my $show_player = "/shogi/view/show-player.cgi?";

my $ire = '[A-Za-z0-9_@.-]+';

my @todays_result = (0,0,0);
my %todays_win;
my %todays_loss;
# XXX:
my @year_result = (0,0,0);
my %year_win;
my %year_loss;
my $range = 7;

sub make_today ($) {
    my ($offset) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time+$offset);
    $year += 1900; $mon += 1;
    return sprintf('%04d%02d%02d', $year, $mon, $mday);
}
sub make_year ($) {
    my ($offset) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time+$offset);
    $year += 1900; 
    return sprintf('%04d', $year);
}

my $today = make_today(0);
my $yesterday = make_today(-86400);
my $ago7days = make_today(-86400*7);
my $this_year = make_year(0);

sub ratio ($$) {
    my ($win, $loss) = @_;
    return 0
	unless ($win+$loss);
    return sprintf("%.2f", 1.0*$win/($win+$loss));
}
sub addparen ($) {
    my ($val) = @_;
    return "(".$val.")"
	if ($val);
    return "-";
}
sub diff_str ($) {
    my ($diff) = @_;
    return "-" unless ($diff);
    return sprintf ("(+%d)", $diff);
}

sub make_date($){
    my ($record) = @_;
    return $record->{year} . $record->{month} . $record->{date}
	. $record->{hour} . $record->{minute} . $record->{second};
}

sub yaml_name($$) {
    my ($filter,$date) = @_;
    my $yaml = ($filter eq "floodgate" && !$date) ? "current" : "players";
    $yaml .= "-$filter"
	if ($filter);
    $yaml .= "-$date"
	if ($date);
    return $yaml.".yaml";
}

my %yaml_cache;
sub read_rating ($) {
    my ($filename) = @_;
    my $root = $yaml_cache{$filename};
    unless ($root) {
      return {}
        unless (-e $filename);
      my $yaml = YAML::Tiny->new;
      $yaml = YAML::Tiny->read($filename);
      $root = $yaml_cache{$filename} = $yaml->[0]->{players};
    }
    my $rating = {};
    foreach my $gid (keys %$root) {
	my $group = $root->{$gid};
	foreach my $user (keys %$group) {
	    $rating->{$user} = $group->{$user} if $group->{$user}->{rate};
	}
    }
    return $rating;
}

sub find_rating ($$) {
    my ($filename, $user) = @_;
    my $rating = read_rating($filename);
    $user =~ s/@[^+]*//;
    my $user_short = $user;
    $user_short =~ s/\+.*//;
    my ($rate, $record, $player_found) = (-3000, undef, undef);
    foreach my $player (keys %$rating) {
      return ($rating->{$player},$player)
	if ($player eq $user);
      ($rate, $record, $player_found)
	= ($rating->{$player}->{rate}, $rating->{$player},$player)
	  if ($player =~ /^$user_short\+/ && $rate < $rating->{$player}->{rate});
    }
    return ($record,$player_found);
}

sub uniq (@) {
    my $prev = 'nonesuch';
    my @result = grep($_ ne $prev && (($prev) = $_), sort @_);
    return @result;
}

sub player_link ($) {
    my ($player) = @_;
    $player =~ /([^@]+)(@.*)*/;
    return '<a href="'.$show_player.'&amp;user='.$player.'">'."$1</a>$2";
}

sub day_link ($) {
    my ($link_range) = @_;
    my $content = $link_range;
    $content = "[".$content."]"
      if ($link_range == $range);
    return '<a href="'.$show_player.'&amp;user='.$user
	.'&amp;range='.$link_range.'">'.$content."</a>";
}

### migrated from bi.pl
my %probability_memoize;
sub probability_cumulative($$$){
    my ($p, $total, $win) = @_;
    my $key = $p."-".$total."-".$win;
    return $probability_memoize{$key}
	if ($probability_memoize{$key});

    my $sum = 0.0;
    foreach my $i ($win .. $total) {
	my $key_i = $p."-".$total."-".$i;
	if ($probability_memoize{$key}) {
	    $sum += $probability_memoize{$key};
	    last;
	}
	$sum += probability($p, $total, $i)
    }
    return $probability_memoize{$key} = $sum;
}
sub log_probability($$$){
    my ($p, $total, $win) = @_;
    return log_cnr($total, $win) + $win * log($p) + ($total-$win) * log(1-$p);
}
sub log_factorial($){
    # Stirling's formula
    my ($n) = @_;
    my $pi = 3.1415926535897932;
    return ($n+0.5)*log($n) - $n+0.5*log(2*$pi)
	+ 1.0/12/$n - 1.0/360/$n/$n/$n + 1.0/1260/$n/$n/$n/$n/$n;
}
sub log_cnr($$){
    my ($n, $r) = @_;
    return 0
	if (($n == $r) || ($r == 0));
    return log_factorial($n)-log_factorial($r)-log_factorial($n-$r);
}
sub probability($$$){
    my ($p, $total, $win) = @_;
    return exp(log_probability($p, $total, $win));
}
sub confidence_bi ($$$) {
    my ($p, $win, $loss) = @_;
    my $total = $win+$loss;
    return probability_cumulative($p,$total,$win)
	if ($p < $win/($win+$loss));
    return probability_cumulative(1.0-$p,$total,$loss);
}
###

my ($last_date, $last_range);
sub delta_days($$$$$$) {
  my ($year0, $mon0, $day0, $year1, $mon1, $day1) = @_;
  return $last_range if ($last_date eq "$year0/$mon0/$day0");
  $last_date = "$year0/$mon0/$day0";
  return ($last_range = Delta_Days($year0, $mon0, $day0, $year1, $mon1, $day1));
}

sub pickup ($$$$$$) {
    my ($indexname, $file_base, $begin, $step, $filter) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime(time);
    $year += 1900;
    $mon += 1;

    my @records;
    open (INDEX, "/usr/bin/tac $file_base/$indexname|")
	|| die "$!";
    while (my $line=<INDEX>) {
	my ($file, $sente, $gote, $moves, $time_black, $time_white,
	    $result, $reason) = split(/\s+/,$line,8);
	next
	    if ($filter && $file !~ /$filter-/);
	next
	    unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/o;
	my $record = { event => $1, game => $2, sente => $3, gote => $4,
		       year => $5, month => $6, date => $7, hour => $8,
		       minute => $9, second => $10, file => $file_base."/".$file,
		     };
	if ($range >= 365) {
	  last unless $record->{year} >= $year-($range+364)/365;
	} else {
	  last
	    unless (($record->{year} == $year)
		    || ($record->{year} == $year-1
			&& ($record->{month} >= $mon))
		   );
	}
	next
	    if ($user
		&& $record->{sente} !~ /^$user(@.*)*$/
		&& $record->{gote} !~ /^$user(@.*)*$/);
	next
	    if ($show_self_play == 0
		&& substr($record->{sente}, 0, 3)
		eq substr($record->{gote}, 0, 3));
	$record->{moves}  = $moves;
	$record->{time}   = [$time_black, $time_white];
	$record->{result} = $result;
	$record->{reason} = $reason;
	if ($file =~ /$today/o) {
	    if ($record->{result} eq "sente") {
		if ($record->{sente} =~ /^$user(@.*)*$/) {
		    $todays_result[0]++;
		    $todays_win{$record->{gote}}++;
		} else {
		    $todays_result[1]++;
		    $todays_loss{$record->{sente}}++;
		}
	    } elsif ($record->{result} eq "gote") {
		if ($record->{sente} =~ /^$user(@.*)*$/) {
		    $todays_result[1]++;
		    $todays_loss{$record->{gote}}++;
		} else {
		    $todays_result[0]++;
		    $todays_win{$record->{sente}}++;
		}
	    } elsif ($record->{result} eq "draw") {
		$todays_result[2]++;
	    }
	}
	last unless (delta_days($record->{year},$record->{month},$record->{date},
				$year,$mon,$mday) < $range);
	$record->{key} = make_date($record);
	push(@records, $record);

	{
	    if ($record->{result} eq "sente") {
		if ($record->{sente} =~ /^$user(@.*)*$/) {
		    $year_result[0]++;
		    $year_win{$record->{gote}}++;
		} else {
		    $year_result[1]++;
		    $year_loss{$record->{sente}}++;
		}
	    } elsif ($record->{result} eq "gote") {
		if ($record->{sente} =~ /^$user(@.*)*$/) {
		    $year_result[1]++;
		    $year_loss{$record->{gote}}++;
		} else {
		    $year_result[0]++;
		    $year_win{$record->{sente}}++;
		}
	    } elsif ($record->{result} eq "draw") {
		$year_result[2]++;
	    }
	}
    }
    close INDEX;
    my @result =
	sort { $b->{key} cmp $a->{key} } @records;
    my $end = $begin+$step-1;
    $end = $#result
	if ($end > $#result);
    return @result[$begin..$end];
}

sub make_match_link($$) {
    my ($sente,$gote) = sort({$a cmp $b} @_);
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime(time);
    $year += 1900;
    $mon += 1;
    return sprintf('%shtml/current/%04d-%02d-%s-%s.html',
		   $url_base,$year,$mon,$sente,$gote);
#    return sprintf('%s%04d/%02d/html/%04d-%02d-%s-%s.html',
#		   $url_base,$year,$mon,$year,$mon,$sente,$gote);
}

sub show($$@) {
    my ($begin, $step, @records) = @_;
    my $i = 0;

    my $num_files = @records + 0;
    print '<div>'."\n";
    if ($num_files == 0) {
	print "no record found in $event";
	print ", user $user"
	    if ($user);
	print ", event $filter"
	    if ($filter);
	print "\n";
    } else {
	print '<div class="summary">'."\n";
	print '<table width="100%" border="0" cellspacing="0" cellpadding="4" summary="summary">'."\n";
	print "<tr><th>date, time</th><th>csa</th><th>black</th><th>sec</th><th>/</th><th>white</th><th>sec</th><th>moves</th><th>result</th></tr>\n";
	my $printed = 0;
    foreach my $record (@records) {
	my $filename = basename $record->{file};
	next
	    unless $filename =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $path = $5 ."/". $6 ."/". $7 . "/" . $filename;
	my $url = $url_base . "/". $path;
	my $header = "$5-$6-$7 $8:$9:$10";
	my $class = ($printed % 2) ? "odd" : "even";
	++$printed;
	print '<tr class="'.$class.'">'."<td><a href=\"".$view."/".$path."\">$header</a></td><td>(<a href=\"".$url."\">csa</a>)</td>";
	# sente
	print '<td>';
	print '<span class="winner">'
	    if ($record->{result} eq "sente");
	print &player_link($record->{sente});
	print "*</span>"
	    if ($record->{result} eq "sente");
	print "</td>";
	print '<td align="right">'.$record->{time}[0]."</td>";
	print '<td><a href="'.make_match_link($record->{sente},$record->{gote})
	    .'">'."v.s.".'</a></td>';
	# gote
	print '<td>';
	print '<span class="winner">'
	    if ($record->{result} eq "gote");
	print &player_link($record->{gote});
	print "*</span>"
	    if ($record->{result} eq "gote");
	print "</td>";

	print '<td align="right">'.$record->{time}[1]."</td>";
	print '<td align="right">'."$record->{moves}</td><td>$record->{reason}</td>";
	print "</tr>\n";
	++$i;
    }
	print "</table>\n";
	print "</div>\n";
    }
    print "</div>\n";
}

my $query = new CGI;
print $query->header(-charset=>'euc-jp');
print $query->start_html(-title=>'Player Statistics', -encoding=>"EUC-JP",
			 -lang=>"ja",
			 -style=>{'src'=>'/shogi/shogi.css'},
			 -head=>CGI::Link({-rel=>'shortcut icon',
					   -href=>'/shogi/favicon.ico',
					   -type=>"image/jpeg"})
);

my $begin = 0;
my $step = 20;
if ($query->param) {
    if (defined $query->param('event')
	&& ($query->param('event') =~ /^[A-z0-9-]+$/)) {
	$event = $query->param('event');
    }
    if (defined $query->param('filter')
	&& ($query->param('filter') =~ /^[A-z0-9-]+$/)) {
	$filter = $query->param('filter');
	$filter =~ s/-[0-9]+-[0-9]+-?//;
    }
    if (defined $query->param('user')
	&& ($query->param('user') =~ /^[A-Za-z0-9-_@\+.]+$/)) {
	$user = $query->param('user');
	$user_at = $user;
	$user =~ s/@[^+]*//;
	$user_plus = $user;
	$user =~ s/\+.*//;
	$user_at =~ s/\+.*//;
    }
    if (defined $query->param('show_self_play')) {
	$show_self_play = 1;
    }
    if (defined $query->param('range') && $query->param('range') =~ /^[0-9]+$/) {
	$range = $query->param('range');
    }
}

$url_base  .= $event . "/";
$file_base .= $event . "/";

my $options = "event=$event&amp;filter=$filter";
$options .= "&amp;show_self_play=1"
    if ($show_self_play);
$table       .= $options;
$show_player .= $options;

print "<h1>Player Statistics: ";
print " $user";
print " ($filter) "
    if ($filter);
print "</h1>\n";

if ($user) {
    my @playing = &pickup("playing.txt", $file_base, $begin, $step, $filter);
    my @finished = &pickup("finished.txt", $file_base, $begin, $step, $filter);

    my $yaml_name = yaml_name($filter,undef);
    my $yaml_html = $yaml_name; $yaml_html =~ s/yaml/html/; $yaml_html =~ s/current/players/;
    my $yaml_html14 = $yaml_name; $yaml_html14 =~ s/\.yaml/14.html/; $yaml_html14 =~ s/current/players/;
    print "<h2><a href=\"$url_base$yaml_html\">Current Status</a>";
    print " (<a href=\"$url_base$yaml_html14\">2 weeks</a>)"
	if ($filter);
    print "</h2>\n";
    my $yaml_file = "$file_base/".$yaml_name;
    my ($my_rating,$user_trip) = find_rating($yaml_file, $user_plus);
    my ($yss_rating) = find_rating($yaml_file, "YSS");

    my $yaml_name_old = yaml_name($filter,$yesterday);
    my ($my_rating_old) = find_rating("$file_base/rating/$yaml_name_old", $user_plus);

    my $yaml_name_old7 = yaml_name($filter,$ago7days);
    my ($my_rating_old7) = find_rating("$file_base/rating/$yaml_name_old7", $user_plus);

    if ($my_rating) {
	my $abegin="";
	my $aend = "";
	if ($my_rating->{rate}) {
	    $abegin = "<a href=\"".$url_base."rating/g/$user_trip-large.png\">";
	    $aend = "</a>";
	}
	print "<div>\n";
	print "<table cellspacing=0 cellpadding=4 class=\"summary\"><tr>\n";
	print "<th>rating (&Delta;day, &Delta;week)</th><th>approximated rating in shogi club 24<th>wins (&Delta;day)</th><th>losses (&Delta;day)</th><th>%</th><th>last played</th></tr>\n";
	printf "<tr><td align=\"center\">%d ($abegin%+d, %+d$aend)</td>",
	    $my_rating->{rate}, $my_rating->{rate} - $my_rating_old->{rate},
		$my_rating->{rate} - $my_rating_old7->{rate};
	printf "<td align=\"center\">%d</td>\n",
	    2300-$yss_rating->{rate}+$my_rating->{rate};
	printf "<td align=\"center\">%d (%+d)</td>\n",
	    $my_rating->{win},  $todays_result[0];
	printf "<td align=\"center\">%d (%+d)</td>",
	    $my_rating->{loss}, $todays_result[1];
	printf "<td align=\"center\">%.3f</td>",
	    $my_rating->{win}/($my_rating->{win}+$my_rating->{loss});
	my $date = $my_rating->{last_modified};
	$date =~ s/\+09:00//;
	print "<td align=\"center\">$date</td></tr>\n";
	print "</table>\n";
	print "</div>\n";
    }
    if (@playing) {
	print "<h2><a href=\"$table\">Now Playing</a></h2>\n";
	show($begin, $step, @playing);
    }
    if ($year_result[0]+$year_result[1]) {
	print "<h2>Summary</h2>";
	print "<table cellspacing=0 cellpadding=4  class=\"summary\">\n";
	print "<tr><th>v.s.</th><th>wins</th><th>(&Delta;day)</th><th>losses</th><th>(&Delta;day)</th><th>%</th><th>(today)</th></tr>\n";
	my $printed = 0;
	my @opponents = grep { $todays_win{$_} + $todays_loss{$_} + $year_win{$_} + $year_loss{$_} } uniq(keys %year_win, keys %year_loss);
	my %ratings;
	foreach my $opponent (@opponents) {
	  my ($r) = &find_rating($yaml_file, $opponent);
	  $ratings{$opponent} = $r;
	}
	foreach my $opponent (sort { $ratings{$b}->{rate} <=> $ratings{$a}->{rate} } @opponents) {
	    my $op_rating = $ratings{$opponent};
	    my $rwin = $year_win{$opponent} / ($year_win{$opponent}+$year_loss{$opponent});
	    my $rwinr = 1.0/(1.0+exp(log(10.0)/400.0
				    *($op_rating->{rate}-$my_rating->{rate})));
	    my $class = ($todays_win{$opponent} + $todays_loss{$opponent})
		? "today"
		    : ($printed++ % 2) ? "odd" : "even";
	    my $confidence =
		confidence_bi($rwinr,$year_win{$opponent},$year_loss{$opponent});
	    my $rwin_string = "";
	    if ($rwinr < $rwin && $confidence <= 0.1) {
		my $title = sprintf("more wins than expected: %.3f &gt; %.3f (confidence %.2f)",
				    $rwin, $rwinr, $confidence);
		$rwin_string = "<span class=\"morewin\" title=\"$title\">";
		if ($confidence <= 0.01) {
		    $rwin_string .= "+++";
		} elsif ($confidence <= 0.05) {
		    $rwin_string .= "++";
		} elsif ($confidence <= 0.1) {
		    $rwin_string .= "+";
		}
		$rwin_string .= "</span>";
	    } elsif ($rwinr > $rwin && $confidence <= 0.1) {
		my $title = sprintf("more losses than expected: %.3f &lt; %.3f (confidence %.2f)",
				    $rwin, $rwinr, $confidence);
		$rwin_string = "<span class=\"moreloss\" title=\"$title\">";
		if ($confidence <= 0.01) {
		    $rwin_string .= "!!!";
		} elsif ($confidence <= 0.05) {
		    $rwin_string .= "!!";
		} elsif ($confidence <= 0.1) {
		    $rwin_string .= "!";
		}
		$rwin_string .= "</span>";
	    }
	    printf "<tr class=\"$class\"><td align=\"center\">%s</td><td align=\"right\">%d</td><td align=\"center\">%s</td><td align=\"right\">%d</td><td align=\"center\">%s</td><td align=\"left\"><a href=\"%s\">%s</a>%s<td align=\"center\">%s</td></tr>\n",
		&player_link($opponent),
		$year_win{$opponent}, &diff_str($todays_win{$opponent}),
		$year_loss{$opponent},&diff_str($todays_loss{$opponent}),
		make_match_link($user_at, $opponent),
		&ratio($year_win{$opponent}, $year_loss{$opponent}),
		$rwin_string,
		&addparen(&ratio($todays_win{$opponent}, $todays_loss{$opponent}));
	}
	print "</table>\n";

	print "<ul><li>List player's games: <a href=\"$table\">all</a>";
	print "<li>Recent days: ".&day_link(1).", ".&day_link(2)
	    .", ".&day_link(7).", ".&day_link(14).", ".&day_link(30)
		.", ".&day_link(365)."";
	print "</ul>\n";
    }
#    my $monthly = $url_base."html/#".$user."-0";
#    print "<h2><a href=\"$monthly\">Monthly Summary</a></h2>\n";
    print "<h2>Game History</h2>\n";
    print '<div><a href="'.$table."&amp;user=$user".'">more</a>'."</div>\n";
    show($begin, $step, @finished);
    print '<div><a href="'.$table."&amp;user=$user".'">more</a>'."</div>\n";
}

print $query->end_html;


# ;;; Local Variables:
# ;;; mode:cperl
# ;;; End:
