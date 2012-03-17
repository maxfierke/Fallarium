#include <allegro5/allegro.h>
#include "hgrcos.inc"

/* Allegro has a 512-entry cosine table.  I need much more.
 * I am using a 4096-entry cosine table.
 */

al_fixed hgrcos(al_fixed theta)
{
  /* rescale theta to 0-4095 */
  theta = ((theta + 0x800) & 0xfff000) >> 12;

  /* Switch on the quadrant. */
  switch(theta >> 10)
  {
    case 0: /* first quadrant, return raw value */
      return hirescostable[theta];
    case 1: /* second quadrant */
      return -hirescostable[2048 - theta];
    case 2: /* third quadrant */
      return -hirescostable[theta - 2048];
    case 3: /* fourth quadrant */
      return hirescostable[4096 - theta];
    default:/* should never get here */
      return 0;
  }
}


al_fixed hgrsin(al_fixed theta)
{
  return hgrcos(theta + 0xc00000);
}

al_fixed hgrtan(al_fixed theta)
{
  return fdiv(hgrcos(theta + 0xc00000), hgrcos(theta));
}
