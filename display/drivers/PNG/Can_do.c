/*
 * Returns 0 (no) or 1 (yes) to indicate whether or not a floating
 * color table is available.
 */

#include "pngdriver.h"

int PNG_Can_do_float(void)
{
	return !true_color ;
}

