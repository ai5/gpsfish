# 棋泉形式の .kif ファイルの読み込みをおこなう
class KisenFile
  def KisenFile.kisen2pos(kpos)
    return (((kpos-1)%9)+1).to_s+(((kpos-1)/9)+1).to_s
  end
  def KisenFile.pos2kisen(pos)
      return (pos[1].to_i-1)*9+(pos[0].to_i-1)+1
  end
  def KisenFile.kisen2move(board,c0,c1)
    if 1<=c1 && c1<=0x51
      from=kisen2pos(c1)
      piece=board.array[from[0..0].to_i][from[1..1].to_i]
      if 0x65<=c0 && c0<=0xb5
        c0-=0x64
        name=piece.promoted_name
      elsif piece.promoted
        name=piece.promoted_name
      else
        name=piece.name
      end
      to=kisen2pos(c0)
      return (board.teban ? '+' : '-')+from+to+name
    else
      to=kisen2pos(c0)
      hands= (board.teban ? board.sente_hands : board.gote_hands)
      ptypes=["HI","KA","KI","GI","KE","KY","FU"]
      ptypes.each do |pt|
        count=hands.find_all{|p| p.name==pt}.size
        if count > 0
          if c1>0x64
            c1-=count
            if c1<=0x64
              name=pt
            end
          end
        end
      end
      return (board.teban ? '+' : '-')+'00'+to+name
    end
  end
  def initialize(f)
    @fp=File.new(f,"rb")
  end
  def read(board,id)
    @fp.seek(id*512,IO::SEEK_SET)
    moves=Array.new
    curboard=board.deep_copy
    cbuf=@fp.read(512)
    (0...512).each do |j|
      c0=cbuf[j*2].ord
      c1=cbuf[j*2+1].ord
      break if c0==0 || c1==0
      m=KisenFile.kisen2move(curboard,c0,c1)
      curboard.handle_one_move(m,curboard.teban)
      moves.push(m)
    end
    return moves
  end
  def close
    @fp.close
  end
end
