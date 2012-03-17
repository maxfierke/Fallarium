/**************************************\
* ALLEGDLG.H                           *
* Dialog control defprocs for Allegro  *
* Copr. 1999 Damian Yerrick            *
\**************************************/

#ifndef _ALLEGDLG_H
#define _ALLEGDLG_H

int DY_check_proc(int msg, DIALOG *d, int c);
int DY_bitmap_proc(int msg, DIALOG *d, int c);
int DY_action_proc(int msg, DIALOG *d, int c);
int DY_idle_proc(int unused1, DIALOG *unused2, int unused3);
int DY_pass_proc(int msg, DIALOG *d, int c);

#endif
