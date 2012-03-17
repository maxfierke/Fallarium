/* Notice

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful and fun, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the License for more details.

 You should have received a copy of the License along with this program,
 in the file COPYING; if not, write to the Free Software Foundation, Inc. /
 59 Temple Place - Suite 330 / Boston, MA  02111-1307 / USA
 or view the License online at http://www.gnu.org/copyleft/gpl.html

 The author can be reached by
 usps:Damian Yerrick / Box 398 / 5500 Wabash Avenue / Terre Haute, IN 47803
 mailto:dosarena@pineight.8m.com
 http://come.to/yerrick

 DOSArena is a trademark of Damian Yerrick. All other trademarks belong to
 their respective owners.

 */

#include "tod.h"

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#define DIFFICULTY 350
#define INITIAL_LEVEL 5 // is 10 in FPA tetanus but FPA has no drugz
/* This table defines the color for each type of block. */
enum {
	BCOL_J = 0x10,
	BCOL_S = 0x20,
	BCOL_STICK = 0x30,
	BCOL_Z = 0x40,
	BCOL_L = 0x50,
	BCOL_T = 0x60,
	BCOL_O = 0x70,
	BCOL_H = 0x90,
	BCOL_R = 0xa0,
	BCOL_MULTISQUARE = 0xe0,
	BCOL_MONOSQUARE = 0xf0,
	BCOL_ALL = 0xf0,

	/* utility tiles */
	BCOL_LINECLEAR = 0x81,
	BCOL_NEUTRAL = 0x80
};

/* This table defines connections.  For example, if BCON_E is set,
 then this block is connected to the block on the E side and should
 be shaded appropriately.  If BCON_E is clear, draw an edge.
 */
enum {
	BCON_NONE = 0,
	BCON_E = 1,
	BCON_W = 2,
	BCON_WE = BCON_W | BCON_E,
	BCON_S = 4,
	BCON_SE = BCON_S | BCON_E,
	BCON_SW = BCON_S | BCON_W,
	BCON_SWE = BCON_S | BCON_WE,
	BCON_N = 8,
	BCON_NE = BCON_N | BCON_E,
	BCON_NW = BCON_N | BCON_W,
	BCON_NWE = BCON_N | BCON_WE,
	BCON_NS = BCON_N | BCON_S,
	BCON_NSE = BCON_N | BCON_SE,
	BCON_NSW = BCON_N | BCON_SW,
	BCON_NSWE = BCON_N | BCON_SWE,
	BCON_ALL = BCON_NSWE
};

enum {
	JPAD_BUTTON_UP = 0,
	JPAD_BUTTON_DOWN = 1,
	JPAD_BUTTON_LEFT = 2,
	JPAD_BUTTON_RIGHT = 3,
	JPAD_BUTTON_ROTL = 4,
	JPAD_BUTTON_ROTR = 5,
	JPAD_BUTTON_SWAP = 6,
	JPAD_BUTTON_MISC = 7,

	JPAD_MASK_UP = 0x80 >> JPAD_BUTTON_UP,
	JPAD_MASK_DOWN = 0x80 >> JPAD_BUTTON_DOWN,
	JPAD_MASK_LEFT = 0x80 >> JPAD_BUTTON_LEFT,
	JPAD_MASK_RIGHT = 0x80 >> JPAD_BUTTON_RIGHT,
	JPAD_MASK_ROTL = 0x80 >> JPAD_BUTTON_ROTL,
	JPAD_MASK_ROTR = 0x80 >> JPAD_BUTTON_ROTR,
	JPAD_MASK_SWAP = 0x80 >> JPAD_BUTTON_SWAP,
	JPAD_MASK_MISC = 0x80 >> JPAD_BUTTON_MISC,

};

static const char gPieceColor[NUM_PIECES] = { BCOL_T, BCOL_Z, BCOL_S, BCOL_J,
		BCOL_L, BCOL_O, BCOL_Z, BCOL_STICK, BCOL_STICK, BCOL_STICK };

/* DATA TABLES

 This might need a little clarification. Take the T piece:
 x={{0,1,2,1},{1,1,1,0},{2,1,0,1},{1,1,1,2}}
 y={{1,1,1,2},{0,1,2,1},{1,1,1,0},{2,1,0,1}}
 This is a set of (x, y) data for each block in each rotation
 of the piece. The shape it defines is
 0 1 2 3   0 1 2 3   0 1 2 3   0 1 2 3
 0 . . . .   . X . .   . X . .   . X . .
 1 X X X .   X X . .   X X X .   . X X .
 2 . X . .   . X . .   . . . .   . X . .
 3 . . . .   . . . .   . . . .   . . . .

 gCBlocks records which blocks exist and how they're connected
 for display purposes.
 */
static const char gXBlocks[NUM_PIECES][NUM_FLIPS][NUM_BLOCKS] = { {
		{ 0, 1, 2, 1 }, { 1, 1, 1, 0 }, { 2, 1, 0, 1 }, { 1, 1, 1, 2 } }, // t
		{ { 0, 1, 1, 2 }, { 2, 2, 1, 1 }, { 2, 1, 1, 0 }, { 1, 1, 2, 2 } }, // z
		{ { 0, 1, 1, 2 }, { 0, 0, 1, 1 }, { 2, 1, 1, 0 }, { 1, 1, 0, 0 } }, // s
		{ { 0, 1, 2, 2 }, { 1, 1, 1, 0 }, { 2, 1, 0, 0 }, { 1, 1, 1, 2 } }, // j
		{ { 0, 0, 1, 2 }, { 0, 1, 1, 1 }, { 2, 2, 1, 0 }, { 2, 1, 1, 1 } }, // l
		{ { 1, 1, 2, 2 }, { 2, 1, 1, 2 }, { 2, 2, 1, 1 }, { 1, 2, 2, 1 } }, // package
		{ { 1, 1, 2, 2 }, { 2, 1, 1, 2 }, { 2, 2, 1, 1 }, { 1, 2, 2, 1 } }, // 3-l
		{ { 0, 1, 2, 3 }, { 1, 1, 1, 1 }, { 3, 2, 1, 0 }, { 1, 1, 1, 1 } }, // 4-stick
		{ { 0, 1, 2, 3 }, { 1, 1, 1, 1 }, { 3, 2, 1, 0 }, { 1, 1, 1, 1 } }, // 2-stick
		{ { 0, 1, 2, 0 }, { 1, 1, 1, 0 }, { 2, 1, 0, 0 }, { 1, 1, 1, 0 } } // 3-stick
};

static const char gYBlocks[NUM_PIECES][NUM_FLIPS][NUM_BLOCKS] = { {
		{ 1, 1, 1, 2 }, { 0, 1, 2, 1 }, { 1, 1, 1, 0 }, { 2, 1, 0, 1 } }, { { 1,
		1, 2, 2 }, { 0, 1, 1, 2 }, { 2, 2, 1, 1 }, { 2, 1, 1, 0 } }, { { 2, 2,
		1, 1 }, { 0, 1, 1, 2 }, { 1, 1, 2, 2 }, { 2, 1, 1, 0 } }, {
		{ 1, 1, 1, 2 }, { 0, 1, 2, 2 }, { 1, 1, 1, 0 }, { 2, 1, 0, 0 } }, { { 2,
		1, 1, 1 }, { 0, 0, 1, 2 }, { 0, 1, 1, 1 }, { 2, 2, 1, 0 } }, { { 1, 2,
		2, 1 }, { 1, 1, 2, 2 }, { 2, 1, 1, 2 }, { 2, 2, 1, 1 } }, {
		{ 1, 2, 2, 1 }, { 1, 1, 2, 2 }, { 2, 1, 1, 2 }, { 2, 2, 1, 1 } }, { { 2,
		2, 2, 2 }, { 0, 1, 2, 3 }, { 2, 2, 2, 2 }, { 3, 2, 1, 0 } }, { { 2, 2,
		2, 2 }, { 0, 1, 2, 3 }, { 2, 2, 2, 2 }, { 3, 2, 1, 0 } }, {
		{ 2, 2, 2, 0 }, { 1, 2, 3, 0 }, { 2, 2, 2, 0 }, { 3, 2, 1, 0 } } };

/* big fat ass connection lut */
static const char gCBlocks[NUM_PIECES][NUM_FLIPS][NUM_BLOCKS] = {
		{ { BCON_E, BCON_SWE, BCON_W, BCON_N }, { BCON_S, BCON_NSW, BCON_N,
				BCON_E }, { BCON_W, BCON_NWE, BCON_E, BCON_S }, { BCON_N,
				BCON_NSE, BCON_S, BCON_W } },
		{ { BCON_E, BCON_SW, BCON_NE, BCON_W }, { BCON_S, BCON_NW, BCON_SE,
				BCON_N }, { BCON_W, BCON_NE, BCON_SW, BCON_E }, { BCON_N,
				BCON_SE, BCON_NW, BCON_S } },
		{ { BCON_E, BCON_NW, BCON_SE, BCON_W }, { BCON_S, BCON_NE, BCON_SW,
				BCON_N }, { BCON_W, BCON_SE, BCON_NW, BCON_E }, { BCON_N,
				BCON_SW, BCON_NE, BCON_S } },
		{ { BCON_E, BCON_WE, BCON_SW, BCON_N }, { BCON_S, BCON_NS, BCON_NW,
				BCON_E }, { BCON_W, BCON_WE, BCON_NE, BCON_S }, { BCON_N,
				BCON_NS, BCON_SE, BCON_W } },
		{ { BCON_N, BCON_SE, BCON_WE, BCON_W }, { BCON_E, BCON_SW, BCON_NS,
				BCON_N }, { BCON_S, BCON_NW, BCON_WE, BCON_E }, { BCON_W,
				BCON_NE, BCON_NS, BCON_S } }, { { BCON_SE, BCON_NE, BCON_NW,
				BCON_SW }, { BCON_SE, BCON_NE, BCON_NW, BCON_SW }, { BCON_SE,
				BCON_NE, BCON_NW, BCON_SW }, { BCON_SE, BCON_NE, BCON_NW,
				BCON_SW } }, { { BCON_SE, BCON_N, 0, BCON_W }, { BCON_SW,
				BCON_E, 0, BCON_N }, { BCON_NW, BCON_S, 0, BCON_E }, { BCON_NE,
				BCON_W, 0, BCON_S } }, { { BCON_E, BCON_WE, BCON_WE, BCON_W }, {
				BCON_S, BCON_NS, BCON_NS, BCON_N }, { BCON_W, BCON_WE, BCON_WE,
				BCON_E }, { BCON_N, BCON_NS, BCON_NS, BCON_S } }, { { 0, BCON_E,
				BCON_W, 0 }, { 0, BCON_S, BCON_N, 0 }, { 0, BCON_W, BCON_E, 0 },
				{ 0, BCON_N, BCON_S, 0 } }, { { BCON_E, BCON_WE, BCON_W, 0 }, {
				BCON_S, BCON_NS, BCON_N, 0 }, { BCON_W, BCON_WE, BCON_E, 0 }, {
				BCON_N, BCON_NS, BCON_S, 0 } } };

#define NUM_gTips 7
/*
 static char gTips[NUM_gTips][2][32] =
 {
 {"When reducing your stack, try", "taking advantage of gravity."},
 {"Lines of all one color", "(H or R) count double."},
 {"Try sliding one block", "under another block."},
 {"Some days you win;", "some days it rains."},
 {"Baking cookies with butter (not", "margarine) improves the flavor."},
 {"To survive longer,", "lay sticks flat."},
 {"The best defense is a good", "offense; make tetrises often."}
 };
 */

/* GLOBAL VARIABLES */

TetGlobals g = { NULL };
Player p[2];
static int attackMode = 0, raceMode = 0;
int nPlayers = 1;
static int curTurn = 0;
#define ME p[curTurn] // to make accessing p[] easier
/* FUNCTION PROTOTYPES AND MACROS */

/* "Carbon" is the code name of the flood-fill based gravity algorithm. */
static char CarbonStillGoing(void);
static void DrawPiece(short x, short y, int inPiece, int inFlip, short color,
		short style);
static void TeFloodFill(int x, int y, unsigned int c);
static void FloodFillLoop(int x, int y, unsigned int c, unsigned int uc);
static short MarkCarbon(int spinMove);
static char CheckOverlap(short x, short y, short inPiece, short inFlip);
static void PushUp(void);
//static void ScrollDown(short top, short bottom);
static void TeDrawBlock(int x, int y, unsigned colour);
//void TeShutDown(void);
//short TeStartUp(void);
static void GetNewPiece(int depth);

int lastClock;

int ReadJPad(int n) {
	static char polled[4] = { 0, 0, 0, 0 };
	ALLEGRO_JOYSTICK_STATE jState;
	int i, out = 0;
	int rightKeys[8] = { ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_LEFT,
			ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_QUOTE, ALLEGRO_KEY_ENTER,
			ALLEGRO_KEY_OPENBRACE, ALLEGRO_KEY_CLOSEBRACE };
	int leftKeys[8] = { ALLEGRO_KEY_R, ALLEGRO_KEY_F, ALLEGRO_KEY_D,
			ALLEGRO_KEY_G, ALLEGRO_KEY_A, ALLEGRO_KEY_S, ALLEGRO_KEY_Q,
			ALLEGRO_KEY_W };

	/* If in a 2P game without joysticks, read the left player
	 * from the left side of the keyboard.
	 */
	if (nPlayers > 1 && (al_get_num_joysticks() == 0 || g.usingJoy == 0)) {
		if (n == 0) {
			for (i = 0; i < 8; i++) {
				out <<= 1;
				if (key[leftKeys[i]])
					out |= 1;
			}
			return out;
		} else
			n--;
	}

	if (n > al_get_num_joysticks())
		return 0;

	/* If we have more players than joysticks, read the first player
	 * after the sticks from the right side of the keyboard.
	 */
	if (n == al_get_num_joysticks() || g.usingJoy == 0) {
		for (i = 0; i < 8; i++) {
			out <<= 1;
			if (key[rightKeys[i]])
				out |= 1;
		}
		return out;
	}

	// If necessary, read the joysticks.
	if (!polled[n]) {
		poll_joystick();
		for (i = 0; i < 4; i++)
			polled[i] = 1;
	}

	// Needs rewrite for new Joystick API and Event handler
	/*
	 if(al_get_joystick_state(al_get_joystick(n), &jState).stick[0].axis[1].d1) // up
	 out |= JPAD_MASK_UP;
	 if(al_get_joystick_state(al_get_joystick(n), &jState).stick[0].axis[1].d2) // down
	 out |= JPAD_MASK_DOWN;
	 if(al_get_joystick_state(al_get_joystick(n), &jState).stick[0].axis[0].d1) // left
	 out |= JPAD_MASK_LEFT;
	 if(al_get_joystick_state(al_get_joystick(n), &jState).stick[0].axis[0].d2) // right
	 out |= JPAD_MASK_RIGHT;
	 if(al_get_joystick_state(al_get_joystick(n), &jState).button[0].b)
	 out |= JPAD_MASK_ROTL;
	 if(al_get_joystick_state(al_get_joystick(n), &jState).button[1].b)
	 out |= JPAD_MASK_ROTR;
	 if(al_get_joystick(n).num_buttons > 2 && al_get_joystick(n).button[2].b)
	 out |= JPAD_MASK_SWAP;
	 if(al_get_joystick(n).num_buttons > 3 && al_get_joystick(n).button[3].b)
	 out |= JPAD_MASK_MISC;
	 */

	/* Mark this joystick as "needs to be read next time." */
	polled[n] = 0;
	return out;
}

/* MakeRepeats() ***********************
 * Autorepeats digital pad motion.  After autoDelay/70 of a second, it repeats
 * a key 70/autoRate times a second.
 */
void MakeRepeats(char *repeatTime, short j, short autoDelay, short autoRate) {
	short i;

	for (i = 7; i >= 0; i--) {
		if (j & 0x01) {
			repeatTime[i]++;
			if (repeatTime[i] >= autoDelay)
				repeatTime[i] -= autoRate;
		} else {
			repeatTime[i] = 0;
		}
		j >>= 1;
	}
}

/* CarbonStillGoing() ******************
 * Moves everything down one line if still floating.
 */
static char CarbonStillGoing(void) {
	static int lastTime = 0;
	signed char flooded = 0;
	short x, d, e, y;

	/* debugging information: enable if you touched any function with Carbon in
	 * its name and your modifications make the program screw up */

	/*
	 for(y = 0; y <= 20; y++)
	 for (x = 0; x < 10; x++)
	 {
	 gotoxy(x * 2 + 11, y + 1);
	 cputs("%d", (int)ME.auxMap.b[y][x]);
	 }
	 */

	/* move everything down if it is floating */
	for (x = 0; x < 10; x++) {
		e = ME.auxMap.b[20][x];
		for (y = 20; y >= 0; y--) {
			d = e;
			if (y > 0)
				e = ME.auxMap.b[y - 1][x];
			else
				e = 0;
			if ((e != 1) && (d != 1)) {
				ME.auxMap.b[y][x] = e;
				if (y < 20) {
					if (y == 0)
						ME.blockMap.b[y][x] = 0;
					else
						ME.blockMap.b[y][x] = ME.blockMap.b[y - 1][x];
					TeDrawBlock(x, y,
							ME.blockMap.b[y][x] ? ME.blockMap.b[y][x] : 0);
				}
			}
		}
	}

	/* if something just hit the ground, mark it as ground */
	for (y = 19; y >= 0; y--)
		for (x = 0; x <= 9; x++)
			if ((ME.auxMap.b[y][x] > 1) && (ME.auxMap.b[y + 1][x] == 1)) {
				TeFloodFill(x, y, 1);
				flooded = 1;
			}

	/* if something hit the ground, play a thud */
	if (flooded)
		/*SendSound(gDropSound)*/;

	ME.y++; /* for scrolling the clear into visibility */

	/* if everything isn't ground, we're still going */
	for (y = 0; y <= 19; y++)
		for (x = 0; x < 20; x++)
			if (ME.auxMap.b[y][x] > 1)
				return 1;

	/* FIXME: these lines spinwait! */
	while (lastTime == retrace_count)
		;
	lastTime = retrace_count;

	return 0;
}

static void BreakLineLoop(int x, int y) {
	int c = ME.blockMap.b[y][x];

	if ((c & BCOL_ALL) >= BCOL_MULTISQUARE) {
		if (y < 19 && ME.blockMap.b[y + 1][x] != 0) {
			ME.blockMap.b[y + 1][x] &= ~BCON_N;
			TeDrawBlock(x, y + 1, ME.blockMap.b[y + 1][x]);
		}
		if (y > 0 && ME.blockMap.b[y - 1][x] != 0) {
			ME.blockMap.b[y - 1][x] &= ~BCON_S;
			TeDrawBlock(x, y - 1, ME.blockMap.b[y - 1][x]);
		}
		return;
	}

	if (g.tntMode)
		ME.blockMap.b[y][x] = BCOL_NEUTRAL;
	else
		ME.blockMap.b[y][x] &= ~BCON_ALL;
	TeDrawBlock(x, y, ME.blockMap.b[y][x]);
	if ((c & BCON_N) && y > 0)
		BreakLineLoop(x, y - 1);
	if ((c & BCON_S) && y < 19)
		BreakLineLoop(x, y + 1);
	if ((c & BCON_W) && y > 0)
		BreakLineLoop(x - 1, y);
	if ((c & BCON_E) && x < 9)
		BreakLineLoop(x + 1, y);
}

#define USE_TENGEN_BREAKLINE 0

/* BreakLine() *************************
 * Break connections between blocks on this row and blocks on
 * neighboring rows.
 */
static void BreakLine(int y) {
	int x;

	/* BreakLine() *************************
	 * Break connections inside blocks on this row.
	 */

#if USE_TENGEN_BREAKLINE
	if(y > 0)
	for(x = 0; x < 10; x++)
	if(ME.blockMap.b[y - 1][x] & BCON_S)
	{
		ME.blockMap.b[y - 1][x] &= ~BCON_S;
		TeDrawBlock(x, y - 1, ME.blockMap.b[y - 1][x]);
	}

	if(y < 19)
	for(x = 0; x < 10; x++)
	if(ME.blockMap.b[y + 1][x] & BCON_N)
	{
		ME.blockMap.b[y + 1][x] &= ~BCON_N;
		TeDrawBlock(x, y + 1, ME.blockMap.b[y + 1][x]);
	}
#else
	for (x = 0; x < 10; x++) {
		if ((ME.blockMap.b[y][x] & BCON_ALL) != 0)
			BreakLineLoop(x, y);
	}
#endif

}

/* MarkSquaresTest() *******************
 * Look for a TNT square in this position.  Return 1 if found.
 * Assumes: 0 <= x <= 6; 0 <= y <= 16
 */
static int MarkSquaresTest(int x, int y, int multiOK) {
	int i, j, firstColor = ME.blockMap.b[y][x] & BCOL_ALL;

	for (i = 0; i <= 3; i++) {
		/* don't allow squares within parts of squares */
		if ((ME.blockMap.b[y + i][x] & BCOL_ALL) >= BCOL_MULTISQUARE)
			return 0;
		/* the block doesn't connect on the left */
		if (ME.blockMap.b[y + i][x] & BCON_W)
			return 0;
		/* the block doesn't connect on the right */
		if (ME.blockMap.b[y + i][x + 3] & BCON_E)
			return 0;
		/* the block doesn't connect on the top */
		if (ME.blockMap.b[y][x + i] & BCON_N)
			return 0;
		/* the block doesn't connect on the bottom */
		if (ME.blockMap.b[y + 3][x + i] & BCON_S)
			return 0;

		for (j = 0; j <= 3; j++) {
			int c = ME.blockMap.b[y + i][x + j];

			/* the square contains no single blocks */
			if ((c & BCON_ALL) == 0)
				return 0;
			/* if looking for monosquares, disallow multisquares */
			if (multiOK == 0 && (c & BCOL_ALL) != firstColor)
				return 0;
		}
	}
	return 1;
}

/* MarkSquare() ************************
 * Places a multisquare or monosquare in the field.
 */
static void MarkSquare(int x, int y, int multiOK) {
	int i, j, c;

	multiOK = (multiOK ? BCOL_MULTISQUARE : BCOL_MONOSQUARE) | BCON_ALL;
	for (i = 0; i <= 3; i++) {
		if (i == 0)
			c = multiOK & ~BCON_N;
		else if (i == 3)
			c = multiOK & ~BCON_S;
		else
			c = multiOK;

		for (j = 0; j <= 3; j++) {
			if (j == 0)
				ME.blockMap.b[y + i][x + j] = c & ~BCON_W;
			else if (j == 3)
				ME.blockMap.b[y + i][x + j] = c & ~BCON_E;
			else
				ME.blockMap.b[y + i][x + j] = c;
			TeDrawBlock(x + j, y + i, ME.blockMap.b[y + i][x + j]);
		}
	}

}

/* MarkSquares() ***********************
 * Create TNT monosquares and multisquares.
 */
static void MarkSquares(int multiOK) {
	int x, y;

	for (y = 16; y >= 0; y--)
		for (x = 0; x <= 6; x++)
			if (ME.blockMap.b[y][x] & BCON_ALL)
				if (MarkSquaresTest(x, y, multiOK))
					MarkSquare(x, y, multiOK);
}

/* CheckLines() *****************
 * Clears horizontal rows using Carbon (flood fill drops) algorithm.
 */
static char CheckLines(int spinMove) {
	int wiener = 0, chainCount = 0;
	int x, y;
	static int fib[20] = { 1, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233,
			377, 610, 987, 1597, 2584, 4181 };

	if (g.tntMode) {
		MarkSquares(0);
		MarkSquares(1);
	}

	ME.y = 19;

	for (y = 0; y <= 19; y++) {
		int hCount = 0, rCount = 0, grayCount = 0;
		for (x = 0; x < 10; x++) {
			switch (ME.blockMap.b[y][x] & BCOL_ALL) {
			case 0:
				break;
			case BCOL_H:
				hCount++;
				break;
			case BCOL_R:
				rCount++;
				break;
			default:
				grayCount++;
				break;
			}
		}
		if (hCount + rCount + grayCount == 10) {
			/* break lines up and down */
			BreakLine(y);

			/* remove the line */
			for (x = 0; x < 10; x++) {
				switch (ME.blockMap.b[y][x] & (BCON_WE | BCOL_ALL)) {
				case BCOL_MONOSQUARE | BCON_E:
					ME.score += 1000;
					break;
				case BCOL_MULTISQUARE | BCON_E:
					ME.score += 500;
					break;
				default:
					break;
				}

				ME.blockMap.b[y][x] = 0;
				TeDrawBlock(x, y, BCOL_LINECLEAR);
			}

			/* score the line */
			if (ME.scoreFac && attackMode)
				p[1 - curTurn].coming++;
			if (g.tntMode)
				ME.score += fib[(int) (ME.scoreFac++)] * 100;
			else
				ME.score += (int) (++ME.scoreFac) * 100;
			ME.lines++;
			wiener++;

			/* if H or R line, score double */
			if (hCount == 10 || rCount == 10) {
				ME.scoreFac++;
				ME.score += (int) (ME.scoreFac) * 100;
				ME.lines++;
				wiener++;
			}
			/* scroll the action on screen */
			if (y < ME.y)
				ME.y = y;
		}
	}
	if (wiener) {
		// Handle the combo counter.
//    textprintf(screen, font, chainCount * 12, 0, 12, "%d", wiener);
		chainCount++;
//    SendSound(gLineSound);

		if (spinMove) {
			/* break apart all the pieces */

			if (g.tntMode) {
				for (y = 0; y < 20; y++)
					for (x = 0; x < 10; x++)
						if (ME.blockMap.b[y][x])
							ME.blockMap.b[y][x] = BCOL_NEUTRAL;
			} else {
				for (y = 0; y < 20; y++)
					for (x = 0; x < 10; x++)
						ME.blockMap.b[y][x] &= ~BCOL_ALL;
			}
		}

		MarkCarbon(spinMove);
	}
	return wiener;
}

/* DrawPiece() ************************
 * This probably won't need to be changed too much when someone
 * ports Tetanus to other platforms.  What you'll need to change
 * is TeDrawBlock.
 */
static void DrawPiece(short x, short y, int inPiece, int inFlip, short color,
		short style) {
	int n, k;

	switch (style) {
	case 0: /* erase */
		for (n = 0; n < NUM_BLOCKS; n++)
			if (gCBlocks[inPiece][inFlip][n])
				TeDrawBlock(x + gXBlocks[inPiece][inFlip][n],
						y + gYBlocks[inPiece][inFlip][n], 0);
		break;
	case 1: /* high intensity */
		for (n = 0; n < NUM_BLOCKS; n++)
			if (gCBlocks[inPiece][inFlip][n])
				TeDrawBlock(x + gXBlocks[inPiece][inFlip][n],
						y + gYBlocks[inPiece][inFlip][n], color);
		break;
	case 2: /* low intensity and copy to ME.blockMap.b */
		for (n = 0; n < NUM_BLOCKS; n++) {
			int blkType = gCBlocks[inPiece][inFlip][n];
			if (blkType != 0) {
				int j = y + gYBlocks[inPiece][inFlip][n];

				if (ME.top > j)
					ME.top = j;
				if (j >= 0) {
					k = x + gXBlocks[inPiece][inFlip][n];
					ME.blockMap.b[j][k] = color | blkType;
					TeDrawBlock(k, j, color | blkType);
				}
			}
		}
		break;
	}
}

/* DrawNext() **************************
 * THIS MUST BE PORTED UP THE ASS for each new tetanus platform.
 */
void DrawNext(int pl) {
	unsigned i, n;

	int xcellsize = al_get_display_width() / 40;
	int ycellsize = al_get_display_height() / 30;

	for (i = 1; i <= 4; i++) {
		unsigned colour = p[pl].curColor[i];
		unsigned inPiece = p[pl].curPiece[i];
		int xSrc = (colour & BCON_ALL) << 3;
		int ySrc = (colour & BCOL_ALL) >> 1;
		int yBase = (i == 4) ? 1 : 5 + i * 3;

		for (n = 0; n < NUM_BLOCKS; n++) {
			if (gCBlocks[inPiece][0][n]) {
				int xDst = gXBlocks[inPiece][0][n];
				int yDst = gYBlocks[inPiece][0][n] + yBase;

				stretch_blit(tetbits, p[pl].seven->frontBuf, xSrc, ySrc, 8, 8,
						xDst * xcellsize + p[pl].seven->nextX, yDst * ycellsize,
						xcellsize, ycellsize);
			}
		}
	}
}

/* TeFloodFill() ************************
 * This pair of functions fills an area with color.  They have been
 * adapted to the context of Tetanus by marking any contiguous block
 * area with a given unique falling block.
 */
static void TeFloodFill(int x, int y, unsigned int c) {
	FloodFillLoop(x, y, c, ME.auxMap.b[y][x]);
	ME.auxMap.b[y][x] = c;
}

static void FloodFillLoop(int x, int y, unsigned int c, unsigned int fillC) {
	int fillL, fillR, i;
	int wiener = 1;

	fillL = fillR = x;
	while (wiener) {
		ME.auxMap.b[y][fillL] = c;
		fillL--;
		wiener = (fillL < 0) ? 0 : (ME.auxMap.b[y][fillL] == fillC);
	}
	fillL++;

	wiener = 1;
	while (wiener) {
		ME.auxMap.b[y][fillR] = c;
		fillR++;
		wiener = (fillR > 9) ? 0 : (ME.auxMap.b[y][fillR] == fillC);
	}
	fillR--;

	for (i = fillL; i <= fillR; i++) {
		if (y > 0)
			if (ME.auxMap.b[y - 1][i] == fillC)
				FloodFillLoop(i, y - 1, c, fillC);
		if (y < 20)
			if (ME.auxMap.b[y + 1][i] == fillC)
				FloodFillLoop(i, y + 1, c, fillC);
	}
}

static void GameOver(int pl) {
	p[pl].state = STATE_GAMEOVER;

	/* UINT_MAX minutes carries us past Y10K */
	p[pl].high.when = p[pl].gameStarted / 60;
	p[pl].high.duration = time(0) - p[pl].gameStarted;
	p[pl].high.score[0] = p[pl].score;
	p[pl].high.score[1] = p[pl].lines;
}

/* GameLoop() *************************
 * Play one frame of the game.  Return 0 for continue or 1 for gameover.
 */
int GameLoop(void) {
	int i, j;
	signed char lastEnter[2] = { 1, 1 };
	signed char won = 0;

//  signed char buf[256];

//  yield_timeslice();

	for (curTurn = 0; curTurn < nPlayers; curTurn++) {
		switch (ME.state) {
		case STATE_INACTIVE:
			/* restart key is both flips */
			j = ReadJPad(curTurn);
			i = ((j & (JPAD_MASK_ROTL | JPAD_MASK_ROTR))
					== (JPAD_MASK_ROTL | JPAD_MASK_ROTR));
			ME.coming = 0;
			if (i && !lastEnter[curTurn] && !won)
				NewGame(curTurn);
			lastEnter[curTurn] = i;
			yield_timeslice();
			break;

		case STATE_GET_NEW_PIECE:
			for (i = 0; i < 3; i++) {
				ME.curPiece[i] = ME.curPiece[i + 1];
				ME.curColor[i] = ME.curColor[i + 1];
			}
			GetNewPiece(3);

			/* Draw the next pieces. Not applicate */
			/*
			 DrawPiece(0, 0, ME.curPiece[1], 0, ME.curColor[1], 3);
			 */

			ME.x = 3;
			ME.y = -3;
			ME.vis = 1;
			ME.state = STATE_FALLING_PIECE;
			ME.stateTime = retrace_count + 16;
			ME.hasSwitched = ME.curFlip = ME.pieceDone = 0;
			DrawPiece(ME.x, ME.y, ME.curPiece[0], 0, ME.curColor[0], 1);
			break;

		case STATE_FALLING_PIECE:
			/* read the keyboard and joystick */
			j = ReadJPad(curTurn);

			MakeRepeats(ME.repeatTime, j, 18, 2);

			if (ME.repeatTime[6] == 1
					|| (ME.repeatTime[4] == 1 && ME.repeatTime[5] > 0)
					|| (ME.repeatTime[5] == 1 && ME.repeatTime[4] > 0)) /* swap button */
					{
				if (!ME.hasSwitched) {
					DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
							ME.curColor[0], 0);

					j = ME.curPiece[0];
					ME.curPiece[0] = ME.curPiece[4];
					ME.curPiece[4] = j;
					j = ME.curColor[0];
					ME.curColor[0] = ME.curColor[4];
					ME.curColor[4] = j;

					ME.x = 3;
					ME.y = -3;
					ME.vis = 1;
					ME.stateTime = retrace_count + 16;
					ME.hasSwitched = 1;
					ME.curFlip = ME.pieceDone = 0;
					DrawPiece(ME.x, ME.y, ME.curPiece[0], 0, ME.curColor[0], 1);
					ME.repeatTime[4] = ME.repeatTime[5] = 2; /* wait for release */
					break; /* change state */
				}
			}

			/* rotate left.  Rotating squares is pointless so don't even try. */
			if (ME.repeatTime[4 + ME.inverse] == 1
					&& ME.curPiece[0] != PIECE_SQUARE) {
				if (!CheckOverlap(ME.x, ME.y, ME.curPiece[0],
						(ME.curFlip + 3) & 3)) {
//            SendSound(gFlipSound);
					if (ME.vis)
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 0);
					ME.curFlip = (ME.curFlip + 3) & 3;
					ME.vis = 0;
					ME.dropMove = 0;
				}
			}

			// flip right
			if (ME.repeatTime[5 - ME.inverse] == 1
					&& ME.curPiece[0] != PIECE_SQUARE) {
				if (!CheckOverlap(ME.x, ME.y, ME.curPiece[0],
						(ME.curFlip + 1) & 3)) {
//            SendSound(gFlipSound);
					if (ME.vis)
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 0);
					ME.curFlip = (ME.curFlip + 1) & 3;
					ME.vis = 0;
					ME.dropMove = 0;
				}
			}

			if (ME.repeatTime[2 + ME.inverse] == 1
					|| ME.repeatTime[2 + ME.inverse] == 17) // move left
					{
				if (!CheckOverlap(ME.x - 1, ME.y, ME.curPiece[0], ME.curFlip)) {
//            Note(1, 81, 112, 0, 1);
					if (ME.vis)
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 0);
					ME.x--;
					ME.vis = 0;
					ME.dropMove = 0;
				} else
					ME.repeatTime[2] = 16; // let player slide the block in
			}

			if (ME.repeatTime[1]) // move down
			{
				if (ME.stateTime - retrace_count > 1) {
					ME.stateTime = retrace_count + 1;
					if (!g.tntMode)
						ME.score++;
					ME.dropMove = 0;
				}
			}

			if (ME.repeatTime[3 - ME.inverse] == 1
					|| ME.repeatTime[3 - ME.inverse] == 17) // move right
					{
				if (!CheckOverlap(ME.x + 1, ME.y, ME.curPiece[0], ME.curFlip)) {
//            Note(1, 81, 112, 0, 1);
					if (ME.vis)
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 0);
					ME.x++;
					ME.vis = 0;
				}
				/*
				 else if(gCheatCodes[2].flag && ME.repeatTime[1] != 0)
				 {
				 ME.stateTime = retrace_count;
				 }
				 */
				else
					ME.repeatTime[3] = 16; // let player slide the block in
			}

			if (ME.repeatTime[0] == 1 && ME.dropMove == 0) // drop
					{
				ME.vis = 0;
				ME.stateTime = retrace_count;
				DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
						ME.curColor[0], 0);
				do {
					ME.y++;
					if (!g.tntMode)
						ME.score++;
				} while (!CheckOverlap(ME.x, ME.y, ME.curPiece[0], ME.curFlip));
				ME.y--;
				if (!g.tntMode)
					ME.score--;
				ME.dropMove = 1;
			}

			// is it time to move the piece down 1?
			if (ME.stateTime - retrace_count <= 0) {
				if (CheckOverlap(ME.x, ME.y + 1, ME.curPiece[0], ME.curFlip)
						!= 0) {
					/* give the player some time to slide the piece in before
					 * it locks into place
					 */
					if (ME.stateTime - retrace_count <= -30) {
						if (ME.pieceDone == 0)
							ME.pieceDone = 1;
//              SendSound(gDropSound);

						/* check for a spinmove */ME.spinMove = CheckOverlap(
								ME.x + 1, ME.y, ME.curPiece[0], ME.curFlip)
								&& CheckOverlap(ME.x - 1, ME.y, ME.curPiece[0],
										ME.curFlip)
								&& CheckOverlap(ME.x, ME.y - 1, ME.curPiece[0],
										ME.curFlip);
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 2);
						ME.scoreFac = 0;
						ME.chainCount = 0;
						ME.state = STATE_CHECK4LINES;
					}
				} else {
					if (ME.vis)
						DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
								ME.curColor[0], 0);
					ME.vis--;
					ME.y++;
					if (raceMode)
						ME.stateTime = retrace_count
								+ DIFFICULTY
										/ (ME.lines + ME.handiLines
												+ INITIAL_LEVEL);
					else
						ME.stateTime = retrace_count
								+ 2 * DIFFICULTY
										/ (ME.lines + ME.handiLines
												+ 2 * INITIAL_LEVEL);
					ME.dropMove = 0;
				}
			}

			if (ME.vis != 1) {
				DrawPiece(ME.x, ME.y, ME.curPiece[0], ME.curFlip,
						ME.curColor[0], 1);
				ME.vis = 1;
			}
			break;

		case STATE_CHECK4LINES:
			/* check for lines */
			if (CheckLines(ME.spinMove)) {
				ME.state = STATE_FALL;
				ME.stateTime = retrace_count + 20;
			} else
				ME.state = STATE_PUSHUP;
			ME.dropMove = 0;
			break;

		case STATE_FALL:
			if (retrace_count - ME.stateTime >= 0) {
				ME.stateTime += 3;
				if (!CarbonStillGoing())
					ME.state = STATE_CHECK4LINES;
			}
			break;

		case STATE_PUSHUP:
			if (retrace_count - ME.stateTime >= 0) {
				ME.stateTime += 3;

				if (p[1].coming > p[0].coming) {
					p[1].coming -= p[0].coming;
					p[0].coming = 0;
				} else {
					p[0].coming -= p[1].coming;
					p[1].coming = 0;
				}

				if (ME.coming > 0) {
					PushUp();
					ME.coming--;
				} else {
					if (MarkCarbon(0))
						ME.state = STATE_FALL;
					else if (ME.pieceDone == 0)
						ME.state = STATE_FALLING_PIECE;
					else if (ME.top >= 0) {
						// remove inverse
						if (ME.scoreFac >= 1) {
							ME.inverse = 0;

							// play tetris sound and add an item
							if (ME.scoreFac >= 4) {
//                  SendSound(gTetrisSound);
								// If player has nothing in the item box, add an item, but
								// don't add an inverse in non-versus mode.
								if (ME.curPiece[4] < 0)
									ME.curPiece[4] = PIECE_MAGNET
											+ rand() % (attackMode + 1);
							}
						}
						ME.state = STATE_GET_NEW_PIECE;
					} else {
						if (attackMode)
							p[1 - curTurn].wins++;
						GameOver(curTurn);
						return 1;
					}
				}
			}
			break;
		case STATE_GAMEOVER:
			return 1;
		}
	}
	return 0;
}

/* GetNewPiece() ***********************
 * Places a new random piece in the piece queue.
 */
static void GetNewPiece(int depth) {
	if (g.tntMode) { /* tnt way (DEBUG ONLY) */
		int piece = ME.curPiece[depth] = rand() % 7;
		ME.curPiece[depth] = (piece == 6) ? 7 : piece;
		ME.curColor[depth] = gPieceColor[ME.curPiece[depth]];
	} else { /* tetanus way */
		ME.curPiece[depth] = rand()
				% (g.teflonMode ? NUM_PIECES - 3 : NUM_PIECES);
		ME.curColor[depth] = ((rand() >> 9) & 0x10) + BCOL_H;
	}
}

/* MarkCarbon() ************************
 * This function marks areas of blocks as either ground or floating by filling
 * contiguous areas with a value denoting to which area each block belongs.
 */
static short MarkCarbon(int spinMove) {
	int markX, markY;
	unsigned char q = 0;

	if (spinMove)
		/*SendSound(gTetrisSound)*/;

	for (markY = 0; markY < 20; markY++)
		for (markX = 0; markX < 10; markX++)
			ME.auxMap.b[markY][markX] = -(ME.blockMap.b[markY][markX] != 0);
	for (markX = 0; markX < 10; markX++)
		ME.auxMap.b[20][markX] = 255;

	for (markY = 20; markY >= 0; markY--)
		for (markX = 0; markX < 10; markX++)
			if (ME.auxMap.b[markY][markX] == 255) {
				if (spinMove) {
					if (markY == 20) // the bottom row should be all 1's
							{
						q = 1;
						ME.auxMap.b[markY][markX] = q;
					} else if (ME.auxMap.b[markY + 1][markX]) // booth's algorithm?
						ME.auxMap.b[markY][markX] =
								ME.auxMap.b[markY + 1][markX];
					else
						ME.auxMap.b[markY][markX] = ++q;
				} else
					TeFloodFill(markX, markY, ++q);
			}
	return (q > 1);
}

/* NewGame() ***************************
 * Sets up the grid and other parameters for a game of Tetanus.
 */
void NewGame(int pl) {
	int x, y;
	int oldTurn = curTurn;

	curTurn = pl;
	/*
	 alert("NewGame() called. displaying", "backbuf, backbits, tetbits", "",
	 "ok", 0, 13, 0);

	 blit(ME.seven->backBuf, ME.seven->backBuf, xDest, yDest, xDest, yDest, 8, 8);
	 readkey();
	 blit(g.backbits, ME.seven->backBuf, xDest, yDest, xDest, yDest, 8, 8);
	 readkey();
	 blit(tetbits, ME.seven->backBuf, xSrc, ySrc, xDest, yDest, 8, 8);
	 readkey();
	 */
	/* set up initial next pieces */
	for (y = 1; y <= 4; y++) {
		GetNewPiece(y);
	}

	/* clear field */
	for (y = 0; y < 20; y++) {
		for (x = 0; x < 10; x++) {
			p[pl].blockMap.b[y][x] = p[pl].auxMap.b[y][x] = 0;
			TeDrawBlock(x, y, 0);
		}
	}

	/* init variables */
	p[pl].score = p[pl].lines = p[pl].coming = p[pl].inverse = 0;
	ME.top = 20;
	ME.state = STATE_GET_NEW_PIECE;
	srand(ME.gameStart = retrace_count);
	ME.repeatTime[6] = 5;

	p[pl].gameStarted = time(0);

	/* add lines for handicap */
	y = g.handicap;
	if (curTurn == 1) /* negative handicaps hurt player 2 */
		y = -y;
	if (y > 0)
		ME.handiLines = y * 10;
	else
		ME.handiLines = 0;
	while (y-- > 0)
		PushUp();

	/* hack to make sure autorepeat doesn't fuck us */
	for (y = 0; y < 8; y++)
		ME.repeatTime[y] = 1;

	curTurn = oldTurn;
}

/* CheckOverlap() *********************
 * Is the piece overlapping something?
 */
static char CheckOverlap(short x, short y, short inPiece, short inFlip) {
	int g, j, k;

	for (g = 0; g < NUM_BLOCKS; g++)
		if (gCBlocks[inPiece][inFlip][g]) // some pieces do not have all the blocks
		{
			j = y + gYBlocks[inPiece][inFlip][g];
			k = x + gXBlocks[inPiece][inFlip][g];
			if (k < 0 || k > 9 || j > 19)
				return 1;
			else if (j >= 0 && ME.blockMap.b[j][k])
				return 1;
		}
	return 0;
}

static void PushUp(void) {
	int x, e, d, g = 0;

	for (x = 0; x < 10; x++) {
		e = 666;
		for (g = 1; g < 20; g++) {
			d = e;
			e = ME.blockMap.b[g][x];
			if (e != d) {
				ME.blockMap.b[g - 1][x] = e;
				TeDrawBlock(x, g - 1, e);
			}
		}
		ME.blockMap.b[19][x] = BCOL_NEUTRAL;
	}

	ME.blockMap.b[19][rand() % 10] = 0;
	for (x = 0; x < 10; x++)
		TeDrawBlock(x, 19, ME.blockMap.b[19][x]);
}

/* ScrollDown() ************************
 * Nintendo's Tetris does this instead of CarbonStillGoing(). I'll put it back
 * in when I implement Purist Mode.
 */
/*
 static void ScrollDown(short top, short bottom)
 {
 short x, e, d, g;

 for(x = 0; x < 10; x++)
 {
 e = 66;
 for (g = bottom; g >= top; g--)
 {
 d = e;
 if(!g)
 e = 0;
 else
 e = ME.blockMap.b[g - 1][x];
 if(e != d || g == bottom)
 {
 if(e == -1)
 e = 0;
 ME.blockMap.b[g][x] = e;
 TeDrawBlock(x, g, e);
 }
 }
 }
 top++;
 }
 */

/* TeDrawBlock() **********************
 * Draws a colored block to the screen.  THIS MUST BE PORTED.
 */
static void TeDrawBlock(int x, int y, unsigned colour) {
	int xDest = x * 8 + 88;
	int yDest = y * 8 + 48;
	int xSrc = (colour & BCON_ALL) << 3;
	int ySrc = (colour & BCOL_ALL) >> 1;

	if (y >= 0 && y < 20) {
		if (colour == 0 && g.backbits != NULL)
			blit(g.backbits, ME.seven->backBuf, xDest, yDest, xDest, yDest, 8,
					8);
		else
			blit(tetbits, ME.seven->backBuf, xSrc, ySrc, xDest, yDest, 8, 8);
	}
}

