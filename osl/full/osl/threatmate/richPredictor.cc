/* richPredictor.cc
 */
#include "osl/threatmate/richPredictor.h"
#include "osl/additionalEffect.h"

double osl::threatmate::RichPredictor::predict(const NumEffectState& state, 
					       const Move move){
  const Player turn = alt(state.turn());
  const Square opKingSquare = state.kingSquare(alt(turn));
  const int x = opKingSquare.x();
  const int y = opKingSquare.y();
  const Square to = move.to();
  const int distance_m = abs(x - to.x()) + abs(y - to.y());
  const int sign = -1 + 2 * (turn == BLACK);
  const int min = -1;
  const int max =  1;

  // Kiki around opKing
  int add_effect = 0;
  int effect_b = 0;
  int effect_w = 0;
  int effect_e = 0;

  for (int i=min; i<=max; i++)
    for (int j=min; j<=max; j++){
      Square pos(x+j, y+i);
      if (pos.isOnBoard()){
	int eff_w   = state.countEffect(alt(turn),pos);
	effect_w   += eff_w;
	add_effect += AdditionalEffect::count(state,turn, pos);
	int eff_b   = state.countEffect(turn,pos);
	effect_b   += eff_b;
	effect_e   += (eff_b > eff_w);
      }
    }

  // King's Escape path
  int escapeKing = 0;
  for (int i=min; i<=max; i++)
    for (int j=min; j<=max; j++){
      Square pos(x+j, y+i);
      if (pos.isOnBoard()){
	Piece pieceOnBoard = state.pieceOnBoard(pos);
	if ((pieceOnBoard == Piece::EMPTY()) || (pieceOnBoard.owner() != alt(turn)))
	  escapeKing += (!state.hasEffectAt(turn, pos));
      }
    }

  // Capture Ptype
  const double coefCapture[16]
    ={0.0, 0.0,  0.0, 0.0, 0.0, 5.06, 4.73, 7.70, 
      0.0, 9.78, 0.0, 0.0, 0.0, 5.06, 4.73, 7.70};

  const double neigh[9]
    ={14.52, 9.13,  8.26,
      0.39, 0.0,   11.87,
      0.53, 11.30, 15.06};

  double neigh8 = 0.0;
  for (int i=min; i<=max; i++)
    for (int j=min; j<=max; j++){
      Square pos(x+sign*j, y+sign*i);
      if (pos.isOnBoard())
	neigh8 += neigh[3*(i+1)+j+1]*state.hasEffectByPiece(state.pieceOnBoard(to), pos);
    }

  const double value_p = 
    9.62*(double)state.countPiecesOnStand(turn, ROOK)
    +  6.07*(double)state.countPiecesOnStand(turn, BISHOP)
    +  8.27*(double)state.countPiecesOnStand(turn, GOLD)
    +  5.64*(double)state.countPiecesOnStand(turn, SILVER)
    +  4.06*(double)state.countPiecesOnStand(turn, KNIGHT)
    +  2.77*(double)state.countPiecesOnStand(turn, LANCE)
    +  1.05*(double)state.countPiecesOnStand(turn, PAWN);

  double est = 
    45.07
    + neigh8
    + 10.20*(double)add_effect
    +  6.41*(double)effect_b
    -  1.24*(double)effect_w
    + 13.79*(double)effect_e
    -  1.98*(double)escapeKing
    -  3.11*(double)distance_m
    + value_p
    + coefCapture[move.capturePtype()];

  return est;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
