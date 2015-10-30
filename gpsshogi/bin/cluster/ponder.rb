# -*- coding: utf-8 -*-
class PonderPlayer
  def initialize(usiArray)
    @usiArray=usiArray
    @mutex=Mutex.new
    @informed_time=Time.now.to_f
    @ponder_finished=true
  end
  attr_accessor :finish_queue, :best_move, :pv
  def ponder_move(m)
    $stderr.print "m=#{m}\n"
    $stderr.print "ponder_move\n"
    local_queue=Queue.new
    @searcher=UsiMulti.new(@usiArray,@search_board,@time_std,@time_extend,@search_moves+[m],[],local_queue,0,$stderr)
    $stderr.print "informed_move=#{@informed_move}\n"
    @mutex.synchronize {
      return if @informed_move
      @current_move=m
      $stderr.print "ponder: #{@current_move}(speculative)\n";
      @searcher.root_search
    }
    v=UsiWrapper.wait_stopped(local_queue)
    @mutex.synchronize {
      @m2m[m]=v
    }
  end
  def ponder(board,time_std,time_extend,moves,finish_queue,expected_move=nil)
    $stderr.print "ponder(board,time_std=#{time_std},time_extend=#{time_extend},moves=#{moves},finish_queue,multi_pv_width=#{MultiPVWidth},expected_move=#{expected_move})\n"
    @search_board=board.deep_copy
    @search_moves=moves.dup
    @time_std=time_std
    @time_extend=time_extend
    @finish_queue=finish_queue
    @m2m=Hash.new # move -> responce
    @current_move=nil
    @informed_move=nil
    @stopped=false
    @mutex.synchronize{ @ponder_finished=false }
    ignore_moves=[]
    @bestmove=nil
    @pv=nil
    if expected_move
      # set "expected_move" to nil if "expected_move" is a illegal move
      csaMoves=UsiMove.moves_to_CSA(board,moves+[expected_move])
      expected_move=nil if csaMoves.size!=moves.size+1
    end
    Thread.new{
      while !@informed_move
        if !expected_move
          time_first=1000
          local_queue=Queue.new
          start_time=Time.now
          @mutex.synchronize {
            @usiArray[0].search(board,time_first,0,moves,local_queue,0,ignore_moves,true)
            @current_move=nil
          }
          v=UsiWrapper.wait_stopped(local_queue)
          break if @informed_move
          expected_move=v[4]
          break if expected_move=='resign' || expected_move=='win' || expected_move=='pass'
        end
        ponder_move(expected_move)
        ignore_moves.push(expected_move)
        expected_move=nil
      end
#      p "end of ponder"
      @mutex.synchronize{
        if @informed_move == 'resign'
          @finish_queue.push([self,:exact,'','','','',''])
          @finish_queue.push([self,:stopped])
        elsif @informed_move
          if @m2m.has_key?(@informed_move)
            @bestmove=@m2m[@informed_move][4]
            @pv=@m2m[@informed_move][6]
            @finish_queue.push(@m2m[@informed_move])
            @finish_queue.push([self,:stopped])
          else
#            @searcher.clear_pipe
            $stderr.print "ponder: miss stop_time=#{Time.now.to_f-@informed_time}\n"
	    	$stderr.print "ponder: #{@informed_move}(stable),n_moves=#{moves.size+1}\n"
            @searcher=UsiMulti.new(@usiArray,@search_board,@time_std,@time_extend,@search_moves+[@informed_move],[],@finish_queue,0,$stderr)
            @searcher.root_search
          end
        end
        @ponder_finished=true
      }
    }
  end
  def inform(csaMove)
    @informed_time=Time.now.to_f
    usiMove=UsiMove.moves_from_CSA(@search_board,UsiMove.moves_to_CSA(@search_board,@search_moves)+[csaMove])[-1]
    raise "cannot convert move #{csaMove}" if !usiMove
#    p "usiMove=#{usiMove}"
#    p "@current_move=#{@current_move}"
#    p "m2m.has_key?(usiMove)=#{@m2m.has_key?(usiMove)}"
    @mutex.synchronize{
      @informed_move=usiMove
      if @ponder_finished
        if @m2m.has_key?(@informed_move)
          @finish_queue.push(@m2m[@informed_move])
          @finish_queue.push([self,:stopped])
        else
       	  $stderr.print "ponder: #{@informed_move}(stable),n_moves=#{@search_moves.size+1}\n";
            @searcher=UsiMulti.new(@usiArray,@search_board,@time_std,@time_extend,@search_moves+[@informed_move],[],@finish_queue,0,$stderr)
            @searcher.root_search
        end
      elsif usiMove!=@current_move
        @searcher.stop
      end
    }
  end
  # ponder中に相手が投了等で終了した際に呼ぶ．
  def stop()
    @informed_move='resign'
    @searcher.stop
  end
end
