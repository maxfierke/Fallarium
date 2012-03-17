#include <stdio.h>
#include "tod.h"
#include "hgrcos.h"
#include "hiscore.h"
#include <time.h>

#define FRAMERATE 60

int windowWid = 320;
int windowHt  = 200;

#define NUM_RES 8

int xRes[NUM_RES] = {320, 320, 400, 512, 640, 640, 800,1024};
int yRes[NUM_RES] = {200, 240, 300, 384, 400, 480, 600, 768};
char lucidName[NUM_RES][NUM_RES] =
{
  "LUCID12", "LUCID12", "LUCID16", "LUCID20",
  "LUCID24", "LUCID24", "LUCID32", "LUCID40"
};

int playing_midi = 1;


/* deformities:

So far we have eight degrees of freedom synchronized to the music.
(Tetripz seems to have had about five.)

InitSidesPhi()
        scaling (old)
        x axis rotation (new)
DisplaceScanlines()
        ay^3 scanline displacement (new)
        by^2 scanline displacement (old)
        cy scanline displacement (new)
        d scanline displacement (new)
        sinusoidal x shift phase, amplitude, frequency (old)
polynomial x shift: DisplaceScanlines()
z axis rotation: RotateSides()

Currently unimplemented:

sinusoidal x scaling (new)
polynomial x scaling (new)
InitSidesPoly() : scaling + polynomial scanline displacement (new)

Degrees of freedom synchronized to the gameplay:
DisplaceSides()
        x shift (old)
        y shift (old)

Game modes:
        Classic (old)
        Block Hole (new)

*/

RGB *pal;
volatile Timers timers;
BITMAP *tetbits;
FONT *lucid;


void timerint(void)
{
  if(!timers.paused)
  {
    timers.odo++;
    timers.trip++;
  }
} END_OF_FUNCTION(timerint);

void *GetResource(DATAFILE *dat, const char *name)
{
  DATAFILE *that = find_datafile_object(dat, (char *)name);
  return that ? that->dat : NULL;
}


/* On some Windows 9x versions + video drivers + DirectX versions +
 * Allegro versions, clear() and its workhorse clear_to_color()
 * cause the program to dump core.  This doesn't.
 */
void clear2color(BITMAP *bmp, int color)
{
  rectfill(bmp, 0, 0, bmp->w - 1, bmp->h - 1, color);
}


static char *GetRes_List(int index, int *list_size)
{
  static char resname[16];
  if(index < 0)
  {
    *list_size = NUM_RES;
    return NULL;
  }
  else
  {
    sprintf(resname, "%4dx%3d", xRes[index], yRes[index]);
    return resname;
  }
}

int GetRes(char *aimsn)
{
  int chosen;

  DIALOG dlg[] =
  {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_box_proc,        0,    0,    288,  176,  0,    7,    0,    0,       0,    0,    NULL },
   { d_button_proc,     208,  160,  48,   12,   0,    7,    13,   D_EXIT,  0,    0,    "Go"},
   { d_button_proc,     144,  160,  64,   12,   0,    7,    27,   D_EXIT,  0,    0,    "Cancel"},
   { d_list_proc,       96,   27,   80,   70,   0,    7,    'r',  D_EXIT,  0,    0,    GetRes_List },
   { DY_check_proc,     24,   108,  144,  8,    0,    7,    'm',  D_SELECTED,0,  0,    "Play &MIDI"},
   { DY_check_proc,     24,   120,  144,  8,    0,    7,    'j',  0,       0,    0,    "Use &joysticks"},
   { d_text_proc,       24,   27,   88,   16,   0,    7,    0,    0,       0,    0,    "&Screen:"},
   { d_text_proc,       24,   10,   136,  16,   0,    7,    0,    0,       0,    0,    "tetanus on drugs."},
   { DY_idle_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL }
  };

  /* Some VGAs garble 400x300 without returning error. */
  if(set_gfx_mode(GFX_AUTODETECT, 320, 240, 0, 0) < 0)
    if(set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0) < 0)
      return -1;

  centre_dialog(dlg);
  install_keyboard();
  install_joystick(JOY_TYPE_AUTODETECT);
  if(num_joysticks == 0)
    dlg[5].flags |= D_DISABLED;
  else
    dlg[5].flags |= D_SELECTED;

  if(popup_dialog(dlg, 3) == 2)
  {
    set_gfx_mode(GFX_TEXT, 80, 50, 0, 0);
    return -1;
  }

  chosen = dlg[3].d1;
  playing_midi = (dlg[4].flags & D_SELECTED) != 0;
  g.usingJoy = (dlg[5].flags & D_SELECTED) != 0;

  windowWid = xRes[chosen];
  windowHt = yRes[chosen];

  if(set_gfx_mode(GFX_AUTODETECT, xRes[chosen], yRes[chosen], 0, 0) < 0)
  {
    set_gfx_mode(GFX_TEXT, 80, 50, 0, 0);
    return -1;
  }

  return chosen;
}


int GetOpts(char *aimsn, BITMAP *todlogo)
{
  DIALOG dlg[] =
  {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_box_proc,        0,    0,    320,  200,  0,    0,    0,    0,       0,    0,    NULL },
   { d_button_proc,     240,  184,  60,   12,   63,   0,    13,   D_EXIT,  0,    0,    "Play"},
   { d_button_proc,     170,  184,  60,   12,   63,   0,    27,   D_EXIT,  0,    0,    "Cancel"},
   { DY_check_proc,     24,   120,  144,  8,    63,   0,    't',  0,       0,    0,    "&The New Tetanus"},
   { DY_check_proc,     24,   132,  144,  8,    63,   0,    '2',  0,       0,    0,    "&2-player (prealpha)"},
   { DY_bitmap_proc,    0,    0,    320,  100,  63,   0,    0,    0,       0,    0,    todlogo },
   { d_text_proc,       128,  104,  136,  8,    63,   0,    'n',  D_EXIT,  16,   0,    aimsn},
   { DY_idle_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL }
  };

  stop_midi();

  if(g.tntMode)
    dlg[3].flags |= D_SELECTED;
  if(nPlayers == 2)
    dlg[4].flags |= D_SELECTED;

  centre_dialog(dlg);
  install_keyboard();
  install_joystick(JOY_TYPE_AUTODETECT);

  if(popup_dialog(dlg, 3) == 2)
  {
    return -1;
  }

  g.tntMode = (dlg[3].flags & D_SELECTED) != 0;
  nPlayers = (dlg[4].flags & D_SELECTED) ? 2 : 1;

  return 0;
}


void SetupLight1(void)
{
  int x, y;
  int c1, c2;

  c1 = getpixel(g.backbits, 0, 0);
  c2 = getpixel(g.backbits, 1, 0);
  putpixel(g.backbits, 1, 0, c1);

  for(y = 0; y < 256; y++)
  {
    char *line = g.backbits->line[y];

    for(x = 0; x < 256; x++)
    {
      if(line[x] == c1)
      {
        fixed f = fsqrt((x - 128) * (x - 128) + (y - 128) * (y - 128)); /* between 0 and 2 */
        fixed cosine = hgrcos(f << 10);
        unsigned c = (cosine + 0x10000 + rand() % 0x1000) * 15 / 32768;
        if(c > 63)
          c = 63;
        line[x] = c + 64;
      }
      else if(line[x] == c2)
      {
        fixed f = fsqrt((x - 128) * (x - 128) + (y - 128) * (y - 128)) +
                  (fcos(x << 20) + fcos(y << 20)) / 64 + 0x10000;
                  /* between 0 and 4 */
        fixed cosine = hgrcos(f << 11);
        unsigned c = (cosine + 0x10000 + rand() % 0x1000) * 15 / 32768;
        if(c > 63)
          c = 63;
        line[x] = c + 128;
      }
    }
  }

  for(y = 0; y < 64; y++)
  {
    pal[y + 64].r = pal[y + 64].g = 0;
    pal[y + 64].b = 63;
    pal[y + 128].r = 31;
    pal[y + 128].g = pal[y + 128].b = 0;
  }
}


#if 0
/* No longer using ThetaLight(), as it added very little effect */

#define THETALIGHT_AMBIENT 15
#define THETALIGHT_POWER (63 - THETALIGHT_AMBIENT)

void ThetaLight(fixed theta)
{
  unsigned r, g, b;
  fixed s = (hgrsin(theta) >> 1) + 0x8000;
  fixed c = (hgrcos(theta) >> 1) + 0x8000;
  unsigned i = 192;
  unsigned side;
  unsigned char zpixel[8];

  /* 012
   * 3 4
   * 567
   */
  zpixel[1] = THETALIGHT_AMBIENT + fmul(THETALIGHT_POWER, s);
  zpixel[3] = THETALIGHT_AMBIENT + fmul(THETALIGHT_POWER, 0x10000 - c);
  zpixel[4] = THETALIGHT_AMBIENT + fmul(THETALIGHT_POWER, c);
  zpixel[6] = THETALIGHT_AMBIENT + fmul(THETALIGHT_POWER, 0x10000 - s);

  zpixel[0] = (zpixel[1] + zpixel[3])/2;
  zpixel[2] = (zpixel[1] + zpixel[4])/2;
  zpixel[5] = (zpixel[6] + zpixel[3])/2;
  zpixel[7] = (zpixel[6] + zpixel[4])/2;

  for(r = 0; r < 2; r++)
    for(g = 0; g < 2; g++)
      for(b = 0; b < 2; b++)
        for(side = 0; side < 8; side++)
        {
          if(r)
            pal[i].r = zpixel[side];
          else
            pal[i].r = 0;
          if(g)
            pal[i].g = zpixel[side];
          else
            pal[i].g = 0;
          if(b)
            pal[i].b = zpixel[side];
          else
            pal[i].b = 0;
          i++;
        }
}
#endif


void InitSidesPhi(Seven *seven, fixed phi, fixed zBase)
{
  int i;
  int wHt = seven->frontBuf->h;
  fixed m, zCos;
  int scanlineWidth = 160 / nPlayers;

  if(fcos(phi) == 0)
  {
    phi -= 0x8000;
  }
  zCos = fdiv(zBase, hgrcos(phi));
  m = hgrtan(phi);

  for(i = 0; i < wHt; i++)
  {
    fixed scanline = fdiv(i - wHt / 2, wHt / 2);
    fixed mline = fmul(m, scanline);
    fixed zLine = fmul(zCos, scanline);
    fixed y1 = fdiv(zLine, mline + 0x10000);
    fixed x1 = fdiv(zBase, mline + 0x10000);

    if(x1 < 0)
    {
      seven->leftX[i] = seven->rightX[i] = 0;
      seven->leftY[i] = seven->rightY[i] = 0;
    }
    else
    {
      seven->leftY[i] = seven->rightY[i] = 120 * y1;
      seven->leftX[i] = -x1 * scanlineWidth;
      seven->rightX[i] = x1 * scanlineWidth;
    }
  }
}

void SevenRender(Seven *seven)
{
  unsigned char **back = seven->backBuf->line;
  unsigned char **front = seven->frontBuf->line;
  int x = 0, y = 0;
  fixed xgap = 0, ygap = 0, sx, sy;
  int width = seven->rightSide - seven->leftSide;
  int height = seven->frontBuf->h;
  unsigned char *dst;

  for(y = 0; y < height; y++)
  {
    sx = (seven->leftX[y] + seven->rightX[y]) / 2;
    xgap = (seven->rightX[y] - seven->leftX[y]) /  width;
    sx -= xgap * width / 2;
    sy = (seven->leftY[y] + seven->rightY[y]) / 2;
    ygap = (seven->rightY[y] - seven->leftY[y]) /  width;
    sy -= ygap * width / 2;
    dst = front[y] + seven->leftSide;
    if(xgap || ygap)
    {
      x = width / 4;
      do
      {
        *dst++ = back[(sy >> 16) & 0xff][(sx >> 16) & 0xff];
        sy += ygap;
        sx += xgap;
        *dst++ = back[(sy >> 16) & 0xff][(sx >> 16) & 0xff];
        sy += ygap;
        sx += xgap;
        *dst++ = back[(sy >> 16) & 0xff][(sx >> 16) & 0xff];
        sy += ygap;
        sx += xgap;
        *dst++ = back[(sy >> 16) & 0xff][(sx >> 16) & 0xff];
        sy += ygap;
        sx += xgap;
      } while(--x);
    }
    else
      memset(dst, 15, width);
  }
}

void RotateSides(Seven *seven, int theta)
{
  fixed lx, ly, rx, ry;
  fixed c = hgrcos(theta);
  fixed s = hgrsin(theta);
  int i;
  int wHt = seven->frontBuf->h;

  for(i = 0; i < wHt; i++)
  {
    lx = seven->leftX[i];
    rx = seven->rightX[i];
    ly = seven->leftY[i];
    ry = seven->rightY[i];

    seven->leftX[i]  = fmul(lx, c) - fmul(ly, s);
    seven->rightX[i] = fmul(rx, c) - fmul(ry, s);
    seven->leftY[i]  = fmul(ly, c) + fmul(lx, s);
    seven->rightY[i] = fmul(ry, c) + fmul(rx, s);
  }
}

void ScaleSides(Seven *seven, int fac)
{
  int i;
  int wHt = seven->frontBuf->h;

  for(i = 0; i < wHt; i++)
  {
    seven->leftX[i]  = fmul(seven->leftX[i],  fac);
    seven->leftY[i]  = fmul(seven->leftY[i],  fac);
    seven->rightX[i] = fmul(seven->rightX[i], fac);
    seven->rightY[i] = fmul(seven->rightY[i], fac);
  }
}

void DisplaceSides(Seven *seven, fixed x, fixed y)
{
  int i;
  int wHt = seven->frontBuf->h;

  for(i = 0; i < wHt; i++)
  {
    seven->leftX[i]  += x;
    seven->leftY[i]  += y;
    seven->rightX[i] += x;
    seven->rightY[i] += y;
  }
}

void DisplaceScanlines(Seven *seven, int ticks,
                       int a, int b, int c, int d,
                       fixed dtheta, fixed amplitude,
                       fixed noise, fixed heat)
{
  fixed lx, ly, rx, ry;
  fixed xOff, yOff;
  int i, y;

  int wHt = seven->frontBuf->h;
  fixed dheat = 0x8000000 / wHt;
  fixed theta = (ticks << 14) & 0x00ffffff;
  fixed heatTheta = (ticks << 16) & 0x00ffffff;

  dtheta /= wHt;

  for(i = 0; i < wHt; i++)
  {
    y = i - wHt / 2;
    xOff = ((a * y / wHt + b) * y / wHt + c) * y / wHt + d +
           fmul(hgrcos(theta), amplitude);
    xOff += fmul((rand() & 0x7ff) - 0x400, noise);
    theta += dtheta;
    heatTheta += dheat;

    yOff = fmul(hgrsin(heatTheta), heat);
  
    lx = seven->leftX[i];
    rx = seven->rightX[i];
    ly = seven->leftY[i];
    ry = seven->rightY[i];

    seven->leftX[i]  = lx + fmul(rx - lx, xOff) - fmul(ry - ly, yOff);
    seven->rightX[i] = rx + fmul(rx - lx, xOff) - fmul(ry - ly, yOff);
    seven->leftY[i]  = ly + fmul(ry - ly, xOff) + fmul(rx - lx, yOff);
    seven->rightY[i] = ry + fmul(ry - ly, xOff) + fmul(rx - lx, yOff);
  }
}

void textout_shadow(BITMAP *screen, const FONT *font, const char *str, int x, int y, int c1, int c2)
{
    al_draw_text((FONT *)font, c2, x + 1, y + 1, ALLEGRO_ALIGN_LEFT, (char *)str);
    al_draw_text((FONT *)font, c1, x, y, ALLEGRO_ALIGN_LEFT, (char *)str);
}


static void wrapblit(BITMAP *a, BITMAP *b, int c, int d, int e, int f, int g, int h)
{
  blit(a, b, c, d, e, f, g, h);
}

static void PlayGame(Seven *seven, ScreenPos *pos, const char *aimsn)
{
  int lastTimer = timers.odo;
  int lastGameloop = timers.odo;
  int lastFrame = timers.odo;
  int done = 0;
  unsigned framesDrawn = 0;
  unsigned totalFrames = 0;
  const int lead = SCREEN_H / 14;

  int i, j;

  while(!done)
  {
    while(timers.odo - lastFrame > 0)
    {
      unsigned int cw = 0;

      lastFrame++;

      for(i = 0; i < nPlayers; i++)
      {
        cw = Mana2CtlWord(&(pos[i]), midi_pos);
        CtlWord2Screen(&(pos[i]), cw);
        pos[i].scale += pos[i].dscale / VIRTUAL_FPS;
        pos[i].theta += pos[i].dtheta / VIRTUAL_FPS;
        pos[i].phi += pos[i].dphi / VIRTUAL_FPS;
        pos[i].a += pos[i].da / VIRTUAL_FPS;
        pos[i].b += pos[i].db / VIRTUAL_FPS;
        pos[i].c += pos[i].dc / VIRTUAL_FPS;
        pos[i].d += pos[i].dd / VIRTUAL_FPS;
        pos[i].amp += pos[i].damp / VIRTUAL_FPS;

        pos[i].xc += (pos[i].xctarget - pos[i].xc) * 2 / VIRTUAL_FPS;
        pos[i].yc += (pos[i].yctarget - pos[i].yc) * 2 / VIRTUAL_FPS;

        pos[i].fricCounter += 10;
        while(pos[i].fricCounter > VIRTUAL_FPS)
          Resonance(&(pos[i]));
      }

    }

    if(timers.odo - lastTimer > 100)
    {
      pos->fps = framesDrawn;
      framesDrawn = 0;
      lastTimer += 100;
    };

    if(timers.odo - lastGameloop > 0)
    {
      done |= GameLoop();
      lastGameloop++;
    }

    for(i = 0; i < nPlayers; i++)
    {
      InitSidesPhi(&(seven[i]), pos[i].phi, pos[i].scale);
      RotateSides(&(seven[i]), pos[i].theta);

      if(timers.odo - lastGameloop > 0)
      {
        done |= GameLoop();
        lastGameloop++;
      }

      DisplaceScanlines(&(seven[i]), lastGameloop,
                        pos[i].a, pos[i].b, pos[i].c, pos[i].d,
                        pos[i].sinusFreq, pos[i].amp >> 8,
                        pos[i].noise, pos[i].heat);
      DisplaceSides(&(seven[i]), pos[i].xc, pos[i].yc);

      if(timers.odo - lastGameloop > 0)
      {
        done |= GameLoop();
        lastGameloop++;
      }

      if(midi_pos > 32)
      {
        pos[i].xctarget = (p[i].x + 13) * 0x80000; // centered on current block
        pos[i].yctarget = (p[i].y + 21) * 0x50000; // centered below current blk
      }
      else
      {
        pos[i].xctarget = 0x800000;
        pos[i].yctarget = 0x780000;
      }

      SevenRender(&(seven[i]));
      DrawNext(i);

      if(timers.odo - lastGameloop > 0)
      {
        done |= GameLoop();
        lastGameloop++;
      }
    }

    if(timers.odo - lastGameloop > 25) /* if you fall too far behind */
      lastGameloop += 15;

    totalFrames++;
    framesDrawn++;
    if(framesDrawn > pos->fps)
      pos->fps++;

    text_mode(-1);
    textprintf(seven->frontBuf, lucid, 0, lead * 13, 0,
               "%4ufps %4lubeat %3udose %3umana",
               pos->fps, midi_pos, pos[0].midiLoop, pos[0].mana);
    textout(seven->frontBuf, lucid, aimsn, lead, lead, 0);
    textprintf(seven->frontBuf, lucid, lead, lead * 2, 0, "sc%7u", p[0].score);
    textprintf(seven->frontBuf, lucid, lead, lead * 3, 0, "line%5u", p[0].lines);

    if(nPlayers > 1)
    {
      textprintf(seven->frontBuf, lucid, lead*10, lead * 2, 0, "sc%7u", p[1].score);
      textprintf(seven->frontBuf, lucid, lead*10, lead * 3, 0, "line%5u", p[1].lines);
    }
    wrapblit(seven->frontBuf, screen, 0, 0, 0, 0, seven->frontBuf->w, seven->frontBuf->h);

/*
      if(key[KEY_Z])
        pos->sinusFreq += 0x1000000 / pos->fps;
      if(key[KEY_X])
        pos->sinusFreq -= 0x1000000 / pos->fps;
*/

    /* animate the palette */
    for(i = 0; i < 64; i++)
    {
      unsigned col;

      j = (i << 18) + (timers.odo << 16);
      col = (hgrcos(j) + 0x10000) >> 11;
      if(col > 63)
        col = 63;

      pal[i + 64].g = col;
      pal[i + 128].g = col / 2;
    }
    /* ThetaLight(0x600000 - pos->theta); */
    set_palette(pal);

    while(keypressed())
      switch(readkey() >> 8)
      {
      case KEY_ESC:
        done = 1;
        break;
      case KEY_P:
        save_pcx("todsnap.pcx", seven->frontBuf, pal);
        break;
      case KEY_INSERT:
        pos->midiLoop++;
        set_palette(black_palette);
        break;
      }
  }
}

int main(void)
{
  int y, i;
  Seven seven[2];
  DATAFILE *dat;
  BITMAP *loadedBG;

  char aimsn[17] = "Anonymous Coward";

  allegro_init();
  install_timer();
  LOCK_VARIABLE(timers);
  LOCK_FUNCTION(timerint);

  install_int(timerint, 10); /* 100 fps timer */
  srand(time(0));

  y = GetRes(aimsn);
  if(y < 0)
  {
    allegro_message("could not set graphics mode\n");
    return 1;
  }

/* FIXME: once I get digi in, change NONE to AUTODETECT */

  install_sound(DIGI_NONE,
                playing_midi ? MIDI_AUTODETECT : MIDI_NONE,
                NULL);

  dat = load_datafile("idltd.dat");
  if(!dat)
  {
    alert("no idltd.dat", "", "", "OK", NULL, 13, 0);
    return 1;
  }

  loadedBG = GetResource(dat, "IDLTD_ELOI_BMP");
  tetbits = GetResource(dat, "BLOX");
  pal = GetResource(dat, "IDLTD_ELOI_PAL");
  lucid = GetResource(dat, lucidName[y]);
  if(!loadedBG)
  {
    alert("idltd.dat missing IDLTD_ELOI_PCX", "", "", "OK", NULL, 13, 0);
    return 1;
  }
  if(!tetbits)
  {
    alert("idltd.dat missing BLOX", "", "", "OK", NULL, 13, 0);
    return 1;
  }
  if(!pal)
  {
    alert("idltd.dat missing IDLTD_ELOI_PAL", "", "", "OK", NULL, 13, 0);
    return 1;
  }
  if(!lucid)
  {
    alert("idltd.dat missing font", lucidName[y], "", "OK", NULL, 13, 0);
    return 1;
  }

  if(InitScore(dat) < 0)
  {
    alert("idltd.dat missing login graphics", "", "", "OK", NULL, 13, 0);
    return 1;
  }

  seven[0].frontBuf = create_bitmap(windowWid, windowHt);
  if(!seven[0].frontBuf)
  {
    alert("no ram", "", "", "OK", NULL, 13, 0);
    unload_datafile(dat);
    return 1;
  }
  seven[1].frontBuf = seven[0].frontBuf;
  clear(seven[0].frontBuf);
  for(i = 0; i < 2; i++)
  {
    p[i].seven = &(seven[i]);

    seven[i].backBuf = create_bitmap(256, 256);
    if(!seven[i].backBuf)
    {
      alert("no ram", "", "", "OK", NULL, 13, 0);
      unload_datafile(dat);
      return 1;
    }
  }

  g.backbits = create_bitmap(256, 256);
  if(!g.backbits)
  {
    alert("no ram", "", "", "OK", NULL, 13, 0);
    unload_datafile(dat);
    return 1;
  }

  set_palette(pal);
  while(Login(aimsn) >= 0)
  {
    while(GetOpts(aimsn, GetResource(dat, "LOGIN_BMP")) >= 0)
    {
      ScreenPos pos[2] =
      {
        {
          0, 0,  0, 0,  0, 0,  0, 0,  /* a b c d */
          0, 0,  0x400000, 0,         /* phi */
          0x13333, 0,                 /* scale */
          0, 0, 0x8000000,            /* amp freq */
          0x400000,                   /* noise */
          0,                          /* heat */
          0x800000, 0x800000,         /* center */
          0x800000, 0x780000,         /* target for center */
          0, FRAMERATE, 0,            /* fps */
          0, 0,                       /* nEffects */
          0, 1                        /* midi to mana */
        }
      };

      if(nPlayers == 2)
      {
        seven[0].nextX = SCREEN_W * 2/5;
        seven[1].nextX = SCREEN_W / 2;
        seven[0].leftSide = 0;
        seven[0].rightSide = seven[1].leftSide = SCREEN_W / 2;
        seven[1].rightSide = SCREEN_W;
      }
      else
      {
        seven[0].nextX = SCREEN_W * 1/5;
        seven[0].leftSide = 0;
        seven[0].rightSide = SCREEN_W;
      }

      wrapblit(loadedBG, g.backbits, 0, 0, 0, 0, 256, 256);
      SetupLight1();
      set_palette(pal);

      for(i = 0; i < nPlayers; i++)
      {
        wrapblit(g.backbits, seven[i].backBuf, 0, 0, 0, 0, 256, 256);
        NewGame(i);
      }

      StartSong(1, dat, &(pos[0]));
      pos[1] = pos[0];

      PlayGame(seven, pos, aimsn);
    }
  }

  destroy_bitmap(seven[0].frontBuf);
  destroy_bitmap(seven[0].backBuf);
  destroy_bitmap(seven[1].backBuf);
  destroy_bitmap(g.backbits);
  unload_datafile(dat);
  remove_int(timerint);

  return 0;
} END_OF_MAIN();
