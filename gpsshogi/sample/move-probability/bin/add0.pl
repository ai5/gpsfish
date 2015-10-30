#!/usr/bin/perl -w
use strict;
my ($a, $b) = (16, 16);
my $i = 0;
while (<>) {
  print;
  next if (++$i % $a);
  foreach my $n (1..$b) {
    print "0\n";
  }
}
