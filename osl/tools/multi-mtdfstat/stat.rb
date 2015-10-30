#! /usr/bin/env ruby

ok_count = 0
total_nodes = 0
ok_nodes = 0
while l = gets
  result = l.chomp.split(/\t/)
  nodes = result[3].to_i
  if result[1] == 'OK' then
    ok_count += 1
    ok_nodes += nodes
  end
  total_nodes += nodes
end

puts "OK: #{ok_count} OK nodes: #{ok_nodes}, average: #{ok_nodes.to_f / ok_count} Total nodes: #{total_nodes}"
