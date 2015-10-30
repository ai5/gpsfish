#!/usr/bin/perl -w
use strict;
my $pawn = 128.0;
my @threshold;
foreach my $i (0..20) {
  push(@threshold, 0.1*$i*$pawn);
}
my %table;
foreach my $file (@ARGV) {
  open(IN, $file) || die "open $!";
  $table{$file} = [];
  my $count = $table{$file};
  while (my $val = <IN>) {
    chomp $val;
    foreach my $i (0..$#threshold) {
      if ($val <= $threshold[$i] || $i == $#threshold) {
	$count->[$i]++;
	last;
      }
    }
  }
}

print "(pawn)    ".join(" ",@ARGV)."\n";
foreach my $i (0..$#threshold) {
  printf("[%.2f - %.2f]", ($i ? $threshold[$i-1]/$pawn : 0.0), $threshold[$i]/$pawn);
  foreach my $file (@ARGV) {
    my $count = $table{$file};
    printf(" %6d", $count->[$i]);
  }
  print "\n";
}

