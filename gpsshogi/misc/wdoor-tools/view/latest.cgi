#!/usr/bin/perl -wT
use strict;
use CGI;
use File::Basename;
use URI::Escape;

my $url_base = "http://wdoor.c.u-tokyo.ac.jp/shogi/LATEST/";
my $file_base = "/var/www/shogi/logs/LATEST/";
my $view = "http://wdoor.c.u-tokyo.ac.jp/shogi/tools/view/index.cgi?go_last=on&csa=";

my $ire = '[A-Za-z0-9_@-]+';

sub make_date($){
    my ($record) = @_;
    return $record->{month} . $record->{date}
	. $record->{hour} . $record->{minute} . $record->{second};
}

sub pickup ($$) {
    my ($begin, $step) = @_;
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
	= localtime(time);
    $year += 1900;
    $mon += 1;

    my @records;
    opendir DIR, $file_base
	|| die "$!";
    while (my $file=readdir(DIR)) {
	next
	    if $file =~ /^\./;
	next
	    unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $base_file = basename $file;
	my $record = { event => $1, game => $2, sente => $3, gote => $4,
		       year => $5, month => $6, date => $7, hour => $8,
		       minute => $9, second => $10, file => $base_file,
		     };
	next
	    unless (($record->{year} == $year)
		    && ($record->{month} >= $mon -1));
#	next
#	    if ($mday - $record->{date} > 1);
	push(@records, $record);
    }
    closedir DIR;
    my @result = map { $_->{file} }
	sort { make_date($b) cmp make_date($a) } @records;
    my $end = $begin+$step-1;
    $end = $#result
	if ($end > $#result);
    return @result[$begin..$end];
}

sub make_navi($$$) {
    my ($begin, $step, $num_files) = @_;
    print "<div>";
    if ($begin > 0) {
	print "(<a href=\"latest.cgi?begin=".($begin-$step)
	    ."\">previous $step</a>)\n";
    } else {
	print "(previous)\n";
    }
    print "[".($begin)."-".($begin+$num_files)."]\n";
    if ($num_files  == $step) {
	print "(<a href=\"latest.cgi?begin=".($begin+$step)
	    ."\">next $step</a>)\n";
    } else {
	print "(next)\n";
    }
    print "</div>\n";
}

sub show($$@) {
    my ($begin, $step, @filenames) = @_;
    my $golast = "on";
    my $i = 0;
    print '<script type="text/javascript" src="./shogi-common.js"></script>', "\n";
    print '<script type="text/javascript" src="./kifu.js"></script>', "\n";

    my $num_files = @filenames + 0;
    if ($num_files == 0) {
	print "<div>no record found</div>\n";
    }
    else {
	make_navi($begin, $step, $num_files);
    }

    foreach my $filename (@filenames) {
	my $url = $url_base . $filename;
	next
	    unless $filename =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
	my $header = "$5-$6-$7 $8:$9:$10";
	print "<h2><a href=\"".$view.uri_escape($url)."\">$header</a> (<a href=\"".$url."\">csa</a>)</h2>\n";
	print '<div id="board'.$i.'" class="board"></div>', "\n";
	if ($golast eq "on") {
	    print "<script>makeBoard(\"board$i\", \"$url\", 'last')</script>\n";
	} else {
	    print "<script>makeBoard(\"board$i\", \"$url\")</script>\n";
	}
	++$i;
    }
    make_navi($begin, $step, $num_files)
	unless ($num_files == 0);
}

my $query = new CGI;
print $query->header(-charset=>'euc-jp');
print $query->start_html(-title=>'Latest Game Records', -encoding=>"EUC-JP",
			 -lang=>"ja");

print "<style> .hidden { display: none; }\n";
print ".board {  border: 1px solid #333333; border-style: outset;  width: 700px;  padding: 2.5% 0% 1% 0%; margin-left: auto; margin-right: auto; }\n";
print ".clickable:hover { color:blue }\n.moves { font-family:monospace }\n";
print ".moves { font-family:monospace; border: 1px solid #333333; border-style: inset; }\n";
print "</style>\n";

print "<h1>Latest Game Records</h1>";

my $begin = 0;
my $step = 3;
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
	$step = 3
	    if ($begin > 5);
    }
}

show($begin, $step, &pickup($begin, $step));

print $query->end_html;

# ;;; Local Variables:
# ;;; mode:cperl
# ;;; End:
