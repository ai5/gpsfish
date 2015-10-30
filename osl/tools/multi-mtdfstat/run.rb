#! /usr/bin/env ruby

require 'thread'
require './machine-conf.rb'

files = Dir[ARGV[0] + "/*.csa"]
mutex = Mutex.new
threads = []
MACHINES.each {|machine|
  threads << Thread.new(machine) {|x|
    while true
      file = nil
      mutex.synchronize do
	file = files.shift
      end
      break unless file
      system("rsh #{x} '#{COMMAND} #{ARG} #{file}'")
    end
  }
}

threads.each {|x|
  x.join
}
