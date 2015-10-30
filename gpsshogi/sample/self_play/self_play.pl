#!/usr/bin/perl -w -I../../bin
# "config" を作ったディレクトリで実行
# もしくは -d で config のあるディレクトリを指定
use Getopt::Std;
use strict;
use network::CsaClient;
use network::GpsShogi;

sub usage () {
    print STDERR "$0 [-d directory (default:.)] [01]\n";
    exit 1;
}

### "3-5, 7:32, 9" => ((3,),(4,),(5,),(7,32),(9,))
sub make_array ($) {
    my ($range) = @_;
    my @elements = split(/,\s*/, $range);
    my @result;
    foreach my $e (@elements){
	if ($e =~ /^\s*(\d+)\s*-\s*(\d+)(?::(\d+))?\s*$/) {
	    my $f = ($1 < $2) ? $1 : $2;
	    my $l = ($1 < $2) ? $2 : $1;
	    foreach my $i ($f .. $l) {
		push(@result, [$i, $3]);
	    }
	}
	elsif ($e =~ /^\s*(\d+)(?::(\d+))?\s*$/) {
	    push(@result, [$1, $2]);
	}
	else{
	    warn "skip $e";
	}
    }
    return @result;
}

sub parse_line ($$$){
    my ($result, $line, $keywords) = @_;
    foreach my $key (@$keywords) {
	if ($line =~ /^$key: *(.+)/) {
	    $result->{$key} = $1;
	    return;
	}
    }
    print STDERR "ignored line: $line\n";
}

sub parse_config ($) {
    my ($filename) = @_;
    open CONFIG, "$filename"
	|| die "open $!";
    my $result = {};
    while (<CONFIG>) {
	chomp;
	parse_line($result, $_, ["gamename", "server", "port", 
				 "timeleft", "byoyomi", 
				 "record_file",
				 "record_list",
				 "csa_dir",
				 "player0", "player0_command",
				 "player1", "player1_command"]);
    }
    close CONFIG;
    $result->{byoyomi} = 0
	unless defined $result->{byoyomi};
    return $result;
}

sub show_config ($) {
    my ($config) = @_;
    foreach my $key (sort keys %$config) {
	print STDERR "$key: $config->{$key}\n";
    }
}

sub make_gamename ($$$$) {
    my ($gamename,$id,$timeleft,$byoyomi) = @_;
    return $gamename."_".$id."-$timeleft-$byoyomi";
}

# main
$| = 1;
my %options=();
getopts("d:s",\%options);
usage()
    if (@ARGV + 0 != 1);
my $player_zero = ($ARGV[0] == 0);
my $directory = $options{d} ? $options{d} : ".";
my $simulation_only = $options{'s'};
my $config_file = "$directory/config";
my $config = &parse_config($config_file);
&show_config($config);
my $user = $player_zero ? $config->{player0} : $config->{player1};
my $opponent = $player_zero ? $config->{player1} : $config->{player0};
my $pass = $user;
$pass =~ s/@.*$//;		# use same password for gps@0 and for gps@1
my $client = new CsaClient($user, $pass);
my $original_command
    = $player_zero ? $config->{player0_command} : $config->{player1_command};

sub set_config ($$$$$)
{
    my ($sente_string, $id, $timeleft, $byoyomi, $forward) = @_;
    my $sente = ($sente_string eq "+");
    my $black = ( $sente ? $user : $opponent);
    my $white = (!$sente ? $user : $opponent);
    my $basedirectory = $directory . "/" . ($player_zero ? "0" : "1");
    my $output = $basedirectory	."/$id$sente_string.csa";
    my $logfile = $basedirectory . "/$id$sente_string.log";
    my $gps_config = { opponent => $opponent,
		       sente => $sente,
		       black => $black, white => $white,
		       timeleft => $timeleft, byoyomi => $byoyomi,
		       record_filename => $output,
		       log_filename => $logfile,
		       base_command => $original_command,
		 };
    if (defined $config->{csa_dir}) {
	$gps_config->{csa_file} = $config->{csa_dir} . "/" . $id . ".csa";
    } else {
	$gps_config->{kisen_id} = $id;
    }
    if ($forward >= 0) {
	$gps_config->{book_moves} = $forward;
    }
    $gps_config->{kisen_file} = $config->{record_file}
	if $config->{record_file};;
    return $gps_config;
}

sub play ($$$) {
    my ($id, $sente_string, $forward) = @_;
    my $gamename = &make_gamename($config->{gamename}, $id, 
				  $config->{timeleft}, $config->{byoyomi});

    if ($simulation_only) {
	my $gps_config = set_config($sente_string, $id,
				    $config->{timeleft}, $config->{byoyomi},
				    $forward);
        my $program = new GpsShogi($gps_config);
	print STDERR "$sente_string $gamename\n";
	print STDERR  $program->make_command($sente_string, $id, 
					     $config->{timeleft},
					     $config->{byoyomi}),"\n";
	return;
    }

    $client->offer_game_x1($gamename, $sente_string);
    my ($sente, $csafilename, $opname, $timeleft, $byoyomi)
	= $client->wait_opponent("$directory/csa/", 1);
    if ($sente == -1) {
    	$client->logout();
	$client->disconnect();
	die "ERR: error in reading csa initial strings";
    }
    if ($sente == -2) {
	die "rejected";
    }

    my $gps_config = set_config($sente_string, $id,
				$timeleft, $byoyomi, $forward);
    my $program = new GpsShogi($gps_config);

    $client->play($program);

    $program->finish();
    sleep 10;			# cpu cool down
}


unless ($simulation_only){
    mkdir "$directory/csa";
    mkdir "$directory/0";
    mkdir "$directory/1";

    $client->connect($config->{server}, $config->{port});
    $client->login_x1();
}

foreach my $ids (make_array($config->{record_list})) {
    my $id = ${$ids}[0];
    my $f = ${$ids}[1] || -1;
    if ($player_zero) {
	play($id, "+", $f);
	play($id, "-", $f);
    }
    else{
	play($id, "-", $f);
	play($id, "+", $f);
    }

}

unless ($simulation_only){
    $client->logout();
    $client->disconnect();
}

