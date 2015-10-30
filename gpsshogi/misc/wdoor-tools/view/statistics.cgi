#!/usr/bin/perl -w
use strict;

use CGI;
use File::Basename;
use URI::Escape;
use FileHandle;
use YAML::Tiny;

my $url_base = "http://wdoor.c.u-tokyo.ac.jp/shogi/logs/";
my $file_base = "/var/www/shogi/logs/";
my $event = "LATEST";
my $filter = "floodgate";
my $user = undef;
my $user_plus = undef;
my $user_at = undef;
my $show_self_play = 0;
my $view = "http://wdoor.c.u-tokyo.ac.jp/shogi/tools/view-a/index.cgi?go_last=on&amp;csa=";
my $table = "http://wdoor.c.u-tokyo.ac.jp/shogi/tools/view/latest-table.cgi?";
my $show_player = "http://wdoor.c.u-tokyo.ac.jp/shogi/tools/view/show-player.cgi?";

my $ire = '[A-Za-z0-9_@-]+';

sub yaml_name($$) {
    my ($filter,$date) = @_;
    my $yaml = "current"; # "players";
    $yaml .= "-$filter"
	if ($filter);
    $yaml .= "-$date"
	if ($date);
    return $yaml.".yaml";
}

sub read_rating ($) {
    my ($filename) = @_;
    my $yaml = YAML::Tiny->new;
    $yaml = YAML::Tiny->read($filename);

    my $rating = {};
    my $root = $yaml->[0]->{players};
    foreach my $gid (keys %$root) {
	my $group = $root->{$gid};
	foreach my $user (keys %$group) {
	    $rating->{$user} = $group->{$user};
	}
    }
    return $rating;
}
my %rating_cache; # todo: see filename
sub find_rating ($$) {
    my ($filename, $user) = @_;
    return @{$rating_cache{$user}}
	if (defined $rating_cache{$user});
    my $rating = read_rating($filename);
    $user =~ s/@[^+]*//;
    foreach my $player (keys %$rating) {
	if ($player =~ /^$user\+/) {
	    $rating_cache{$user} = [$rating->{$player},$player];
	    return @{$rating_cache{$user}};
	}
    }
    return (undef,undef);
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

my $count = {};
my $total = 0;
my $wins = {};

my $rating_width = 200; # 100;
my $wins_r100 = {};

my $sente_wins = {};
my $sente_losses = {};
my $gote_wins = {};
my $gote_losses = {};

sub pickup ($$$$) {
    my ($indexname, $file_base, $filter, $yaml_file) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime(time);
    $year += 1900;
    $mon += 1;

    my @records;
    open INDEX, $file_base."/$indexname"
	|| die "$!";
    while (my $line=<INDEX>) {
	chomp $line;
	my ($file, $sente, $gote, $moves, $time_black, $time_white,
	    $result, $reason) = split(/\s+/,$line,8);
	next
	    if ($filter && $file !~ /$filter-/);
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
	next	# not started
	    unless $reason;
	$count->{$reason}++;
	$wins->{$result}++;
	my ($rsente) = &find_rating($yaml_file, $record->{sente});
	my ($rgote) =  &find_rating($yaml_file, $record->{gote});
	if ($rsente->{rate} > 0 && $rgote->{rate} > 0 && abs($rsente - $rgote) <= $rating_width) {
	    $wins_r100->{$result}++;
	}
	if ($result eq "sente") {
	    $sente_wins->{$record->{sente}}++;
	    $gote_losses->{$record->{gote}}++;
	} elsif ($result eq "gote") {
	    $gote_wins->{$record->{gote}}++;
	    $sente_losses->{$record->{sente}}++;
	}
	++$total;
    }
    close INDEX;
}

my $query = new CGI;
print $query->header(-charset=>'euc-jp');
print $query->start_html(-title=>'Statistics', -encoding=>"EUC-JP",
			 -lang=>"ja",
			 -style=>{'src'=>'/shogi/shogi.css'},
);

if ($query->param) {
    if (defined $query->param('event')
	&& ($query->param('event') =~ /^[A-z0-9-]+$/)) {
	$event = $query->param('event');
    }
    if (defined $query->param('filter')
	&& ($query->param('filter') =~ /^[A-z0-9-]+$/)) {
	$filter = $query->param('filter');
    }
}

$url_base  .= $event . "/";
$file_base .= $event . "/";

my $options = "event=$event&amp;filter=$filter";
$options .= "&amp;show_self_play=1"
    if ($show_self_play);
$table       .= $options;
$show_player .= $options;

my $yaml_name = yaml_name($filter,undef);
my $yaml_file = "$file_base/".$yaml_name;

print "<h1>$filter statistics: ";
print "</h1>\n";

&pickup("finished.txt", $file_base, $filter, $yaml_file);

print "<h2>games finished by..</h2>\n";

my @reasons = ("toryo", "time up", "kachi", "sennichite", "uchifuzume", "oute_sennichite", "illegal move", "abnormal");
#my @reasons = sort keys %$count;
print "<div class=\"summary\"><table style=\"width:100%\" border=0 cellspacing=0 cellpadding=4>\n";
print "<tr>";
foreach my $reason (@reasons) {
    print "<th>$reason</th>";
}
print "</tr>\n<tr>";
foreach my $reason (@reasons) {
    printf "<td align=\"right\">%d</td>", $count->{$reason};
}
print "</tr>\n<tr>";
foreach my $reason (@reasons) {
    printf "<td align=\"right\">%.2f%%</td>", 100.0*$count->{$reason}/$total;
}
print "</tr>\n</table></div>\n";

print "<h2>winner</h2>\n";
my @winners = ("sente", "gote", "draw", "unknown");

print "<div class=\"summary\"><table style=\"width:100%\" border=0 cellspacing=0 cellpadding=4>\n";
print "<tr>";
foreach my $result (@winners) {
    print "<th>$result</th>";
}
printf "</tr>\n<tr>";
foreach my $result (@winners) {
    printf "<td align=\"right\">%d</td>", $wins->{$result}
}
printf "</tr>\n<tr>";
foreach my $result (@winners) {
    printf "<td align=\"right\">%.2f%%</td>", 100.0*$wins->{$result}/$total;
}
print "</tr>\n</table></div>\n";

print "<h2>sente's advantage?</h2>\n";
print "<h3>all games</h3>\n";
my $decided = $wins->{sente}+$wins->{gote};
print "<div class=\"summary\"><table style=\"width:100%\" border=0 cellspacing=0 cellpadding=4>\n";
print "<tr><th>sente win</th><th>gote win</th><th>confidence</th></tr>\n";
printf "<tr><td align=\"right\">%d</td><td align=\"right\">%d</td><td align=\"right\">%.2f</td></tr>\n",
    $wins->{sente}, $wins->{gote},
    confidence_bi(0.5, $wins->{sente}, $wins->{gote});
printf "<tr><td align=\"right\">%.2f%%</td><td align=\"right\">%.2f%%</td><td></td></tr>\n",
    100.0*$wins->{sente}/$decided, 100.0*$wins->{gote}/$decided;
printf "</table></div>\n";

print "<h3>games where abs(rate(sente) - rate(gote)) <= $rating_width</h3>\n";
my $decided_r100 = $wins_r100->{sente}+$wins_r100->{gote};
print "<div class=\"summary\"><table style=\"width:100%\" border=0 cellspacing=0 cellpadding=4>\n";
print "<tr><th>sente win</th><th>gote win</th><th>confidence</th></tr>\n";
printf "<tr><td align=\"right\">%d</td><td align=\"right\">%d</td><td align=\"right\">%.2f</td></tr>\n",
    $wins_r100->{sente}, $wins_r100->{gote},
    confidence_bi(0.5, $wins_r100->{sente}, $wins_r100->{gote});
printf "<tr><td align=\"right\">%.2f%%</td><td align=\"right\">%.2f%%</td><td></td></tr>\n",
    100.0*$wins_r100->{sente}/$decided_r100, 100.0*$wins_r100->{gote}/$decided_r100;
printf "</table></div>\n";


print "<h3>for each player (&gt; 100 wins)</h3>\n";

my @players =
    sort { my ($aa) = &find_rating($yaml_file, $a); my ($bb) = &find_rating($yaml_file, $b); $bb->{rate} <=> $aa->{rate} }
    grep { $sente_wins->{$_} >= 100 && $gote_wins->{$_} >= 100; }
    keys %$sente_wins;
print "<div class=\"summary\"><table style=\"width:100%\" border=0 cellspacing=0 cellpadding=4>\n";
print "<tr><th>player</th><th>sente wins</th><th>losses</th><th></th><th>gote wins</th><th>losses</th><th></th></tr>\n";

my $printed = 0;
foreach my $player (@players) {
    my $class = ($printed++ % 2) ? "odd" : "even";
    printf "<tr class=\"$class\"><td>%s</td><td align=\"right\">%d</td><td align=\"right\">%d</td><td align=\"right\">%.2f%%</td><td align=\"right\">%d</td><td align=\"right\">%d</td><td align=\"right\">%.2f%%</td></tr>\n", $player,
	$sente_wins->{$player}, $sente_losses->{$player},
	100.0*$sente_wins->{$player}/($sente_wins->{$player}+$sente_losses->{$player}),
	$gote_wins->{$player}, $gote_losses->{$player},
	100.0*$gote_wins->{$player}/($gote_wins->{$player}+$gote_losses->{$player});
}
printf "</table></div>\n";


print $query->end_html;


# ;;; Local Variables:
# ;;; mode:cperl
# ;;; End:
