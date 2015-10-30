#!/usr/bin/perl -w
use strict;
sub value($) {
  my ($line) = @_;
  my ($name, $a00, $v00, $p0, $a01, $v01, $p1, $a02, $v02, $p2, $a03, $v03, $p3, $a04, $v04, $p4, $a05, $v05, $p5, $a06, $v06, $p6, $a07, $v07, $p7, $a1, $v1) = split(/\s+/,$line);
  return abs($a1) + $v1;
}

my @lines = <>;

print sort { value($b) <=> value($a) }  @lines;;
