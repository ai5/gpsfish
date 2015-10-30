#!/usr/bin/perl -w
use Getopt::Std;
use strict;
sub usage() {
  print STDERR "usage: $0 prich-x.txt prich-x-info.txt\n";
  exit 1;
}

my $options = {};
getopts("o:v", $options);
my $output = $options->{o} ? $options->{o} : ".";
my $verbose = $options->{v};
system("mkdir", "-p", $output);

my ($weight, $info) = @ARGV;
usage()
  if (@ARGV+0 != 2);


open INFO, $info || die "open $info $!";
open WEIGHT, $weight || die "open $weight $!";
my $mode = "";
while (my $info=<INFO>) {
  print $info;
  chomp $info;
  if ($info =~ /^\#\*\s+([a-z0-9]+)$/) {
    $mode = $1;
    print STDERR "  make $output/$mode\n"
      if ($verbose);
    system("mkdir", "-p", $output . "/" . $mode);
    next;
  }
  next if ($info =~ /^\#/);

  if ($info =~ /^([A-Za-z0-9]+) ([0-9]+)$/) {
    my ($eval, $dim) = ($1, $2);
    my $out = "$output/$mode/$eval.txt";
    print STDERR "  write in $out, $dim lines\n"
      if ($verbose);
    print STDERR "  WARNING $out exists\n"
      if (-f $out);
    open OUT, "> $out" || die "open $!";
    foreach my $f (1..$dim) {
      my $line = <WEIGHT>;
      print OUT $line;
    }
    close OUT;
  }
}
close INFO;
