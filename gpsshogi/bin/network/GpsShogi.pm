package GpsShogi;
use strict;
use IPC::Open2;
use IO::Handle;
use IO::Select;
use POSIX qw(strftime);

BEGIN {
  $| = 1;
  $SIG{'PIPE'} = 'IGNORE';
}

sub make_command ($) {
    my ($config) = @_;
    my $options = { initial_filename => '-f',
		    black => '-b', white => '-w',
		    limit => '-l', table_size => '-t',
		    table_record_limit => '-L',
		    node_limit => '-n',
		    timeleft => '-T', byoyomi => '-B',
		    record_filename => '-o',
		    kisen_id => '-k',
		    kisen_file => '-K',
		    csa_file => '--csa-file',
		    book_moves => '-m',
		  };
    die "record_filename or csa_file not defined"
	unless (defined $config->{record_filename} ||
		defined $config->{csa_file});
    die "log_filename not defined"
	unless (defined $config->{log_filename});
    my $base_command = $config->{base_command}
	? $config->{base_command}
	    : "./gpsshogi -v -c";
    my $command = $base_command . " " . $config->{other_options};
    $command .= " -s"
	if ($config->{sente});
    foreach my $key (keys %$config) {
	next
	    unless (defined($options->{$key}));
	$command .= " " . $options->{$key} . " " . $config->{$key};
    }
    $command .= " 2> ".$config->{log_filename};
    return $command;
}

sub new ($$) {
    my ($pkg, $config) = @_;
    my $this = { config => $config, num_respawned => 0, };

    $config->{record_filename}
	= make_record_filename($config->{initial_filename},
			       $config->{black}, $config->{white})
	    unless $config->{record_filename};
    my $time_string = make_time_string();
    unless ($config->{log_filename}) {
	my $logfile = sprintf('%s%s-%s-%s.log', $config->{logfile_basename},
			      $time_string, $config->{black}, $config->{white});
	$config->{log_filename} = $logfile;
    }

    my $command = make_command($config);
    $this->{command} = $command;
    print STDERR "SYSTEM:execute '$command'\n";
    $this->{pid} = open2($this->{reader}, $this->{writer}, $command);
    bless $this;
    return $this;
}

sub need_servertime ($) {
    my ($this) = @_;
    return 0;
}

sub set_logger ($$) {
    my ($this, $logger) = @_;
    $this->{logger} = $logger;
    $this->{logger}->autoflush(1);
}

sub set_master_record ($$) {
    my ($this, $master) = @_;
    $this->{master_record} = $master;
}

sub respawn ($) {
    my ($this) = @_;
    my ($logger) = ($this->{logger});
    print STDERR  "SYSTEM:try respawn ".$this->{num_respawned}."\n";
    print $logger "SYSTEM:try respawn ".$this->{num_respawned}."\n"
	if ($logger);
    return 0
	if (++$this->{num_respawned} > 10);
    # backup
    my $config = $this->{config};
    system("mv", $config->{record_filename},
	   $config->{record_filename}.".".$this->{num_respawned});
    system("mv", $config->{log_filename},
	   $config->{log_filename}.".".$this->{num_respawned});
    if (! $this->{master_record}) {
	print STDERR "master_record not defined\n";
	return 0;
    }
    $config->{initial_filename} = $this->{master_record};
    my $respawn_command = make_command($config);
    print STDERR "SYSTEM:execute '".$respawn_command."'\n";
    print $logger "SYSTEM:execute '".$respawn_command."'\n"
	if ($logger);
    $this->{pid} = open2($this->{reader}, $this->{writer},
			 $respawn_command);
    return 1;
}

sub send ($$)
{
    my ($this, $message) = @_;
    my ($writer, $logger) = ($this->{writer}, $this->{logger});
    my $now = strftime("%Y %b %d %a %H:%M:%S %z\n", localtime);
    print STDERR "TGPS:$message",$now;
    print $logger "TGPS:$message",$now
	if ($this->{logger});
    print $writer "$message"
	|| $this->respawn_in_write();
}

sub respawn_in_write ($){
    my ($this) = @_;
    warn "write failed\n";
    $this->respawn();
    # message to send will be read from record_file
}


sub respawn_and_read ($)
{
    my ($this) = @_;
    return undef
	unless ($this->respawn());
    return $this->read();
}

sub read ($)
{
    my ($this) = @_;
    my ($reader, $logger) = ($this->{reader}, $this->{logger});
    my $line = <$reader>;
    my $now = strftime("%Y %b %d %a %H:%M:%S %z\n", localtime);
    if (defined $line) {
	print STDERR "FGPS:$line",$now;
	print $logger "FGPS:$line",$now
	    if ($this->{logger});
    } else {
	warn "prog_read failed";
	return $this->respawn_and_read();
    }
    return $line;
}

sub read_or_timeout ($$)
{
  my ($this, $timeout) = @_;
  my $selector = new IO::Select($this->{reader});
  return (undef,1) unless $selector->can_read($timeout);
  return ($this->read, undef);
}

sub finish ($) {
  my ($this) = @_;
  print STDERR "waitpid ".$this->{pid}."\n";
  waitpid $this->{pid}, 0;
  print STDERR "wait done\n";
}

sub newsearch_for_study ($$$) {
  my ($this, $filename, $turn) = @_;
  my $config =
    { initial_filename => $filename,
      sente => ($turn eq "+"), limit => 800, byoyomi => 10,
      log_filename => "/dev/null", record_filename => "/dev/null",
      base_command => $this->{config}->{base_command},
      other_options => $this->{config}->{other_options}
    };
  $config->{limit} = $this->{config}->{limit} - 400
    if ($this->{config}->{limit}>1200);
  my $program = new GpsShogi($config);
  my $line = $program->read;
  $program->send("%TORYO\n");
  $program->finish;
  return $line;
}

# utility
sub make_time_string() {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $year += 1900; $mon += 1;
    return sprintf('%04d%02d%02d-%02d-%02d-%02d',
		   $year,$mon,$mday,$hour,$min,$sec);
}

sub make_record_filename($$$) {
    my ($initial_filename, $black, $white) = @_;
    my $record_filename = $initial_filename;
    $record_filename =~ s/\.[^.]*//;
    $record_filename .= "-$black-$white.csa";
    return $record_filename;
}


# END
return 1;
