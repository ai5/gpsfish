$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'kisen'
require 'piece'
require 'board'
require 'config'

class Test_kisen < Test::Unit::TestCase
  def test_kisen2pos
    assert_equal('11',KisenFile.kisen2pos(0x1))
    assert_equal('91',KisenFile.kisen2pos(0x9))
    assert_equal('12',KisenFile.kisen2pos(0xa))
    assert_equal('92',KisenFile.kisen2pos(0x12))
    assert_equal('99',KisenFile.kisen2pos(0x51))
  end
  def test_pos2kisen
    assert_equal(0x1,KisenFile.pos2kisen('11'))
    assert_equal(0x9,KisenFile.pos2kisen('91'))
    assert_equal(0xa,KisenFile.pos2kisen('12'))
    assert_equal(0x12,KisenFile.pos2kisen('92'))
    assert_equal(0x51,KisenFile.pos2kisen('99'))
  end
  def test_bidirectional
    (1..9).each do |x|
      (1..9).each do |y|
        pos=x.to_s+y.to_s
        kisen_pos=KisenFile.pos2kisen(pos)
        assert_equal(pos,KisenFile.kisen2pos(kisen_pos))
      end
    end
    (0x1..0x51).each do |kisen_pos|
      pos=KisenFile.kisen2pos(kisen_pos)
      assert_equal(kisen_pos,KisenFile.pos2kisen(pos))
    end
  end
  def test_black_move
    b = ShogiServer::Board.new
    b.set_from_str(<<EOM)
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
    # normal move
    assert_equal('+2716GI',KisenFile.kisen2move(b,KisenFile.pos2kisen('16'),KisenFile.pos2kisen('27')))
    assert_equal('+7463GI',KisenFile.kisen2move(b,KisenFile.pos2kisen('63'),KisenFile.pos2kisen('74')))
    assert_equal('+2131NK',KisenFile.kisen2move(b,KisenFile.pos2kisen('31'),KisenFile.pos2kisen('21')))
    # promote move
    assert_equal('+7463NG',KisenFile.kisen2move(b,0x64+KisenFile.pos2kisen('63'),KisenFile.pos2kisen('74')))
    # drop move
    assert_equal('+0044GI',KisenFile.kisen2move(b,KisenFile.pos2kisen('44'),0x65))
    assert_equal('+0044KE',KisenFile.kisen2move(b,KisenFile.pos2kisen('44'),0x65+1))
    assert_equal('+0044FU',KisenFile.kisen2move(b,KisenFile.pos2kisen('44'),0x65+2))
  end
  def test_read
    k=KisenFile.new(GPSSHOGI_HOME+'/data/kisen/01.kif')
    b = ShogiServer::Board.new
    b.initial
    moves=k.read(b,35)
    assert_equal(133,moves.size)
    assert_equal(['+7776FU','-8384FU','+1716FU'],moves[0...3])
    assert_equal(['+4635GI','-0045KI','+0046KI'],moves[100...103])
    moves=k.read(b,41)
    assert_equal(105,moves.size)
    assert_equal(['+4838OU','-4131OU','+3828OU'],moves[20...23])
    assert_equal(['+3725KE','-3324KI','+8848HI'],moves[70...73])
  end
end
