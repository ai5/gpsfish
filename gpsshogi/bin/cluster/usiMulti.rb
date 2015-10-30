# -*- coding: utf-8 -*-
require 'pp'
class UsiMulti
  @@logmutex=Mutex.new
  def set_time_all
    @time_all=[((@@root_end-Time.now.to_f)*1000).to_i,1000].max
  end
  def initialize(usiArray,board,time_all,time_extension,moves,ignore_moves,finish_queue,ply,logger=nil)
    raise "finish_queue is nil" if !finish_queue
#    $stderr.puts "UsiMulti.new(ply=#{ply})"
    @@rank_base=-1 if ply==0
    @rank=@@rank_base
    @@rank_base-=1
    @usiArray=usiArray
    @board=board
    @time_all=time_all.to_f
    @time_extension=time_extension.to_f
    @moves=moves
    @ignore_moves=ignore_moves
    @finish_queue=finish_queue
    @ply=ply
    @stopped=false
    @searchers=[]
    @mutex=Mutex.new
    set_logger(logger)
  end
  private
  def set_logger(logger)
    @logger=logger
    @logger.sync=true if @logger
  end
  def logging(s)
    if @logger
      @@logmutex.synchronize{
        msec=((Time.now.to_f-@@root_start)*1000).to_i
        @logger.print("#{msec} #{rank} "+s+"\n")
      }
    end	 
  end
  # stopをかけられたり，結論が出ていたらfalseを返す 
  # そうでなかったら，presearchのqueueの返事を返す
  def presearch
    local_queue=Queue.new
    time_start=Time.now.to_f
    @mutex.synchronize {
      if @stopped
        @finish_queue.push([self,:stopped])
        return false
      end
      @searchers=[@usiArray[0],@usiArray[1]]
      @usiArray[0].search(@board,TimeFirst,0,@moves,local_queue,MultiPVWidth,[],false)
      @usiArray[1].checkmate_search(@board,TimeFirst,@moves,local_queue)
    }
#    raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
    mate_done=false
    ret=nil
    stopped=false
    while !mate_done || !stopped
      m=local_queue.pop()
      logging "child=#{m[0].rank} #{m[1..-1]}" if Params[:verbose]
      case m[1]
      when :checkmate
        mate_done=true
        time_start=Time.now.to_f
        @mutex.synchronize{ @searchers.delete(@usiArray[1]) }
#        raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
        next if m[3]!=:checkmate
        bestmove=m[4]
        pv=@moves+[bestmove]
        bestvalue=Infty
        @finish_queue.push([self,:exact,@moves,[],bestmove,bestvalue,pv])
        if !stopped
          @usiArray[0].stop
          UsiWrapper.wait_stopped(local_queue)
        end
        @finish_queue.push([self,:stopped])
        return false
      when :exact
        ret=m
      when :stopped
        time_start=Time.now.to_f
        @mutex.synchronize{ @searchers.delete(@usiArray[0]) }
#        raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
        stopped=true
      else
        raise "presearch: unexpected message from in queue child=#{m[0].rank} #{m[1..-1]}"
      end
    end
    raise "searchers must be deleted" if @searchers.size>0
    time_start=Time.now.to_f
    @mutex.synchronize {
      if @stopped
        @finish_queue.push([self,:stopped])
        return false
      end
    }
#    raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
    raise "presearch: no exact message before stopped" if !ret
    return ret
  end
  def single_search
    logging "single_search: moves=#{@moves}, rank=#{@usiArray[0].rank}"  if Params[:verbose]
    time_start=Time.now.to_f
    @mutex.synchronize {
      if @stopped
        @finish_queue.push([self,:stopped])
        return
      end
      @searchers=[@usiArray[0]]
      set_time_all
      @usiArray[0].search(@board,@time_all,@time_extension,@moves,@finish_queue,0,[],@ply==0)
    }
#    raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
    return
  end
  def is_final(maxval,bounds)
    return false if !maxval
    bounds.each do |v|
      return false if !v || v>maxval
    end
    return true
  end
  def split_search(ks)
    logging "moves=#{@moves}, ks=#{ks}"  if Params[:verbose]
    if ks.size==0
      single_search
      return
    end
    weights=Array.new(ks.size)
    w=1.0
    for i in 0...weights.size
      #            v=w/3.0
      if @ply==0
        v=w/4.0
      else
        v=w/2.0
      end
      weights[i]=v
      w-=v
    end
    for i in 0...weights.size
      weights[i]*=(@usiArray.size-1).to_f/(1-w)
    end
    i_weights=UsiArray.dist(weights,@usiArray.size-1)
    my_ignore_moves=[]
    local_queue=Queue.new
    arrays=Array.new(i_weights.size)
    changed_flag=true
    index=1
    while changed_flag
      changed_flag=false
      for i in 0...i_weights.size
        if i_weights[i] >=1
#          logging "i_weights[#{i}]=#{i_weights[i]}, index=#{index}"
          arrays[i]=Array.new if !arrays[i]
          arrays[i].push(@usiArray[index])
          index+=1
          i_weights[i]-=1
          changed_flag=true
        end
      end
    end
#    logging "arrays=#{arrays}"
    local_searchers=[]
    for i in 0...i_weights.size
      if arrays[i]
        set_time_all
        s=UsiMulti.new(arrays[i],@board,@time_all,@time_extension,@moves+[ks[i]],[],local_queue,@ply+1,@logger)
        local_searchers.push(s)
        my_ignore_moves.push(ks[i])
      end
    end
    time_start=Time.now.to_f
    @mutex.synchronize {
      @searchers=local_searchers
      @searchers.each{|s| s.search}
      @searchers.push(@usiArray[0])
      set_time_all
      @usiArray[0].search(@board,@time_all,@time_extension,@moves,local_queue,0,my_ignore_moves,false)
    }
#    raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
    stop_count=0
    searcher_count=@searchers.size
    bounds=Array.new(searcher_count,nil)
    maxval=nil
    while stop_count<searcher_count
      logging "stop_count=#{stop_count},search_count=#{searcher_count}"  if Params[:verbose]
      m=local_queue.pop()
      logging "child=#{m[0].rank} #{m[1..-1]}"  if Params[:verbose]
      if m[1]==:stopped
        stop_count+=1
        next
      end
      child_moves=m[2]
      child_ignore=m[3]
      child_pv=m[6]
      if child_ignore==[]
        v=-m[5]
        move=child_moves[-1]
        i=my_ignore_moves.index(move)
        raise "move #{move} is not in my_ignore_moves #{my_ignore_moves}" if i==nil
      else
        v=m[5]
        move=m[4]
        i=searcher_count-1
      end
      bounds[i]=v
      maxval_updated=false
      if m[1]==:exact && (!maxval || maxval<v)
        maxval_updated=true
        maxval=v
        bestmove=move
        bestpv=child_pv
      end
      if is_final(maxval,bounds)
        @finish_queue.push([self,:exact,@moves,[],bestmove,maxval,bestpv,false,[]])
        stop
      elsif maxval_updated
        @finish_queue.push([self,:update,@moves,[],bestmove,maxval,bestpv,false,[]])
      end
    end
    @finish_queue.push([self,:stopped])
    logging "end of root_search: bestmove=#{bestmove}, value=#{maxval}\n----------------------------" if @ply==0
  end
  def multi_search
    logging "multi_search: ranks=#{ranks}"  if Params[:verbose]
    return if !(preret=presearch)
    end_time=Time.now
    set_time_all
    @time_all-=((end_time.to_f-@search_start)*1000).to_i
    @time_all=[1000,@time_all].max
    logging "multi_search: @time_all=#{@time_all}"  if Params[:verbose]
    forced_move=preret[7]
    root_bestmove=preret[4]
    if forced_move
      logging "forced move bestmove=#{root_bestmove}, ply==#{@ply}"  if Params[:verbose]
      if @ply==0
        @finish_queue.push([self,:exact,@moves,[],root_bestmove,0,preret[6],true,[]])
        @finish_queue.push([self,:stopped])
        logging "end of root_search: bestmove=#{root_bestmove}, value=#{preret[6]}\n----------------------------"
      else
        local_queue=Queue.new
        set_time_all
        s=UsiMulti.new(@usiArray,@board,@time_all,@time_extension,@moves+[root_bestmove],[],local_queue,@ply+1,@logger)
        @mutex.synchronize {
          @searchers=[s]
          s.search
        }
        m=UsiWrapper.wait_stopped(local_queue)
        @mutex.synchronize {
          if @stopped
            @finish_queue.push([self,:stopped])
            return
          end
        }
        @finish_queue.push([self,:exact,@moves,@ignore_moves,root_bestmove,-m[5],m[6],true,Hash.new])
        @finish_queue.push([self,:stopped])
      end
      return
    end
    root_info=preret[8]
    ks= root_info.keys
    ks.delete('resign')
    ks.sort! {|a,b| root_info[a][0]!=root_info[b][0] ? root_info[b][0] <=> root_info[a][0] : root_info[b][1] <=> root_info[a][1]}
    split_search(ks)
  end
  def timeless_multi_search
    logging "timeless_multi_search: ranks=#{ranks}"  if Params[:verbose]
    local_queue=Queue.new
    @usiArray[0].genmove_probability(@board,@moves,local_queue)
    m=local_queue.pop
    logging "#{m[1..-1]}"  if Params[:verbose]
    ks=(0...m[2].size/2).collect{|i| m[2][i*2]}
    split_search(ks)
  end
  protected
  def search()
#    case UsiMove.last_move_type(@board,@moves)
#    when :uchifuzume
#      @finish_queue.push([self,:exact,@moves,@ignore_moves,'pass',Infty,@moves+['pass'],false,Hash.new])
#      @finish_queue.push([self,:stopped])
#      return
#    when :sennichite
#      @finish_queue.push([self,:exact,@moves,@ignore_moves,'pass',0,@moves+['pass'],false,Hash.new])
#      @finish_queue.push([self,:stopped])
#      return
#    when :oute_sennichite_sente_lose
#      teban_is_sente=((((@board.teban ? 0 : 1)+@moves.size) % 2)==0) # searchの手番が黒の時は偶数
#      v=(teban_is_sente ? -Infty : Infty)
#      @finish_queue.push([self,:exact,@moves,@ignore_moves,'pass',v,@moves+['pass'],false,Hash.new])
#      @finish_queue.push([self,:stopped])
#      return
#    when :oute_sennichite_gote_lose
#      teban_is_sente=((((@board.teban ? 0 : 1)+@moves.size) % 2)==0) # searchの手番が黒の時
#      v=(teban_is_sente ? Infty : -Infty)
#      @finish_queue.push([self,:exact,@moves,@ignore_moves,'pass',v,@moves+['pass'],false,Hash.new])
#      @finish_queue.push([self,:stopped])
#      return
#    when :normal
#    else
## must raise      
#      single_search
#      return
#    end
    @search_start=Time.now.to_f
    set_time_all
    logging "search(x=#{@time_all},e=#{@time_extension},_moves=#{@moves},multi_pv_width=#{MultiPVWidth},ignore_moves=#{@ignore_moves}) ranks=#{ranks}"  if Params[:verbose]
    if @usiArray.size==1
      single_search
    elsif @time_all<=10000
      Thread.new{ timeless_multi_search }
    else
      Thread.new{ multi_search }
    end
  end
  public
  def root_search()
    @@root_start=Time.now.to_f
    @@root_end=Time.now.to_f+[@time_all,35000].min/1000.0
    local_queue=Queue.new
    @usiArray[0].searchbook(@board,@moves,Params[:book_moves],local_queue)
    v=UsiWrapper.wait_stopped(local_queue)
    if v[4]!='pass'
      @finish_queue.push(v[0..4]+[0,[],false,Hash.new])
      @finish_queue.push([self,:stopped])
      logging "end of root_search(book move): bestmove=#{v[4]}, value=0\n----------------------------"
      return
    end
    @usiArray[0].genmoves(@board,@moves,local_queue)
    m=local_queue.pop
    if m[2].size==1
      @finish_queue.push([self,:exact,@moves,@ignore_moves,m[2][0],0,@moves+m[2],true,Hash.new])
      @finish_queue.push([self,:stopped])
      logging "end of root_search(forced move): bestmove=#{m[2][0]}, value=0\n----------------------------"
      return
    end
    search
  end
  def stop()
    time_start=Time.now.to_f
    logging "stopping searchers=#{@searchers.collect{|s| s.rank}}"  if Params[:verbose]
    @mutex.synchronize {
      @stopped=true
      logging 'stop'  if Params[:verbose]
      @searchers.each do |s| 
        logging "call stop for rank #{s.rank}"  if Params[:verbose]
        s.stop 
      end
      @searchers=[]
    }
#    raise "staying #{Time.now.to_f-time_start} sec in critical section" if Time.now.to_f-time_start>1
  end
  def ranks
    @usiArray.collect{|u| u.rank.to_s}.join(',')
  end
  attr_reader :rank
end
