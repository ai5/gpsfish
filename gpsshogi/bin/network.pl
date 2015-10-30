#!/usr/bin/perl -w
use strict;
use IO::Handle;
use File::Basename;
use network::CsaClient;
use network::GpsShogi;
use network::CsaFileClient;

#--- Global Variables
my $test = shift || 0;
my $host = "wdoor.c.u-tokyo.ac.jp";
$host = $ENV{SERVER}
    if ($ENV{SERVER});
my $port = '4081';
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
#my $table_size = 10000000;	# for 6GB
$table_size = $ENV{TABLE_SIZE}
    if ($ENV{TABLE_SIZE});
my $table_record_limit = 200;
$table_record_limit = $ENV{TABLE_RECORD_LIMIT}
    if ($ENV{TABLE_RECORD_LIMIT});
my $gps_opts = ($ENV{GPSOPTS} ? $ENV{GPSOPTS} : "");
my $challenge_needed = 0;
$challenge_needed = 1
    if ($ENV{CHALLENGE});

my $use_file_session = $ENV{CSAFILESESSION} || $ENV{CSAFILESESSIONDIR} || 0;
my $file_session_dir = $ENV{CSAFILESESSIONDIR};

my $logfile_basename = "logs/network_";
my $csafile_basename = "csa/network_";
mkdir (dirname($logfile_basename));
mkdir (dirname($csafile_basename));

$| = 1;
#--- Subroutines
sub set_config ($$$$$)
{
    my ($sente, $initial_filename, $opname, $timeleft, $byoyomi) = @_;
    my $black = ( $sente ? "gps" : $opname);
    my $white = (!$sente ? "gps" : $opname);
    my $config = { initial_filename => $initial_filename,
		   opponent => $opname,
		   sente => $sente,
		   black => $black, white => $white,
		   limit => $limit, table_size => $table_size,
		   table_record_limit => $table_record_limit,
		   node_limit => $node_limit,
		   timeleft => $timeleft, byoyomi => $byoyomi,
		   logfile_basename => $logfile_basename,
		   other_options => $gps_opts,
		   base_command => $gpsname,
		 };
    return $config;
}

#--- Main routine
my $client = new CsaClient($user, $pass);
$client->connect($host, $port);
$client->login();

$client->send("CHALLENGE\n")
  if ($challenge_needed);

while (1) {
  my $file_session_config = { csa_file_session_dir => $file_session_dir };
  my $file_client = $use_file_session
    ? new CsaFileClient($file_session_config) : undef;
    my ($sente, $initial_filename, $opname, $timeleft, $byoyomi)
	= $client->wait_opponent($csafile_basename);
    if ($sente == -1) {
	$client->logout();
	$client->disconnect();
	die "ERR: error in reading csa initial strings";
    }
    if ($sente == -2) {
	warn "rejected";
	next;
    }

    my $time_string = GpsShogi::make_time_string();
    my $logfilename = "$logfile_basename$time_string-$opname-comm.log";
    my $log_handle;
    open $log_handle, "> $logfilename" || die "open $! $logfilename";
    print $log_handle "log start - $logfilename\n";

    my $config = &set_config($sente, $initial_filename, $opname,
			     $timeleft, $byoyomi);
    my $program = ($use_file_session ? $file_client : new GpsShogi($config));
    my $command = $program->{command};
    print $log_handle "$command\n";
    $program->set_logger($log_handle);

    $client->play($program);

    print STDERR "end game\n";
    close $log_handle;
    last
	unless ($ENV{'LOOP'});
    sleep 30;			# cpu cool down
}

$client->logout();
$client->disconnect();
print "end game\n";
