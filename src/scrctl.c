#include "tod.h"
#include "hgrcos.h"
#include <string.h>

/* Format of screen control word:

                  3322222222221111 1111110000000000
                  1098765432109876 5432109876543210
                  |||||||||||||||| ||||||||||||||||
                 -+||||||||||||||| |||||||||||||||+-clockwise
                 --+|||||||||||||| ||||||||||||||+--counterclockwise
                 ---+||||||||||||| |||||||||||||+---out
                 ----+|||||||||||| ||||||||||||+----in
                      |||||||||||| ||||||||||||
                   ---+||||||||||| |||||||||||+--- spin down
                   ----+|||||||||| ||||||||||+---- spin up
                   -----+||||||||| |||||||||+----- sinus amp -
                   ------+|||||||| ||||||||+------ sinus amp +
                          |||||||| ||||||||
                     -----+||||||| |||||||+----- shift right
                     ------+|||||| ||||||+------ shift left
                     -------+||||| |||||+------- shear \
                     --------+|||| ||||+-------- shear /
                              |||| ||||
                       -------+||| |||+------- shear )
                       --------+|| ||+-------- shear (
                  heat ---------+| |+--------- shear Z
                 noise ----------+ +---------- shear S

*/



/* ASSUMES unsigned int is the 32-bit datatype */
void CtlWord2Screen(ScreenPos *pos, unsigned int ctlword)
{
  if(ctlword & 0x00000001)
    pos->dtheta += 0x400000 / VIRTUAL_FPS;
  if(ctlword & 0x00000002)
    pos->dtheta -= 0x400000 / VIRTUAL_FPS;
  if(ctlword & 0x00000004)
    pos->dscale -= 0x20000 / VIRTUAL_FPS;
  if(ctlword & 0x00000008)
    pos->dscale += 0x20000 / VIRTUAL_FPS;
  if(ctlword & 0x00000010)
    pos->dphi -= 0x190000 / VIRTUAL_FPS;
  if(ctlword & 0x00000020)
    pos->dphi += 0x190000 / VIRTUAL_FPS;
  if(ctlword & 0x00000040)
    pos->damp -= 0x20000 / VIRTUAL_FPS;
  if(ctlword & 0x00000080)
    pos->damp += 0x20000 / VIRTUAL_FPS;
  if(ctlword & 0x00000100)
    pos->dd -= 0x6000 / VIRTUAL_FPS;
  if(ctlword & 0x00000200)
    pos->dd += 0x6000 / VIRTUAL_FPS;
  if(ctlword & 0x00000400)
    pos->dc -= 0x8000 / VIRTUAL_FPS;
  if(ctlword & 0x00000800)
    pos->dc += 0x8000 / VIRTUAL_FPS;
  if(ctlword & 0x00001000)
    pos->db -= 0xe000 / VIRTUAL_FPS;
  if(ctlword & 0x00002000)
    pos->db += 0xe000 / VIRTUAL_FPS;
  if(ctlword & 0x00004000)
    pos->da -= 0x14000 / VIRTUAL_FPS;
  if(ctlword & 0x00008000)
    pos->da += 0x14000 / VIRTUAL_FPS;
  if(ctlword & 0x00010000)
    pos->noise += 0x10000 / VIRTUAL_FPS;
  if(ctlword & 0x00020000)
    pos->heat += 0x400 / VIRTUAL_FPS;
  if(ctlword & 0x00040000);
  if(ctlword & 0x00080000);

}

static const int ctlword_keys[32] =
{
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  ALLEGRO_KEY_Q, ALLEGRO_KEY_A, ALLEGRO_KEY_W, ALLEGRO_KEY_S,
  ALLEGRO_KEY_E, ALLEGRO_KEY_D, ALLEGRO_KEY_R, ALLEGRO_KEY_F,
  ALLEGRO_KEY_C, ALLEGRO_KEY_V, ALLEGRO_KEY_T, ALLEGRO_KEY_G,
  ALLEGRO_KEY_I, ALLEGRO_KEY_K, ALLEGRO_KEY_J, ALLEGRO_KEY_L
};


unsigned int Keys2CtlWord(void)
{
  unsigned int a = 0;
  unsigned int i;

  for(i = 0; i < 32; i++)
  {
    a <<= 1;
    if(ctlword_keys[i])
      if(key[ctlword_keys[i]])
        a++;
  }
  return a;
}



/*
static const int morph_lands[][2] =
{
  { 16,  1},
  { 32,  1}, // he got flat baby
  { 40,  1},
  { 48,  1}, // a hot kiss honey
  { 56,  1},
  { 64,  1}, // i'm such a swine baby
  { 72,  1},
  { 76,  1},
  { 80,  1},
  { 88,  1}, // a hot buzz baby
  { 92,  1},
  { 96,  1},
  {104,  1},
  {108,  1}
};

static const int idltd_lands[][2] =
{
  { 32,  1},
  { 48,  1},
  { 64,  2},
  { 80,  2},
  { 96,  1}, // you and i are underdosed and we're
  {104,  1}, // ready to fall
  {112,  1}, // raised to be stupid taught to be
  {120,  1}, // nothing at all
  {132,  1},
  {138,  1},
  {144,  1},
  {149,  1},
  {154,  1},
  {158,  1},
  {162,  1},
  {166,  1},
  {170,  1},
  {174,  1},
  {177,  1},
  {180,  1},
  {183,  1},
  {186,  1},
  {189,  1},
  {192,  1}
};

static const int pressure_lands[][2] =
{
  { 44,  1}, // pace yourself
  { 60,  1}, // everybody else
  { 76,  2},
  { 92,  2}, 
  { 96,  1}, // so far
  {112,  1},
  {120,  1},
  {128,  1},
  {136,  1},
  {144,  1}, // pressure
  {148,  1},
  {152,  1},
  {156,  1},
  {160,  1}, // pressure
  {164,  1},
  {168,  1},
  {172,  1},
  {176,  2}, // pressure
  {180,  2},
  {184,  2},
  {188,  2},
  {192,  2}, // pressure
  {196,  2},
  {200,  2}
};
*/
typedef struct Song
{
  char title[32];
  char play1st[32];
  char name[32];
  char lands[32];
} Song;

static const Song songs[2] =
{
  { "I Don't Like", "IDLTD_INTRO", "IDLTD_MID", "IDLTD_MANA" },
  { "Kalinka", "KALINKA_INTRO", "KALINKA_MID", "KALINKA_MANA" }
/*
  { "Morphine", "no", "MORPHINE_MID",
    (sizeof(morph_lands) / sizeof(morph_lands[0])), morph_lands },
  { "Pressure", "no", "PRESSURE_MID",
    (sizeof(pressure_lands) / sizeof(pressure_lands[0])), pressure_lands }
*/
};

static int intoLoop = 0;
static DATAFILE *songdat;

int intro_lands[1024], loop_lands[1024];
int intro_len, loop_len;

void LoadLands(int songNo, DATAFILE *dat)
{
  DATAFILE *item;
  char *linebuf;
  const char *tok;
  int inLoop = 0;

  intro_len = loop_len = 0;

  item = find_datafile_object(dat, songs[songNo].lands);
  if(!item)
    return;

  linebuf = malloc(item->size + 1);
  if(!linebuf)
    return;

  memcpy(linebuf, item->dat, item->size);
  linebuf[item->size + 1] = 0;


  for(tok = strtok(linebuf, " ,\n"); tok != 0; tok = strtok(0, " ,\n"))
  {
    /* look for spacer */
    if(tok[0] == '=')
      inLoop = 1;
    else
    {
      int n = strtol(tok, NULL, 10);
      if(n > 0)
      {
        if(inLoop)
          loop_lands[loop_len++] = n;
        else
          intro_lands[intro_len++] = n;
      }
    }
  }
  free(linebuf);
}


#define TOTAL_EFFECTS 10

void StartSong(int songNo, DATAFILE *dat, ScreenPos *pos)
{
  static const int initEffect[TOTAL_EFFECTS] = {0, 1, 2, 7, 9, 8, 5, 4, 6, 3};

  MIDI *mid = GetResource(dat, songs[songNo].play1st);
  int i;


  LoadLands(songNo, dat);

  if(mid)
    play_midi(mid, FALSE);

  songdat = dat;

  pos->lastBeat = -1;
  intoLoop = 0;
  pos->midiLoop = 0;
  pos->curSong = songNo;
  pos->mana = 0;
  for(i = 0; i < nPlayers; i++)
  {
    pos->a = pos->da = pos->b = pos->db = pos->phi = pos->dphi = 0;
    pos->c = pos->dc = pos->d = pos->dd = pos->amp = pos->damp = 0;
    pos->theta = pos->dtheta = pos->dscale = pos->heat = 0;
    pos->noise = 0x400000;
  }

  for(i = 0; i < TOTAL_EFFECTS; i++)
    pos->fxQueue[i] = initEffect[i];
}


static int compare_ints(const void *ckey, const void *celem)
{
  int curBeat = *(const int *)ckey;
  int thatBeat = *(const int *)celem;

  return curBeat - thatBeat;
}

unsigned int Mana2CtlWord(ScreenPos *pos, unsigned int beat)
{
  unsigned ctl = 0;
  int *found;
  int i;
  int *song_lands;
  int song_len;

  if(intoLoop == 0)
  {
    song_lands = intro_lands;
    song_len = intro_len;

    if(midi_pos < 0)
    {
      MIDI *mid = GetResource(songdat, songs[pos->curSong].name);

      if(mid)
        play_midi(mid, TRUE);
      intoLoop = 1;
    }
  }
  else
  {
    song_lands = loop_lands;
    song_len = loop_len;
  }

  // build keys
  i = pos->nEffects;
  while(i-- > 0)
  {
    switch(pos->fxQueue[i])
    {
      case 0:
        if(pos->dtheta > 0)
          ctl |= 0x00000001;
        else
          ctl |= 0x00000002;
        break;
      case 1:
        if(pos->dscale < 0)
          ctl |= 0x00000004;
        else
          ctl |= 0x00000008;
        break;
      case 2:
        if(pos->dphi < 0)
          ctl |= 0x00000010;
        else
          ctl |= 0x00000020;
        break;
      case 3:
        if(pos->damp < 0)
          ctl |= 0x00000040;
        else
          ctl |= 0x00000080;
        break;
      case 4:
        if(pos->dd < 0)
          ctl |= 0x00000100;
        else
          ctl |= 0x00000200;
        break;
      case 5:
        if(pos->dc < 0)
          ctl |= 0x00000400;
        else
          ctl |= 0x00000800;
        break;
      case 6:
        if(pos->db < 0)
          ctl |= 0x00001000;
        else
          ctl |= 0x00002000;
        break;
      case 7:
        if(pos->da < 0)
          ctl |= 0x00004000;
        else
          ctl |= 0x00008000;
        break;
      case 8:
        ctl |= 0x00010000;
        break;
      case 9:
        ctl |= 0x00020000;
        break;
    }
  }

  if(beat == pos->lastBeat)
    return ctl;

  if(beat < pos->lastBeat)
    pos->midiLoop++;
  pos->lastBeat = beat;

  if(beat < song_lands[0])
  {
    pos->nEffects = 0;
    return 0;
  }

  pos->nEffects = (pos->mana + 7) / 8;
  if(pos->nEffects > 7)
    pos->nEffects = 7;
  pos->mana -= pos->nEffects;


  found = bsearch(&beat, song_lands, song_len,
                  sizeof(int), compare_ints);
  if(found)
  {
    int maxEffect = pos->midiLoop * 2;
    int r1, r2;

    if(maxEffect > TOTAL_EFFECTS)   // switch effects
      maxEffect = TOTAL_EFFECTS;
    r1 = rand() % maxEffect;
    r2 = pos->fxQueue[0];
    pos->fxQueue[0] = pos->fxQueue[1];
    pos->fxQueue[1] = pos->fxQueue[r1];
    pos->fxQueue[r1] = r2;

    pos->mana += pos->midiLoop; // collect mana

    pos->nEffects = 1;
  }

  return ctl;
}


void Resonance(ScreenPos *pos)
{
  pos->fricCounter -= VIRTUAL_FPS;

  if(pos->scale < 0) // flip
  {
    pos->scale = -pos->scale;
    pos->dscale = -pos->dscale;
    pos->theta = (pos->theta & 0xffffff) ^ 0x801000;
  }
  pos->dscale = (pos->dscale + (54614 - pos->scale)) * 19/20;
  pos->da = pos->da * 15/16;
  pos->db = pos->db * 15/16;
  pos->dc = pos->dc * 15/16;
  pos->dd = pos->dd * 15/16;
  pos->damp = pos->damp * 15/16;
  pos->dtheta = pos->dtheta * 31/32; /* momentum for spinning needs low friction */
  pos->dphi = pos->dphi * 63/64;

  pos->da -= pos->a / 4;
  pos->db -= pos->b / 4;
  pos->dc -= pos->c / 4;
  pos->dd -= pos->d / 4;
  pos->damp -= pos->amp / 8;
  pos->dtheta -= hgrsin(pos->theta) * 32; /* the so-called "weeble mode" */
  pos->dphi -= hgrsin(pos->phi) * 32;

  if(pos->midiLoop < 1)
  {
    pos->da = pos->da * 15/16;
    pos->db = pos->db * 15/16;
    pos->dc = pos->dc * 15/16;
    pos->dd = pos->dd * 15/16;
    pos->damp = pos->damp * 15/16;
    pos->dtheta = pos->dtheta * 31/32;
    pos->dphi = pos->dphi * 63/64;
  }

  pos->noise = pos->noise * 9/10;
  pos->heat = pos->heat * 9/10;
}

