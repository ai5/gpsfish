#!/usr/bin/env ruby1.9.1
# -*- coding: utf-8 -*-
## $Id$
# 
# multi level
require 'piece'
require 'board'
require 'usi'
require 'usiWrapper'
require 'usiMove'
require 'usiArray'
require 'usiMulti'
require 'csaRecord'
require 'config'
# require 'compat'

require 'optparse'
require 'thread'
require 'fileutils'

Thread.abort_on_exception = true
STDOUT.sync = true
STDERR.sync = true
  
time_all=4000 # 1問あたりの探索時間(msec)
#time_extension=20000
time_extension=0
time_first=1000 # 1手目の候補を求めるための探索時間
#  multi_pv_width=1500 # 1手目の候補を求める際の pv width
multi_pv_width=500 # 1手目の候補を求める際の pv width
forward_move=0
opts=OptionParser.new
opts.on("-f forward_move"){|v| forward_move=v.to_i}
opts.on("-v"){|v| Params[:verbose]=true} # ignore
opts.on("-S sec"){|v| time_all=v.to_i*1000}
opts.parse!(ARGV)

$logBase="usilogs/"+Time.now.strftime("%Y%m%d-%H-%M-%S")
FileUtils.mkpath($logBase)
@usiArray=UsiArray.new(UsiPrograms)
sum=0 # 問題数
correct=0 # 正解数
ARGV.each do |f|
  p "File: "+f
  record=CsaRecord.new(f)
  board=record.board
  moves=record.moves
  (0...forward_move).each do |i|
    board.handle_one_move(moves[i],board.teban)
  end
  finish_queue=Queue.new # 探索終了を知らせるキュー
  searcher=UsiMulti.new(@usiArray,board,time_all,time_extension,[],[],finish_queue,0,$stderr)
  start_time=Time.now
  searcher.root_search
  v=UsiWrapper.wait_stopped(finish_queue) # rubyプロセスも停止する．
  $stderr.puts "#{v[1..-1]}"
  end_time=Time.now
  bestmove_csa=UsiMove.to_CSA(board,v[4])
  score=v[5]
  m= (moves.size==0 ? '' : moves[forward_move])
  sum+=1
  if bestmove_csa == m
    correct+=1
  end
    $stderr.puts correct.to_s+"/"+sum.to_s+" "+m.to_s+" "+bestmove_csa.to_s+" "+score.to_s+" "+(end_time.to_f-start_time.to_f).to_s+" sec"
#    puts searcher.csa_pv.join(" ")
end
@usiArray.quit
