#!/usr/bin/perl -w
use Test;
use strict;

BEGIN { plan tests=>7777 };

use module::UsiEngine;
use module::UsiPosition;

my $program = "./gpsusi";
my $config = {id=>1, command=>$program, logdir=>"test" };
my $gpsusi = new UsiEngine($config);
ok($gpsusi->{id}, 1);
ok($gpsusi->init());

$gpsusi->set_position(new UsiPosition("startpos"));

# genmove
{
  my @moves = $gpsusi->genmove();
  my $initial_moves = "9g9f 8g8f 7g7f 6g6f 5g5f 4g4f 3g3f 2g2f 1g1f 1i1h 9i9h 3i4h 3i3h 7i6h 7i7h 4i5h 4i3h 4i4h 6i7h 6i5h 6i6h 2h3h 2h4h 2h5h 2h6h 2h7h 2h1h 5i6h 5i5h 5i4h";
  ok(join(' ',@moves), $initial_moves);

  my $initial_rp = $gpsusi->genmove_probability();
  my $min_value = (sort { $a <=> $b } values %$initial_rp)[0];
  my $good_rp = 1;
  foreach my $move (@moves) {
    my $rp = $initial_rp->{$move};
    $good_rp &= $rp > 0 && $rp <= 1400;
    ok($move, "7g7f") if ($rp == $min_value);
  }
  ok($good_rp);
}

# csashow
{
  my $str = $gpsusi->csashow();
  my $csa_str = <<"EOM";
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA * 
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA *  *  *  *  * +HI * 
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
+
EOM
  ok($str, $csa_str);
  ok($gpsusi->csamove("7g7f"), "+7776FU");
  ok(join(' ', $gpsusi->csamoves(["7g7f","3c3d","8h7g"])),
     "+7776FU -3334FU +8877KA");
  ok($str, $csa_str);
}

# book
{
  $gpsusi->set_position(new UsiPosition("startpos"));
  my $book_move = $gpsusi->gobook(10);
  ok($book_move eq "7g7f" || $book_move eq "2g2f");
}
{
  $gpsusi->set_position(new UsiPosition("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/KN4P2/2PP4B/SP2P2+sP/B+rS6/7NL w R3Gsnl4p 1"));
  my $book_move = $gpsusi->gobook(10);
  ok($book_move, "pass");
}

# mate
{
  $gpsusi->set_position(new UsiPosition("startpos"));
  my ($mate) = $gpsusi->gomate(msec=>1000);
  ok($mate, "nomate");
}

{
  $gpsusi->set_position(new UsiPosition("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/KN4P2/2PP4B/SP2P2+sP/B+rS6/7NL w R3Gsnl4p 1"));
  my ($mate, $pv) = $gpsusi->gomate(msec=>1000);
  ok($mate, "checkmate");
  ok($pv->[0], "8h9g");
}

# fullsearch
{
  $gpsusi->set_position(new UsiPosition("startpos"));
  my $ret = $gpsusi->fullsearch(msec=>1000);
  ok($ret->{bestmove}, "2g2f");
  ok($ret->{depth} > 3);
  ok(abs($ret->{value}) < 5000);
  ok($ret->{pv}->[0], "2g2f");
  ok($ret->{pv}->[1] eq "3c3d" || $ret->{pv}->[1] eq "5a4b");
  ok(! defined $ret->{forced});
}

{
  $gpsusi->set_position(new UsiPosition("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/Kg4P2/2PP4B/SPN1P2+sP/B+rS6/7NL b R2Gsnl4p 1"));
  my $ret = $gpsusi->fullsearch(msec=>1000);
  ok($ret->{bestmove}, "7g8e");
  ok($ret->{depth} > 4);
  ok($ret->{forced});
}

{
  $gpsusi->set_position(new UsiPosition("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/KN4P2/2PP4B/+rP2P2+sP/B1S6/7NL b R3G2snl4p 1"));
  my $ret = $gpsusi->fullsearch(msec=>1000);
  ok($ret->{bestmove}, "resign");
  ok($ret->{depth} > 4);
  ok(! defined $ret->{forced});
}

# fullsearch_onereply_extension
{
  my $position = new UsiPosition("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/Kg4P2/2PP4B/SPN1P2+sP/B+rS6/7NL b R2Gsnl4p 1");
  my $ret = $gpsusi->fullsearch_onereply_extension($position, msec=>1000);
  # $gpsusi->show_search_result($ret);
  ok($ret->{bestmove}, "7g8e");
  ok($ret->{depth} > 2);
  ok($ret->{value} < 1000000);
  ok(! $ret->{forced});
}

# cleanup
$gpsusi->finish();
