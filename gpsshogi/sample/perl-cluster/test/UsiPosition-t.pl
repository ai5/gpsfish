#!/usr/bin/perl -w
use Test;
use strict;

BEGIN { plan tests=>7777 };

use module::UsiPosition;

# init
{
  {
    my $usi = new UsiPosition();
    ok($usi->to_s(), "position startpos");
    ok($usi->turn(), 0);
  }
  {
    my $usi = new UsiPosition("position startpos");
    ok($usi->to_s(), "position startpos");
  }
  {
    my $usi = new UsiPosition("startpos");
    ok($usi->to_s(), "position startpos");
  }
  {
    my $usi = new UsiPosition("sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/7P1/PPPPPPP1P/1B5R1/LNSGKGSNL w - 1");
    ok($usi->turn(), 1);
  }
  {
    my @positions =
      ("sfen l6nl/3g1k3/p3p2+P1/1ppp1p2p/Kg4P2/2PP4B/SPN1P2+sP/B+rS6/7NL b R2Gsnl4p 1",
       "sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/7P1/PPPPPPP1P/1B5R1/LNSGKGSNL w - 1",
       "sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1 moves 7g7f 3c3d 2g2f 4c4d 2f2e 2b3c 3i4h 3a3b 5g5f 3b4c 5i6h 8b2b 6h7h 5a6b 8h7g 6b7b 7h8h 7b8b 4i5h 7a7b 7i7h 4a5b 9g9f 9c9d 8g8f 6c6d 7h8g 7c7d 5h6h 2c2d 2e2d 2b2d 2h2d 3c2d R*2h P*2c P*2b 2a3c 2b2a+ 8a7c 6i7h 3c4e 2a1a R*4i 1a1b 7c6e 7g5e 5b6c L*4f 4e5g+ 4h5g 6e5g+ 6h5g 4i3i+ 2h2e 3i4h 1b1c 4h5g 1c2c 2d5a 2c3c 5a3c 5e6f 5g5f N*5e P*2d 2e1e 6d6e 6f7g 6e6f 7g6f 4c5b 5e6c 5b6c 1e1c+ G*2b 1c1a 2b3b 1a2a 3b2b 2a3a S*6i 7h7i 6i5h N*8e N*8a 8h9g 5h6g+ P*6b 6a7a G*6a 8a7c 8e9c+ 9a9c 6a7a 7c8e 8f8e 6g6f 7f7e");
    foreach my $str (@positions) {
      my $usi = new UsiPosition($str);
      ok($usi->to_s(), "position ".$str);
    }
  }
}

# make_move
{
  {
    my $usi = new UsiPosition();
    my $copy = $usi->clone;
    ok($usi->to_s(), "position startpos");
    ok($usi->to_s, $copy->to_s);
    ok($usi->turn(), 0);
    ok($usi->turn_sign(), 1);
    ok($usi->is_better_than(100, -100));
    $usi->make_move("7g7f");
    ok($usi->to_s(), "position startpos moves 7g7f");
    ok($usi->turn(), 1);
    ok($usi->turn_sign(), -1);
    ok($usi->is_better_than(-100, 100));
    $usi->make_move("3c3d");
    ok($usi->to_s(), "position startpos moves 7g7f 3c3d");
    ok($usi->turn(), 0);
    ok($usi->to_s ne $copy->to_s);
  }
}

# csamove_to_usi
{
  ok(UsiPosition::csamove_to_usi("+7776FU"), "7g7f");
  ok(UsiPosition::csamove_to_usi("-3334FU"), "3c3d");
  ok(UsiPosition::csamove_to_usi("+0043GI"), "S*4c");
}

# sequence_from
{
  my $position = new UsiPosition("startpos moves 7g7f 3c3d 2g2f");
  my $initial = new UsiPosition("startpos");
  ok($position->sequence_from($initial));
  ok(join(' ', @{$position->sequence_from($initial)}), "7g7f 3c3d 2g2f");
  ok($position->sequence_from($position));
  ok(join(' ', @{$position->sequence_from($position)}), "");
  ok(! $initial->sequence_from($position));
  my $p7g7f = new UsiPosition("startpos moves 7g7f");
  ok($position->sequence_from($p7g7f));
  ok(join(' ', @{$position->sequence_from($p7g7f)}), "3c3d 2g2f");
  my $p2g2f = new UsiPosition("startpos moves 2g2f");
  ok(! $p2g2f->sequence_from($p7g7f));
  ok(! $p7g7f->sequence_from($p2g2f));
}

# initial_position
{
  my $position = new UsiPosition("startpos moves 7g7f 3c3d 2g2f");
  my $initial = new UsiPosition("startpos");
  ok($position->to_s ne $initial->to_s);
  ok($position->initial_position->to_s eq $initial->to_s);
}
