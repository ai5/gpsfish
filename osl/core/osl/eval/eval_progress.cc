#include "osl/eval/progress.h"

osl::CArray<int, 256> osl::eval::ml::ProgressBonus::table;

int osl::eval::ml::ProgressBonus::eval(Progress16 black,
						 Progress16 white)
{
  return table[index(black, white)];
}

void osl::eval::ml::ProgressBonus::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }

  for (int black = 0; black < 16; ++black)
  {
    for (int white = 0; white < 16; ++white)
    {
      if (black <= white)
      {
	table[index(Progress16(black), Progress16(white))] =
	  -table[index(Progress16(white), Progress16(black))];
      }
    }
  }
}


osl::CArray<int, 256> osl::eval::ml::ProgressAttackDefense::table;

int osl::eval::ml::ProgressAttackDefense::eval(
  Progress16 black_attack, Progress16 white_defense,
  Progress16 white_attack, Progress16 black_defense)
{
  return table[index(black_attack, white_defense)] -
    table[index(white_attack, black_defense)];
}

void osl::eval::ml::
ProgressAttackDefense::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
}


osl::CArray<int, 65536>
osl::eval::ml::ProgressAttackDefenseAll::table;

int osl::eval::ml::ProgressAttackDefenseAll::eval(
  Progress16 black_attack, Progress16 white_defense,
  Progress16 white_attack, Progress16 black_defense)
{
  return table[index(black_attack, white_defense,
		     white_attack, black_defense)];
}

void osl::eval::ml::
ProgressAttackDefenseAll::setUp(const Weights &weights)
{
  for (size_t i = 0; i < weights.dimension(); ++i)
  {
    table[i] = weights.value(i);
  }
  for (int ba = 0; ba < 16; ++ba)
  {
    for (int wd = 0; wd < 16; ++wd)
    {
      for (int wa = 0; wa < 16; ++wa)
      {
	for (int bd = 0; bd < 16; ++bd)
	{
	  if (ba + wd < wa + bd ||
	      (ba + wd == wa + bd &&
	       ba <= wa))
	  {
	    table[index(Progress16(ba), Progress16(wd),
			Progress16(wa), Progress16(bd))] =
	      -table[index(Progress16(wa), Progress16(bd),
			   Progress16(ba), Progress16(wd))];
	  }
	}
      }
    }
  }
}
