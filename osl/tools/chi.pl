#!/usr/bin/perl -w
use strict;
use Getopt::Std;
my %options=();
getopts("t:w:p:s:",\%options);

my $total = $options{t} ? $options{t} : 20;
my $win   = defined $options{w} ? $options{w} : $total/2;
my $p     = defined $options{p} ? $options{p} : 0.5;
#my $significance = $options{'s'} ? $options{'s'} : 0.05;

# XXX: must be compatible with $chi2_threshold
my $significance = 0.05;
#my $chi2_threshold = 6.63490; # for 1%
my $chi2_threshold = 3.84146; # for 5%
#my $chi2_threshold = 2.70554; # for 10%

printf "%4d / %4d \t... %s\n",
    $win, $total, (&is_significant($win, $p, $total) ? "*" : "");
printf "%4d / %4d\tfor significance %f\n",
    &significant_win($p, $total), $total, $significance;
printf "[%f : %f ]\tfor significance %f\n",
    &significant_prob($total, $win, $significance),
    1.0 - &significant_prob($total, $total-$win, $significance), $significance;

sub significant_win($$) {
    my ($p, $total) = @_;
    for (my $i=$total;  $i>=($total*$p); --$i) {
	if (! &is_significant($i, $p, $total)) {
	    return $i+1;
	}
    }
    return $total;
}

sub is_significant($$$) {
    my ($win, $p, $total) = @_;
    my $expected_win = $total*$p;
    my $expected_lose = $total - $expected_win;
    my $lose = $total - $win;
    my $chi = 1.0*($expected_win - $win)*($expected_win - $win)/$expected_win
	+ 1.0*($expected_lose - $lose)*($expected_lose - $lose)/$expected_lose;
    return $chi > $chi2_threshold;
}

sub significant_prob ($$$) {
    my ($total, $win, $significance) = @_;
    my $last_significant = 0.0;
    my $step = 1; # 1.0/$step is used
    while ($step <= 128) {
	for (my $i=1; $last_significant*$step+$i<$step; ++$i) {
	    my $p=($step*$last_significant + $i) / $step;
	    last
		if (($win/$total < $p)
		    || ! is_significant($win, $p, $total));
	    $last_significant = $p;
	}
	$step *= 2;
    }
    return $last_significant;
}
