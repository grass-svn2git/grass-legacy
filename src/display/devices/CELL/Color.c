/*
 * Identify a color that has been set in the reset_color() (found in Reset_clr.c
 * file in this directory).  Subsequent graphics calls will use this color.
 *
 * Called by:
 *      Color() in ../lib/Color.c
 */
#include "cell.h"

color(number)
	int number ;
{
    Cur_color = number;
}
