$:.unshift File.join(File.dirname(__FILE__), "..", "ext")
require 'test/unit'
require 'osl4r.so'

HIRATE = <<-EOF
P1-KY-KE-GI-KI-OU-KI-GI-KE-KY
P2 * -HI *  *  *  *  * -KA *
P3-FU-FU-FU-FU-FU-FU-FU-FU-FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  *
P6 *  *  *  *  *  *  *  *  * 
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA *  *  *  *  * +HI *
P9+KY+KE+GI+KI+OU+KI+GI+KE+KY
P+
P-
EOF

class OSLTest < Test::Unit::TestCase
  def initialize(a)
    super
    OSL::setup
  end

  def setup
  end

  def test_osl_search
    val, move = OSL::search(HIRATE, 1, false);
    assert(val > 0)
    assert(/\+\d\d\d\dFU/ =~ move)
  end

  def test_checkmate_is_losing_true
    ret = OSL::checkmate_is_losing(<<-EOF, 1);
P1 *  *  *  *  *  * -OU *  * 
P2 * -HI *  *  *  * +GI *  * 
P3 *  *  *  *  * +TO+TO *  * 
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7 *  *  *  *  *  *  *  *  * 
P8 *  *  *  *  *  *  *  *  * 
P9 *  *  *  *  * +OU *  *  * 
P+
P-00AL
-
EOF
    assert(ret)
  end

  def test_checkmate_is_losing_false
    ret = OSL::checkmate_is_losing(HIRATE, 1);
    assert(!ret)
  end

  def test_checkmate_is_winning_true
    ret, val, move = OSL::checkmate_is_winning(<<-EOF, 1);
P1 *  *  *  *  *  * -OU *  * 
P2 * -HI *  *  *  *  *  *  * 
P3 *  *  *  *  * +TO+TO *  * 
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  *  *  *  *  *  *  * 
P6 *  *  *  *  *  *  *  *  * 
P7 *  *  *  *  *  *  *  *  * 
P8 *  *  *  *  *  *  *  *  * 
P9 *  *  *  *  * +OU *  *  * 
P+00GI
P-00AL
+
EOF
    assert(ret)
    assert_equal(move, "+0032GI")
  end

  def test_checkmate_is_winning_false
    ret, val, move = OSL::checkmate_is_winning(HIRATE, 1);
    assert(!ret)
    assert_equal(val, nil)
    assert_equal(move, nil)
  end
end

