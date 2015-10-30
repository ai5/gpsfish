require 'socket'
require 'fileutils'

def gets_safe(socket, timeout=nil)
  r = select([socket], nil, nil, timeout)
  if r
    return r[0].first.gets
  else
    return :timeout
  end
rescue Exception => ex
  $stderr.puts "gets_safe: #{ex.class}: #{ex.message}\n\t#{ex.backtrace[0]}"
  return :exception
end

class CsaClient

  def initialize(name, password) 
    @name, @password = name, password
    @verbose     = false
    @in_game     = false
    @socket      = nil
    @read_error  = 0
    @record_file = ""
  end

  def send(message)
    raise "not connected" unless @socket

    @socket.write(message) # message ends with "\n"
    if message != "\n"
      $stderr.print "SEND:#{message}"
    else
      $stderr.print '.'
    end
  end

  def try_read_in_sec(sec)
    #vec($rin,fileno($socket),1) = 1 # TODO
    loop do
      line = gets_safe(@socket, sec)

      case line
      when :timeout
        return nil
      when :exception
        @read_error += 1
        raise "connection closed?" if @read_error > 10
      else
        @read_error = 0
        if line != "\n"
          $stderr.print "RECV:#{line}"
        else
          $stderr.print ","
        end
        return line unless line == "\n"
      end
      # if line is "\n" then read again
    end
  end

  def try_read
    return try_read_in_sec(0.001)
  end

  def read
    loop do 
      line = try_read_in_sec(531.0)
      return line if line
      send("\n")
    end
  end

  def read_skip_chat
    loop do
      line = ""
      line = read()
      return line unless (line && line =~ /^\#\#\[CHAT\]/)

      line.gsub!(/\#\#\[CHAT\]\[[A-Za-z0-9_@-]+\]\s+/, "")
      if line =~ /^([A-Za-z0-9_@-])+\s+(verbose|silent)/
        command = $2
        if @name =~ /#{$1}/
          if command == "verbose"
            STDERR.puts "verbose"
            @verbose = true
          else
            STDERR.puts "silent"
            @verbose = false
          end
        end
      end
    end
  end

  def read_or_gameend
    line0 = read_skip_chat()
    line = line0.dup
    skipped = []
    if line =~ /^\#/ || line =~ /^\%/
      begin 
        skipped << line
        if line =~ /^\#(WIN|LOSE|DRAW|CHUDAN)/
          case line
          when /^\#WIN/
            chat_com("kachi-mashita")
          when /^\#DRAW/
            chat_com("hikiwake-deshita")
          when /^\#LOSE/
            chat_com("make-mashita")
          end
          in_game = false
          return true, line0, *skipped
        end
        line = read_skip_chat()
      end while line =~ /\#/
      STDERR.puts "CsaClient: unknown path #{line}"
      return true, line, *skipped	# このパスなんだろう
    end
    return false, line
  end

  def connect(host, port)
    $stderr.puts "SYSTEM:connect to #{host}:#{port}"
    raise "No port" unless port

    @socket = TCPSocket.open(host, port) # may raise SocketError
    @socket.sync = true
    $stderr.puts "SYSTEM:connected"

    sleep 1
  end

  def login_x1
    send("LOGIN #{@name} #{@password} x1\n")

    user_without_trip = @name.gsub(/,.*/, "")
    line = ""
    while line !~ /LOGIN:(#{user_without_trip}|incorrect)/ do
      line = read()
    end

    raise "ERR :#{line}" unless line =~ /OK/
    while line !~ /\#\#\[LOGIN\] \+OK x1/ do
      line = read()
    end
    raise "ERR :#{line}" unless line =~ /OK/
    $stderr.puts "SYSTEM:login ok"
  end

  def login
    send("LOGIN #{@name} #{@password}\n")

    line = ""
    while line !~ /LOGIN:(#{@name}|incorrect)/ do
      line = read()
    end

    raise "ERR :#{line}" unless line =~ /OK/
  end

  def logout
    send("LOGOUT\n")

    line = ""
    while line = try_read do
      print line # TODO STDERR?
    end
  end

  def disconnect
    @socket.close
  end

  def chat(message)
    send('%%CHAT #{message}\n')
  end

  def chat_com(message)
    chat("com: #{message}") if @verbose
  end

  def offer_game_x1(gamename, sente_string)
    now = Time::now.strftime('%Y-%m-%d %H:%M')
    $stderr.puts "SYSTEM:offer_game (#{now})"
    game_string = "%%GAME #{gamename} #{sente_string}\n" 
    send(game_string)
  end

  def wait_opponent(csafile_basename)
    STDERR.puts "SYSTEM: wait_opponent"
    sente      = -1
    line       = ""
    timeleft   = 0
    byoyomi    = 0
    sente_name = "unknown"
    gote_name  = "unknown"

    now = Time::now.strftime('%Y%m%d-%H-%M-%S')
    record_base  = "#{csafile_basename}#{now}"
    csafilename  = "#{record_base}-#{$$}.init"
    @record_file = "#{record_base}-comm-#{$$}.csa"

    loop do 
      line = read_skip_chat
      return unless line

      if line =~ /^\#/
        $stderr.print "unexpected #{line}"
        next
      end

      case line
      when /^Name\+:(.+)/
        sente_name = $1
      when /^Name\-:(.+)/
        gote_name = $1
      when /^Your_Turn:(.)/
        sente = 1 if line =~ /\+/
        sente = 0 if line =~ /\-/
        STDERR.puts "SYSTEM: we are #{sente}"
      when /^Total_Time:(.+)/
        #we assume that Time_Unit is 1sec
        timeleft = $1.to_i
      when /^Byoyomi:(.+)/
        byoyomi = $1.to_i
        byoyomi -= byoyomi/10 if byoyomi/10 > 0
      when /^BEGIN Position/
        STDERR.print "SYSTEM:#{line}"
        open(csafilename, "w") do |f|
          f.puts "N+#{sente_name}"
          f.puts "N-#{gote_name}"
          loop do 
            line = read_skip_chat()
            next  if line =~ /^Jishogi_Declaration:/
            break if line =~ /^END Position/
            f.print line #unless line =~ /P[\+|\-]/
          end
        end
        FileUtils::copy(csafilename, @record_file)
        FileUtils::chmod(0644, @record_file)
      when /^END Game_Summary/
        break
      end
    end
    sleep 2
    send("AGREE\n")
    line = read_skip_chat()
    sente = -2 if line =~ /^REJECT/

    @in_game = 1 if sente >= 0
    @sente = sente
    return sente, csafilename, (sente > 0 ? gote_name : sente_name), timeleft, byoyomi
  end

  def record_move(out, move)
    case move.chomp!
    when /^[#%]/
      out.puts "'#{move}"
    when /^(.*),(T\d+)$/
      out.puts $1
      out.puts $2
    else
      out.puts move
    end
  end

  #
  # $program : new GpsShogi したもの(など)
  def play(program)
    unless @in_game
      $stderr.puts "CsaClient: cannot play not in_game status"
      return
    end
    program.set_master_record(@record_file)
    record_handle = nil
    begin
      record_handle = File.open(@record_file, "a") # or die
    rescue
      raise "Failed to open a file: #@record_file [#$!]"
    end
    record_handle.sync = true

    if @sente == 0
      # op turn
      gameend, line, *skip = read_or_gameend
      record_move(record_handle, line)
      skip.each {|s| record_move(record_handle, s)}
      if gameend
        program.send('%TORYO\n')
        return
      end
      line.gsub!(/,T(\d+)$/, "") # cuts \n
      program.send("#{line.chomp}\n")
    end

    loop do
      # my turn
      line = program.read
      if line # %TORYOの時は undefined
        send(line) 
      else
        break
      end

      gameend, line, *skip = read_or_gameend()
      record_move(record_handle, line)
      skip.each {|s| record_move(record_handle, s)}

      break if gameend

      # op turn
      gameend, line, *skip = read_or_gameend()
      record_move(record_handle, line)
      skip.each {|s| record_move(record_handle, s)}
      break if gameend
      line.gsub!(/,T(\d+)$/, "") # cuts \n
      program.send("#{line.chomp}\n")
      if line =~ /^%/
        gameend, line, *skip = read_or_gameend()
        $stderr.print "game does not end #{line}" unless gameend
        break if gameend
      end
    end
    record_handle.close
  end

end
