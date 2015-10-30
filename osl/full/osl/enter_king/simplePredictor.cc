#include "osl/enter_king/simplePredictor.h"
#include "osl/enter_king/enterKingUtil.h"
#include <math.h>

namespace {
  // 入玉に必要な点数と現在の点数、調べた行の点数と枚数から
  // 入玉に必要な駒の数を返す
  // cur_points にpoints_on_row を足す前に呼ぶ
  int calcNumPiecesNeeded(const int cur_points, const int points_on_row, 
			  const int num_pieces_on_row, const int threshold) {
    if (points_on_row == num_pieces_on_row)
      return threshold - cur_points;
    const int points_needed = threshold - cur_points;
    const int num_major = (points_on_row - num_pieces_on_row) / 4;
    if (points_needed <= 5 * num_major) 
      return (points_needed + 4) / 5;
    else 
      return num_major + (points_needed - 5 * num_major);
  }
}

// Turn が入玉できる確率を返す
template <osl::Player Turn>
double osl::enter_king::SimplePredictor::getProbability(const NumEffectState& state) {
  // 確率計算用のパラメータ
  // 最後にexp() で確率に戻す
  static const double normal_penalty = log(0.9);
  static const double strong_penalty = log(0.8);
  static const double weak_penalty   = log(0.95);
  static const double lower_bound    = log(0.05);
  double prob = 0.0;

  const osl::Square myKingSquare = state.kingSquare<Turn>();
  const int my_king_y = myKingSquare.y();
  const int enemyCampMin = (Turn==osl::BLACK) ? 1 : 7;
  const int enemyCampMax = enemyCampMin + 2;
  const int winning_threshold = (Turn==osl::BLACK) ? 
    winning_threshold_black : winning_threshold_white;

  // 玉のy軸を見て入玉からの距離でペナルティをかける
  if (my_king_y < enemyCampMin || my_king_y > enemyCampMax) {
    prob += ((Turn==osl::BLACK)? (my_king_y - enemyCampMax) :
	     (enemyCampMin - my_king_y)) * weak_penalty;
  }

  // 敵陣にある駒の点数と枚数を計算
  int num_pieces = 0;
  int cur_piece_points = osl::enter_king::countPiecePointsInRange<Turn>(state, num_pieces, 1, 9, enemyCampMin, enemyCampMax);

  // 敵陣に駒が少ない時にペナルティ
  if (num_pieces == 0) prob += 2 * strong_penalty;
  else if (num_pieces < 3) prob += normal_penalty;

  // 自玉の前方にある敵からの利きの数を計算
  // 相手の利きが多い時は入玉しにくいのでペナルティ
  const int opp_effect_kingfront = osl::enter_king::countEffectInFrontOf(state, alt(Turn), myKingSquare, Turn);
  prob += (opp_effect_kingfront / 4) * strong_penalty;

  // ボーナス
  // 王の前方に敵からの利きがない, 敵陣に8枚以上の駒がある
  if (opp_effect_kingfront == 0 && prob < normal_penalty)
    prob -= normal_penalty;
  if (num_pieces > 7 && prob < weak_penalty)
    prob -= weak_penalty;

  if (prob < lower_bound) return 0.0;
  // 持駒を点数に加える
  // 入玉に必要な点数以上なら確率を返す
  cur_piece_points += osl::enter_king::countPiecePointsOnStand(state, Turn);
  if (cur_piece_points >= winning_threshold) 
    return exp(prob);

  // 不足している点数を補うため、中段や自陣にある駒を敵陣へ持っていく
  // そのためのコスト(枚数)でペナルティをかける
  const int direction = (Turn==osl::BLACK)? 1:-1;
  const int base_y = (Turn==osl::BLACK)? enemyCampMax : enemyCampMin;
  double pos_penalty = weak_penalty;
  for (int yi = 1; yi <= 6; yi++) {
    if (yi == 5) pos_penalty = normal_penalty;
    int num_pieces_on_row = 0; 
    // ある行の駒の点数と枚数を計算
    const int points_on_row = osl::enter_king::countPiecePointsOnRow<Turn>(state, num_pieces_on_row, base_y + direction * yi);
    // 必要な点数に達したら、必要な枚数だけペナルティをかけて確率を返す
    if (cur_piece_points + points_on_row >= winning_threshold) {
      prob += calcNumPiecesNeeded(cur_piece_points, points_on_row, num_pieces_on_row, winning_threshold) * pos_penalty;
      return exp(prob);
    }
    // まだ点数が不足しているので、点数や確率を更新して次の行へ
    cur_piece_points += points_on_row;
    prob += num_pieces_on_row * pos_penalty;
    if (prob < lower_bound) return 0.0;
  }
  // 駒の点数が足りないので0.0 を返す
  // 敵陣に駒があれば補えるかもしれない
  return 0.0;
}
double osl::enter_king::SimplePredictor::getProbability(const NumEffectState& state, const osl::Player Turn) {
  if (Turn == osl::BLACK)
    return getProbability<osl::BLACK>(state);
  else 
    return getProbability<osl::WHITE>(state);
}

// Turn が入玉できる確率を返す
// 宣言法への対応
template <osl::Player Turn>
double osl::enter_king::SimplePredictor::getProbability27(const NumEffectState& state) {
  // 確率計算用のパラメータ
  // 最後にexp() で確率に戻す
  static const double normal_penalty = log(0.9);
  static const double strong_penalty = log(0.8);
  static const double weak_penalty   = log(0.95);
  static const double lower_bound    = log(0.05);
  double prob = 0.0;

  const osl::Square myKingSquare = state.kingSquare<Turn>();
  const int my_king_y = myKingSquare.y();
  const int enemyCampMin = (Turn==osl::BLACK) ? 1 : 7;
  const int enemyCampMax = enemyCampMin + 2;
  const int winning_threshold = (Turn==osl::BLACK) ? 
    winning_threshold_black_27 : winning_threshold_white_27;

  // 玉のy軸を見て入玉からの距離でペナルティをかける
  if (my_king_y < enemyCampMin || my_king_y > enemyCampMax) {
    const int dist = (Turn==osl::BLACK)? (my_king_y - enemyCampMax) : (enemyCampMin - my_king_y);
    prob += dist * normal_penalty;
  } 

  // 敵陣にある駒の点数と枚数を計算 (王は除外)
  int num_pieces = 0;
  int cur_piece_points = osl::enter_king::countPiecePointsInRange<Turn>(state, num_pieces, 1, 9, enemyCampMin, enemyCampMax);

  // 敵陣に駒が少ない時にペナルティ
  // 宣言法では敵陣に王以外に10枚の駒が必要
  if (num_pieces == 0) prob += 2 * strong_penalty;
  else if (num_pieces < 10)
    prob += ((12 - num_pieces) / 3) * normal_penalty;

  // 自玉の前方にある敵からの利きの数を計算
  // 相手の利きが多い時は入玉しにくいのでペナルティ
  const int opp_effect_kingfront = osl::enter_king::countEffectInFrontOf(state, alt(Turn), myKingSquare, Turn);
  prob += (opp_effect_kingfront / 4) * strong_penalty;

  // ボーナス
  // 王の前方に敵からの利きがない
  if (opp_effect_kingfront == 0 && prob < normal_penalty)
    prob -= normal_penalty;

  if (prob < lower_bound) return 0.0;
  // 持駒を点数に加える
  // 入玉に必要な点数以上なら確率を返す
  cur_piece_points += osl::enter_king::countPiecePointsOnStand(state, Turn);
  if (cur_piece_points >= winning_threshold) 
    return exp(prob);

  // 不足している点数を補うため、中段や自陣にある駒を敵陣へ持っていく
  // そのためのコスト(枚数)でペナルティをかける
  const int direction = (Turn==osl::BLACK)? 1:-1;
  const int base_y = (Turn==osl::BLACK)? enemyCampMax : enemyCampMin;
  double pos_penalty = weak_penalty;
  for (int yi = 1; yi <= 6; yi++) {
    if (yi == 5) pos_penalty = normal_penalty;
    int num_pieces_on_row = 0; 
    // ある行の駒の点数と枚数を計算
    const int points_on_row = osl::enter_king::countPiecePointsOnRow<Turn>(state, num_pieces_on_row, base_y + direction * yi);
    // 必要な点数に達したら、必要な枚数だけペナルティをかけて確率を返す
    if (cur_piece_points + points_on_row >= winning_threshold) {
      prob += calcNumPiecesNeeded(cur_piece_points, points_on_row, num_pieces_on_row, winning_threshold) * pos_penalty;
      return exp(prob);
    }
    // まだ点数が不足しているので、点数や確率を更新して次の行へ
    cur_piece_points += points_on_row;
    prob += num_pieces_on_row * pos_penalty;
    if (prob < lower_bound) return 0.0;
  }
  // 自分の駒だけでは点数が足りない
  // 5点以内なら取れるかもしれない
  if (winning_threshold - cur_piece_points <= 5)
    prob += (winning_threshold - cur_piece_points) * strong_penalty;
  else return 0.0;
  if (prob < lower_bound) return 0.0;
  return exp(prob);
}
double osl::enter_king::SimplePredictor::getProbability27(const NumEffectState& state, const osl::Player Turn) {
  if (Turn == osl::BLACK)
    return getProbability27<osl::BLACK>(state);
  else 
    return getProbability27<osl::WHITE>(state);
}

// 入玉の予測を行う関数. 確率がthreshold を超えていたらtrue そうでないならfalse
template <osl::Player Turn>
bool osl::enter_king::SimplePredictor::predict(const NumEffectState& state, double threshold) {
  if (getProbability<Turn>(state) > threshold)
    return true;
  return false;
}
bool osl::enter_king::SimplePredictor::predict(const NumEffectState& state,
					       osl::Player Turn, double threshold) {
  if (getProbability(state, Turn) > threshold)
    return true;
  return false;
}
// 入玉の予測を行う関数. 確率がthreshold を超えていたらtrue そうでないならfalse
template <osl::Player Turn>
bool osl::enter_king::SimplePredictor::predict27(const NumEffectState& state, double threshold) {
  if (getProbability27<Turn>(state) > threshold)
    return true;
  return false;
}
bool osl::enter_king::SimplePredictor::predict27(const NumEffectState& state,
						 osl::Player Turn, double threshold) {
  if (getProbability27(state, Turn) > threshold)
    return true;
  return false;
}

namespace osl {
  namespace enter_king {
    template
    double SimplePredictor::getProbability<osl::BLACK>
    (const NumEffectState& state);
    template
    double SimplePredictor::getProbability<osl::WHITE>
    (const NumEffectState& state);

    template
    double SimplePredictor::getProbability27<osl::BLACK>
    (const NumEffectState& state);
    template
    double SimplePredictor::getProbability27<osl::WHITE>
    (const NumEffectState& state);

    template
    bool SimplePredictor::predict<osl::BLACK>
    (const NumEffectState& state, double threshold);
    template
    bool SimplePredictor::predict<osl::WHITE>
    (const NumEffectState& state, double threshold);

    template
    bool SimplePredictor::predict27<osl::BLACK>
    (const NumEffectState& state, double threshold);
    template
    bool SimplePredictor::predict27<osl::WHITE>
    (const NumEffectState& state, double threshold);
  }
}
