#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use network::CsaFileSession;
use Time::Piece;

my %options;
getopts("s:d:",\%options);

my $dir = $options{d} || ".";
my $id = $options{s} || 0;

my $session = new CsaFileSession($dir, $id);

while (<>) {
  chomp;
  if (/^$/) {
    print STDERR "(nop)\n";
    next;
  }
#  my $now = Time::Piece::localtime;
#  print STDERR $now->datetime." > $_\n";
  $session->write($_);
}
