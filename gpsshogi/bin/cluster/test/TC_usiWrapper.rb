# -*- coding: utf-8 -*-
$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'usiMove'
require 'piece'
require 'board'
require 'usiWrapper'
require 'config'

class Test_UsiWrapper < Test::Unit::TestCase
  def setup
    Thread.abort_on_exception = true
    @usi=UsiWrapper.new(UsiPrograms[0],0)
    @usi.waitok
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
    @usi.quit
  end
  def test_book
    b=ShogiServer::Board.new
    b.initial
    moves=['7g7f','3c3d','1i1h','4c4d','9i9h']
    finish_queue=Queue.new
    # 定跡内のはず
    time_start=Time.now.to_f
    @usi.searchbook(b,moves[0..0],20,finish_queue)
    m=UsiWrapper.wait_stopped(finish_queue)
    bestmove=m[4]
    elapsed=Time.now.to_f-time_start
    assert('pass' != bestmove)
    if ['3c3d','8c8d'].index(bestmove) == nil
      $stderr.puts "soko: bestmove=#{bestmove}"
    end
    assert(['3c3d','8c8d'].index(bestmove))
    assert(elapsed < 1.0)
    # 定跡外のはず
    time_start=Time.now.to_f
    @usi.searchbook(b,moves[0..4],20,finish_queue)
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    elapsed=Time.now.to_f-time_start
    assert('pass' == bestmove)
    assert(elapsed < 1.0)
    # 1手進んだ局面からは定跡外のはず(movesが[])
    b1=b.deep_copy
    b1.handle_one_move('+7776FU')
    time_start=Time.now.to_f
    @usi.searchbook(b1,[],20,finish_queue)
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    elapsed=Time.now.to_f-time_start
    assert('pass' == bestmove)
    assert(elapsed < 1.0)
    # 1手進んだ局面からは定跡外のはず(movesがあり)
    b1=b.deep_copy
    b1.handle_one_move('+7776FU')
    time_start=Time.now.to_f
    @usi.searchbook(b1,moves[1..1],20,finish_queue)
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    elapsed=Time.now.to_f-time_start
    assert('pass' == bestmove)
    assert(elapsed < 1.0)
  end
  def test_genmoves
    finish_queue=Queue.new
    @usi.genmoves(@b,[],finish_queue)
    m=finish_queue.pop
    assert_equal(:genmove,m[1])
    assert_equal(249,m[2].size)
    assert(m[2].index('S*1h'))
    # returns [] for checkmate positions
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY+RY-OU *  *  *  * -KE-KY
P2 *  * -KI *  *  * -KI-KA * 
P3 *  * +KE * +GI-GI * -FU * 
P4-FU-FU * +GI-FU-FU *  *  * 
P5 *  *  * -FU *  * -FU * -FU
P6+FU * -UM *  * +FU *  *  * 
P7 * +FU *  * +FU+KI+FU+FU+FU
P8 *  * -HI *  *  * +KI+GI+KY
P9+KY+KE *  *  *  *  * +KE+OU
P+00FU
P-00FU00FU
-
EOM
    @usi.genmoves(b,[],finish_queue)
    m=finish_queue.pop
    assert_equal(:genmove,m[1])
    assert_equal([],m[2])
  end
  def test_genmoves_probability
    finish_queue=Queue.new
    # returns [] for checkmate positions
    b=ShogiServer::Board.new
    b.set_from_str(<<EOM)
P1-KY+RY-OU *  *  *  * -KE-KY
P2 *  * -KI *  *  * -KI-KA * 
P3 *  * +KE * +GI-GI * -FU * 
P4-FU-FU * +GI-FU-FU *  *  * 
P5 *  *  * -FU *  * -FU * -FU
P6+FU * -UM *  * +FU *  *  * 
P7 * +FU *  * +FU+KI+FU+FU+FU
P8 *  * -HI *  *  * +KI+GI+KY
P9+KY+KE *  *  *  *  * +KE+OU
P+00FU
P-00FU00FU
-
EOM
    @usi.genmove_probability(b,[],finish_queue)
    m=finish_queue.pop
    assert_equal(:genmove_probability,m[1])
    assert_equal([],m[2])
  end
  def test_checkmate
    finish_queue=Queue.new
    @usi.checkmate_search(@b,1000,[],finish_queue)
    m=UsiWrapper.wait_checkmate(finish_queue)
    assert(m[3] == :checkmate)
    @usi.checkmate_search(@b,1000,['8b9c','7b6c'],finish_queue)
    m=UsiWrapper.wait_checkmate(finish_queue)
    assert(m[3] == :checkmate)
    assert(m[4] == 'R*6a')
  end
  def test_forcedmove
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
    finish_queue=Queue.new
    @usi.genmoves(b,['6e7f','6a8c+'],finish_queue)
    m=finish_queue.pop
    assert_equal(:genmove,m[1])
    assert_equal(['8b8c'],m[2])
    @usi.search(b,1000,0,['6e7f','6a8c+'],finish_queue,0,[])
    m=UsiWrapper.wait_stopped(finish_queue)
    assert(m[7])
    assert_equal('8b8c',m[4])
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
    @usi.search(b,1000,0,[],finish_queue,0,[])
    bestmove=UsiWrapper.wait_stopped(finish_queue)[4]
    assert_equal('1f2d',bestmove)
  end
end
