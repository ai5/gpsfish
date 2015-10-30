#!/usr/bin/perl -w
use strict;
use Getopt::Std;

sub run ($$@);

my $options = {};
getopts('c:', $options);
my $config_file = $options->{c} || "config";

run ($config_file, [1]);

sub show ($$) {
  my ($config, $levels) = @_;
  printf "%s: %s\n", join('.', @$levels), $config;
}

my %fetch_ok;
sub fetch ($$) {
  my ($host, $file) = @_;
  open (IN, "/usr/bin/ssh $host cat $file|") || die "open $!";
  my @lines = <IN>;
  close IN;
  $fetch_ok{$host,$file} = 1 if (@lines);
  return join('', @lines);
}

sub run ($$@) {
  my ($config, $levels, $num_slaves) = @_;
  show($config, $levels);
  open (IN, $config) || die "open $! $config"; # todo ssh
  my @lines = <IN>;
  close IN;

  my $i = 0;
  foreach my $line (@lines) {
    chomp $line;
    next if $line =~ /^#/;
    $line =~ s/#.*//;
    $line =~ s/^\s+//;
    next if $line =~ /^$/;
    show ($line, [@$levels, $i]);
    # local usi
    die unless $line =~ m|^([^\s]+)|;
    my $command = $1;
    if ($command !~ /(^ssh|\/ssh[^0-9a-zA-Z])/) {
      die "cannot find $command" unless -x $command;
    }
    # remote usi
    if ($line =~ m|^(/usr/bin/)?ssh\s+([a-z0-9]+)\s+([^\s]+)|) {
      my ($host, $command)=($2,$3);
      my $file = $fetch_ok{$host,$command} || fetch($host, $command);
      die "cannnot fetch $command on $host" unless $file;
    } elsif ($line =~ /[^a-z]ssh[^a-z]/) {
      warn "unexpected ssh line $_";
    }
    # local usi.pl
    if ($command =~ /[^A-Za-z0-9]usi\.pl/) {
      my $config = "config";
      $config = $1
	if ($line =~ /-c\s?([^\s]+)/);
      my $slave_limit = 0;
      $slave_limit = $1 if ($line =~ /-N\s?(\d+)/);
      run($config, [@$levels, $i], $slave_limit);
    }
    # remote usi.pl
    #
    $i++;
    last if ($num_slaves && $i > $num_slaves);
  }
}

