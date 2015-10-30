#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use NKF;
use Net::Twitter;
use Encode;

my %options;
getopts('t:TS:u:', \%options);

# ./tweet.pl [-et:TS:] files

my $name = "gpsshogi";
open(PASS, "passwd") || die "open $!";
my $pass = <PASS>;
close PASS;
my $dryrun=  ! defined $options{T};

my $host = 'twitter.com';
my $apiurl = "http://$host";
my $apihost = "$host:80";

my $twit = Net::Twitter->new({username=>$name, password=>$pass, apiurl=>$apiurl, apihost=>$apihost});

sub say ($) {
  my ($original) = @_;
  my $content = nkf("-w", $original);
  my $utf8 = Encode::decode_utf8($content);
  $content = Encode::encode_utf8(substr($utf8, 0, 140))
    if (length($utf8) > 140);
  print STDERR "\nSEND $name:$content\n\n";
  return
    if ($dryrun);
  foreach my $i (1..3) {
    my $ret = $twit->update({status => $content});
    print STDERR $twit->http_code."\n";
    print STDERR $twit->http_message."\n";
    sleep 1;
    return $ret if $ret;
    print STDERR "failed $i\n";
    print STDERR join(' ', %{$twit->get_error})."\n";
  }
}

sub replacecsa ($) {
  my ($_) = @_;
  s/OU/王/g;
  s/HI/飛/g;
  s/RY/竜/g;
  s/KA/角/g;
  s/UM/馬/g;
  s/KI/金/g;
  s/GI/銀/g;
  s/NG/成銀/g;
  s/KE/桂/g;
  s/NK/成桂/g;
  s/KY/香/g;
  s/NY/成香/g;
  s/FU/歩/g;
  s/TO/と/g;
  if (/^([+-])([1-9][1-9])([1-9][1-9])([^0-9]+)/) {
    my $turn = ($1 eq "+") ? "▲" :"△";
    return $turn.$3.$4."(".$2.")";
  } elsif (/^([+-])(00)([1-9][1-9])([^0-9]+)/) {
    my $turn = ($1 eq "+") ? "▲" :"△";
    return $turn.$3.$4;
  }
  return $_;
}

while (<>) {
  chomp;
  print STDERR $_."\n";
  s/,\'\*//g;
  my $ret = join (' ', map { replacecsa($_) } (@_=split(/\s+/, $_)));
  print STDERR "$ret\n";
  say($ret);
}

