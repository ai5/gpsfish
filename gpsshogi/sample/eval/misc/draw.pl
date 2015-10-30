#!/usr/bin/perl -w
use strict;

foreach my $filename (@ARGV) {
    open FILE, "< $filename"
	|| die "open $!";
    while (<FILE>) {
	chomp;
	print $_." ";
    }
    print "\n";
    close FILE;
}
