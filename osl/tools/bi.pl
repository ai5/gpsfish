#!/usr/bin/perl -w
# 二項分布の確率計算
use strict;
use Getopt::Std;
my %options=();
getopts("t:w:p:s:",\%options);

my $total = $options{t} ? $options{t} : 20;
my $win   = defined $options{w} ? $options{w} : $total/2;
my $p     = defined $options{p} ? $options{p} : 0.5;
my $significance = $options{'s'} ? $options{'s'} : 0.05;

my $prob = &probability_cumulative($p, $total, $win);
printf "%4d / %4d \t... %f %s\n",
    $win, $total, $prob, ($prob < $significance) ? "*" : "";
printf "%4d / %4d \tfor significance %f\n",
    &significant_win($p, $total, $significance), $total, $significance;
printf "[%f : %f ]\tfor significance %f\n",
    &significant_prob($total, $win, $significance), 
    1.0 - &significant_prob($total, $total-$win, $significance), $significance;

# foreach my $s (0.01, 0.05, 0.1) {
#     foreach my $t (10, 20, 50, 100, 200, 500, 1000) {
# 	printf "%4d / %5d \tfor significance %f\n",
# 	    &significant_win($p, $t, $s), $t, $s;
#     }
# }

sub pow($$){
    my ($p, $n) = @_;
    my $result = 1.0;
    foreach my $i (1..$n) {
	$result *= $p;
    }
    return $result;
}

sub significant_win($$$) {
    my ($p, $total, $significance) = @_;
    my $sum = 0.0;
    my $win = $total;
    while ($sum < $significance) {
	$sum += &probability($p, $total, $win);
	$win--;
 }
    return $win+2;
}

my %probability_memoize;
sub probability_cumulative($$$){
    my ($p, $total, $win) = @_;
    my $key = $p."-".$total."-".$win;
    return $probability_memoize{$key}
	if ($probability_memoize{$key});

    my $sum = 0.0;
    foreach my $i ($win .. $total) {
	my $key_i = $p."-".$total."-".$i;
	if ($probability_memoize{$key}) {
	    $sum += $probability_memoize{$key};
	    last;
	}
	$sum += probability($p, $total, $i)
    }
    return $probability_memoize{$key} = $sum;
}

sub probability($$$){
    my ($p, $total, $win) = @_;
    return exp(log_probability($p, $total, $win));
}

sub log_probability($$$){
    my ($p, $total, $win) = @_;
    return log_cnr($total, $win) + $win * log($p) + ($total-$win) * log(1-$p);
}

sub log_factorial($){
    # Stirling's formula
    my ($n) = @_;
    my $pi = 3.1415926535897932;
    return ($n+0.5)*log($n) - $n+0.5*log(2*$pi)
	+ 1.0/12/$n - 1.0/360/$n/$n/$n + 1.0/1260/$n/$n/$n/$n/$n;
}

sub log_cnr($$){
    my ($n, $r) = @_;
    return 0
	if (($n == $r) || ($r == 0));
    return log_factorial($n)-log_factorial($r)-log_factorial($n-$r);
}

sub cnr($$){
    my ($n, $r) = @_;
    return 1
	if (($n == $r) || ($r == 0));
    return exp(log_cnr($n,$r));
}

sub significant_prob ($$$) {
    my ($total, $win, $significance) = @_;
    my $last_significant = 0.0;
    my $step = 1; # 1.0/$step is used
    while ($step <= 128) {
	for (my $i=1; $last_significant*$step+$i<$step; ++$i) {
	    my $p=($step*$last_significant + $i) / $step;
	    my $prob = &probability_cumulative($p, $total, $win);
	    last
		if ($prob > $significance);
	    $last_significant = $p;
	}
	$step *= 2;
    }   
    return $last_significant;
}
