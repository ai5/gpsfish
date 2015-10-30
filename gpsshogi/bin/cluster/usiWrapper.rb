# -*- coding: utf-8 -*-
require 'thread'
require 'usi'

class UsiWrapper
  def UsiWrapper.wait_stopped(queue)
    ret=nil
    while true
      x=queue.pop
      case x[1]
      when :exact
        ret=x
      when :update
        ret=x
      when :stopped
        return ret
      else
        raise "wait_stopped: received unexpected message #{x[1..-1]}"
      end
    end
  end
  def UsiWrapper.wait_checkmate(queue)
    while true
      x=queue.pop
      case x[1]
        when :checkmate
        return x
      else
        raise "wait_checkmate: received unexpected message #{x[1..-1]}"
      end
    end
  end
  def initialize(program,rank,logger=nil)
    @rank=rank
    set_logger(logger)
    @pipe_mutex=Mutex.new
    if program.class==Array
      @speed=program[1]
      program=program[0]
    else
      @speed=1
    end
    @pipe=IO.popen(program,"r+")
    @pipe.sync=true
    @mutex=Mutex.new
    @readyok=Queue.new
    @searching=false
    @quit=false
    reset
    Thread.new { keep_alive }
    Thread.new { reader }
    puts("usi")
    puts("setoption name UsiOutputPawnValue value 1000")
    puts("setoption name BookDepth value 0")
    puts("setoption name ErrorLogFile value")
    puts("setoption name Verbose value 1")
#    puts("setoption name LimitDepth value 7")
    puts("setoption name LimitDepth value 9") # for --use-log-linear-probability 1
    puts("isready")
  end
  private 
  def handle_bestmove(m)
    elapsed=Time.now.to_f-@go_time
    @cur_bestmove=m
    if @extend==0
      @bestmove=m
    end
    @mutex.synchronize {
      @logger.puts "@cur_bestmove=#{@cur_bestmove},@istop=#{@istop},@cur_forced_move=#{@cur_forced_move},@hasinfo=#{@hasinfo},@stopped=#{@stopped},@is_book_search=#{@is_book_search}" if @logger
      if @cur_bestmove != 'resign' && @cur_bestmove != 'win' && !@istop && (@cur_forced_move || !@cur_info.has_key?(@cur_bestmove)) && @hasinfo && !@is_book_search && !@stopped
        #            @bestmove=m
        if @logger
          @logger.puts "cur_bestmove #{@cur_bestmove} is found"
          @logger.puts "cur_search_moves=#{@cur_search_moves}"
        end
        @cur_search_moves.push(m)
        @extend+=1
        if @logger 
          @logger.puts "cur_search_moves=#{@cur_search_moves}"
          @logger.puts "byoyomi_time= #{@byoyomi_time}"
          @logger.puts "byoyomi_extension= #{@byoyomi_extension}"
          @logger.puts "elapased=#{elapsed}"
        end
        @byoyomi_time -= (elapsed.to_f*1000.0).to_i
        @logger.puts "byoyomi_time= #{@byoyomi_time}, byoyomi_extension=#{@byoyomi_extension}" if @logger
        @cur_ignore_moves=[]
        position_go
      else
        @finish_queue.push([self,:exact,@search_moves,@ignore_moves,@bestmove,bestvalue,pv,@forced_move,@info])
        @searching=false
        @finish_queue.push([self,:stopped])
      end
    }
  end
  def reader
    while(line = gets)
      break if !line
      line.chomp!
      case line
      when /^usiok/
      when /^id name (.*)$/
      when /id author (.*)$/
      when /^readyok/
        @readyok.push(0)
      when /^bestmove (.*)$/
        handle_bestmove($1)
      when /^checkmate (\S*)/
        @searching=false
        @checkmate=$1
        @logger.puts("received checkmate #{@checkmate}") if @logger
        @logger.puts("moves=#{@search_moves}") if @logger
        @logger.puts("bestmove=#{@bestmove}") if @logger
       case @checkmate
        when 'nomate'
          type=:nomate
        when 'timeout'
          type=:timeout
        else
          type=:checkmate
        end
        @finish_queue.push([self,:checkmate,@search_moves,type,@checkmate])
      when /^info depth (\d*) seldepth \d* time \d* score cp ([\-]*\d*) nodes \S* nps [\d\.]* pv (.*)$/
        update_info($1,$3,$2.to_i)
      when /^info depth (\d*) seldepth \d* time \d* score cp mate ([\-]*\d*) nodes \d* nps [\d\.]* pv (.*)$/
        ply=$2.to_i
        score = (ply >= 0 ? Infty-ply : -Infty+ply)
        update_info($1,$3,$score)
      when /^info string forced move at the root:/
        @hasinfo=true
        if @info_depth>=7
          @logger.puts "forced move !!!!" if @logger
          @cur_forced_move=true
          @forced_move=true if @extend==0
        end
      when /^info depth\s*(\d*)/
        @info_depth=[@info_depth,$1.to_i].max
        @cur_forced_move=false
        @forced_move=false if @extend==0
        @hasinfo=true
      when /^info /
        @hasinfo=true
      when /^answer searchtime (\d*) (\d*)$/
        @finish_queue.push([self,:searchtime,$1.to_i,$2.to_i])
      when /^genmove$/
        @finish_queue.push([self,:genmove,[]])
      when /^genmove\s+(.*)$/
        @finish_queue.push([self,:genmove,$1.chomp.split(/\s/)])
      when /^genmove_probability$/
        @finish_queue.push([self,:genmove_probability,[]])
      when /^genmove_probability\s+(.*)$/
        @finish_queue.push([self,:genmove_probability,$1.chomp.split(/\s/)])
      else
        @logger.puts "USI: "+line if @logger
      end
    end
  end
  def keep_alive
    while !@quit
      @pipe_mutex.synchronize{ @pipe.puts ''  }
      sleep 10
    end
  end
  def puts(s)
    @logger.puts ">>> "+s if @logger
    @pipe_mutex.synchronize{ @pipe.puts s  }
  end
  def gets
    s=@pipe.gets
    if s==nil
      if !@quit
        raise "closed pipe"
      end
    else
      @logger.print "<<< "+s if @logger
    end
    s
  end
  # 
  def reset
    @cur_bestmove=nil
    @cur_info=Hash.new
    if @extend==0
      @info=@cur_info
    end
    @info_depth=0
    @go_time=Time.now.to_f
    @checkmate=nil
  end
  def update_info(d,m,s)
    @info_depth=[@info_depth,d.to_i].max
    @hasinfo=true
    ms=m.chomp.split(/\s/)
    if (ms.size>0)
      k=ms[0]
      @cur_info[k]=[d.to_i,s,@cur_search_moves+ms]
      @logger.puts "update_info(#{k.to_s},#{d.to_s},#{s.to_s})\n" if @logger
    end
  end
  def bestvalue
    if @cur_bestmove == 'resign'
      v=-Infty
    elsif @cur_bestmove == 'win'
      v=Infty
    elsif !@hasinfo # opening book move
      v=0
    elsif @cur_info.has_key?(@cur_bestmove)
      v=@cur_info[@cur_bestmove][1]
    else
      v=Infty
    end
    if (@extend % 2) ==0
      v
    else
      -v
    end
  end
# pv をrootからのusi形式のmoveの配列で返す．
  def pv
    if @cur_info.has_key?(@cur_bestmove)
      return @cur_info[@cur_bestmove][2]
    else
      return []
    end
  end
  def setMulti(w)
    puts "setoption name MultiPVWidth value #{w}"
  end
  def position(board=nil,moves=[])
    if(board == nil)
      boardstr='startpos'
    else
      boardstr='sfen '+ShogiServer::Usi.new.board2usi(board,board.teban)+' 1'
    end
    if moves.size==0
      puts "position #{boardstr} moves"
    else
      puts "position #{boardstr} moves #{moves.join(' ')}"
    end
  end
  def gomate(x)
    @checkmate=nil
    @bestmove=nil
    @go_time=Time.now.to_f
    x=x.to_i
    puts "go mate #{x}"
  end
  def gobyoyomi(x,e)
    @go_time=Time.now.to_f
    puts("go byoyomi #{x} byoyomi_extension #{x+e}")
  end
  def position_go
    reset
    position(@search_board,@cur_search_moves)
    setMulti(@multi_pv_width)
    if @cur_ignore_moves.size>0
      puts("ignore_moves #{@cur_ignore_moves.join(' ')}")
    end
    gobyoyomi(@byoyomi_time.to_i,@byoyomi_extension.to_i)
  end

  public
  def set_logger(logger)
    @logger=logger
    @logger.sync=true if @logger
  end
  def waitok
    @readyok.pop
  end
  def searchtime(board,moves,limit,byoyomi,elapsed,finish_queue)
    @finish_queue=finish_queue
    position(board,moves)
    puts("query searchtime #{limit} #{byoyomi} #{elapsed}")
  end
  def searchbook(board,moves,book_moves,finish_queue)
    @extend=0
    @go_time=Time.now.to_f
    @is_book_search=true
    @finish_queue=finish_queue
    @info=Hash.new
    @forced_move=false
    @extend=0
    @hasinfo=false
    position(board,moves)
    puts "go book #{book_moves}"
  end
  def genmoves(board,moves,finish_queue)
    @finish_queue=finish_queue
    position(board,moves)
    puts "genmove"
  end
  def genmove_probability(board,moves,finish_queue)
    @finish_queue=finish_queue
    position(board,moves)
    puts "genmove_probability"
  end
  def stop()
    @mutex.synchronize {
      @stopped=true
      puts('stop')
    }
  end
  def quit()
    @quit=true    # tell the keep_alive thread to die
    puts('quit')
    Thread.new {
      sleep 2
      if @logger
        @logger.close
        @logger=nil
      end
      @pipe.close()
      @pipe=nil
    }
  end
  def search(board,x,e,moves,finish_queue,multi_pv_width,ignore_moves,istop=false)
    raise "new search while searching" if @searching
    @searching=true
    @istop=istop
    @is_book_search=false
    @stopped=false
    @extend=0
    @bestmove=nil
    @forced_move=false
    @cur_forced_move=false
    @search_board=board
    @search_moves=moves.dup
    @cur_search_moves=@search_moves.dup
    @byoyomi_time=x.to_f
    @byoyomi_extension=e.to_f
    @finish_queue=finish_queue
    @hasinfo=false
    @multi_pv_width=multi_pv_width
    @ignore_moves=ignore_moves
    @cur_ignore_moves=ignore_moves
    position_go
  end
  def checkmate_search(board,x,moves,finish_queue)
    raise "new checkmate_search while searching" if @searching
    @searching=true
    @stopped=false
    @bestmove=nil
    @search_board=board
    @search_moves=moves.dup
    @cur_search_moves=moves.dup
    @byoyomi_time=x.to_f
    @finish_queue=finish_queue
    @logger.puts('start position') if @logger
    position(@search_board,@cur_search_moves)
    @logger.puts('start gomate') if @logger
    gomate(@byoyomi_time)
    @logger.puts('end checkmate_search') if @logger
  end
  def clear_pipe
    @pipe.readpartial(4096) if IO.select([@pipe],[],[],0.001)
  end
  attr_reader :rank, :speed
end

