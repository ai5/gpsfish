$:.unshift File.join(File.dirname(__FILE__), "..", "ext")
require 'test/unit'
require 'osl4r.so'

class OSLTest < Test::Unit::TestCase
  def initialize(a)
    super
    OSL::setup
  end

  def test_osl_search_benchmark
    val, move = OSL::search(<<-EOF, 30, true);
P1-KY-KE *  *  *  *  * -KE-OU
P2 *  *  * +UM-KY-GI * -GI-KY
P3 *  *  *  *  * -FU *  *  * 
P4 *  * -FU-FU-FU *  *  * -FU
P5-FU+KE * +FU *  *  *  *  * 
P6 *  *  *  *  * +FU+FU * +FU
P7+FU+FU *  * +KI * +KE+FU * 
P8 *  *  *  * +FU * +GI+OU * 
P9-RY-HI *  *  * +KI *  * +KY
P+00KA00KI00KI00FU00FU
P-00GI00FU00FU
+
EOF
    assert(val > 0)
    assert_equal(move, "+0039KI")
  end
end
