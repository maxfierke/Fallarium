First, there was Tetripz from Mute Fantasies.  Then there was...



      ________________       __________       _____________
     |___   __________|     /  ______  \     |___   _____  \ TM
        /  /               /  /      \  \       /  /     \  \ 
       /  /               /  /        \  \     /  /       \  \ 
      /  /               |  |          |  |   /  /         |  |
     |  |                |  |          |  |  |  |          |  |
     |  |           __   |  |          |  |  |  |          |  |
     |  |          |  |  |  |          |  |  |  |          |  |
      \  \        /  /    \  \        /  /    \  \        /  /
       \  \______/  /      \  \______/  /      \  \______/  /
        \__________/        \__________/        \__________/

     t  e  t  a  n  u  s        o  n            d  r  u  g  s


TOD(TM) simulates a game of TETRIS(R) experienced under the influence
of hallucinogenic drugs.  Officially, TOD stands for Tetanus On
Drugs, but it also is German for "death."


=== System Requirements ===

All systems:
  Keyboard (joypad recommended)
  MCGA, VGA, or better video card capable of displaying 8-bit color

GNU/Linux:
  Pentium
  X server or Linux framebuffer support
  4 MB of RAM free after loading fb or X
  Allegro MIDI-compatible sound card recommended (OSS works)
  GCC compiler

PC DOS:
  486 (Pentium recommended)
  4 MB RAM
  DOS 3.3x, or 5 or later
  DPMI server
    ftp://ftp.simtel.net/pub/simtelnet/gnu/djgpp/v2misc/csdpmi4b.zip
  Sound Blaster-compatible sound card recommended

Windows (tod.exe):
  Pentium
  32 MB of RAM (29 MB for Windows and 3 MB for the game)
  Windows 3.1 or later (Windows 9x/ME preferred; NT can't do sound
    or high resolution graphics)
  VESA video card recommended
  Sound Blaster-compatible sound card recommended

Windows (wintod.exe):
  Pentium
  32 MB of RAM (29 MB for Windows and 3 MB for the game)
  Windows 95, 98, ME, 2000, or XP with DirectX 6 or later
  MIDI Mapper-compatible sound card recommended


=== Installation ===

GNU/Linux:
You should already have the Allegro library, version 3.9.32 or later,
installed.
  http://sunsite.dk/allegro/
Then simply:
  gcc -O3 *.c `allegro-config --libs` -o xtod

DOS/Windows:
Simply run tod.exe or wintod.exe
To recompile using DJGPP or MinGW, use the makefile.
  http://www.delorie.com/djgpp/
  http://www.mingw.org/


=== Game Play ===

If you have a joystick plugged in, the game will use the joystick;
otherwise, it'll use the arrow keys, quote, and enter. For the left
player in a two-player game, use R F D G to move and A S to rotate.

Pieces fall from the top of the playfield.  It's your job to move and
rotate those pieces such that they fall neatly into place in complete,
unbroken horizontal lines.  Your opponents are:
 o time.  The pieces will pile up on you if you don't think fast.
 o hallucinogens.  Once the drug dosage begins to increase, you start
   to lose track of what's happening on-screen.
 o the other player.  In two-player games, your opponent can send you
   either garbage from the bottom of the screen or a dose of
   hallucinogens.


=== Technical Data ===

TOD is made up of three engines: a game engine, a rendering engine,
and an intelligent screen manipulator.

tetanus.c contains the Tetanus game engine, which also powers
freepuzzlearena Tetanus (see also http://pineight.8m.com/fpa.htm).
The Tetanus engine has Carbon technology that eliminates the
"floating blocks after clearing a line" bug in most TETRIS(R)
implementations and simultaneously adds depth to the game by allowing
ten-line or even larger chain reactions to be triggered with one
falling piece.  And now, Tetanus even supports a near-exact emulation
of Nintendo's The New TETRIS.

rec.c contains the Seven engine, which can perform general rotation
and scaling scanline of backgrounds per scanline, in a manner similar
to that of Nintendo's Super NES Display Mode 7.  It also contains
scanline generators that can perform effects on a bitmap with eight
degrees of freedom:  cubic polynomial and sinusoidal scanline
displacement in x and y directions, global scaling, and rotation
about the x and z axes.  It supports resolutions from 320x200 pixels
to 1024x768 pixels in 8-bit color.

scrctl.c contains the PM screen manipulator (absolutely no connection
to PRECIOUS MOMENTS(R) figurines is implied) that tweaks the knobs of
the Seven engine, responding to instructions embedded in the music
and providing increased distortion later in the game.  It keeps the
action smooth with a Newtonian physics engine.  To keep the game fair,
it starts out mild and gradually increases the "dosage" of the virtual
"drugs" administered, while creating precious moments of fleeting
invisibility (hence the name PM).

hgrcos.c contains a cosine lookup table accurate to 1/11 degree.
hiscore.c and crc32.c are involved in the high-score system.


=== Change log ===

2001 April 15: Added scanline displacement perpendicular to the
scanline axis, creating a ripple effect similar to that used in the
backgrounds of Super NES games such as Tetris Attack and Donkey Kong
Country.


=== Fine Print ===

<small>
Copyright 2000-2001 Damian Yerrick

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

TOD and the TOD logo are trademarks of Damian Yerrick.
Windows is a registered trademark of Microsoft Corporation.
Nintendo is a registered trademark of Nintendo of America Inc.
Precious Moments is a registered trademark of Precious Moments Inc.
Tetris is a registered trademark of The Tetris Company LLC.
</small>
