#!/usr/bin/perl -w

# - ログインする
# - 引数で与えたファイルから命令を読みこんで実行する
# - ログアウトする

# 命令
# - サーバに送るもの: /^%+-[A-Z]/ そのまま送る
# - 拡張命令
#   nop " \n" を送る
#   read 1行読む (なければblockする)
#   read_all blockせずに読めるだけ読む
#   sleep x

# 実行例
# replay.pl filename.csa

use CsaClient;
use IO::Handle;
use strict;

my $host = $ENV{SERVER} ? $ENV{SERVER} : "localhost"; # "wdoor.c.u-tokyo.ac.jp";
my $port = $ENV{PORT} ? $ENV{PORT} : '4081';
my $user = $ENV{SHOGIUSER} ? $ENV{SHOGIUSER} : "replay$$";
my $pass = $ENV{SHOGIPASS} ? $ENV{SHOGIPASS} : "replay$$";
my $sleep = $ENV{SLEEP} ? $ENV{SLEEP} : 0;
my $gentle = $ENV{GENTLE} ? $ENV{GENTLE} : 0;

$| = 1;

my $client = new CsaClient($user, $pass);
$client->connect($host, $port);
$client->login_x1();
$client->send("%%GAME replaytest-900-0 *\n");
my ($sente, $initial_filename, $opname, $timeleft, $byoyomi)
  = $client->wait_opponent($$);
if (!$sente) {
  $client->read();
}

while (<>) {
    chomp;
    if (/^sleep ([0-9]+)/) {
	sleep $1;
    } elsif (/^LOGOUT/) {
	last;
    } elsif (/^(\+.+)/) {
      if ($sente) {
	$client->send("$1\n");
	sleep $sleep
	  if ($sleep);
	$client->read();
	$client->read()
	  if ($gentle);
      }
    } elsif (/^(-.+)/) {
      if (!$sente) {
	$client->send("$1\n");
	sleep $sleep
	  if ($sleep);
	$client->read();
	$client->read()
	  if ($gentle);
      }
    } elsif (/^read_all/) {
	while ($client->try_read()) {
	}
    } elsif (/^read/) {
	$client->read();
    } elsif (/^nop/) {
	$client->send(" \r");
	$client->read();
    } elsif (/^T[0-9]+/) {
      ;				# silently ignore
    } else {
	warn "ignored $_\n";
    }
}

$client->logout();
$client->disconnect();
exit 0;

