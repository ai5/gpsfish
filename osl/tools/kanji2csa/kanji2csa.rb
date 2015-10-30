#!/usr/bin/env ruby
require "jcode"
require 'nkf'

$KCODE = "e"

#-- Tables
PtypeConvert = {
  "��" => nil,
  "��" => "OU",
  "��" => "HI",
  "��" => "KA",
  "��" => "KI",
  "��" => "GI",
  "��" => "KE",
  "��" => "KY",
  "��" => "FU",
  "ζ" => "RY",
  "��" => "UM",
  "��" => "NG",
  "��" => "NK",
  "��" => "NY",
  "��" => "TO"
}

NumberConvert = {
  "��" => 2,
  "��" => 3,
  "��" => 4,
  "��" => 5,
  "ϻ" => 6,
  "��" => 7,
  "Ȭ" => 8,
  "��" => 9
#������¿���Τ�̵��
}

BLACK = 0
WHITE = 1

PlayerString = [
 "+", "-"
]


#-- Convert functions
def convert_line line
  ret = ""
  while line
    piece = line
    ret << piece.to_s
    line = nil
  end
  return ret
end

def convert_player line
  if /��/.match(line) || / /.match(line)
    return BLACK
  elsif /��/.match(line) || /v/.match(line)
    return WHITE
  else
    STDERR.print "which player? :#{line}\n"
    return
  end
end

def convert_line line
  ret = ""
  while piece = /([ |v])(.)/.match(line)
    line.sub!(/([ |v])(.)/, "")
    pplayer = PlayerString[convert_player(piece[1])]
    ptype = PtypeConvert[piece[2]]
    if ptype == nil
      ret << " * "
    else
      ret << pplayer << ptype
    end
  end
  return ret
end

def convert_mochigoma line
  ret = []
  last_ptype = nil
  while piece = /(.)/.match(line)
    line.sub!(/(.)/, "")
    if ptype = PtypeConvert[piece[1]]
      ret.push ptype
      last_ptype = ptype
    elsif pcount = NumberConvert[piece[1]]
      if last_ptype == nil
	STDERR.print "Piece not set and count of it found\n"
	STDERR.print [piece[1]]
	STDERR.print pcount
	STDERR.print "\n"
	return
      end
      while pcount > 1
	ret.push last_ptype
	pcount -= 1
      end
    end
  end
  return ret
end


#-- Main
#-  Values
bancount = 0 #banmen's line number
banmen = []
mochigoma = [[],[]]
comments = []
turn = -1

#- Input
while !STDIN.eof?
  line = NKF.nkf("-e", STDIN.readline)

  if banline = /\|(.+)\|/.match(line)
    banmen[bancount] = convert_line(banline[1])
    bancount+=1
  elsif mochiline = /(.+)����(.+)/.match(line)
    player = mochiline[1]
    komaline = mochiline[2]
    mochigoma[convert_player(player)] = convert_mochigoma(komaline)
  elsif turnline = /(.+)��/.match(line)
    turn = convert_player(turnline[1])
  else
    comments.push line
  end
end

comments.reverse!

#- Output
while !comments.empty?
  print "'#{comments.pop}"
end
  print "\n"

for i in 0...9
  print "P#{i+1}#{banmen[i]}\n"
end

for i in 0..1
  if !mochigoma[i].empty?
    print "P#{PlayerString[i]}"
    while !mochigoma[i].empty?
      piece = mochigoma[i].pop
      print "00#{piece}"
    end
    print "\n"
  end
end

print "#{PlayerString[turn]}\n"
