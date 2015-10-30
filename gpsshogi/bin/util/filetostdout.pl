#!/usr/bin/perl -w
use strict;
use Time::HiRes;
use Time::Piece;
use Getopt::Std;
use network::CsaFileSession;
use Sys::Hostname;

my %options;
getopts("s:d:k",\%options);

my $dir = $options{d} || ".";
my $id = $options{s} || 0;
my $keep_alive = $options{k} || 0;

my $session = new CsaFileSession($dir, $id);

my $now = Time::Piece::localtime;
# print STDERR $now->datetime." wait for $dir/$id\n";
my $sleep_count = 0;
while (1) {
  if (my $message = $session->try_read()) {
    print $message."\n";
#    my $now = Time::Piece::localtime;
#    print STDERR $now->dir." wait for $dir/".$session->id()."\n";
  } else {
    Time::HiRes::usleep(5*1000);
    $sleep_count += 5;
    if ($keep_alive && $sleep_count >= 60*1000) {
      print "\n";
      $sleep_count = 0;
    }
  }
}
