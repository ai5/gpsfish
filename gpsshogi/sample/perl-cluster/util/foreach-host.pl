#!/usr/bin/perl -w
use strict;
use Getopt::Std;

my %count;

my $options = {};
getopts('c:LRS:', $options);
my $config_file = $options->{c} || "config";
my $run = $options->{R};
my $localhost = $options->{L};
my $ssh_option = $options->{S};
my $command = join(' ', @ARGV);
my $prevhost = "";

sub go ($) {
  my ($host) = @_;
  print STDERR "ssh $host $command\n";
  if ($run && $prevhost ne $host) {
    if ($ssh_option) {
      system ("/usr/bin/ssh", $ssh_option, "$host", $command)
    } else {
      system ("/usr/bin/ssh", "$host", $command)
    }
  }
  $count{$host}++;
}

open (IN, $config_file) || die "open $! $config_file";
while (<IN>) {
  chomp;
  next if /^#/;
  s/#.*//;
  if (m|^(/usr/bin/)?ssh\s+([a-z0-9]+)\s+([^\s].*)|) {
    my $host=$2;
    go($host);
    $prevhost = $host;
  } elsif (/[^a-z]ssh[^a-z]/) {
    warn "unexpected ssh line $_";
  }
}
close IN;
go("localhost") if (defined $localhost);
foreach my $host (sort keys %count) {
    printf STDERR "count %12s %2d\n", $host, $count{$host};
}
