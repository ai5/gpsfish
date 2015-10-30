#!/usr/local/bin/ruby

NormalPtypeList = ["HI", "KA", "KI", "GI", "KE", "KY" ,"FU"]
PtypeList = ["FU", "KY", "KE", "GI", "KI", "KA", "HI", "OU", "RY", "UM", "NG", "NK", "NY", "TO"]
PlayerList = ["+", "-"]

class PtypeO
  def initialize player, ptype
    unless PlayerList.index player
      raise Exception.new("\"#{player}\" not a player") 
    end
    unless PtypeList.index ptype
      raise Exception.new("\"#{ptype}\" not a ptype") 
    end
    @player = player
    @ptype = ptype
  end

  def owner
    return @player
  end

  def ptype
    return @ptype
  end

  def to_s
    return "#{@player}#{@ptype}"
  end
end

class ShogiState
  def initialize
    @board = Array.new
    (1..9).each {|x|
      @board[x] = Array.new
      (1..9).each {|y|
        @board[x][y] = nil
      }
    }

    @mochigoma = Hash.new
    PlayerList.each { |player|
      @mochigoma[player] = Hash.new
      NormalPtypeList.each {|ptype|
	@mochigoma[player][ptype] = 0
      }
    }

  end

  def addPiece x, y, ptypeO
    @board[x][y]=ptypeO
  end


  def getPiece x, y
    return @board[x][y]
  end

  def addMochigoma player, ptype
    @mochigoma[player][ptype] += 1
  end

  def getMochigoma player, ptype
    return @mochigoma[player][ptype]
  end

end

module PrintMyshogi
  SenteGote = {
    "+" => "\\sente",
    "-" => "\\gote"
  }
    
  PtypeConvert = {
    "FU" => "\\fu",
    "KY" => "\\kyou",
    "KE" => "\\kei",
    "GI" => "\\gin",
    "KI" => "\\kin",
    "KA" => "\\kaku",
    "HI" => "\\hi",
    "OU" => "\\ou",
    "RY" => "\\ryu",
    "UM" => "\\uma",
    "NG" => "\\narigin",
    "NK" => "\\narikei",
    "NY" => "\\narikyou",
    "TO" => "\\tokin",
  }

  def to_s
    ret = " \\begin{myshogi}\n  \\banmen\n"

    PlayerList.each { |player|
      ret << "  \\mochigoma{#{SenteGote[player]}}"
      NormalPtypeList.each { |ptype|
	ret << "{#{getMochigoma player, ptype}}"
      }
      ret << "\n"
    }
    (1..9).each {|y|
      (1..9).each {|x|
        ptypeO = getPiece x, y
        if ptypeO
          player = SenteGote[ptypeO.owner]
          ptype = PtypeConvert[ptypeO.ptype]
          ret << " \\koma{#{x}#{y}}{#{player}}{#{ptype}}\n"
        end
      }
    }

    ret << " \\end{myshogi}\n"
    return ret
  end
end

module ParseCsa
  private
  def readline
    return nil if @input.eof?
    begin
      line = @input.readline
    end while line =~ /^\'/
    line.chomp!
    return line
  end

  def parseBoard
    (1..9).each {|y|
      line = readline
      if !line.sub! /P#{y}/, ""
        throw Exception.new("While parsing line:#{y} of board got invalid input: \"#{line}\"") 
      end

      (1..9).each {|invx|
	x = 10 - invx
	line.sub! /(.)(..?)/, ""
	if $1 == " " && $2 == "* "
	  addPiece x, y, nil
	elsif $1 == " " && $2 == "*"
	  STDERR.print "line ending with \" *\" found, for #{y}, #{x}\n"
	  addPiece x, y, nil
	else
	  begin
	    ptypeO = PtypeO.new($1, $2)
	    addPiece x, y, ptypeO
	  rescue
	    	    raise
	  end
	end
      }
    }
  end

  def parseMochigomaAndTeban
    while true
      line = readline

      break if line =~ /^[\-|\+]$/
      
      if !line.sub! /^P([\+|\-])/, ""
        throw Exception.new("#got invalid input \"#{line}\"") 
      end

      player = $1

      while line.sub! /(\d\d)(..)/, ""
	if $1 != "00"
          throw Exception.new("#got invalid input \"#{line}\"") 
        end
	addMochigoma player, $2
      end
    end 
    @teban = line
  end

  def parseMoves
    @moves = Array.new
    begin
      line = readline
      return if line == nil
      @moves.push line
    end while !line.sub! /^P([\+|\-])/, ""
  end

  public
  def parse input
    @input = input
    parseBoard
    parseMochigomaAndTeban
    parseMoves
  end
end

class ReadCsaOutputMyshogi < ShogiState
  include ParseCsa
  include PrintMyshogi

  def initialize
    super
  end

end

shogiState = ReadCsaOutputMyshogi.new
shogiState.parse STDIN
print shogiState
