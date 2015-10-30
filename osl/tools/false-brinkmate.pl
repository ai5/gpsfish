#!/usr/bin/perl -w
use FileHandle;
use strict;

# usage: ./false-brinkmate.pl dir0 [dir1] ...
#   csa files in dir* will be processed.

my $program = "gps_l";
my $threshold = 10000;

my ($total, $target, $brinkmate, $false_brinkmate, $blunder)=0;

sub analyze ($$) {
  my ($file,$sente) = @_;
  my $prefix = $sente ? "+" : "-";
  open (CSA, $file) || die "open $file !";
  ++$target;
  my @lines = grep { my $l=$_; ($l =~ /^[+-][0-9]/ || substr($l,0,3) eq "'**") } <CSA>;
  close CSA;
  my ($almost_win, $move, $lasteval) = 0;
  for (my $i=0; $i<=$#lines; ++$i) {
    my $line = $lines[$i];
    ++$move
      if ($line =~ /^[+-][0-9]/);
    next unless (substr($line,0,1) eq $prefix);
    next
      unless (substr($lines[$i+1], 0, 3) eq "'**"
	      && substr($lines[$i+1], 4) =~ /(-*[0-9]+)/);
    my $eval = $sente ? $1 : -$1;
    if ($eval > $threshold) {
      $almost_win = $move;
    } elsif ($almost_win && $eval < 0) {
      ++$false_brinkmate;
      print $file." ".$almost_win." ".$eval."\n";
      last;
    } elsif ($lasteval && $eval && $eval +3000 < $lasteval && $eval > -$threshold && $lasteval > 0 && $lasteval < $threshold) {
      ++$blunder;
      print $file." ".$move." ".$eval." <= ".$lasteval."\n";
      last;
    }
    $lasteval = $eval;
  }
  ++$brinkmate if ($almost_win);
}

my $ire = '[A-Za-z0-9_@.-]+';
sub find ($) {
  my ($dir) = @_;
  opendir (DIR, $dir) || die "opendir $dir !";
  while (my $file=readdir(DIR)) {
    next
      unless $file =~ /floodgate-900-0.*\.csa$/;
    next
      unless $file =~ /^($ire)\+($ire\-[0-9]+-[0-9]+)[\+:]($ire)[\+:]($ire)[\+:]([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})\.csa$/;
    my $record = { event => $1, game => $2, sente => $3, gote => $4,
		   year => $5, month => $6, date => $7, hour => $8,
		   minute => $9, second => $10, file => "$file"
		 };
    ++$total;
    if ($record->{sente} eq $program) {
      analyze($dir."/".$file, 1);
    } elsif ($record->{gote} eq $program) {
      analyze($dir."/".$file, 0);
    }
  }
  closedir DIR;
}

foreach my $dir (@ARGV) {
  find($dir);
}

print STDERR "total $total, $program $target, brinkmate $brinkmate, false_brinkmate $false_brinkmate, blunder $blunder\n";
