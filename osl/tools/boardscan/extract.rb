#! /usr/bin/env ruby

require 'pgm.rb'

1.upto(100) {|i|
  filename = "default-" + "%03d" % i + ".pnm"
  puts filename
  pnm = Pgm.create(filename)
  File.open(i.to_s + "s.pnm", "w+") {|f|
    f.puts(pnm.sub(100, 500, 1200, 1200))
  }
}
