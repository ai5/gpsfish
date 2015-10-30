#!/usr/bin/perl -w -I.
#
#Some environments that this programs looks
#
#LIMIT 1600
#GAMENAME "testgps"
#ISSENTE, ISGOTE
#LOOP
#SHOGIUSER
#SHOGIPASS

use strict;
use IO::Handle;
use File::Basename;
use network::CsaClient;
use network::GpsShogi;

### TODO: select を使って，長考中にもサーバから来る通信を読み流す

#--- Global Variables
my $test = shift || 0;
my $host = "wdoor.c.u-tokyo.ac.jp";
$host = $ENV{SERVER}
    if ($ENV{SERVER});
my $port = '4081';
$port = $ENV{PORT}
    if $ENV{PORT};
my $user = $test ? 'teamgps' : 'testgps';
my $pass = $test ? 'os4QRTvls' : 'hogetaro';
$user = $ENV{SHOGIUSER}
    if ($ENV{SHOGIUSER});
$pass = $ENV{SHOGIPASS}
    if ($ENV{SHOGIPASS});
my $gpsname = $ENV{GPSNAME} ? $ENV{GPSNAME} : "./gpsshogi -v -c";
my $limit = ($ENV{LIMIT}
	     ? $ENV{LIMIT} : 1600);
my $node_limit = ($ENV{NODELIMIT} ? $ENV{NODELIMIT} : 16000000);
my $table_size = 4800000;	# for 8GB
$table_size = $ENV{TABLE_SIZE}
    if ($ENV{TABLE_SIZE});
my $table_record_limit = 200;	# for 8GB
$table_record_limit = $ENV{TABLE_RECORD_LIMIT}
    if ($ENV{TABLE_RECORD_LIMIT});
my $force_byoyomi = $ENV{FORCE_BYOYOMI} || 0;
my $gps_opts = ($ENV{GPSOPTS} ? $ENV{GPSOPTS} : "");
my $gps_opts_human = $gps_opts;
$gps_opts_human =~ s/-P / /;
$gps_opts_human =~ s/P//;


my $logfile_basename = "logs/x1_";
my $csafile_basename = "csa/x1_";
mkdir (dirname($logfile_basename));
mkdir (dirname($csafile_basename));

$| = 1;
#--- Subroutines
sub set_config ($$$$$)
{
    my ($sente, $initial_filename, $opname, $timeleft, $byoyomi) = @_;
    my $black = ( $sente ? "gps" : $opname);
    my $white = (!$sente ? "gps" : $opname);
    if ($opname =~ /human/) {
	print STDERR "disable prediction search for human opponent\n";
    }
    my $config = { initial_filename => $initial_filename,
		   opponent => $opname,
		   sente => $sente,
		   black => $black, white => $white,
		   limit => $limit, table_size => $table_size,
		   table_record_limit => $table_record_limit,
		   node_limit => $node_limit,
		   timeleft => $timeleft, byoyomi => $byoyomi,
		   logfile_basename => $logfile_basename,
		   other_options => (($opname =~ /human/)
				     ? $gps_opts_human
				     : $gps_opts),
		   base_command => $gpsname
		 };
    return $config;
}

#--- Main routine
my $config = { sleep_before_move => $ENV{GPSSHOGI_SLEEP} };
my $client = new CsaClient($user, $pass, $config);
$client->connect($host, $port);
$client->login_x1();

my $gamename = ($ENV{'GAMENAME'} ? $ENV{'GAMENAME'} : "testgps");
my $sente_string = "*"; # dont care
if ($ENV{'ISSENTE'}) {
    $sente_string = "+";
}
elsif ($ENV{'ISGOTE'}) {
    $sente_string = "-";
}

my $loop_limit = $ENV{'LOOP'} ? $ENV{'LOOP'} : 1;

for (my $i=0; $loop_limit < 0 || $i<$loop_limit; ++$i) {
    $client->offer_game_x1($gamename, $sente_string);
    my ($sente, $initial_filename, $opname, $timeleft, $byoyomi)
	= $client->wait_opponent($csafile_basename,$ENV{'MULTIGPS'});
    if ($sente == -1) {
	$client->logout();
	$client->disconnect();
	die "ERR: error in reading csa initial strings";
    }
    if ($sente == -2) {
	warn "rejected";
	next;
    }

    $client->chat_com("yoroshiku-onegai-simasu > $opname-san");
    if ($host =~ /81dojo/) {
        my $msg = sprintf("I'm a computer program, GPSShogi Level=%d", $limit/200);
	$client->game_chat($msg);
	$client->game_chat("$opname san, yoroshiku onegai-shimasu");
	$byoyomi-= 5 if ($byoyomi >= 30);
    }
    $byoyomi = $force_byoyomi
  if $force_byoyomi > 0 && $byoyomi > $force_byoyomi;
    my $time_string = GpsShogi::make_time_string();
    my $logfilename = "$logfile_basename$time_string-$opname-comm.log";
    my $log_handle;
    open $log_handle, "> $logfilename" || die "open $! $logfilename";
    print $log_handle "log start - $logfilename\n";

    my $config = set_config($sente, $initial_filename, $opname,
			    $timeleft, $byoyomi);
    my $program = new GpsShogi($config);
    $program->set_logger($log_handle);

    $client->play($program);

    print STDERR "end game\n";
    close $log_handle;
    $program->finish();

    if ($host =~ /81dojo/) {
	$client->game_chat("Arigatou gozaimashita");
	$client->study($program);
	$client->game_chat("going to the next game");
	$client->send("CLOSE\n");
    }

    last
	unless ($ENV{'LOOP'});
    sleep 10;			# cpu cool down
}

$client->logout();
$client->disconnect();
