/**************************************\
* ALLEGDLG.C                           *
* Dialog control defprocs for Allegro  *
* Copr. 1999 Damian Yerrick            *
\**************************************/

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <allegro5/allegro.h>

#include "a4_aux.h"
/* dotted_rect:
 *  Draws a dotted rectangle, for showing an object has the input focus.
 *  By Shawn Hargreaves
 */
static void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
   int x = ((x1+y1) & 1) ? 1 : 0;
   int c;

   for (c=x1; c<=x2; c++) {
      putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
      putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
   }

   for (c=y1+1; c<y2; c++) {
      putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
      putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
   }
}


/* d_check_proc:
 *  Who needs C++ after all? This is derived from d_button_proc, 
 *  but overrides the drawing routine to provide a check box.
 *  Original by Shawn Hargreaves (Allegro 3.11)
 *  Changes by Damian Yerrick as of 30 April 1999:
 *    Moved checkbox to the left to make it look like a Mac
 *    or MS Windows checkbox.
 */
int DY_check_proc(int msg, DIALOG *d, int c)
{
   int x;
   int fg;

   if (msg==MSG_DRAW) {
      fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
      text_mode(d->bg);
      gui_textout(screen, d->dp, d->x+d->h+3, d->y+(d->h-(text_height(font)-gui_font_baseline))/2, fg, FALSE);
      x = d->x;
      rectfill(screen, x+1, d->y+1, x+d->h-1, d->y+d->h-1, d->bg);
      rect(screen, x, d->y, x+d->h, d->y+d->h, fg);
      if (d->flags & D_SELECTED) {
	 line(screen, x, d->y, x+d->h, d->y+d->h, fg);
	 line(screen, x, d->y+d->h, x+d->h, d->y, fg); 
      }
      if (d->flags & D_GOTFOCUS)
	 dotted_rect(x+1, d->y+1, x+d->h-1, d->y+d->h-1, fg, d->bg);
      return D_O_K;
   } 

   return d_button_proc(msg, d, 0);
}


/* DY_bitmap_proc() ********************
 * Displays the transparent bitmap in dp, scaled as necessary.
 */
int DY_bitmap_proc(int msg, DIALOG *d, int c)
{
  if(msg == MSG_DRAW)
  {
    ALLEGRO_BITMAP *bmp = (ALLEGRO_BITMAP *)(d->dp);

    if(bmp != NULL)
    {
      if(al_get_bitmap_width(bmp) == al_get_bitmap_width(d) && al_get_bitmap_height(bmp) == al_get_bitmap_height(d))
        draw_sprite(screen, bmp, d->x, d->y);
      else
        stretch_sprite(screen, bmp, d->x, d->y, al_get_bitmap_width(d), al_get_bitmap_height(d));
    }
  }

  return D_O_K;
}


/* DY_action_proc() ********************
 * By Damian Yerrick
 * Inherits from d_button_proc()
 * Declare this control as D_EXIT in your dialog item list.
 *
 * When clicked, this button calls the function in dp2:
 *   int foobar(DIALOG *d);
 * Then it un-highlights the button and returns what the callback returned.
 * (Most callbacks should return D_REDRAW.)
 */
int DY_action_proc(int msg, DIALOG *d, int c)
{
  int ret;

  /* call the parent object */
  ret = d_button_proc(msg, d, c);

  /* trap the close return value and call the function */
  if(ret == D_CLOSE)
  {
    int (*callback)(DIALOG *d);

    // redraw button without highlight
    scare_mouse();
    ret = d_button_proc(MSG_DRAW, d, c);
    unscare_mouse();

    // call the callback function
    callback = d->dp2;
    if(callback != NULL)
      ret |= (*callback)(d);
  }

  /* otherwise just return */
  return ret;
}


/* DY_idle_proc() **********************
 * By Damian Yerrick
 * When placed into a dialog box in Allegro 3.9.31 or later, this function
 * plays nice with the scheduler.
 */
int DY_idle_proc(int msg, DIALOG *unused2, int unused3)
{
  if(msg == MSG_IDLE)
    yield_timeslice();
  return D_O_K;
}


/* DY_pass_proc() **********************
 * By Damian Yerrick
 * An editable text object that doesn't show the text.
 * Useful for passwords.
 */
int DY_pass_proc(int msg, DIALOG *d, int c)
{
  int f, l, p, w, x, fg, b, scroll;
  char buf[16];
  char *s;

  s = d->dp;
  l = ustrlen(s);
  if (d->d2 > l) 
    d->d2 = l;

  /* calculate maximal number of displayable characters */
  if (d->d2 == l)
  {
    usetc(buf+usetc(buf, ' '), 0);
    x = text_length(font, buf);
  }
  else
    x = 0;

  b = 0;

  for (p=d->d2; p>=0; p--)
  {
    usetc(buf+usetc(buf, ugetat(s, p)), 0);
    x += text_length(font, buf);
    b++;
    if (x > d->w) 
      break;
  }

  if (x <= d->w)
  {
    b = l; 
    scroll = FALSE;
  }
  else
  {
    b--; 
    scroll = TRUE;
  }

  switch (msg)
  {
    case MSG_START:
    case MSG_CLICK:
    case MSG_WANTFOCUS:
    case MSG_LOSTFOCUS:
    case MSG_KEY:
    case MSG_CHAR:
    case MSG_UCHAR:
      return d_edit_proc(msg, d, c);
      break;

    case MSG_DRAW:
      fg = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
      x = 0;

      if (scroll)
      {
        p = d->d2-b+1; 
        b = d->d2; 
      }
      else 
        p = 0; 

      for (; p<=b; p++)
      {
        f = ugetat(s, p);
        usetc(buf+usetc(buf, (f) ? '*' : ' '), 0);
        w = text_length(font, buf);
        if (x+w > d->w) 
          break;
        f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
        text_mode((f) ? fg : d->bg);
        textout(screen, font, buf, d->x+x, d->y, (f) ? d->bg : fg);
        x += w;
      }
      if (x < d->w)
        rectfill(screen, d->x+x, d->y, d->x+d->w-1, d->y+text_height(font)-1, d->bg);
      break;
  }

  return D_O_K;
}


