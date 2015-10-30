# -*- coding: utf-8 -*-
class UsiArray
  # n個のプロセッサをウェイトfa(総和はn)に分ける．
  def UsiArray.dist(fa,n)
    a=Array.new(fa.size)
    r=Array.new(fa.size)
    for i in 0...fa.size
      a[i]=fa[i].floor.to_i
      n-=a[i]
      r[i]=[fa[i]-a[i].to_f,i]
    end
    raise "assert" unless n >= 0
    r.sort! {|a,b| b[0] <=> a[0]}
    for i in 0...n
      a[r[i][1]]+=1
    end
    return a
  end
  def initialize(usiPrograms)
    usiPrograms=usiPrograms.collect {|s| (s.class==Array ? s : [s,1])}
    usiPrograms.sort!{|a,b| b[1]<=>a[1]}
    @n_workers=usiPrograms.length
  # usiのstubを作成する
    @usis=Array.new(@n_workers)
    (0...@n_workers).each do |i|
      usiProgram=[usiPrograms[i][0]+" 2>#{$logBase}/stderr_#{i}.txt",usiPrograms[i][1]]
      @usis[i]=UsiWrapper.new(usiProgram,i,File.open("#{$logBase}/#{i}.txt","w",0644))
#      $stderr.puts "#{i} #{usiPrograms[i]}"
#      sleep 1 if (i%4)==3
    end
    (0...@n_workers).each do |i|
      @usis[i].waitok
    end
  end
  def size
    @usis.size
  end
  def [](n)
    @usis[n]
  end
  def quit
    (0...@n_workers).each do |i|
      @usis[i].stop
    end
    (0...@n_workers).each do |i|
      @usis[i].quit
    end
    sleep 2
  end
  def collect(&block)
    @usis.collect(&block)
  end
end
