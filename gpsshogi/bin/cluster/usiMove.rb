#
module UsiMove
  PTYPE = {'k' => 'OU','r' =>'HI','b' => 'KA','g' => 'KI',
    's' =>'GI','n' => 'KE','l' => 'KY','p' => 'FU'}
  def ptype(s)
    PTYPE[s.downcase]
  end
  def position(a,b)
    a[0]+(b[0].ord-96).to_s
  end
  def rposition(x,y)
    x.to_s+(96+y).chr
  end
  def to_CSA(board,usiMove)
    begin
      turns=(board.teban ? '+' : '-')
      case usiMove
      when /win/
        '%KACHI'
      when /resign/
        '%TORYO'
      when /^(\w)\*(\d)(\w)$/
        turns+'00'+position($2,$3)+ptype($1)
      when /^(\d)(\w)(\d)(\w)$/
        x=$1.to_i
        y=$2[0].ord-96
        piece=board.array[x][y]
        if piece.promoted
          turns+position($1,$2)+position($3,$4)+piece.promoted_name
        else
          turns+position($1,$2)+position($3,$4)+piece.name
        end
      when /^(\d)(\w)(\d)(\w)\+$/
        x=$1.to_i
        y=$2[0].ord-96
        turns+position($1,$2)+position($3,$4)+board.array[x][y].promoted_name
      end
    rescue => e
      $stderr.puts(e.to_s)
      '%TORYO'
    end
  end
  def from_CSA(board,csaMove)
    case csaMove
    when /%KACHI/
      'win'
    when /%TORYO/
      'resign'
    when /^[+-](\d)(\d)(\d)(\d)(\w\w)$/
      from_x=$1.to_i
      from_y=$2.to_i
      to_x=$3.to_i
      to_y=$4.to_i
      ptype=$5
      if from_x == 0 && from_y==0
        usitype=PTYPE.invert[ptype].upcase
        raise "assert ptype #{ptype} must be in PTYPE" if !usitype
        return usitype+'*'+rposition(to_x,to_y)
      else
        piece=board.array[from_x][from_y]
        raise "assert no piece in board=#{board},(x,y)=(#{from_x},#{from_y})" if !piece
        usimove=rposition(from_x,from_y)+rposition(to_x,to_y)
        if !PTYPE.invert[ptype] && !piece.promoted
          return usimove+'+'
        else
          return usimove
        end
      end
    else
      ''
    end
  end
  def moves_to_CSA(board,usiMoves)
    b=board.deep_copy
    ret=Array.new
    usiMoves.each do |m|
      break if m=='pass'
      csaMove=to_CSA(b,m)
      ret.push(csaMove)
      b.handle_one_move(csaMove,b.teban)
    end
    return ret
  end
  def moves_from_CSA(board,csaMoves)
    b=board.deep_copy
    ret=Array.new
    csaMoves.each do |m|
      usiMove=from_CSA(b,m)
      break if b.handle_one_move(m,b.teban)==:illegal
      ret.push(usiMove)
    end
    return ret
  end
  # 最後の手のtypeを
  # :kachi_win, :kachi_lose, :toryo, :illegal, :oute_kaihimore, :sennichite, 
  # :uchifuzume, :oute_sennichite_sente_lose, :oute_sennichite_gote_lose, 
  # :outori から返す．
  def last_move_type(board,usiMoves)
    return :normal if usiMoves.size==0
    b=board.deep_copy
    usiMoves[0...-1].each do |m|
      raise "last_move_type: pass in usiMoves" if m=='pass'
      csaMove=to_CSA(b,m)
      rt=b.handle_one_move(csaMove,b.teban)
      raise "last_move_type: not normal move type=#{rt}, move=#{csaMove}\n#{b}" if rt!=:normal
    end
    m=usiMoves[-1]
    raise "last_move_type: pass in usiMoves" if m=='pass'
    csaMove=to_CSA(b,m)
    return b.handle_one_move(csaMove,b.teban)
  end
  module_function :ptype, :position, :rposition, :to_CSA, :from_CSA
  module_function :moves_to_CSA, :moves_from_CSA, :last_move_type
end
