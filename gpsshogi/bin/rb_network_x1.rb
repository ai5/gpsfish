#!/usr/bin/ruby
$:.unshift File.dirname(__FILE__)
require 'network/rb_csa_client.rb'
require 'network/rb_gpsshogi.rb'

STDOUT.sync = true
STDERR.sync = true

#--- Global Variables
$test               = ARGV.shift                 || false
$host               = ENV['SERVER']              || "wdoor.c.u-tokyo.ac.jp"
$port               = ENV['PORT']                || '4081'
$user               = ENV['SHOGIUSER']           || ($test ? 'teamgps'   : 'testgps')
$pass               = ENV['SHOGIPASS']           || ($test ? 'os4QRTvls' : 'hogetaro')
$limit              = (ENV['LIMIT']              || 1600).to_i
$node_limit         = (ENV['NODELIMIT']          || 16000000).to_i
$table_size         = (ENV['TABLE_SIZE']         || 4800000).to_i # for 8GB
$table_record_limit = (ENV['TABLE_RECORD_LIMIT'] || 200).to_i	  # for 8GB
$gps_opts           = ENV['GPSOPTS']             || ""
$base_command       = ENV['BASE_COMMAND']        || "./gpsshogi -v -c"
$logfile_basename   = "logs/x1_"
$csafile_basename   = "csa/x1_"
[$logfile_basename, $csafile_basename].each do |file|
  unless File.exist? File.dirname(file)
    Dir.mkdir File.dirname(file)
  end
end

#--- Subroutines
def set_config(sente, initial_filename, opname,
	       timeleft, byoyomi)
  raise "Illegal sente strng: %s" % [sente] unless sente == 0 || sente == 1
  black = (sente == 1 ? "gps" : opname)
  white = (sente == 0 ? "gps" : opname)

  if (opname =~ /human/) 
      $stderr.puts "disable prediction search for human opponent"
      $gps_opts.gsub!(/-P /, " ") # TODO
      $gps_opts.gsub!(/P/, "")    # TODO
  end
  config = { :initial_filename => initial_filename,
             :opponent => opname,
             :sente => (sente == 1 ? true : false),
             :black => black, 
             :white => white,
             :limit => $limit, 
             :table_size => $table_size,
             :table_record_limit => $table_record_limit,
             :node_limit => $node_limit,
             :timeleft => timeleft, 
             :byoyomi => byoyomi,
             :logfile_basename => $logfile_basename,
             :other_options => $gps_opts,
             :base_command => $base_command
           }
  return config
end

#--- Main routine
def main
  client = CsaClient.new($user, $pass)
  client.connect($host, $port)
  client.login_x1()

  gamename = ENV['GAMENAME'] || "testgps"
  sente_string = case
                   when ENV['ISSENTE'] then "+"
                   when ENV['ISGOTE']  then "-"
                   else "*"
                 end
  loop_limit = (ENV['LOOP'] || 1).to_i

  while (loop_limit < 0 || loop_limit > 0)
    client.offer_game_x1(gamename, sente_string)
    sente, initial_filename, opname, timeleft, byoyomi = 
      client.wait_opponent($csafile_basename)
    client.chat_com("yoroshiku-onegai-simasu > #{opname}-san")
    case sente
      when -1
        client.logout()
        client.disconnect()
        raise "ERR: error in reading csa initial strings"
      when -2
        warn "WARN: rejected"
        next
    end

    time_string = GpsShogi::make_time_string()
    logfilename = "#{$logfile_basename}#{time_string}-#{opname}-comm.log"
    File.open(logfilename, "w") do |loghandle|
      raise "ERR: open #{logfilename}" unless loghandle
      loghandle.puts "log start - #{logfilename}"

      config = set_config(sente, initial_filename, opname,
                          timeleft, byoyomi)
      program = GpsShogi.new(config)
      program.set_logger(loghandle)
      sleep 1 # wait for the program starting up. 
      client.play(program) # play

      $stderr.puts "end game"
    end
    sleep 10 # cpu cool down
  
    loop_limit -= 1 unless loop_limit < 0
  end

  client.logout()
  client.disconnect()
end

if __FILE__ == $0
  main
end
