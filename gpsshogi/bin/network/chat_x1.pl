#!/usr/bin/perl -w

# ログインして fortune をチャットする

use CsaClient;
use IO::Handle;
use strict;

my $host = "wdoor.c.u-tokyo.ac.jp";
my $port = $ENV{PORT} ? $ENV{PORT} : '5081';
my $user = $ENV{SHOGIUSER} ? $ENV{SHOGIUSER} : "chatboys$$";
my $pass = $ENV{SHOGIPASS} ? $ENV{SHOGIPASS} : "chatboys$$";

$| = 1;

my $client = new CsaClient($user, $pass);
$client->connect($host, $port);
$client->login_x1();

my $has_fortune = `fortune`;
foreach (1 .. 5) {
    my @messages = split(/\n/, &fortune());

    foreach my $line (@messages) {
	$client->chat($line);
	while ($client->try_read()) {
	}
	select(undef, undef, undef, 0.1); # short sleep
    }
}

$client->logout();
$client->disconnect();
exit 0;

sub fortune () {
    if ($has_fortune) {
	my $result = `fortune`;
	return $result
	    if ($result);
	$has_fortune = 0;
    }
    return "hello, world";
}
