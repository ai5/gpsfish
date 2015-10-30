#!/usr/bin/ruby

#
# Move Komaochi (hadicapped game) records
#

require 'find'
require 'nkf'
require 'fileutils'

dir = ARGV.shift
Dir.glob(File.join(dir, "*.ki2")) do |path|
  puts "Processing... #{path}"
  sjis = open(path, "r") {|f| f.read}
  euc = NKF.nkf("-Se", sjis)

  if /^¼ê¹ç³ä¡§/ =~ euc
    to_path = File.join(File.dirname(path), "KOMAOCHI", File.basename(path))
    puts "mv #{path} #{to_path}"
    FileUtils.mv(path, to_path) 
  elsif /È¿Â§¾¡¤Á/ =~ euc
    to_path = File.join(File.dirname(path), "ILLEGAL", File.basename(path))
    puts "mv #{path} #{to_path}"
    FileUtils.mv(path, to_path) 
  end
end
