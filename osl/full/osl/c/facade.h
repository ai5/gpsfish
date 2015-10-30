/* facade.h
 */
#ifndef OSL_FACADE_H
#define OSL_FACADE_H

extern "C" {
  extern void osl_init();
  // size of move must be at least 8
  extern int checkmate_attack(const char *state, int& limit, char *move);
  extern int checkmate_escape(const char *state, int limit);
  extern int search(const char *state, int seconds, int verbose, char *move);

  /**
   * Converts moves in a USI format string to a Kanji representation.
   *
   * @param command a command string for gpsusi. ex. "ki2moves 7g7f 3c3d"
   * @param out a buffer to return a result string.
   * @param out_size size of the out buffer.
   * @return the actual size with which the buffer is filled. It does not
   *         include a null terminator.
   */
  extern int usiMovesToKanji(const char *command, char *out, int out_size);
  /**
   * Generates a Kanji position spcified by moves in a USI format string.
   *
   * @param moves_str USI moves string.
   * @param out a buffer to return a result string.
   * @param out_size size of the out buffer.
   * @return the actual size with which the buffer is filled. It does not
   *         include a null terminator.
   */
  extern int usiMovesToPositionString(const char *moves_str, char *out, int out_size);
}

#endif /* OSL_FACADE_H */
// ;;; Local Variables:
// ;;; mode:c
// ;;; c-basic-offset:2
// ;;; End:
