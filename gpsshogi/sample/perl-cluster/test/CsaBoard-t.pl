#!/usr/bin/perl -w
use Test;
use strict;

BEGIN { plan tests=>7777 };

use module::CsaBoard;

# csashow
{
  my $board = new CsaBoard;
my $initial_board=<<EOM
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
;
  ok($board->csa_show, $initial_board);
}

# csamove_to_usi
{
  my $board = new CsaBoard;
  ok($board->csamove2usi("+7776FU"), "7g7f");
  ok($board->csamove2usi("-3334FU"), "3c3d");
  ok($board->csamove2usi("+0044GI"), "S*4d");
}

# usi2csa
{
  my $board = new CsaBoard;
  ok($board->usi2csa("7g7f") , "+7776FU");
  ok($board->usi2csa("R*4e") , "+0045HI");
  ok($board->usi2csa("8h2b+"), "+8822UM");
}

# makemove
{
  my $board = new CsaBoard;
my $initial_board=<<EOM
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
;
my $b7776fu=<<EOM
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA * 
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  * +FU *  *  *  *  *  * 
P7+FU+FU * +FU+FU+FU+FU+FU+FU
P8 * +KA *  *  *  *  * +HI * 
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
-
EOM
;
my $b8822hi=<<EOM
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 *  *  *  *  *  *  * -HI * 
P3-FU-FU-FU-FU-FU-FU * -FU-FU
P4 *  *  *  *  *  * -FU *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  * +FU *  *  *  *  *  * 
P7+FU+FU * +FU+FU+FU+FU+FU+FU
P8 *  *  *  *  *  *  * +HI * 
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
P+00KA
P-00KA
+
EOM
;
  ok($board->csa_show, $initial_board);
  $board->make_move("77","76","FU");
  ok($board->csa_show, $b7776fu);
  $board->unmake_move;
  ok($board->csa_show, $initial_board);
  $board->make_csamove("+7776FU");
  $board->make_csamove("-3334FU");
  $board->make_csamove("+8822UM");
  ok($board->count_hand("+", "KA"), 1);
  ok($board->count_hand("-", "KA"), 0);
  $board->make_csamove("-8222HI");
  ok($board->csa_show, $b8822hi);
  ok($board->count_hand("+", "KA"), 1);
  ok($board->count_hand("-", "KA"), 1);
  $board->unmake_move;
  ok($board->count_hand("+", "KA"), 1);
  ok($board->count_hand("-", "KA"), 0);
  $board->unmake_move;
  ok($board->count_hand("+", "KA"), 0);
  ok($board->count_hand("-", "KA"), 0);
  $board->unmake_move;
  ok($board->count_hand("+", "KA"), 0);
  ok($board->count_hand("-", "KA"), 0);
  ok($board->csa_show, $b7776fu);
}

# board2usi
{
  my $board = new CsaBoard;
  ok(CsaBoard::board2usi(@{$board->{board}}),
     "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL");
}

# usi_show_position
{
  my $board = new CsaBoard;
  ok($board->usi_show_position, "position startpos");
}

# usi_show
{
  my $board = new CsaBoard;
  ok($board->usi_show, "position startpos");
  $board->make_csamove("+7776FU");
  $board->make_csamove("-3334FU");
  $board->make_csamove("+2726FU");
  ok($board->usi_show, "position startpos moves 7g7f 3c3d 2g2f");
}

# parse_usi
{
  my $board = CsaBoard::parse_usi("position startpos");
  ok($board->usi_show, "position startpos");
  my $usi = "position sfen 1nsg4l/lk4g2/ppppp1npp/6R2/9/2P1P1P2/PP1PG1S1P/2K2+n3/LNSG3+rL b BS2Pb2p 1";
  $board = CsaBoard::parse_usi($usi);
  ok($board->usi_show, $usi);
}
