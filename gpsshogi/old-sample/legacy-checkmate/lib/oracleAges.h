/* oracleAges.h
 */
#ifndef _ORACLEAGES_H
#define _ORACLEAGES_H

namespace osl
{
  namespace checkmate
  {
    struct AttackOracleAges
    {
      unsigned short proof;
      unsigned short proof_last_move;
      unsigned short disproof;
      AttackOracleAges() : proof(0), proof_last_move(0), disproof(0)
      {
      }
    };
  } // namespace checkmate
  using checkmate::AttackOracleAges;
} // namespace osl

#endif /* _ORACLEAGES_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
