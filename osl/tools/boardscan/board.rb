#! /usr/bin/env ruby

require 'pgm.rb'
require 'util.rb'

def is_black(pnm, x, y)
  if 0 <= x && x < pnm.width && 0 <= y && y < pnm.height then
    return pnm.get(x, y) < 128
  else
    false
  end
end

def point_is_black(pnm, point)
 return is_black(pnm, point.x, point.y)
end

def move(pnm, point, first, second)
  return unless point_is_black(pnm, point)
  while true
    if point_is_black(pnm, first.call(point)) then
      point = first.call(point)
    elsif point_is_black(pnm, second.call(point)) then
      point = second.call(point)
    else
      break
    end
  end
  return point
end

def southwest(pnm, x, y)
  return unless is_black(pnm, x, y)
  while true
    if is_black(pnm, x - 1, y + 1) then
      x -= 1

      y += 1
    elsif is_black(pnm, x, y + 1) then
      y += 1
    else
      break
    end
  end
  return Point.new(x, y)
end

def southwest_west(pnm, x, y)
  return unless is_black(pnm, x, y)
  while true
    if is_black(pnm, x - 1, y + 1) then
      x -= 1
      y += 1
    elsif is_black(pnm, x - 1, y) then
      x -= 1
    else
      break
    end
  end
  return Point.new(x, y)
end

def east(pnm, x, y)
  return unless is_black(pnm, x, y)
  while true
    if is_black(pnm, x + 1, y) then
      x += 1
    elsif is_black(pnm, x + 1, y - 1) then
      x += 1
      y -= 1
    else
      break
    end
  end
  return Point.new(x, y)
end

def east_south(pnm, x, y)
  return unless is_black(pnm, x, y)
  while true
    if is_black(pnm, x + 1, y) then
      x += 1
    elsif is_black(pnm, x + 1, y + 1) then
      x += 1
      y += 1
    else
      break
    end
  end
  return Point.new(x, y)
end

def southeast(pnm, x, y)
  return unless is_black(pnm, x, y)
  while true
    if is_black(pnm, x + 1, y + 1) then
      x += 1
      y += 1
    elsif is_black(pnm, x, y + 1) then
      y += 1
    else
      break
    end
  end
  return Point.new(x, y)
end

def detect_kin(pnm)
  #右上 から 左下 の斜線を探す
  line1 = 
    line_exist?(pnm, pnm.width * 2 / 5, pnm.width * 3 / 5,
		5, pnm.height / 4,
		lambda {|start_point, point|
		  point.x < pnm.width / 4 && point.y > pnm.height * 2 / 5},
		lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					 lambda{|p| Point.new(p.x - 1, p.y + 1)},
					 lambda{|p| Point.new(p.x - 1, p.y)})})

  return false unless line1

  line2 = 
    line_exist?(pnm, pnm.width * 2 / 5, pnm.width * 3 / 5,
		5, pnm.height / 4,
		lambda {|start_point, point|
		  point.x > pnm.width * 3 / 4 && point.y > pnm.height * 7 / 20},
		lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					 lambda{|p| Point.new(p.x + 1, p.y + 1)},
					 lambda{|p| Point.new(p.x + 1, p.y)})})

  return false unless line2

  kyo_line =
    line_exist?(pnm, line1.start_point.x, line1.start_point.x + 1, 
		line1.start_point.y, line1.start_point.y + 1,
		lambda {|start_point, point|
		  point.x > pnm.width * 5 / 8},
		lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					 lambda{|p| Point.new(p.x + 1, p.y)},
					 lambda{|p| Point.new(p.x + 1, p.y + 1)})})

  if kyo_line then
    return false
  else
    return true
  end
end

def detect_kyo(pnm)
  puts "DETECTING KY" if DEBUG_DETECT
  #右上 から 左下 の斜線を探す
  line1 =  line_exist?(pnm, pnm.width * 2 / 5, pnm.width * 3 / 5,
		       5, pnm.height / 2,
		       lambda {|start_point, point|
#			 point.x < pnm.width * 3 / 11 && point.y > pnm.height * 5 / 12
			 start_point.x - point.x > pnm.width / 4 && point.y - start_point.y > pnm.height / 4
},
		       lambda {|pnm, x, y| move(pnm, Point.new(x,y),
						lambda{|p| Point.new(p.x - 1, p.y + 1)},
						lambda{|p| Point.new(p.x, p.y + 1)})})
  return false unless line1
  puts "line1 found" if DEBUG_DETECT

  line2 =  line_exist?(pnm, line1.start_point.x, line1.start_point.x + 5,
		       line1.start_point.y, line1.start_point.y + 10,
		       lambda {|start_point, point|
			 point.x > pnm.width * 5 / 8 },
		       lambda {|pnm, x, y| move(pnm, Point.new(x,y),
						lambda{|p| Point.new(p.x + 1, p.y)},
						lambda{|p| Point.new(p.x + 1, p.y + 1)})})

  return false unless line2
  puts "line2 found" if DEBUG_DETECT

  line3 = line_exist?(pnm, 5, pnm.width / 4,
		      5, pnm.height / 3,
		      lambda {|start_point, point|
			point.x > pnm.width * 5 / 8 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y - 1)})})

  if line3 then
    puts "line3 found" if DEBUG_DETECT
    return true
  end
  return false
end

def detect_gyoku(pnm)
  lines = Array.new
  # 左 1/4 の縦領域からの横棒三つ
  5.upto(pnm.height - 5 - 1) {|j|
    5.upto(pnm.width / 4) {|i|
      if (is_black(pnm, i, j)) then
	point = east(pnm, i, j)
	if point.x > pnm.width * 3 / 4 then
	  if lines.empty? || point.y - lines.last.y > pnm.height / 6 then
	    lines.push(point)
	  end
	end
      end
    }
  }
  if lines.length == 3 && lines[0].y < pnm.height / 3 \
    && lines.last.y > pnm.height * 3 / 4 \
    && ((lines[2].y - lines[1].y) - (lines[1].y - lines[0].y)).abs < 10 \
    && (lines[2].y - lines[1].y) > (lines[1].y - lines[0].y) then
    return true
  end

  return false
end

def line_exist?(pnm, start_x, end_x, start_y, end_y, predicate, move)
  start_y.upto(end_y - 1) {|j|
    start_x.upto(end_x - 1) {|i|
      if (is_black(pnm, i, j)) then
	point = move.call(pnm, i, j)
	if predicate.call(Point.new(i, j), point) then
#	  print i, " ", j, " "
#	  puts point
	  return Line.new(Point.new(i, j), point)
	end
      end
    }
  }
  return false
end

def detect_fu(pnm)
  puts "DETECTING FU" if DEBUG_DETECT
  # 真中の縦棒と右下から左下へのはらい
  line1 =  line_exist?(pnm, pnm.width * 2 / 5, pnm.width * 3 / 5,
		 5, pnm.height / 4,
		 lambda {|start_point, point| point.y > pnm.height * 5 / 8 },
		 lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					  lambda{|p| Point.new(p.x, p.y + 1)},
					  lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1
  puts "line1 found" if DEBUG_DETECT

  line2 = line_exist?(pnm, pnm.width * 5 / 8, pnm.width - 5,
		 pnm.height / 2, pnm.height - 5,
		lambda {|start_point, point| point.x < pnm.width * 7 / 20 && point.y > pnm.height * 3 / 4 },
		lambda {|pnm, i, j| return southwest_west(pnm, i, j) })
  return false unless line2
  puts "line2 found" if DEBUG_DETECT

  # 右上の横棒
  line3 = line_exist?(pnm, pnm.width / 2, pnm.width - 5,
		      5, pnm.height / 3,
		      lambda {|start_point, point|
			point.x > pnm.width / 3 && start_point.x - point.x > pnm.width / 6},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x - 1, p.y)},
					       lambda{|p| Point.new(p.x - 1, p.y + 1)})})
  if line3 then
    puts "line3 found" if DEBUG_DETECT
    return true
  end

  return false
end

def detect_kei(pnm)
  # 縦棒二本
  line1 = line_exist?(pnm, 5, pnm.width / 3,
		      5, pnm.height / 4,
		      lambda {|start_point, point| point.y > pnm.height * 3 / 4 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1
  line2 = line_exist?(pnm, pnm.width / 2, pnm.width - 5,
		      5, pnm.height / 4,
		      lambda {|start_point, point| point.y > pnm.height * 3 / 4 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  # 向き
  if !line2 then
    line2 = line_exist?(pnm, pnm.width / 2, pnm.width - 5,
		      5, pnm.height / 4,
			lambda {|start_point, point|
			  point.y - start_point.y > pnm.height / 4 && \
			  line_exist?(pnm, point.x, point.x + 1,
				      point.y, point.y + 5,
				      lambda {|start_point, point| point.y > pnm.height * 3 / 4 },
				      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
							       lambda{|p| Point.new(p.x, p.y + 1)},
							       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
			},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  end
  if line2 && line1.end_point.x < pnm.width - line2.end_point.x then
    return true
  end
  return false
end

def detect_kaku(pnm)
  line1 = line_exist?(pnm, 5, pnm.width / 3,
		      pnm.height / 4, pnm.height / 2,
		      lambda {|start_point, point|
			point.x < start_point.x && point.y > pnm.height * 3 / 4 \
			&& (point.y - start_point.y) / (start_point.x - point.x) < 10},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x - 1, p.y + 1)})})
  return false unless line1

  line2 = line_exist?(pnm, line1.start_point.x, line1.start_point.x + 3,
		      line1.start_point.y, line1.start_point.y + 10,
		      lambda {|start_point, point|
			point.x > pnm.width * 3 / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y - 1)})})
  return false unless line2

  line3 = line_exist?(pnm, line2.end_point.x - 5, line2.end_point.x,
		      line2.end_point.y, line2.end_point.y + 5,
		      lambda {|start_point, point|
			point.y > pnm.height * 3 / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})

  if line3 then
    return true
  end
  return false
end

def detect_hisha(pnm)
  puts "DETECTING HI" if DEBUG_DETECT
  # 左側のはらい
  line1 = line_exist?(pnm, 5, pnm.width / 2,
		      pnm.height / 4, pnm.height / 2,
		      lambda {|start_point, point|
			point.x < start_point.x && point.y > pnm.height * 3 / 4 \
			&& (point.y - start_point.y) / (start_point.x - point.x) < 25},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x - 1, p.y + 1)},
	    				       lambda{|p| Point.new(p.x, p.y + 1)})})
  # 優先する傾きを変えてもう一回
  if !line1 then
    line1 = line_exist?(pnm, 5, pnm.width / 2,
			pnm.height / 4, pnm.height / 2,
			lambda {|start_point, point|
			  point.x < start_point.x && point.y > pnm.height * 3 / 4 \
			  && (point.y - start_point.y) / (start_point.x - point.x) < 25},
			lambda {|pnm, x, y| move(pnm, Point.new(x,y),
						 lambda{|p| Point.new(p.x, p.y + 1)},
						 lambda{|p| Point.new(p.x - 1, p.y + 1)})})
  end

  return false unless line1
  puts "line1 found" if DEBUG_DETECT

  #真中の縦棒
  line2 = line_exist?(pnm, pnm.width * 2 / 5, pnm.width * 3 / 5,
		      5, pnm.height / 3,
		      lambda {|start_point, point|
			point.y > pnm.width * 3 / 4 && point != line1.end_point},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  puts "line2 found" if DEBUG_DETECT
  return false unless line2

  if line_exist?(pnm, 5, pnm.width / 4,
		      5, line2.start_point.y + 5,
		      lambda {|start_point, point| point.x > pnm.width / 2},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y - 1)})}) then
    puts "line3 found" if DEBUG_DETECT
    return true
  end

  return false
end

def detect_gin(pnm)
  line1 = line_exist?(pnm, pnm.width / 3, pnm.width * 2 / 3,
		      5, pnm.height / 4,
		      lambda {|start_point, point| point.y > pnm.width * 3 / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1

  line2 = line_exist?(pnm, line1.start_point.x, line1.start_point.x + 1,
		      line1.start_point.y, line1.start_point.y + 5,
		      lambda {|start_point, point| point.x > pnm.width * 3 / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y - 1)})})

  return false unless line2

  line3 = line_exist?(pnm, line2.end_point.x - 5, line2.end_point.x,
		      line2.end_point.y, line2.end_point.y + 5,
		      lambda {|start_point, point| point.y - start_point.y > pnm.height / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})

  if line3 then
    return true
  end

  return false
end

def detect_to(pnm)
  line1 = line_exist?(pnm, pnm.width * 2 / 3, pnm.width - 5,
		      pnm.height / 3, pnm.height / 2,
		      lambda {|start_point, point| point.x < pnm.width / 3 && point.y > pnm.height * 2 / 3
 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x - 1, p.y + 1)},
					       lambda{|p| Point.new(p.x - 1, p.y)})})
  return false unless line1
  line2 = line_exist?(pnm, line1.end_point.x, line1.end_point.x + 5,
		      line1.end_point.y, line1.end_point.y + 5,
		      lambda {|start_point, point|
			line_exist?(pnm, point.x, point.x + 1,
				    point.y, point.y + 1,
				    lambda {|start_point, point|
				      point.x > pnm.width * 2 / 3 && point.y > pnm.height * 3 / 4},
				    lambda {|pnm, x, y| move(pnm, Point.new(x,y),
							     lambda{|p| Point.new(p.x + 1, p.y)},
							     lambda{|p| Point.new(p.x + 1, p.y - 1)})})},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y)})})
  if line2 then
    return true
  end
  return false
end

def detect_nari(pnm)
  line1 = line_exist?(pnm, 5, pnm.width / 2,
		      5, pnm.height / 2,
		      lambda {|start_point, point| point.x < pnm.width / 2 \
			&& point.x - start_point.x > pnm.height / 4
 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1

  line2 = line_exist?(pnm, line1.start_point.x, line1.end_point.x,
		      5, line1.start_point.y,
		      lambda {|start_point, point| point.y < pnm.height / 2 \
			&& point.y - start_point.y > pnm.height / 4
 },
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x - 1, p.y + 1)})})
  return line2
end

def detect_narikyo(pnm)
  return detect_nari(pnm) && detect_kyo(pnm)
end

def detect_ryu(pnm)
  line1 = line_exist?(pnm, 5, pnm.width / 3,
		      pnm.height / 3, pnm.height * 2 / 3,
		      lambda {|start_point, point| point.y - start_point.y > pnm.height / 4},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1

  line2 = line_exist?(pnm, line1.start_point.x, line1.start_point.x + 5,
		      line1.start_point.y, line1.start_point.y + 5,
		      lambda {|start_point, point| point.x > pnm.width * 2 / 3},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y - 1)})})
  return false unless line2

  line3 = line_exist?(pnm, line2.end_point.x - 5, line2.end_point.x,
		      line2.end_point.y, line2.end_point.y + 5,
		      lambda {|start_point, point| point.y - start_point.y > pnm.height / 5},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line3

  line4 = line_exist?(pnm, line2.start_point.x, line2.end_point.x,
		      line2.start_point.y, line2.start_point.y + 5,
		      lambda {|start_point, point| point.y > line1.end_point.y},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  if line4 then
    return true
  end
  return false
end

def detect_uma(pnm)
  line1 = line_exist?(pnm, 5, pnm.width / 3,
		      5, pnm.height / 4,
		      lambda {|start_point, point|
			point.y > pnm.height / 2 && point.y < pnm.height * 5 / 6},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x, p.y + 1)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})
  return false unless line1
  line2 = line_exist?(pnm, line1.start_point.x, line1.start_point.x + 5,
		      line1.start_point.y, line1.start_point.y + 5,
		      lambda {|start_point, point|
			point.x > pnm.width * 2 / 3},
		      lambda {|pnm, x, y| move(pnm, Point.new(x,y),
					       lambda{|p| Point.new(p.x + 1, p.y)},
					       lambda{|p| Point.new(p.x + 1, p.y + 1)})})

  if line2 then
    return true
  end

  return false
end

def detect(pnm)
  if detect_gyoku(pnm) then
    return "+OU"
  elsif detect_gyoku(pnm.rotate()) then
    return "-OU"
  elsif detect_kei(pnm) then
    return "+KE"
  elsif detect_kei(pnm.rotate()) then
    return "-KE"
  elsif detect_hisha(pnm) then
    return "+HI"
  elsif detect_hisha(pnm.rotate()) then
    return "-HI"
  elsif detect_kaku(pnm) then
    return "+KA"
  elsif detect_kaku(pnm.rotate()) then
    return "-KA"
  elsif detect_kin(pnm) then
    return "+KI"
  elsif detect_kin(pnm.rotate()) then
    return "-KI"
  elsif detect_gin(pnm) then
    return "+GI"
  elsif detect_gin(pnm.rotate()) then
    return "-GI"
  elsif detect_kyo(pnm) then
    return "+KY"
  elsif detect_kyo(pnm.rotate()) then
    return "-KY"
  elsif detect_fu(pnm) then
    return "+FU"
  elsif detect_fu(pnm.rotate()) then
    return "-FU"
  elsif detect_to(pnm) then
    return "+TO"
  elsif detect_to(pnm.rotate()) then
    return "-TO"
  elsif detect_narikyo(pnm) then
    return "+NY"
  elsif detect_narikyo(pnm.rotate()) then
    return "-NY"
  elsif detect_ryu(pnm) then
    return "+RY"
  elsif detect_ryu(pnm.rotate()) then
    return "-RY"
  elsif detect_uma(pnm) then
    return "+UM"
  elsif detect_uma(pnm.rotate()) then
   return "-UM"
  else
    return " * "
  end
end

if (ARGV[0][-4, 4] == ".pnm") then
  DEBUG_DETECT = true
  puts detect(Pgm.create(ARGV[0]))
else 
  DEBUG_DETECT = false
  0.upto(8) {|y|
    print "P", y + 1
    0.upto(8) {|x|
      filename = ARGV[0] + "/test" + "%d%d" % [y, x] + ".pnm"
      print detect(Pgm.create(filename))
    }
    puts
  }
  puts "+"
end
