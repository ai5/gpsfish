#!/usr/bin/perl -w
use strict;
use Getopt::Std;

# usage ./same-opening.pl [-d] -n 15 -m 3 *.csa
# -d 合流検知
# -n 何手目まで調べるか
# -m 出力する最小頻度

my %opts=();
getopts("dm:n:",\%opts);

my $dag = $opts{d};
my $num_moves = $opts{n} ? $opts{n} : 3;
my $minimum = $opts{m} ? $opts{m} : 0;

my %data;
sub parse ($$) {
    my ($file, $num) = @_;
    open FILE, $file
	|| die "open $! $file";
    my @moves;
    while (<FILE>) {
	next
	    unless (/^([+-][0-9]{4}[A-Z]{2})/);
	push(@moves, $1);
	last
	    if (@moves+0 >= $num);
    }
    close FILE;
    my $key = join(',', ($dag ? (sort @moves) : @moves));
    if (defined $data{$key}) {
	$data{$key}++;
    } else {
	$data{$key} = 1;
    }
}


foreach my $file (@ARGV) {
    parse($file, $num_moves);
}

my %reverse;
foreach my $key (keys %data) {
    my $frequency = $data{$key};
    if (defined $reverse{$frequency}) {
	push (@{$reverse{$frequency}}, $key);
    } else {
	$reverse{$frequency} = [$key];
    }
}

foreach my $freq (sort {$b <=> $a} keys %reverse) {
    last
	if ($freq < $minimum);
    foreach my $state (@{$reverse{$freq}}) {
	print $freq . "\t" . $state . "\n";
    }
}
