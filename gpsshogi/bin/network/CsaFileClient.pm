package CsaFileClient;
use strict;
use IO::Handle;
use network::CsaFileSession;

BEGIN {
  $| = 1;
}

sub new ($$) {
  my ($pkg, $config) = @_;
  my $this = { config => $config, command => "CsaFileClient" };
  my $dir = $config->{csa_file_session_dir} || make_time_string();
  $this->{dir} = $dir;
  if (! -d $dir) {
    mkdir ($dir, 0755) || die "mkdir $dir";
  }
  if (! -d "$dir/in") {
    mkdir ($dir. "/in",  0755) || die "mkdir $dir/in";
  }
  if (! -d "$dir/out") {
    mkdir ($dir. "/out", 0755) || die "mkdir $dir/out";
  }
  $this->{session_svr} = new CsaFileSession($dir."/in", 0, 0);
  $this->{session_clt} = new CsaFileSession($dir."/out", 0, 0);

  bless $this;
  $this->send("isready\n");
  my $ready = $this->read();
  chomp $ready;
  die unless ($ready eq "readyok");
  return $this;
}

sub need_servertime ($) {
    my ($this) = @_;
    return 1;
}

sub set_logger ($$) {
  my ($this, $logger) = @_;
  $this->{logger} = $logger;
  $this->{logger}->autoflush(1);
}

sub send ($$) {
  my ($this, $message) = @_;
  my ($session, $logger) = ($this->{session_svr}, $this->{logger});
  print STDERR "TFILE ".localtime(time).": $message";
  print $logger "TFLIE ".localtime(time).": $message"
    if ($this->{logger});
  $session->write($message);
}

sub read ($) {
  my ($this) = @_;
  my ($session, $logger) = ($this->{session_clt}, $this->{logger});
  my $line = $session->read();
  if (defined $line) {
    print STDERR "FFILE ".localtime(time).": $line\n";
    print $logger "FFILE ".localtime(time).": $line\n"
      if ($this->{logger});
  } else {
    die "prog_read failed";
  }
  return $line . "\n";
}

sub read_or_timeout ($$)
{
  my ($this, $timeout) = @_;
  # ignore $timeout for now
  return ($this->read, undef);
}

sub set_master_record ($$) {
  my ($this, $master) = @_;
  $this->{record_file} = $master;
}

# utility
sub make_time_string() {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);
    $year += 1900; $mon += 1;
    return sprintf('%04d%02d%02d-%02d-%02d-%02d',
		   $year,$mon,$mday,$hour,$min,$sec);
}

# END
return 1;
