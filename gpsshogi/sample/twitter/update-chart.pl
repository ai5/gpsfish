#!/usr/bin/perl -w
use strict;
use URI::Escape;
use Getopt::Std;
my %options;
getopts('n:', \%options);
my $number = $options{n} ? sprintf("%03d", $options{n}) : "000";

my $extract = './util/extract-eval.pl';
my $make_chart = './util/make-chart.pl';
my $tweet = './tweet-oauth.pl';
my $urlfile = 'url'.$number.'.txt';
my $chartfile = "chart".$number.".txt";
my $chartfile_en = "chart".$number."-en.txt";

#usage: cat analyses*[0-9].txt | ./update-chart.pl

open (MAKEURL, "| $extract |$make_chart > $urlfile") || die "open $!";
while (<>) {
  print MAKEURL $_;
}
close MAKEURL;

open (URL, "$urlfile") || die "open $!";
my $url = <URL>;
close URL;

my $encoded = uri_escape($url);
my $isgdapi = 'http://is.gd/api.php?longurl=' . $encoded;

open (ISGD, "curl '$isgdapi'|") || die "open $!";
my $shortened = <ISGD>;
close ISGD;

open (CHART, ">$chartfile") || die "open $!";
print CHART "これまでの流れ: $shortened\n";
close CHART;

open (CHART_EN, ">$chartfile_en") || die "open $!";
print CHART_EN "evaluation: $shortened\n";
close CHART_EN;

system $tweet, "-T", $chartfile, $chartfile_en;
