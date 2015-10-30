$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'usiMove'
require 'piece'
require 'board'
require 'usiWrapper'
require 'usiArray'
require 'usiMulti'
require 'config'
require 'ponder'

class Test_Ponder < Test::Unit::TestCase
  def setup
    Thread.abort_on_exception = true
    $logBase="usilogs/"
    @usiArray=UsiArray.new(UsiPrograms)
    @b=ShogiServer::Board.new
    @b.set_from_str(<<EOM)
P1-GI-KE-KI-GI-KE-TO-TO-TO-OU
P2 *  *  *  *  *  *  *  * -KI
P3-FU * -FU-HI *  *  * -TO-KY
P4 * -KY * +FU *  *  * -UM-FU
P5 *  *  *  *  *  * -FU *  * 
P6 *  * +KE *  * +KY *  * +KE
P7+FU+FU+FU * +FU+FU+FU+FU+FU
P8 * +KA+KI * +OU *  * +HI * 
P9+KY * +GI *  * +KI+GI *  * 
P-00FU
+
EOM
  end
  def tear_down
    @usiArray.quit
  end
  # ponderがhitしたケース
  def hittest(usiArray)
    $time_start=Time.now.to_f
    finish_queue=Queue.new
    ponder=PonderPlayer.new(usiArray)
    root_bestmove='1f2d'
    csamove=UsiMove.to_CSA(@b,root_bestmove)
    # 1手目探索中に1手目が一致
    ponder.ponder(@b,4000,0,[],finish_queue,root_bestmove)
    sleep 2
    time_start=Time.now.to_f
    ponder.inform(csamove)
    ret=finish_queue.pop
    time_end=Time.now.to_f
    $stderr.puts("ret=#{ret[1..-1]},elapsed=#{time_end-time_start}")
    assert(time_end-time_start < 4.0-2.0+2.0)
    # 2手目探索中に1手目が一致
    ponder.ponder(@b,5000,0,[],finish_queue,root_bestmove)
    sleep 7
    time_start=Time.now.to_f
    ponder.inform(csamove)
    ret=finish_queue.pop
    time_end=Time.now.to_f
    $stderr.puts("ret=#{ret[1..-1]},elapsed=#{time_end-time_start}")
    assert(time_end-time_start < 1.0)
    # 2手目探索終了後に1手目が一致
    ponder.ponder(@b,3000,0,[],finish_queue,root_bestmove)
    sleep 7
    time_start=Time.now.to_f
    ponder.inform(csamove)
    ret=finish_queue.pop
    time_end=Time.now.to_f
    $stderr.puts("ret=#{ret[1..-1]},elapsed=#{time_end-time_start}")
    assert(time_end-time_start < 1.0)
    # 1手目探索中に別の手が一致
    ponder.ponder(@b,5000,0,[],finish_queue,root_bestmove)
    sleep 2
    time_start=Time.now.to_f
    ponder.inform('+4641NY')
    ret=finish_queue.pop
    time_end=Time.now.to_f
    $stderr.puts("ret=#{ret[1..-1]},elapsed=#{time_end-time_start}")
    assert(time_end-time_start < 5.0+1.0)
  end
  def test_hit
    hittest(@usiArray)
  end
end
