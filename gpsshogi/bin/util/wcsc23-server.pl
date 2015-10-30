#!/usr/bin/perl -w
use strict;
use IO::Socket;
use IO::Select;
use Getopt::Long;
use network::CsaFileSession;
my $config = { csa_host => '192.168.20.1',
	       csa_port => 4081,
	       dir      => "local-csa"
	     };
GetOptions($config, 'csa_host=s', 'csa_port=i', 'dir=s') || die "$!";

sub read_line ($@);
sub default_timeout () { return 0.05; }
sub connection_closed () { return ":connection closed:"; }

my $server = { buf=>"" };
$server->{reader} = $server->{writer} = $server->{socket}
  = new IO::Socket::INET(PeerAddr => $config->{csa_host},
			 PeerPort => $config->{csa_port},
			 Proto    => 'tcp');

my $engine_in  = new CsaFileSession($config->{dir}."/in", 0);
my $engine_out = new CsaFileSession($config->{dir}."/out", 0);

while (1) {
  if (my $line = read_line($server, default_timeout)) {
    $engine_in->write($line."\n");

    if ($line =~ /^[#]/ || $line eq connection_closed) {
      warn "game end" if ($line eq connection_closed);
      last;
    }
    next;
  }
  if (my $line = $engine_out->try_read()) {
    print {$server->{writer}} $line."\n";
  }
}

sub read_line ($@) {
  my ($object, $timeout) = @_;
  my $in = $object->{reader};
  $object->{selector} = new IO::Select($in) unless $object->{selector};
  my $selector =  defined $timeout ? $object->{selector} : undef;
  if (defined $timeout && $timeout < 0) {
    warn $timeout;
    $timeout = 0;
  }

  while (1) {
    my $len = index($object->{buf},"\n");
    if ($len >= 0) {
      my $line = substr($object->{buf}, 0, $len+1);
      $object->{buf} = substr($object->{buf}, $len+1);
      $line =~ s/\r?\n$//;
      $object->{last_activity} = time;
      return $line;
    }
    return undef if ($selector && ! $selector->can_read($timeout));
    my $buf;
    unless ($in->sysread($buf, 65536)) {
      return connection_closed;
    }
    $object->{buf} .= $buf;
    $timeout = 0 if defined $timeout;
  }
}
