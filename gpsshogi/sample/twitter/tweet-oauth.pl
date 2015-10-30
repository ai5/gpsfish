#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use NKF;
use Net::Twitter::Lite;
use Encode;

my %options;
getopts('t:TS:u:', \%options);

# ./tweet.pl [-et:TS:] files

my $name = "";
my $name_en = "";

my @tokens;
{
  open(PASS, "token") || die "open $!";
  $name = <PASS>;
  push(@tokens, [ map {chomp; $_} <PASS>]);
  close PASS;
}
if (open(PASS, "token-en")) 
{
  $name_en = <PASS>;
  push(@tokens, [ map {chomp; $_} <PASS>]);
  close PASS;
}

my $dryrun=  ! defined $options{T};
my $url = $options{u};
my $seconds = $options{S};
my $titlefile = $options{t};

my $twit = Net::Twitter::Lite->new(
                            traits          => ['API::REST', 'OAuth'],
                            consumer_key    => $tokens[0]->[0],
                            consumer_secret => $tokens[0]->[1],
                           );
$twit->access_token($tokens[0]->[2]);
$twit->access_token_secret($tokens[0]->[3]);
my $twit_en;
if ($name_en) {
    $twit_en = Net::Twitter::Lite->new(
	traits          => ['API::REST', 'OAuth'],
	consumer_key    => $tokens[1]->[0],
	consumer_secret => $tokens[1]->[1],
	);
    $twit_en->access_token($tokens[1]->[2]);
    $twit_en->access_token_secret($tokens[1]->[3]);
}
sub say ($) {
  my ($original) = @_;
  my $content = nkf("-w", $original);
  my $utf8 = Encode::decode_utf8($content);
  $content = Encode::encode_utf8(substr($utf8, 0, 140))
    if (length($utf8) > 140);
  print STDERR "\nSEND $name:$content\n\n";
  return
    if ($dryrun);

  $content = Encode::decode_utf8($content);
  eval { $twit->update({status => $content}); };
  warn "twitter: $@\n" if $@;
}

sub say_en ($) {
  my ($content) = @_;
  $content = substr($content, 0, 140)
    if (length($content) > 140);
  print STDERR "\nSEND $name_en:$content\n\n";
  return
    if ($dryrun || !$twit_en);
  eval { $twit_en->update({status => $content}); };
  warn "twitter_en: $@\n" if $@;
}

sub announce_title ($$$$) {
  my ($title, $url, $seconds, $cont) = @_;
  open (PROG, "./analyze-kif --version|") || die "open $!";
  my $version = <PROG>; chomp $version;
  close PROG;
  my $name = "gpsshogi ";
  if ($cont) {
    $name = " ( " . $name;
    $url = $url . " ) ";
  }
  say($name . $version . " (max " . $seconds . " sec)  \n" . $title . " " . $url);
  say_en($name . $version . " (max " . $seconds . " sec)  \n" . " " . $url);
}

if ($titlefile && $url && $seconds) {
  open (TITLE, $titlefile) || die "open $!";
  my @title = <TITLE>;
  close TITLE;
  &announce_title(join("",@title), $url, $seconds, 1);
}


foreach my $file (@ARGV) {
  open (IN, $file) || die "open $!";
  my @contents = <IN>;
  close IN;
  if ($file =~ /-en.txt/) {
    say_en(join("", @contents));
  }
  else {
    say(join("", @contents));
  }
}
while (<STDIN>) {
    say($_);
}
