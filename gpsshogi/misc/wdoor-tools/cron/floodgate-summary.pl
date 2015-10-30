#!/usr/bin/perl -w
use strict;
use YAML::Tiny;
use Date::Calc qw( Delta_Days );
use Time::ParseDate;
my ($players_file, $history_file) = @ARGV;

my $db = '/home/shogi-server/www/x/floodgate-results.txt';
my $rating_file = '/home/shogi-server/www/x/current-floodgate.yaml';
my $rating14_file = '/home/shogi-server/www/x/current-floodgate14.yaml';
my $strip_trip = 1;

sub read_rating ($) {
    my ($filename) = @_;
    my $yaml = YAML::Tiny->new;
    $yaml = YAML::Tiny->read($filename);

    my $rating = {};
    my $root = $yaml->[0]->{players};
    foreach my $gid (keys %$root) {
	my $group = $root->{$gid};
	foreach my $user (keys %$group) {
	    $rating->{$user} = $group->{$user};
	}
    }
    return $rating;
}

my $rating = read_rating($rating_file);
my $rating14 = read_rating($rating14_file);
my ($wins, $losses) = ({}, {});
my $winpairs = {};
my $winpairs_today = {};
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
  = localtime(time);
$year += 1900; $mon += 1;
open (INDEX, "/usr/bin/tac $db|")
  || die "$!";
while (my $line=<INDEX>) {
  next if ($line =~ /^draw/);
  chomp $line;
  my ($time1, $time2, $how, $ret1, $player1, $player2, $ret2) = split(/\s+/,$line,7);
  my $time = parsedate($time1." ".$time2); 
  print STDERR $line."\n" unless ($time);
  my ($sec2,$min2,$hour2,$mday2,$mon2,$year2,$wday2,$yday2,$isdst2)
    = localtime($time);
  $year2 += 1900; $mon2 += 1;
  if ($ret1 eq "win") {
    $winpairs->{"$player1 $player2"} += 1;
  } else {
    $winpairs->{"$player2 $player1"} += 1;
  }
  next
    unless ($year2 == $year && $mon2 == $mon && $mday2 == $mday);
  if ($ret1 eq "win") {
    $wins->{$player1} += 1;
    $losses->{$player2} += 1;
    $winpairs_today->{"$player1 $player2"} += 1;
  }
  elsif ($ret1 eq "loss") {
    $losses->{$player1} += 1;
    $wins->{$player2} += 1;
    $winpairs_today->{"$player2 $player1"} += 1;
  }
}

open (PLAYERS, ">$players_file") || die "open $!";
foreach my $player (keys %$rating) {
  my $player_notrip = $player;
  $player_notrip =~ s/\+.*// if ($strip_trip);
  printf(PLAYERS "%s %d %d %d %d %d %d\n", $player_notrip,
	 $rating->{$player}->{rate},
	 $rating14->{$player}->{rate} || 0,
	 $rating->{$player}->{win}, $rating->{$player}->{loss},
	 $wins->{$player} || 0, $losses->{$player} || 0);
}
close PLAYERS;

open (HISTORY, ">$history_file") || die "open $!";
foreach my $pair (keys %$winpairs) {
  my ($player1,$player2) = split(/\s+/, $pair);
  my $player1_notrip = $player1;
  my $player2_notrip = $player2;
  $player1_notrip =~ s/\+.*// if ($strip_trip);
  $player2_notrip =~ s/\+.*// if ($strip_trip);
  printf(HISTORY "%s %s %d %d %d %d\n", $player1_notrip, $player2_notrip,
	 $winpairs->{$pair},
	 $winpairs->{"$player2 $player1"} || 0,
	 $winpairs_today->{$pair} || 0,
	 $winpairs_today->{"$player2 $player1"} || 0);
}
close HISTORY;
