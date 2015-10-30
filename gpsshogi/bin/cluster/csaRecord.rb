class CsaRecord
  def initialize(name)
    position=''
    movestr=''
    @elapsed={:black => 0, :white => 0}
    turn=nil
    File.open(name,'r:EUC-JP:UTF-8') do |f|
      begin 
        f.each_line do |line|
          case line
          when /^P/
            position+=line
          when /^T(\d*)$/
            if turn=='+'
              @elapsed[:black]+=$1.to_i
            else
              @elapsed[:white]+=$1.to_i
            end
          when /^[\+|\-]$/
            position+=line
          when /^([\+|\-])\d{4}\w{2}$/
            movestr+=line
            turn=$1
          end
        end
      ensure
        f.close
      end
    end
    @board=ShogiServer::Board.new
    @board.set_from_str(position);
    @moves=ShogiServer::Board.split_moves(movestr.gsub(/\n/,''))
  end
  attr_reader :board, :moves, :elapsed
end
