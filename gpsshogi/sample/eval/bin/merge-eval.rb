#! /usr/bin/env ruby

require 'optparse'

random_fill = false
USAGE = 'Usage: ./merge-prich.rb [--random-fill] info.txt'
OptionParser.new do |opts|
  opts.banner = USAGE
  opts.on("-r", "--random-fill", "Fill new feature with random values") {|x|
    random_fill = x
  }
end.parse!

if ARGV.length != 1 then
  $stderr.puts USAGE
  exit(1)
end

File.open(ARGV[0]) {|f|
  mode = ''
  while l = f.gets
    l.chomp!

    if l =~ /^#\*\s+([a-z0-9]+)$/ then
      mode = $1
    elsif l =~ /^#/ then
      next
    else
      eval_type, num = l.split
      num = num.to_i
      valid_file_found = false
      filename = File.join(mode, "#{eval_type}.txt")
      if File.exists?(filename) then
        File.open(filename) {|eval|
          content = eval.readlines
          if content.length == num then
            puts content
            valid_file_found = true
          end
        }
      end
      if !valid_file_found then
        $stderr.puts "#{filename} not found"
        0.upto(num - 1) {|i|
          if random_fill then
            puts rand(255) - 127
          else
            puts 0
          end
        }
      end
    end
  end
}
