#!/usr/bin/env ruby1.9.1
# -*- coding: utf-8 -*-
require 'piece'
require 'board'
require 'usi'
require 'usiWrapper'
require 'usiMove'
require 'usiMulti'
require 'usiArray'
require 'ponder'
require 'csaRecord'
# require 'compat'
require 'kisen'
require 'timeControl'
require 'config'

require 'thread'
require 'optparse'
require 'fileutils'

Thread.abort_on_exception = true
STDOUT.sync = true
STDERR.sync = true
$logBase="usilogs/"+Time.now.strftime("%Y%m%d-%H-%M-%S")
FileUtils.mkpath($logBase)
log=File.open("#{$logBase}/gpsshogi_par.log","w")
log.sync=true
log.puts ARGV.to_s
no_extend=false

opts=OptionParser.new
opts.on("-f initial_filename"){|v| Params[:initial_filename]=v}
opts.on("-b black"){|v| Params[:black]=v}
opts.on("-w white"){|v| Params[:white]=v}
opts.on("-s"){|v| Params[:issente]=true}
opts.on("-l limit"){|v| } # ignore
opts.on("-x"){|v| Params[:xmode]=true} # ignore
opts.on("-v"){|v| Params[:verbose]=true} # ignore
opts.on("-c"){|v| } # ignore
opts.on("-P"){|v| Params[:ponder]=true} # ignore
opts.on("-t time"){|v| } # ignore
opts.on("-L limit"){|v| } # ignore
opts.on("-n limit"){|v| } # ignore
opts.on("-T time"){|v| Params[:time_limit]=v.to_i} 
opts.on("-B byoyomi"){|v| Params[:byoyomi]=v.to_i}
opts.on("-o record"){|v| Params[:record_filename]=v}
opts.on("-k id"){|v| Params[:kisen_id]=v}
opts.on("-K file"){|v| Params[:kisen_file]=v}
opts.on("--csa-file file"){|v| Params[:csa_file]=v}
opts.on("--no-extend"){|v| no_extend=true}
opts.on("-f file"){|v| Params[:file]=v}
opts.on("-m m"){|v| Params[:book_moves]=v.to_i}
opts.parse!(ARGV)

usiArray=UsiArray.new(UsiPrograms)
$stderr.puts "ready"
# make initial board
moves=[]
n_moves=0
time_elapsed=0
if Params[:kisen_id] #demonstration
  board=ShogiServer::Board.new
  board.initial
  kisen_file=KisenFile.new(Params[:kisen_file])
  moves=kisen_file.read(board,Params[:kisen_id].to_i)
  moves.each do |m| log.puts m end
elsif Params[:file]
  record=CsaRecord.new(Params[:file])
  board=record.board
  moves=record.moves
  moves.each do |m|
    board.handle_one_move(m,board.teban)
  end
  if Params[:issente]
    time_elapsed=record.elapsed[:black]
  else
    time_elapsed=record.elapsed[:white]
  end
  n_moves=moves.size
  moves=[]
else
  board=ShogiServer::Board.new
  board.initial
end
pv_file=File.open("/tmp/pv-"+Time.now.strftime("%Y%m%d-%H-%M-%S")+".txt","w",0644)
pv_file.sync=true
if Params[:record_filename]
  record_file=File.open(Params[:record_filename],"w",0600)
  record_file.sync=true
else
  record_file=nil
end
book_moves=Params[:book_moves]
moves=moves[0...book_moves]
origboard=board.deep_copy
csaMoves=[]
# opening book moves
time_limit=Params[:time_limit]
byoyomi=Params[:byoyomi]
myturn = ( Params[:issente] ? 0 : 1 )
log.puts "myturn=#{myturn}"
if record_file
  record_file.puts("N+#{Params[:black]}")
  record_file.puts("N-#{Params[:white]}")
  record_file.puts(board.to_s)
  record_file.puts("'#{Time.now}")
  record_file.puts("'game start")
end
for i in 0...moves.size
  n_moves+=1
  if i%2==myturn # myturn
    record_file.puts moves[i] if record_file
    log.puts "i=#{i},myturn=#{myturn},move=#{moves[i]}"
    log.puts "myturn"
    STDOUT.puts moves[i]
    board.handle_one_move(moves[i],board.teban)
    csaMoves.push(moves[i])
    time_elapsed+=1
    log.puts(board.to_s)
  else
    m=STDIN.gets.chomp
    record_file.puts m if record_file
    log.puts "m=#{m}"
    log.puts "histurn"
    board.handle_one_move(m,board.teban)
    csaMoves.push(moves[i])
    log.puts(board.to_s)
    break if m!=moves[i]
  end
end
log.puts "break opening book"
log.puts "n_moves=#{n_moves}"
log.puts board.to_s
@ponder_searcher=nil
@in_book=true
time_start=Time.now.to_f
expected_move=nil
while true
  if n_moves%2==myturn
    usiMoves=UsiMove.moves_from_CSA(origboard,csaMoves)
    # book moveがあるかどうかをチェックする
    finish_queue=Queue.new # 探索終了を知らせるキュー
    if @in_book
      usiArray[0].searchbook(origboard,usiMoves,book_moves,finish_queue)
      m=UsiWrapper.wait_stopped(finish_queue)
      bestmove=m[4]
      if bestmove != 'pass'
        m=UsiMove.to_CSA(board,bestmove)
        v=0
        pv=[]
      else
        @in_book=false
      end
    end
    if !@in_book
      if @ponder_searcher
        searcher=@ponder_searcher
        finish_queue=@ponder_searcher.finish_queue
      else
        time_all=second_for_this_move(usiArray[0],origboard,usiMoves,time_limit,byoyomi,time_elapsed)
        time_all[1]=time_all[0] if no_extend
        $stderr.puts "second_for_this_move(n_moves=#{n_moves},time_limit=#{time_limit},time_elapsed=#{time_elapsed},byoyomi=#{byoyomi},time_all=#{time_all}" if Params[:verbose]
        searcher=UsiMulti.new(usiArray,origboard,time_all[0],time_all[1]-time_all[0],usiMoves,[],finish_queue,0,$stderr)
        searcher.root_search
      end
      ret=UsiWrapper.wait_stopped(finish_queue)
      log.puts "usi.bestmove=#{ret[4]}"
      m=UsiMove.to_CSA(board,ret[4])
      v=ret[5]
      if !board.teban
        v=-v
      end
      pv=UsiMove.moves_to_CSA(origboard,ret[6])
      if pv.size > n_moves
        pv=pv[(n_moves+1)..-1]
      elsif pv.size > 1
        pv=pv[1..-1]
      end
    end
    log.puts "m=#{m},v=#{v},pv=#{pv}"
    v=v/10
    if Params[:xmode]
      STDOUT.puts "#{m},\'* #{v} #{pv.join(' ')}"
    else
      STDOUT.puts m
    end
    pv_file.puts "#{m},\'* #{v} #{pv.join(' ')}"
    if pv.size>0 && pv[0]!='%TORYO' && pv[0]!='%KACHI'
      expected_move=ret[6][n_moves+1]
    else
      expected_move=nil
    end
    #      STDOUT.puts m
    time_onemove=[1,(Time.now.to_f-time_start).to_i].max
    $stderr.puts("#{m},T#{time_onemove}")
    time_elapsed+=time_onemove
  else
    log.puts "histurn"
    m=$stdin.gets.chomp
    time_start=Time.now.to_f
    log.puts "m=#{m}"
  end
  if m=='%TORYO' || m=='%KACHI' || m=='#TIME_UP' 
    if @podner_searcher && n_moves%2 != myturn
      @ponder_searcher.stop
      finish_queue=@ponder_searcher.finish_queue
      ret=UsiWrapper.wait_stopped(finish_queue)
    end
    break
  end
  record_file.puts m if record_file
  board.handle_one_move(m,board.teban)
  csaMoves.push(m)
  usiMoves=UsiMove.moves_from_CSA(origboard,csaMoves)
  if !@in_book && Params[:ponder]
    if (n_moves%2)==myturn
      if !@ponder_searcher
        @ponder_searcher=PonderPlayer.new(usiArray)
      end
      time_all=second_for_this_move(usiArray[0],origboard,usiMoves,time_limit,byoyomi,time_elapsed)
      time_all[1]=time_all[0] if no_extend
      $stderr.puts "second_for_this_move(n_moves=#{n_moves},time_limit=#{time_limit},time_elapsed=#{time_elapsed},byoyomi=#{byoyomi},time_all=#{time_all}" if Params[:verbose]
      finish_queue=Queue.new # 探索終了を知らせるキュー
      @ponder_searcher.ponder(origboard,time_all[0],time_all[1]-time_all[0],usiMoves,finish_queue,expected_move)
    else
      @ponder_searcher.inform(m) if @ponder_searcher
    end
  end
  log.puts board.to_s
  n_moves+=1
end
record_file.close if record_file
pv_file.close
usiArray.quit
