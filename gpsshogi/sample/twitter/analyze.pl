#!/usr/bin/perl -w
use strict;
use Getopt::Std;
use NKF;
use Net::Twitter::Lite;
use Encode;
my %options;
getopts('k:u:S:s:e:Tt:R', \%options);

# -T write twitter, -k stopprocess

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

my $dryrun = ! defined $options{T};

my $twit = Net::Twitter::Lite->new(
                            traits          => ['API::REST', 'OAuth'],
                            consumer_key    => $tokens[0]->[0],
                            consumer_secret => $tokens[0]->[1],
                           );
$twit->access_token($tokens[0]->[2]);
$twit->access_token_secret($tokens[0]->[3]);
my $twit_en;
if ($name_en) {
    $twit_en  = Net::Twitter::Lite->new(
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
  return
    if (!$twit_en);
  $content = substr($content, 0, 140)
    if (length($content) > 140);
  print STDERR "\nSEND $name_en:$content\n\n";
  return
    if ($dryrun);
  eval { $twit_en->update({status => $content}); };
  warn "twitter_en: $@\n" if $@;
}

sub announce_title ($$$) {
  my ($title, $url, $seconds, $cont) = @_;
  open (PROG, "./analyze-kif --version|") || die "open $!";
  my $version = <PROG>; chomp $version;
  close PROG;
  $url =~ s|http://||;
  my $name = "gpsshogi ";
  if ($cont) {
    $name = " ( " . $name;
    $url = $url . " ) ";
  }
  say($name . $version . " (max " . $seconds . " sec)  \n" . $title . " " . $url);
  say_en($name . $version . " (max " . $seconds . " sec)  \n" . " " . $url);
}


sub resignedkif ($) {
  my ($kif) = @_;
  my $ret = 0;
  open (KIF, $kif) || die $!;
  while (my $line=<KIF>) {
    my $euc = nkf("-e",$line);
    if ($euc =~ /^ *[0-9]+ *≈ÍŒª/) {
      $ret = 1;
      last;
    }
  }
  close KIF;
  return $ret;
}

sub updatekif ($$) {
  # ≥¡Ã⁄∑¡º∞
  my ($url,$out) = @_;
  if ($url =~ /.kif$/) {
    system("curl", "-m", "10", "-o", $out, $url);
    return resignedkif($out);
  }
  # ≤¶∞Ã¿ÔÕ— javascript≤Ú¿œ
  my $tmpfile = "tmp";
  system("curl", "-o", $tmpfile, $url);
  open (TMP, "/usr/bin/nkf -e $tmpfile|") || die "open $!";
  open (OUT, "|/usr/bin/nkf -s >$out") || die "open $!";
  print OUT "# KIFU\n";
  my @file=("£∞", "£±", "£≤", "£≥", "£¥", "£µ", "£∂", "£∑", "£∏", "£π");
  my @rank=("ŒÌ", "∞Ï", "∆Û", "ª∞", "ªÕ", "∏ﬁ", "œª", "º∑", "»¨", "∂Â");
  my @koma=("", " ‚", "§»", "π·", "¿Æπ·", "∑À", "¿Æ∑À",
	    "∂‰",		# 7
	    "¿Æ∂‰",
	    "≥—", 		# 9
	    "«œ",
	    "»Ù",
	    "Œµ",
	    "∂‚", # 13,
	    "¿Æ∂‚?",
	    "∂Ã");
  while (<TMP>) {
    next unless / *te\[[0-9]+\]= *([0-9]+); *co\[[0-9]+\]=[0-9]; *ki\[[0-9]+\]=([0-9]+); *fx\[[0-9]+\]=([0-9]+); *fy\[[0-9]+\]=([0-9]+); *tx\[[0-9]+\]=([0-9]); *ty\[[0-9]+\]=([0-9]); *na\[[0-9]+\]=([0-9]);/;
    my ($number, $ptype, $fromx, $fromy, $tox, $toy, $promote) = ($1, $2, $3, $4, $5, $6, $7);
    if ($fromx < 10) {
      printf(OUT "%4d %s%s%s(%d%d)\n", $number, $file[$tox], $rank[$toy],
	     $koma[$ptype].($promote?"¿Æ" : ""), $fromx, $fromy);
    } else {
      printf(OUT "%4d %s%s%s\n", $number, $file[$tox], $rank[$toy], $koma[$ptype]."¬«");
    }
  }
  close OUT;
  close TMP;
  return 0;
}

my $url = $options{u};
my $seconds = $options{S};
my $start = $options{s} || 35;
my $end = $options{e} || 350;
my $titlefile = $options{t};
my $kill_process = $options{k};
my $silent_rejume = $options{R};

exit 1
  unless ($url && $seconds && $titlefile);
open (TITLE, "/usr/bin/nkf -w $titlefile|") || die "open $!";
my $title = <TITLE>;
close TITLE;

# my $kif = ($url =~ /.kif$/) ? "target.kif" : "target.ki2";
my $kif = "target.kif";

&announce_title($title, $url, $seconds, 0)
  unless $silent_rejume;
my $last_announce = time();
my $last_announce_move = $start;
my $move = $start;
my $resigned = 0;
my $sleep_count = 0;
while ($move < $end) {
  $resigned = updatekif($url, $kif);
  print STDERR "finish? " . $resigned . "\n";
  my $status = 0;

  if ($kill_process) {
    print STDERR "sending kill STOP to $kill_process\n";
    kill "STOP" => $kill_process;
  }

  while ($status == 0 && $move < $end) {
    print STDERR "\nGO $move\n\n";
    $status = system("./analyze-kif", "-i", "1", "-f", $kif, "-m", $move, "-S", $seconds);
    if ($status == 0) {
      # evalchart
      if ($move % 10 == 0 && $move >= 50) {
	system("./update-chart.pl -n $move analyses*[0-9].txt");
      }
      # root info
      my $infofile = sprintf("info%03d.txt", $move);
      if ( -r $infofile) {
	open(INFO, $infofile) || die "open $!";
	my @info = <INFO>;
	close INFO;
	say(join("", @info));
      }
      my $infofile_en = sprintf("info%03d-en.txt", $move);
      if ( -r $infofile_en) {
	open(INFO, $infofile_en) || die "open $!";
	my @info_en = <INFO>;
	close INFO;
	say_en(join("", @info_en));
      }
      # recorddb
      if ( ! -r $infofile && $move % 5 == 0) {
	open(DB, "./querydb.pl $kif $move|") || die "open $!";
	my @db = <DB>;
	close DB;
	my $contents = join("", @db);
	if ($contents) {
	  $contents = nkf("-w", $contents);
	  my $dbfile = sprintf("recorddb%03d.txt", $move);
	  open(DBOUT, "> $dbfile") || die "open $!";
	  print DBOUT $contents;
	  close DBOUT;
	  say("* ".$contents);
	}
      }

      # search result
      my $now = time();
      if ($now - $last_announce > 60*60*2
	  || $move >= $last_announce_move + 20) {
	&announce_title($title, $url, $seconds, 1);
	$last_announce = $now;
	$last_announce_move = $move;
      }
      $status = system("./analyze-kif", "-f", $kif, "-m", $move, "-S", $seconds);
      warn "status changed? $status" if $status;
      my $retfile = sprintf("analyses%03d.txt", $move);
      open(RET, $retfile) || die "open $!";
      my @ret = <RET>;
      close RET;
      say(join("", @ret));

      my $retfile_en = sprintf("analyses%03d-en.txt", $move);
      open(RET, $retfile_en) || die "open $!";
      my @ret_en = <RET>;
      close RET;
      say_en(join("", @ret_en));

      ++$move;
      $sleep_count = -1;
    } else {
      ++$sleep_count;
    }
  }
  if ($kill_process) {
    print STDERR "sending kill CONT to $kill_process\n";
    kill "CONT" => $kill_process;
  }
  exit if ($status == 1 || $resigned);

  if ($sleep_count >= 1) {
    my $wait = $sleep_count*10;
    $wait = 120 if ($wait > 120);
    print STDERR "\nWAIT $wait seconds for kif update\n\n";
    sleep $wait;
  }
}
