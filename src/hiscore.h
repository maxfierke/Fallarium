#ifndef PINEIGHT_HISCORE_H
#define PINEIGHT_HISCORE_H

#include <allegro5/allegro.h>

typedef struct HiGame
{
  unsigned long gameID;  /* crc32 of game name */
  unsigned long when;  /* minutes (not seconds) since 1970;
                         will fail after y10k */
  unsigned long duration;  /* seconds of game time */
  unsigned int r1;  /* reserved for future use */
  int score[8];  /* records are sorted on descending score[0] */
} HiGame;

/* InitScore() *************************
 * Initializes the high score system.
 */
int InitScore();

/* AddScore() **************************
 * Adds a score to a user's personal high score list.  Returns 0 for
 * success or -1 for failure.
 */
int AddScore(const HiGame *game);

/* Login() *****************************
 * Logs a user in.  Returns 0 for success or -1 for exit program.
 * ASSUMES: Screen is set to 320x200x8
 */
int Login(char *aimsn);

#endif
