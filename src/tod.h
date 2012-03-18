#ifndef PINEIGHT_TOD_H
#define PINEIGHT_TOD_H

#include <allegro5/allegro.h>
#include <time.h>
#include "allegdlg.h"
#include "hiscore.h"

#define NUM_PIECES    10
#define NUM_FLIPS      4
#define NUM_BLOCKS     4

#define VIRTUAL_FPS  100

// CONSTANTS
enum {
  STATE_INACTIVE = 0,
  STATE_GET_NEW_PIECE,
  STATE_FALLING_PIECE,
  STATE_CHECK4LINES,
  STATE_WAIT,
  STATE_FALL,
  STATE_PUSHUP,
  STATE_GAMEOVER,
  STATE_HISCORE
};

enum {
  PIECE_T = 0,
  PIECE_Z, PIECE_S, PIECE_J, PIECE_L, PIECE_SQUARE, PIECE_SMALL_L,
  PIECE_STICK, PIECE_TINY_STICK, PIECE_SHORT_STICK,
  PIECE_MAGNET,
  PIECE_INVERSE
};

// DATA TYPES

typedef struct TetGlobals
{
  ALLEGRO_BITMAP *backbits;
  int endTime, doneTime;
  signed char winLimit, timeLimit;
  /* teflonMode: generate no stick pieces
     tntMode: emulate The New Tetris for N64 */
  unsigned char teflonMode, tntMode, usingJoy;
  int handicap, yBase;
} TetGlobals;

typedef struct Timers
{
  int odo, trip;
  char paused;
} Timers;

typedef struct Seven
{
  al_fixed leftX[768], rightX[768], leftY[768], rightY[768];
  ALLEGRO_BITMAP *backBuf, *frontBuf;
  int leftSide, rightSide;
  int nextX;
} Seven;

typedef struct Field
{
  unsigned char b[21][10];
} Field;

typedef struct Player
{
  Field blockMap, auxMap;
  HiGame high;
  Seven *seven;

  time_t gameStarted;

  signed char  repeatTime[8];

  int curPiece[5];
  int curColor[5];
  int curFlip, dropMove, spinMove;
  int coming, top;
  int scoreFac, chainCount, vis, state;
  int pieceDone, hasSwitched, inverse, handiLines;

  int x, y;
  int lines, wins, hsMode;
  int score, gameStart, stateTime;
} Player;

typedef struct ScreenPos
{
  al_fixed a, da;
  al_fixed b, db;
  al_fixed c, dc;
  al_fixed d, dd;
  al_fixed theta, dtheta;
  al_fixed phi, dphi;
  al_fixed scale, dscale;
  al_fixed amp, damp;
  al_fixed sinusFreq, noise, heat;

  al_fixed xc, yc, xctarget, yctarget;
  int mana; // more mana == more effects
  int fps;
  unsigned int fricCounter;
  int coming, nEffects;
  unsigned int lastBeat, midiLoop, curSong;
  int fxQueue[10];
} ScreenPos;

// GLOBALS

extern volatile Timers timers;
extern ALLEGRO_BITMAP *tetbits;
extern Player p[2];
extern TetGlobals g;
extern int nPlayers;

/* tetanus.c */
int GameLoop(void);
void NewGame(int pl);
void DrawNext(int pl);

/* scrctl.c */
unsigned int Keys2CtlWord(void);
void CtlWord2Screen(ScreenPos *pos, unsigned int ctlword);
void Resonance(ScreenPos *pos);
unsigned int Mana2CtlWord(ScreenPos *pos, unsigned int beat);
void StartSong(int songNo, DATAFILE *dat, ScreenPos *pos);

#endif
