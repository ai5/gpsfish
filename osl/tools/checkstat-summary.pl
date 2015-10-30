#!/usr/bin/perl -w
# summary.pl checkstat results
use File::Basename;
use strict;

my $problems = ();
sub parse ($) {
    my ($filename) = @_;
    my $result = ();
    unless (open(FILE, $filename)) {
	warn "open $! $filename";
	return undef;
    }
    while (<FILE>) {
	chomp;
	next
	    unless /solving (.+)/;
	my $problem = basename $1;
	$_ = <FILE>
	    while (!/total/);
	/^total  : take (\d+) clocks, loop= (\d+)/
	    || die "unknown syntax $_";
	my $cycles = $1;
	my $total = $2;
	$_ = <FILE>;
	/^unique : take \d+ clocks, loop= (\d+)/
	    || die "unknown syntax $_";
	my $uniq = $1;
	$_ = <FILE>;
	/^real ([0-9.]+) sec./;
	my $real = $1;
	$problems->{$problem} = 1;
	$result->{$problem} = [$cycles, $total, $uniq, $real];
    }
    close FILE;
    return $result;
}

my $tables = ();
my @files;

foreach my $file (@ARGV){
    my $contents = parse($file);
    if (defined $contents) {
	$tables->{$file} = $contents;
	push(@files, $file);
    }
}

print "# problem";
foreach my $config (@files) {
    print "\t",$config;
}
print "\n";
foreach my $problem (sort keys %$problems){
    print $problem, "\t";
    foreach my $config (@files) {
	my $tuple = $tables->{$config}->{$problem};
	print join("\t",@$tuple), "\t";
    }
    print "\n";
}

