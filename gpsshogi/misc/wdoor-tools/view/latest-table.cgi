#!/usr/bin/perl
# -wT
use strict;
use CGI;
use File::Basename;
use URI::Escape;

my $url_base = "http://".$ENV{HTTP_HOST}.$ENV{REQUEST_URI};
$url_base =~ s|view/.*||;
my $file_base = "/home/shogi-server/www/";
my $event = "LATEST";
my $filter = undef;
my $user = undef;
my $show_self_play = 0;
my $range = 364;
my $view = "/shogi/view";
my $table = "/shogi/view/latest-table.cgi?";
my $show_player = "/shogi/view/show-player.cgi?";

my $ire = '[A-Za-z0-9_@.-]+';

sub make_date($){
    my ($record) = @_;
    return $record->{year} . $record->{month} . $record->{date}
	. $record->{hour} . $record->{minute} . $record->{second};
}

sub make_navi($$$) {
    my ($begin, $step, $num_files) = @_;
    print "<div>";
    my $url = "$table";
    $url .= "&amp;user=$user"
	if ($user);
    $url .= "&amp;filter=$filter"
	if ($filter);
    if ($begin > 0) {
	print "(<a href=\"$url&amp;begin=".($begin-$step)
	    ."\">previous $step</a>)\n";
    } else {
	print "(previous)\n";
    }
    print "[".($begin)."-".($begin+$num_files)."]\n";
    if ($num_files  == $step) {
	print "(<a href=\"$url&amp;begin=".($begin+$step)
	    ."\">next $step</a>)\n";
    } else {
	print "(next)\n";
    }
    print "</div>\n";
}

sub player_link ($) {
    my ($player) = @_;
    return ""
	unless $player;
    $player =~ /([^@]+)(@.*)*/;
    return '<a href="'.$show_player.'&amp;user='.$player.'">'."$1</a>$2";
}

sub pickup_index ($$$) {
    my ($indexname, $file_base, $filter) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime(time);
    $year += 1900;
    $mon += 1;

    my @records;
    open (INDEX, "/usr/bin/tac $file_base/$indexname|")
	|| die "$! $file_base/$indexname";
    while (my $line=<INDEX>) {
	my ($file, $sente, $gote, $moves, $time_black, $time_white,
	    $result, $reason) = split(/\s+/,$line,8);
	next
	    if ($filter && $file !~ /$filter[-+]/);
	next
	    unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $record = { event => $1, game => $2, sente => $3, gote => $4,
		       year => $5, month => $6, date => $7, hour => $8,
		       minute => $9, second => $10, file => $file_base."/".$file,
		     };
	$record->{moves}  = $moves;
	$record->{time}   = [$time_black, $time_white];
	$record->{result} = $result;
	$record->{reason} = $reason;
	if ($filter =~ /-9000/ || $filter =~ /-[0-9]{5}/) {
	  last unless $record->{year} >= $year-10;
	} elsif ($range >= 365) {
	  last unless $record->{year} >= $year-($range+364)/365;
	} else {
	  last
	    unless (($record->{year} == $year
		     && $record->{month} >= $mon -1)
		    || ($record->{year} == $year-1
			&& ($record->{month} >= $mon+11))
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
	push(@records, $record);
    }
    close INDEX;
    return @records;
}

sub pickup ($$$$$) {
    my ($file_base, $begin, $step, $filter) = @_;
    my @result
	= (pickup_index("playing.txt", $file_base, $filter),
	   pickup_index("finished.txt", $file_base, $filter));
    @result = sort { make_date($b) cmp make_date($a) } @result;
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
	make_navi($begin, $step, $num_files);
	print '<div class="summary">'."\n";
	print '<table width="100%" border="0" cellspacing="0" cellpadding="4" summary="summary">'."\n";
	print "<tr><th>date, time</th><th>csa</th><th>black</th><th>sec</th><th>/</th><th>white</th><th>sec</th><th>moves</th><th>result</th></tr>\n";
	my $printed = 0;
    foreach my $record (@records) {
	my $filename = basename $record->{file};
	next
	    unless $filename =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $header = "$5-$6-$7 $8:$9:$10";
	my $path = $5 ."/". $6 ."/". $7 . "/" . $filename;
	my $url = $url_base . "/". $path;
	my $class = ($printed % 2) ? "odd" : "even";
	if ($filter eq "floodgate") {
	    $class = ($9 =~ /^0/) ? "floodgate00" : "floodgate30";
	}
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
	make_navi($begin, $step, $num_files);
    }
    print "</div>\n";
}

my $query = new CGI;
print $query->header(-charset=>'euc-jp');
print $query->start_html(-title=>'Recent Game Records', -encoding=>"EUC-JP",
			 -lang=>"ja",
			 -style=>{'src'=>'/shogi/shogi.css'},
			 -head=>CGI::Link({-rel=>'shortcut icon',
					   -href=>'/shogi/favicon.ico',
					   -type=>"image/jpeg"})
);

my $begin = 0;
my $step = 40;
if ($query->param) {
    if (defined $query->param('begin')
	&& ($query->param('begin') =~ /^\d+$/)) {
	$begin = $query->param('begin');
	$begin = 0
	    if ($begin < 0);
    }
    if (defined $query->param('step')
	&& ($query->param('step') =~ /^\d+$/)) {
	$step = $query->param('step');
    }
    if (defined $query->param('event')
	&& ($query->param('event') =~ /^[A-z0-9-]+$/)) {
	$event = $query->param('event');
    }
    if (defined $query->param('filter')
	&& ($query->param('filter') =~ /^[A-z0-9-]+$/)) {
	$filter = $query->param('filter');
    }
    if (defined $query->param('user')
	&& ($query->param('user') =~ /^[A-Za-z0-9-_@\.]+$/)) {
	$user = $query->param('user');
    }
    if (defined $query->param('show_self_play')) {
	$show_self_play = 1;
    }
    if (defined $query->param('range') && $query->param('range') =~ /^[0-9]+$/) {
	$range = $query->param('range');
    }
}
$url_base .= $event . "/";
$file_base .= $event . "/";

my $options = "event=$event&amp;filter=$filter";
$options .= "&amp;show_self_play=1"
    if ($show_self_play);
$table       .= $options;
$show_player .= $options;

print "<h1>Recent Game Records</h1>\n";
if ($filter || $user) {
    print "<div><dl>";
    if ($filter) {
	my $plain_table = $table."&amp;user=$user";
	$plain_table =~ s/&amp;filter=$filter//;
	print "<dt><a href=\"$plain_table\">Event</a>:</dt>";
	print "<dd><a href=\"$url_base/players-$filter.html\">".$filter."</a></dd>\n";
    }
    if ($user) {
	print "<dt><a href=\"$table\">User</a>:</dt><dd>$user</dd>\n";
    }
    print "</dl></div>\n";
}

show($begin, $step, &pickup($file_base, $begin, $step, $filter));

print $query->end_html;

# ;;; Local Variables:
# ;;; mode:cperl
# ;;; End:
