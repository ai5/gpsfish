#!/usr/bin/perl
use Time::HiRes qw( gettimeofday );
use IPC::Open2;
use Getopt::Std;
use strict;
use IO::Handle;

require("ctime.pl");

#-- settings
my $line = "/dev/ttyS0";
my $speed = "1200";
my $gpsshogi="./gpsshogi";

#-- options to gpsshogi
my %opts=();
getopts("B:Df:g:l:n:st:T:o",\%opts);

my $debug = $opts{D};
my $sente = defined $opts{'s'} ? '-s' : '';
my $node_limit = $opts{n} || 1600000;
my $table_limit = $opts{t} || 10000000;
my $table_record_limit = -5;	# for 6GB
$table_record_limit = $ENV{TABLE_RECORD_LIMIT}
    if ($ENV{TABLE_RECORD_LIMIT});
my $time_limit = defined $opts{T} ? $opts{T} : 1500;
my $byoyomi    = $opts{B} || 0;
my $read_limit = $opts{l} || 2000;
my $eval_function = $opts{e} || 'progress';
my $game_name = $opts{g}
    || die "please specify game (oppenent's) name by -g option";
my $stored_game = $opts{f};	# 中断からの再開

my ($s,$m,$h,$D,$M,$Y) = localtime;
$Y +=1900;
$M +=1;
my $time_string =
    sprintf "%04d%02d%02d%02d%02d%02d",$Y,$M,$D,$h,$m,$s;

mkdir "$game_name" || die "mkdir $!";
mkdir "$game_name/logs" || die "mkdir $!";
mkdir "$game_name/csa" || die "mkdir $!";

my $cmd = "$gpsshogi -v -c -P1 ";

$cmd .= " $sente ";
$cmd .= " -o $game_name/csa/$time_string-gps.csa ";
$cmd .= " -n $node_limit ";
$cmd .= " -t $table_limit -L$table_record_limit ";
$cmd .= " -T $time_limit -B $byoyomi ";
$cmd .= " -l $read_limit ";
$cmd .= " -e $eval_function ";
$cmd .= " -f $stored_game "
    if ($stored_game);
my $black = ( $sente ? "gps" : "rs232c_" . $game_name);
my $white = (!$sente ? "gps" : "rs232c_" . $game_name);
$cmd .= " -b $black -w $white ";
$cmd .= " 2> $game_name/logs/$time_string-gps.log";

#-- open gpsshogi
my $pid=open2(\*RDR,\*WTR,$cmd)
    || die "could note start $cmd, $!";
if (! $debug) {
    open(IN,"$line",)
	|| die "open $!";
    open(OUT,"> $line",)
	|| die "open $!";
    system("/bin/stty -brkint -icrnl ixoff -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke $speed <$line");
}
else {
    # use stdin/stdout instead of serial line
    open(IN, "-")
	|| die "open $!";
    open OUT, ">-"
	|| die "open $!";
}

open(ERR,"> $game_name/csa/$time_string-comm.csa")
    || die "open $!";
ERR->autoflush(1);
print ERR "'$cmd\n";
my $turn = &setup_header($black, $white, $stored_game);

$SIG{'INT'} = sub { print ERR "'sigint\n%CHUDAN\n"; print OUT "%CHUDAN\n"; exit 1; };
$SIG{'PIPE'} = sub { print ERR "'pipe $!\n"; print ERR "%CHUDAN\n"; print OUT "%CHUDAN\n"; exit 1; };

my $sente_lastmove = 0;
my $sente_lastmove_micro = 0;
my $gote_lastmove = 0;
my $gote_lastmove_micro = 0;

if ($sente eq '')
{
    print "gote\n";
}
else
{
    print "sente\n";
}

if ((($turn eq '+') && (! $sente)) # have to wait opponent's move
    || (($turn eq '-') && $sente))
{
    print "'waiting opponent's move\n";
    my $cominput=<IN>;
    print $cominput;
    print WTR $cominput;
    print ERR "'RECV:".&ctime(time);
    print ERR "$cominput";
    goto endgame
	if( $cominput =~ /\%/ );
    print ERR "T1\n";
    (($turn eq '+')
     ? ($sente_lastmove, $sente_lastmove_micro)
     : ($gote_lastmove, $gote_lastmove_micro))
	= gettimeofday();
}
else				# my turn
{
    (($turn eq '-')
     ? ($sente_lastmove, $sente_lastmove_micro)
     : ($gote_lastmove, $gote_lastmove_micro))
	= gettimeofday();
}

while(1){
    my $gpsinput=<RDR>;
    print  $gpsinput;
    print OUT $gpsinput;
    print ERR "'SEND:".&ctime(time);
    print ERR "$gpsinput";
    update_time($gpsinput);
    last
	if( $gpsinput =~ /\%/ );

    my $cominput=<IN>;
    print $cominput;
    print WTR $cominput;
    print ERR "'RECV:".&ctime(time);
    print ERR "$cominput";
    update_time($cominput);
    last
	if( $cominput =~ /\%/ );
}
endgame:
close ERR;
print "end game\n";

sub update_time($){
    my ($move) = @_;
    my $is_sente = ($move =~ /\+/);
    my ($now, $now_micro) = gettimeofday();
    my $diff = 0;
    if ($is_sente){
	$diff = csa_diff($now, $now_micro,
			 $gote_lastmove, $gote_lastmove_micro);
	($sente_lastmove, $sente_lastmove_micro) = ($now, $now_micro);
    }
    else{
	$diff = csa_diff($now, $now_micro,
			 $sente_lastmove, $sente_lastmove_micro);
	($gote_lastmove, $gote_lastmove_micro) = ($now, $now_micro);
    }
    print ERR "T$diff\n";
}


sub csa_diff ($$$$) {
    my ($a, $a_micro, $b, $b_micro) = @_;
    my $result = $a - $b;
    --$result
	if ($a_micro - $b_micro < 0);
    return 1
	if ($result == 0);
    return $result;
# print csa_diff(3, 7, 1, 8)."\n"; # 1
# print csa_diff(1, 7, 0, 8)."\n"; # 1
# print csa_diff(1, 9, 0, 8)."\n"; # 1
# print csa_diff(3, 9, 2, 8)."\n"; # 1
# print csa_diff(4, 9, 2, 8)."\n"; # 2
# print csa_diff(4, 2, 2, 2)."\n"; # 2
# print csa_diff(14, 0, 1, 0)."\n"; # 13
# print csa_diff(14, 0, 1, 2)."\n"; # 12
}

sub setup_header($$$) {
    my ($sente_name, $gote_name, $stored_game) = @_;
    my $turn;
    print ERR "N+$sente_name\n";
    print ERR "N-$gote_name\n";
    if ($stored_game) {
	print ERR "'resuming stored game: $stored_game\n";
	open (STORED, $stored_game)
	    || die "open $! $stored_game";
	while (my $line = <STORED>) {
	    chomp $line;
	    $line = "'" . $line
		if ($line =~ /^[N%]/);
	    print ERR $line, "\n";
	    $turn = '-'
		if ($line =~ /^\+/);
	    $turn = '+'
		if ($line =~ /^-/);
	}
	close STORED;
	print ERR "'end of $stored_game\n";
    } else {
	print ERR "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n";
	print ERR "P2 * -HI *  *  *  *  * -KA * \n";
	print ERR "P3-FU-FU-FU-FU-FU-FU-FU-FU-FU\n";
	print ERR "P4 *  *  *  *  *  *  *  *  * \n";
	print ERR "P5 *  *  *  *  *  *  *  *  * \n";
	print ERR "P6 *  *  *  *  *  *  *  *  * \n";
	print ERR "P7+FU+FU+FU+FU+FU+FU+FU+FU+FU\n";
	print ERR "P8 * +KA *  *  *  *  * +HI * \n";
	print ERR "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n";
	print ERR "+\n";
	$turn = '+';
    }
    print ERR "'rs232c game start\n";
    return $turn;
}
