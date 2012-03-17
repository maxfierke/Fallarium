/**************************************\
* hiscore.c                            *
* High score file routines             *
*                                      ********************************\
* Copyright 2000 Damian Yerrick                                        *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This program is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program; if not, write to the Free Software          *
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
* Or view the License online at http://www.gnu.org/copyleft/gpl.html   *
*                                                                      *
* Damian Yerrick's World Wide Web pages are located at                 *
*   http://www.PinEight.com                                            *
*                                                                      *
\**********************************************************************/

#include "hiscore.h"
#include "allegdlg.h"


/* Private data structures */

typedef struct HiPlayer
{
  char playerName[20];
  unsigned int password; /* CRC-32 of player's password */
  unsigned int nGames;
} HiPlayer;

/* basic disk format of HiPlayer data structure:
   All ints are big-endian.
   (I'm not telling you how it's encrypted; you'll have to read the
   fucking source for that.)

(4 bytes) crc-32 of following data, big-endian
(4 to 17 bytes) playerName, null terminated
(4 bytes) password
(4 bytes) game ID

*/


/* Private globals */

static HiPlayer *curPlayer = 0;

unsigned long curPlayerFilename = 0;
/* How the fuck are you storing the filename in a long?
 *
 * It's easy: the filename is the hex of the crc32 of the player's name.
 * sprintf(filename, "%08lx.ljp", strcrc(playerName, 16));
 */
char playersPath[256];
DATAFILE *theData;
BITMAP *todlogo, *spin8;
int loggedIn = 0;

int InitScore(DATAFILE *dat)
{
  char buf1[256];
  DATAFILE *obj;
                           
  get_executable_name(buf1, 255);
  replace_filename(playersPath, buf1, "plyr/", 255);

  obj = find_datafile_object(dat, "LOGIN_BMP");
  if(!obj)
    return -1;
  todlogo = obj->dat;

  obj = find_datafile_object(dat, "SPIN8");
  if(!obj)
    return -1;
  spin8 = obj->dat;



  return 0;
}


/* LoginDialog() ***********************
 * Presents a player login dialog box.
 * Returns -1 for cancel or 0 for enter.
 */
static int LoginDialog(char *aimsn, char *pass)
{
  DIALOG dlg[] =
  {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_box_proc,        0,    0,    320,  200,  0,    0,    0,    0,       0,    0,    NULL },
   { d_button_proc,     170,  184,  60,   12,   63,   0,    27,   D_EXIT,  0,    0,    "Cancel"},
   { d_edit_proc,       128,  104,  136,  8,    63,   2,    'n',  D_EXIT,  16,   0,    aimsn},
   { DY_pass_proc,      128,  116,  136,  8,    63,   2,    'p',  D_EXIT,  16,   0,    pass},
   { d_text_proc,       24,   104,  104,  16,   63,   0,    0,    0,       0,    0,    "Screen &name:"},
   { d_text_proc,       24,   116,  104,  16,   63,   0,    0,    0,       0,    0,    "&Password:"},
   { DY_bitmap_proc,    0,    0,    320,  100,  63,   0,    0,    0,       0,    0,    todlogo },
   { DY_idle_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL }
  };

  int pressed;

  centre_dialog(dlg);

  pressed = do_dialog(dlg, 2);

  if(pressed == 1)
    return -1;

  else
    return 0;
}

int Login(char *aimsn)
{
  char pass[17] = {0};
//  int done;

  return LoginDialog(aimsn, pass);
}

int AddScore(const HiGame *game)
{
  return -1;
}


