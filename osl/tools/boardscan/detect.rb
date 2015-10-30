#! /usr/bin/env ruby

require 'pgm.rb'
require 'util.rb'

if ARGV.length != 1
  puts "Usage: ./detect.rb FILE"
  exit 1
end

pnm = Pgm.create(ARGV[0])

# look for about 400 pixel wide horizontal line

def is_black(pnm, x, y)
  if 0 <= x && x < pnm.width && 0 <= y && y < pnm.height then
    return pnm.get(x, y) < 128
  else
    false
  end
end

lines = Array.new()
10.upto(pnm.width - 1) {|i|
  in_line = false
  start = 0

  while true
    while (! is_black(pnm, i, start) && start < pnm.height)
      start += 1
    end

    if start == pnm.height
      break
    end

    x = i
    y = start
    start_point = Point.new(x, y)
    while true
      if is_black(pnm, x, y + 1) then
	nil
      elsif is_black(pnm, x + 1, y + 1) then
	x = x + 1
      elsif is_black(pnm, x - 1, y + 1) then
	x = x - 1
      else
	y += 1
	break
      end
      y += 1
    end
    end_point = Point.new(x, y)

    if end_point.y - start_point.y > 400 \
      && start_point.y != 0 && end_point.y != pnm.height then
      lines.push(Line.new(start_point, end_point))
    end

    start = y
  end
}

if lines.length > 10 then
  i = 1
  while true
    a = lines[i-1]
    b = lines[i]
    if a.end_point == b.end_point then
      lines.delete_at(if a.start_point.y < b.start_point.y then i else i-1 end)
    elsif a.end_point.y + 20 >= pnm.height then
      lines.delete_at(i-1)
    else
      i += 1
    end
    break if i >= lines.length
  end
end

if lines.length != 10 then
  width = (lines.last.start_point.x - lines[0].start_point.x) / 9
  i = 1
  while true
    a = lines[i-1]
    b = lines[i]
    if b.start_point.x - a.start_point.x < width / 2 then
      lines.delete_at(i)
    elsif b.start_point.x - a.start_point.x > width * 3 / 2 then
      lines[i, 0] = Line.new(Point.new(a.start_point.x + width,
				       (a.start_point.y + b.start_point.y / 2)),
			     Point.new(a.end_point.x + width,
				       (a.end_point.y + b.end_point.y / 2)))
      i += 1
    else
      i += 1
    end
    break if i >= lines.length
  end
end

0.upto(lines.length - 2) {|i|
  a = lines[i]
  b = lines[i+1]
  if a.start_point.y >= b.start_point.y + 100 then
    a.end_point.y = b.end_point.y
  elsif a.start_point.y + 100 <= b.start_point.y then
    b.start_point.y = a.start_point.y
  end

  if a.end_point.y >= b.end_point.y + 100 then
    a.end_point.y = b.end_point.y
  elsif a.end_point.y + 100 <= b.end_point.y then
    b.end_point.y = a.end_point.y
  end
}

width = (lines.last.start_point.x - lines[0].start_point.x) * 9 / 10
new_y = lines[0].start_point.y
while true
  x = lines[0].start_point.x + 5
  y = new_y
  while true
    if is_black(pnm, x + 1, y) then
      x += 1
    elsif is_black(pnm, x + 1, y - 1) then
      x += 1
      y -= 1
    elsif is_black(pnm, x + 1, y + 1) then
      x += 1
      y += 1
    else
      break
    end
  end

  if x - lines[0].start_point.x > width then
    new_y += 1
  else
    break
  end
end

if new_y != lines[0].start_point.y then
  diff = new_y - lines[0].start_point.y
  0.upto(lines.length - 1) {|i|
    lines[i] = Line.new(Point.new(lines[i].start_point.x,
				  lines[i].start_point.y + diff),
			Point.new(lines[i].end_point.x,
				  lines[i].end_point.y + diff / 2))
  }
end

puts lines

0.upto(8) {|i|
  0.upto(8) {|j|
    filename = "test" + "%d" % i + "%d" % j + ".pnm"
    File.open(filename, "w+") {|f|
      width = lines[j+1].start_point.x - lines[j].start_point.x
      height = (lines[j].end_point.y - lines[j].start_point.y) / 9
      x_compensation = (lines[j].end_point.x.to_f - lines[j].start_point.x) / (lines[j].end_point.y - lines[j].start_point.y) * height * i
      f.print(pnm.sub((lines[j].start_point.x + x_compensation).to_i,
		      (lines[j].start_point.y + height * i).to_i,
		      width, height))
    }
  }
}
