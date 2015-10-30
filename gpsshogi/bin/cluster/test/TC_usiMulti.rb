$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'usiMove'
require 'piece'
require 'board'
require 'usiWrapper'
require 'usiArray'
require 'usiMulti'
require 'config'

class Test_UsiMulti < Test::Unit::TestCase
  def setup
    Thread.abort_on_exception = true
    $logBase="usilogs/"
    @usiArray=UsiArray.new(UsiPrograms)
    @b=ShogiServer::Board.new
    @b.set_from_str(<<EOM)
P1-KY-KE *  *  *  *  *  * +UM
P2 * +GI-OU *  *  *  *  * -KY
P3 *  * -FU *  *  *  *  *  * 
P4-FU * -GI * -KI *  * -FU-FU
P5 * +KE * -FU *  *  * +KE * 
P6+FU+OU *  * +FU+FU *  * +FU
P7 *  *  *  * +GI *  *  *  * 
P8 *  *  *  *  *  *  * +HI * 
P9+KY *  * -UM *  *  *  * +KY
P+00HI00KI00KI00GI00FU00FU00FU00FU00FU00FU00FU00FU
P-00FU00KE00KI
+
EOM
  end
  def teardown
    @usiArray.quit
  end
  def test_checkmate
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,@b,10000,0,[],[],finish_queue,0,$stderr)
    usiMulti.root_search
    m=finish_queue.pop
    assert(m[5] > 10000)
  end
  def test_uchifu
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
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,[],[],finish_queue,0,$stderr)
    usiMulti.root_search
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    $stderr.puts "root: #{bestmove}"
    assert('P*1b' != bestmove)
  end
  def test_search
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-GI-KE-KI-GI-KE-TO-TO-TO-OU
P2 *  *  *  *  *  *  * -TO-KI
P3-FU * -FU-FU *  *  *  * -KY
P4 * -KY *  *  *  *  * -UM-FU
P5 *  *  *  *  * -HI-FU *  * 
P6 *  * +KE *  * +KY *  * +KE
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA+KI * +OU *  * +HI * 
P9+KY * +GI *  * +KI+GI *  * 
+
EOM
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,[],[],finish_queue,0,$stderr)
    usiMulti.root_search
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    $stderr.puts "root: #{bestmove}"
    assert_equal('1f2d',bestmove)
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY *  *  *  *  *  * -KE-KY
P2 *  * +KI *  *  * -OU *  * 
P3 *  * -KE * -FU *  * -FU-FU
P4-FU-HI *  *  * -FU-FU *  * 
P5 * -FU-FU * +FU *  * +FU * 
P6 *  * +FU+FU * -UM *  *  * 
P7+FU+FU * +KI-NK *  *  * +FU
P8 * +OU+KI *  *  * -UM *  * 
P9+KY+KE *  *  *  *  * -RY+KY
P+00KI00GI00GI
P-00GI00GI00FU00FU00FU
+
EOM
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,[],[],finish_queue,0,$stderr)
    usiMulti.root_search
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    $stderr.puts "root: #{bestmove}"
    assert_equal('S*4a',bestmove)
  end
  def test_forced
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY-KE-GI+KA *  * +RY-KE-KY
P2 * -OU *  * -KI *  *  *  * 
P3-FU-FU-FU-FU-FU-GI-KI * -FU
P4 *  *  *  *  *  *  *  *  * 
P5 *  *  * -KA *  * -FU *  * 
P6 *  * +FU * +FU * +FU *  * 
P7+FU+FU+KE+FU * +GI * -HI+FU
P8 *  * +OU * +KI *  *  *  * 
P9+KY * +GI+KI *  *  * +KE+KY
P+00FU00FU
P-00FU00FU
-
EOM
    time_start=Time.now.to_f
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,['6e7f','6a8c+'],[],finish_queue,0,$stderr)
    usiMulti.root_search
    m=UsiWrapper.wait_stopped(finish_queue)
    assert_equal('8b8c',m[4])
    assert_equal(0,m[5])
    assert_equal(['6e7f','6a8c+','8b8c'],m[6])
    assert(m[7]) # forced_move
    assert(Time.now.to_f-time_start<0.5)
    # level_two
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,['6e7f'],[],finish_queue,0,$stderr)
    usiMulti.root_search
    m=UsiWrapper.wait_stopped(finish_queue)
    puts "#{m[1..-1]}"
  end
  def test_oute_sennnichite
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY-KE *  *  *  * +GI-KE-KY
P2 *  *  *  * +RY-KI *  * -OU
P3-FU * -FU *  *  *  * -FU-FU
P4 * -FU * +FU-GI+GI-FU *  * 
P5 *  *  *  * -FU *  *  * +FU
P6 *  * +FU *  *  * +KI *  * 
P7+FU+FU-UM-TO+FU * +FU+FU+KY
P8+KY *  *  *  *  *  * +OU * 
P9 *  *  *  * -HI *  * -KI * 
P+00KA00KI00GI00KE00FU
P-00KE00FU
+
EOM
    finish_queue=Queue.new
    moves=['2h3h','2i3i','3h2h','3i2i']*3
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,moves,[],finish_queue,0,$stderr)
    usiMulti.root_search
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    $stderr.puts "root: #{bestmove}"
    assert('2h3h' == bestmove)

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
    finish_queue=Queue.new
    moves=['2i3i','3h2h','3i2i','2h3h']*3
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,moves,[],finish_queue,0,$stderr)
    usiMulti.root_search
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    $stderr.puts "root: #{bestmove}"
    assert('2i3i' != bestmove)
  end
  def test_sennnichite1
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY-KE * -KI *  *  *  * -KY
P2 * -OU-GI *  *  * -KI *  * 
P3-FU-FU-FU-FU *  *  *  * -FU
P4 *  *  *  * -HI *  *  *  * 
P5 *  *  * +FU *  * -KA+FU * 
P6 *  * +FU * +KI-FU *  *  * 
P7+FU+FU+KA *  *  * +KI * +FU
P8+KY+GI *  *  *  *  *  *  * 
P9+OU+KE *  *  *  *  * +HI+KY
P+00GI00KE00FU00FU00FU
P-00GI00KE00FU00FU00FU
+
EOM
    finish_queue=Queue.new
    moves=['P*5e','5d8d','S*7e']+['8d8e','7e8f','8e8d','8f7e']*2+['8d8e','7e8f','8e8d']
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,moves,[],finish_queue,0,$stderr)
    usiMulti.root_search
    m=UsiWrapper.wait_stopped(finish_queue)
    bestmove=m[4]
    $stderr.puts "root: #{bestmove}"
    assert('8f7e' != bestmove || m[5]==0)
  end
  def test_stop
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-GI-KE-KI-GI-KE-TO-TO-TO-OU
P2 *  *  *  *  *  *  * -TO-KI
P3-FU * -FU-FU *  *  *  * -KY
P4 * -KY *  *  *  *  * -UM-FU
P5 *  *  *  *  * -HI-FU *  * 
P6 *  * +KE *  * +KY *  * +KE
P7+FU+FU+FU+FU+FU+FU+FU+FU+FU
P8 * +KA+KI * +OU *  * +HI * 
P9+KY * +GI *  * +KI+GI *  * 
+
EOM
    finish_queue=Queue.new
    usiMulti=UsiMulti.new(@usiArray,b,10000,0,[],[],finish_queue,0,$stderr)
    usiMulti.root_search
    sleep 2
    time_stop=Time.now.to_f
    usiMulti.stop
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    time_end=Time.now.to_f
    $stderr.puts("bestmove=#{bestmove},elapsed=#{time_end-time_stop}")
    assert(time_end-time_stop < 2.0)
  end
end
