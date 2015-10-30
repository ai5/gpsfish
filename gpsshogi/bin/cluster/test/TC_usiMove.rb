$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'usiMove'
require 'piece'
require 'board'

class Test_UsiMove < Test::Unit::TestCase
  def setup
    @b=ShogiServer::Board.new
    @b.set_from_str(<<EOM)
P1 *  * +KI *  *  *  * -NK-KY
P2 *  *  *  *  *  *  *  *  * 
P3-FU-FU *  *  * -OU-KE *  * 
P4 *  * +GI *  *  *  *  *  * 
P5+FU *  *  *  * +FU-FU * -FU
P6 *  *  *  * +FU *  * +FU * 
P7 *  *  *  *  *  * +FU+GI+FU
P8 *  * -HI *  *  * +KI * +OU
P9+KY *  *  * -UM-HI * +KE+KY
P+00GI00KE00FU00FU00FU00FU00FU00FU
P-00KA00KI00KI00GI00KY00FU00FU
+
EOM
  end
  def test_ptype
    assert_equal('OU',UsiMove.ptype('K'))
    assert_equal('OU',UsiMove.ptype('k'))
    assert_equal('HI',UsiMove.ptype('R'))
    assert_equal('HI',UsiMove.ptype('r'))
    assert_equal('KA',UsiMove.ptype('B'))
    assert_equal('KA',UsiMove.ptype('b'))
    assert_equal('KI',UsiMove.ptype('G'))
    assert_equal('KI',UsiMove.ptype('g'))
    assert_equal('GI',UsiMove.ptype('S'))
    assert_equal('GI',UsiMove.ptype('s'))
    assert_equal('KE',UsiMove.ptype('N'))
    assert_equal('KE',UsiMove.ptype('n'))
    assert_equal('KY',UsiMove.ptype('L'))
    assert_equal('KY',UsiMove.ptype('l'))
    assert_equal('FU',UsiMove.ptype('P'))
    assert_equal('FU',UsiMove.ptype('p'))
  end
  def test_position
    assert_equal('11',UsiMove.position('1','a'))
    assert_equal('19',UsiMove.position('1','i'))
    assert_equal('21',UsiMove.position('2','a'))
    assert_equal('91',UsiMove.position('9','a'))
    assert_equal('99',UsiMove.position('9','i'))
  end
  def test_rposition
    assert_equal('1a',UsiMove.rposition(1,1))
    assert_equal('1i',UsiMove.rposition(1,9))
    assert_equal('2a',UsiMove.rposition(2,1))
    assert_equal('9a',UsiMove.rposition(9,1))
    assert_equal('9i',UsiMove.rposition(9,9))
  end
  def test_to_CSA
    assert_equal('%TORYO',UsiMove.to_CSA(@b,'resign'))
    assert_equal('%KACHI',UsiMove.to_CSA(@b,'win'))
    assert_equal('+7172KI',UsiMove.to_CSA(@b,'7a7b'))
    assert_equal('+7463NG',UsiMove.to_CSA(@b,'7d6c+'))
    assert_equal('+7463GI',UsiMove.to_CSA(@b,'7d6c'))
    assert_equal('+2122NK',UsiMove.to_CSA(@b,'2a2b'))
    assert_equal('+0063GI',UsiMove.to_CSA(@b,'S*6c'))
    assert_equal('+0063KE',UsiMove.to_CSA(@b,'N*6c'))
    assert_equal('+0063FU',UsiMove.to_CSA(@b,'P*6c'))
  end
  def test_from_CSA
    assert_equal('resign',UsiMove.from_CSA(@b,'%TORYO'))
    assert_equal('win',UsiMove.from_CSA(@b,'%KACHI'))
    assert_equal('7a7b',UsiMove.from_CSA(@b,'+7172KI'))
    assert_equal('7d6c+',UsiMove.from_CSA(@b,'+7463NG'))
    assert_equal('7d6c',UsiMove.from_CSA(@b,'+7463GI'))
    assert_equal('2a2b',UsiMove.from_CSA(@b,'+2122NK'))
    assert_equal('S*6c',UsiMove.from_CSA(@b,'+0063GI'))
    assert_equal('N*6c',UsiMove.from_CSA(@b,'+0063KE'))
    assert_equal('P*6c',UsiMove.from_CSA(@b,'+0063FU'))
  end
  def test_moves_to_CSA
    usiMoves=['2g1f','1e1f','1h2g','1f1g+','pass']
    csaMoves=['+2716GI','-1516FU','+1827OU','-1617TO']
    assert_equal(csaMoves,UsiMove.moves_to_CSA(@b,usiMoves))
    usiMoves=['3h4h','4i4h+','S*3h','pass']
    csaMoves=['+3848KI','-4948RY','+0038GI']
    assert_equal(csaMoves,UsiMove.moves_to_CSA(@b,usiMoves))
  end
  def test_moves_from_CSA
    usiMoves=['2g1f','1e1f','1h2g','1f1g+']
    csaMoves=['+2716GI','-1516FU','+1827OU','-1617TO']
    assert_equal(usiMoves,UsiMove.moves_from_CSA(@b,csaMoves))
    usiMoves=['3h4h','4i4h+','S*3h']
    csaMoves=['+3848KI','-4948RY','+0038GI']
    assert_equal(usiMoves,UsiMove.moves_from_CSA(@b,csaMoves))
  end
  def test_last_move_type
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1 *  *  *  *  *  * -KE * -OU
P2 *  *  * -GI * -KI-KI *  * 
P3 *  *  * -FU *  * -GI-FU+UM
P4-KY-FU *  * -FU+FU-FU * -KY
P5 *  *  *  *  * -FU * +KE * 
P6-HI * -FU+FU+FU *  * +GI * 
P7+KE+FU * +KI *  * +KE+FU * 
P8+FU+OU+KI+KA * -NY *  *  * 
P9 *  *  *  *  *  *  *  *  * 
P+00FU00FU
P-00HI00GI00KY00FU00FU00FU
+
EOM
    rt=UsiMove.last_move_type(b,['P*1b'])
    assert_equal(:normal,rt)
    # 打ち歩詰めのチェック
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1 *  *  *  *  *  *  * -KE-OU
P2 *  *  * -GI * -KI-KI *  * 
P3 *  *  * -FU *  * -GI-FU+UM
P4-KY-FU *  * -FU+FU-FU * -KY
P5 *  *  *  *  * -FU * +KE * 
P6-HI * -FU+FU+FU *  * +GI * 
P7+KE+FU * +KI *  * +KE+FU * 
P8+FU+OU+KI+KA * -NY *  *  * 
P9 *  *  *  *  *  *  *  *  * 
P+00FU00FU
P-00HI00GI00KY00FU00FU00FU
+
EOM
    rt=UsiMove.last_move_type(b,['P*1b'])
    assert_equal(:uchifuzume,rt)
    # 王手千日手のチェック
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY-KE *  *  *  * +GI-KE-KY
P2 *  *  *  * +RY-KI *  * -OU
P3-FU * -FU *  *  *  * -FU-FU
P4 * -FU * +FU-GI+GI-FU *  * 
P5 *  *  *  * -FU *  *  * +FU
P6 *  * +FU *  *  * +KI *  * 
P7+FU+FU-UM-TO+FU * +FU+FU+KY
P8+KY *  *  *  *  * +OU *  * 
P9 *  *  *  * -HI *  * -KI * 
P+00KA00KI00GI00KE00FU
P-00KE00FU
-
EOM
    moves=['2i3i','3h2h','3i2i','2h3h']
    move='2i3i'
    rt=UsiMove.last_move_type(b,moves*2)
    assert_equal(:normal,rt)
    rt=UsiMove.last_move_type(b,moves*3)
    assert_equal(:normal,rt)
    rt=UsiMove.last_move_type(b,moves*3+[move])
    assert_equal(:oute_sennichite_gote_lose,rt)
    # 千日手のチェック
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
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
    moves=['5i5h','5a5b','5h5i','5b5a']
    move='5i5h'
    rt=UsiMove.last_move_type(b,moves*2)
    assert_equal(:normal,rt)
    rt=UsiMove.last_move_type(b,moves*3)
    assert_equal(:normal,rt)
    rt=UsiMove.last_move_type(b,moves*3+[move])
    assert_equal(:sennichite,rt)
    #
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA * 
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8+KY *  *  *  *  *  * +HI * 
P9+OU+KE+GI+KI+KA+KI+GI+KE+KY
+
EOM
    moves=['5i6h']+['5a5b','9i8h','5b5a','8h9i']*2+['5a5b','9i8h','5b5a']
    rt=UsiMove.last_move_type(b,moves+['8h9i'])
    assert_equal(:sennichite,rt)
  end
end
