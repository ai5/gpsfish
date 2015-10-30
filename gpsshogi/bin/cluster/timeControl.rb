  def second_for_this_move(usi,board,moves,limit,byoyomi,elapsed)
    finish_queue=Queue.new
    usi.searchtime(board,moves,limit,byoyomi,elapsed,finish_queue)
    tp=finish_queue.pop()
#    return [(tp[0] > 10 ? 10+(tp[0]-10)*0.5 : tp[0]).to_i,
#            (tp[1] > 10 ? 10+(tp[1]-10)*0.5 : tp[1]).to_i]
    return [tp[2].to_i,tp[3].to_i]
  end

