#! /usr/bin/env ruby

left = ARGV[0]
right = ARGV[1]

result1 = nil
result2 = nil
File.open(left) {|f1|
  File.open(right) {|f2|
    result1 = f1.readlines.sort
    result2 = f2.readlines.sort
  }
}
left_only = Array.new
right_only = Array.new
left_nodes = Array.new
right_nodes = Array.new
0.upto(result1.length - 1) {|i|
  r1 = result1[i].split(/\t/)
  r2 = result2[i].split(/\t/)
  nodes1 = r1[3].to_i
  nodes2 = r2[3].to_i
  if (r1[1] == 'OK' && r2[1] != 'OK') then
    left_only.push("#{r1[0]} #{nodes1 - nodes2}")
  elsif (r1[1] != 'OK' && r2[1] == 'OK') then
    right_only.push("#{r1[0]} #{nodes2 - nodes1}")
  elsif (r1[1] == 'OK' && r2[1] == 'OK') then
    if (nodes1 > nodes2) then
      left_nodes.push("#{r1[0]} #{nodes1 - nodes2}")
    elsif (nodes1 < nodes2) then
      right_nodes.push("#{r1[0]} #{nodes2 - nodes1}")
    end
  end
}

puts 'Left:'
left_only.each {|x|
  puts x
}
puts 'Right:'
right_only.each {|x|
  puts x
}

puts "Left More: #{left_nodes.length}"
left_nodes.each {|x|
  puts x
}
puts "Right More: #{right_nodes.length}"
right_nodes.each {|x|
  puts x
}
