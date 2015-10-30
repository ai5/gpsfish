#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use network::CsaFileSession;
use IPC::Open2;
use IO::Handle;
my %options;
getopts("sd:c:",\%options);
# s sente
# d dir
# c command + args
my $sente = $options{s} || 0;
my $command = $options{c};
my $dir = $options{d} || ".";
my ($reader, $writer);
my $my_color = $sente ? '+' : '-';
my $pid = open2($reader, $writer, $command);

my $session_in = new CsaFileSession($dir."/in", 0);
my $session_out = new CsaFileSession($dir."/out", 0);

$| = 1;

my $ready = $session_in->read();
chomp $ready;
die "protocol error $ready" unless $ready eq "isready";
$session_out->write("readyok\n");

my $line;
if ($sente) {
  $line = <$reader>;
  chomp $line;
  print STDERR "FGPS:$line \n";
  $session_out->write($line."\n");
}
while ($line = $session_in->read()) {
  chomp $line;
  print STDERR "TGPS:$line\n";
  print $writer $line."\n";
  if ($line =~ /^$my_color/) {	# the last line was echoback of my move
    $line = $session_in->read();
    chomp $line;
    print STDERR "TGPS:$line\n";
    print $writer $line."\n";
  }
  $line = <$reader>;
  chomp $line;
  print STDERR "FGPS:$line\n";
  $session_out->write($line."\n");
}
