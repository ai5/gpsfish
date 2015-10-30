#!/usr/bin/perl -w
use strict;
use File::Basename;
use URI::Escape;
use FileHandle;

my $db = '/home/shogi-server/www/x/finished.txt';
my $db_cur = '/home/shogi-server/www/x/playing.txt';
my $dir = '/home/shogi-server/www/x/';
#my $cgi_base = "/shogi/tools/view/index.cgi?go_last=on&csa=";
my $cgi_base = "/shogi/view";
my $top_url = "/shogi/";
my $record_base = "http://wdoor.c.u-tokyo.ac.jp/shogi/x/";
my $css = '<link rel="StyleSheet" type="text/css" href="/shogi/shogi.css">';
my $ignore_users = "(test|yowai|debug|chu.*gps)";
my $threshold = 1;
my $table = "/shogi/view/latest-table.cgi?show_self_play=1";
my $event = "LATEST";
my $use_year = 0;
my $use_month = 0;
my %matches;

my %user_month;

sub make_filename ($) {
    my ($name) = @_;
    #$name =~ s|\.\./||g;
    #return uri_escape($record_base . $name);
    $name =~ s/^[^0-9]+//;
    return $name;
}

sub write_footer($){
    my ($fh) = @_;
    print $fh '<hr><a href="'.$top_url.'">'.'shogi-server@wdoor'.'</a>'."\n";
}

sub make_key ($$)
{
    my ($sente, $gote) = @_;
    return $sente. "-" . $gote;
}

sub make_year_month ($)
{
    my ($record) = @_;
    return "$record->{year}-$record->{month}";
}

sub make_current_date () {
    my ($sec,$min,$hour,$mday,$mon,$year) = localtime(time);
    $year += 1900; $mon += 1;
    return sprintf('%04d-%02d-%02d', $year, $mon, $mday);
}

sub make_date_time ($)
{
    my ($record) = @_;
    return &make_year_month($record). "-$record->{date} $record->{hour}:$record->{minute}";
}

sub write_match_of ($$$)
{
    my ($fh, $match, $records) = @_;
    my $count = {sente => 0, gote => 0, draw => 0, unknown => 0};
    return ($count, "")
	unless defined $records;
    my $latest = undef;
    if (@$records) {
	print $fh "<h2>$match</h2>\n";
	print $fh '<div class="summary"><table width="100%" border="0" cellspacing="0" cellpadding="4" summary="summary"><tbody>'."\n";
	print $fh "<tr><th>date</th><th>result</th><th></th><th>moves</th><th>game</th></tr>\n";
	my $printed = 0;
	foreach my $record (sort {&make_date_time($b) cmp &make_date_time($a)} @$records) {
	    $latest = &make_date_time($record)
		unless $latest;
	    my $url = $cgi_base . "/" . make_filename($record->{file});
	    my $class = ($printed++ % 2) ? "odd" : "even";
	    print $fh '<tr class="'.$class.'"><td><a href="' . $url . '">' 
		. &make_date_time($record) . "</a>\n";
	    print $fh '(<a href="../../' . $record->{file} . '">csa</a>'.")\n";
	    print $fh "</td>\n";
	    if ($record->{win}) {
		print $fh "<td>$record->{win}</td><td>$record->{reason}</td>";
	    } else {
		print $fh "<td></td><td></td>";
	    }
	    print $fh "<td align=\"right\">$record->{moves}</td>\n";
	    print $fh "<td>".$record->{game}."</td>\n";
	    ++$count->{$record->{result}};
	}
	print $fh "</table></div>\n";
    }


    print $fh "<div>$count->{sente}/$count->{gote}/$count->{draw}/$count->{unknown}<div>";
    return ($count, $latest);
}

sub write_match ($$$$) {
    my ($ym, $matches, $sente, $gote) = @_;
    my $match0 = &make_key($sente, $gote);
    my $records0 = $matches->{$match0};
    my $match1 = &make_key($gote, $sente);
    my $records1 = $matches->{$match1};
    return (0, 0, "", "")
	unless ((defined $records0) || (defined $records1));

    my $match = &make_key(sort({$a cmp $b} $sente, $gote));
    my $filename = $ym . "-" . $match . ".html";
    my $fh = new FileHandle "> $filename"
	|| die "$!";
    print $fh "<html><head><title>$match</title>"
      . '<link rel="shortcut icon" href="/shogi/favicon.ico" type="image/jpeg" />'
      . "</head>\n"
	. $css ."\n"
	. "<body><h1>$match</h1>\n";

    my ($count0, $latest0) = write_match_of($fh, $match0, $records0);
    my ($count1, $latest1) = write_match_of($fh, $match1, $records1);

    print $fh "<h2>total</h2>\n<ul>\n";
    print $fh "<li>$sente win: " . ($count0->{sente} + $count1->{gote}) . "\n";
    print $fh "<li>$gote  win: " . ($count1->{sente} + $count0->{gote}) . "\n";
    print $fh "<li>draw: " . ($count0->{draw} + $count1->{draw}) . "\n"
	if ($count0->{draw} + $count1->{draw});
    print $fh "<li>other: " . ($count0->{unknown} + $count1->{unknown}) . "\n"
	if ($count0->{unknown} + $count1->{unknown});
    print $fh "</ul>\n";

    write_footer($fh);
    print $fh "</body></html>\n";
    close $fh;

    my $count = {
		 draw => $count0->{draw}+$count1->{draw},
		 unknown => $count0->{unknown}+$count1->{unknown}
		};
    if ($sente cmp $gote) {
	$count->{sente} = $count0->{sente} + $count1->{gote};
	$count->{gote}  = $count0->{gote}  + $count1->{sente};
    }
    else{
	$count->{gote}  = $count0->{sente} + $count1->{gote};
	$count->{sente} = $count0->{gote}  + $count1->{sente};
    }

    my $latest = undef;
    if (! $latest0) {
	$latest = $latest1;
    } elsif (! $latest1) {
	$latest = $latest0;
    } else { 
	$latest = ($latest0 cmp $latest1) ? $latest0 : $latest1;
    }
    return (1, $count, $filename, $latest);
}

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
  = localtime(time);
$year += 1900; $mon += 1;
$use_year = $year;
$use_month = $mon;
my $ire = '[A-Za-z0-9_@.-]+';

sub get_records ($) {
  my ($db) = @_;
  open (DB, "/usr/bin/tac $db|")
    || die "$! $db";
  while (my $line=<DB>) {
    my ($file, $sente, $gote, $moves, $time_black, $time_white,
	$result, $reason) = split(/\s+/,$line,8);
    next
      unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/o;
    my $record = { event => $1, game => $2, sente => $3, gote => $4,
		   year => $5, month => $6, date => $7, hour => $8,
		   minute => $9, second => $10, file => "$5/$6/$7/$file"
		 };
    last
      if ($use_year > $record->{year});
    next
      unless ($use_year == $record->{year});
    last
      if ($use_month > 0 && $use_year == $record->{year} && $use_month > $record->{month});
    next
      unless ($use_month <= 0 || $use_month == $record->{month});
    next
      if ($record->{sente} =~ /$ignore_users/o
	  && $record->{game} !~ /floodgate/);
    next
      if ($record->{gote} =~ /$ignore_users/o
	  && $record->{game} !~ /floodgate/);
    $record->{moves} = $moves;
    $record->{result} = $result || "unknown";
    $record->{reason} = $reason;
    if ($result eq 'sente') {
      $record->{win} = $sente;
      $record->{lose} = $gote;
    } elsif ($result eq 'gote') {
      $record->{win} = $gote;
      $record->{lose} = $sente;
    } elsif ($result eq 'draw') {
      $record->{win} = $record->{loss} = "draw";
    }

    my $key = &make_key($record->{sente}, $record->{gote});
    my $ym = &make_year_month($record);
    $matches{$ym} = {}
      unless defined $matches{$ym};
    $matches{$ym}->{players} = {}
      unless defined $matches{$ym}->{players};
    $matches{$ym}->{matches} = {}
      unless defined $matches{$ym}->{matches};
    $matches{$ym}->{matches}->{$key} = ()
      unless defined $matches{$ym}->{matches}->{$key};
    push(@{$matches{$ym}->{matches}->{$key}}, $record);
    $matches{$ym}->{players}->{$record->{sente}}++;
    $matches{$ym}->{players}->{$record->{gote}}++;
  }
  close DB;
}

$use_year = $ARGV[0] if ($ARGV[0]);
$use_month = $ARGV[1] if ($ARGV[1]);

&get_records($db_cur);
&get_records($db);

my $INDEX = (new FileHandle "> index.html")
    || die "open $!";
print $INDEX '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">'."\n";
print $INDEX '<html lang="ja">'."<head><title>Game Summary</title>\n"
    . '<link rel="shortcut icon" href="/shogi/favicon.ico" type="image/jpeg" />'
    . $css ."\n"
    ."</head>\n";
print $INDEX "<body><h1>Game Summary</h1>\n";

print $INDEX "<h2>Index</h2>\n<div>\n";
print $INDEX join(" | ",
		  map { "<a href=\"$_.html\">$_</a>" }
		  sort {$b cmp $a} keys %matches);
# map { "<a href=\"#$_\">$_</a>" }
print $INDEX "\n</div>\n";

my $today = make_current_date();

sub make_table($$$$){
    my ($file, $ym, $pair_table, $pair_list) = @_;
    print $file "<h2><a name=\"$ym\">$ym</a></h2>\n";
    print $file '<div class="summary"><table width="100%" border="0" cellspacing="0" cellpadding="4" summary="summary">'."\n";
    print $file '<thead><tr><th>player 1</th><th>player 2</th><th>win/loss/draw/other</th><th>last play</th></tr>'."\n<tbody>\n";
    $user_month{$file} = {}
	unless ($user_month{$file});
    foreach my $sente (sort keys %{$matches{$ym}->{players}}) {
	my @pairs = split("/", $pair_list->{$sente});
	if (@pairs+0 == 1) {
	    my @others = split("/", $pair_list->{$pairs[0]});
	    next
		if (@others+0 > 1);
	}
	my $new_player = 1;
	my $printed = 0;
	foreach my $gote (@pairs) {
	    my $new_player_class = "";
	    my ($count, $filename, $latest) = @{$pair_table->{$sente."/".$gote}};
	    ++$printed;
	    if ($new_player) {
		$new_player_class = 'class="newplayer"';
		print $file "<tr $new_player_class><td $new_player_class>";
		my $anum = $user_month{$file}->{$sente} ? $user_month{$file}->{$sente} : 0;
		$user_month{$file}->{$sente} = $anum+1;
		print $file '<a name="'.$sente."-".$anum.'"> </a><a href="'.$table."event=$event&amp;user=".$sente.'">'.$sente."</a>";
		print $file "<span class=\"super\"><a href=\"#".$sente."-".($anum+1)."\">+</a></span>";
		print $file "<span class=\"super\"><a href=\"#".$sente."-".($anum-1)."\">-</a></span>"
		    if ($anum >= 1);
		print $file "</td>";
		$new_player = 0;
	    } elsif ($printed % 2) {
		print $file '<tr class="odd"><td></td>';
	    } else {
		print $file '<tr class="even"><td></td>';
	    }
	    print $file "<td $new_player_class>".'<a href="'.$table."event=$event&amp;user=".$gote.'">'.$gote."</a></td>";
	    print $file "<td $new_player_class align=".'"right"'.">".'<a href="'.$filename.'">' 
		."$count->{sente}/$count->{gote}/$count->{draw}/$count->{unknown}"
		    .'</a></td>';
	    my $is_today = ($latest =~ /$today/o);
	    my $today_string = $is_today ? " today!" : "";
	    $latest =~ s/[0-9]+:[0-9]+//;
	    print $file " <td $new_player_class>" . $latest . $today_string."</td>";
	    print $file "</tr>\n";
	}
    }
    print $file "</tbody></table></div>\n";
}

foreach my $ym (sort {$b cmp $a} keys %matches) {
    my $pair_table = {};
    my $pair_list = {};
    foreach my $sente (sort keys %{$matches{$ym}->{players}}) {
	next
	    if ($matches{$ym}->{players}->{$sente} < $threshold);
	foreach my $gote (sort keys %{$matches{$ym}->{players}}) {
	    next
		if ($matches{$ym}->{players}->{$gote} < $threshold);
	    my ($has_match, $count, $filename, $latest)
		= write_match($ym, $matches{$ym}->{matches}, $sente, $gote);
	    $pair_table->{$sente."/".$gote} = [$count, $filename, $latest];
	    if ($has_match) {
		if ($pair_list->{$sente}) {
		    $pair_list->{$sente} .= "/".$gote;
		} else {
		    $pair_list->{$sente} = $gote;
		}
	    }
	}
    }

    make_table($INDEX, $ym, $pair_table, $pair_list);

    my $YM = (new FileHandle "> $ym.html")
	|| die "open $!";
    print $YM '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">'."\n";
    print $YM '<html lang="ja">'."<head><title>Game Summary $ym</title>\n"
	. $css ."\n"
	    ."</head>\n";
    print $YM "<body><h1>Game Summary $ym</h1>\n";

    make_table($YM, $ym, $pair_table, $pair_list);

    write_footer($YM);
    print $YM "</body></html>\n";
    close $YM;
}

write_footer($INDEX);
print $INDEX "</body></html>\n";

# print "$make_html\n";
