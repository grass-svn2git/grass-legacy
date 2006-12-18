/**
 * \file maskfd.c
 *
 * \brief Mask functions.
 *
 * This program is free software under the GNU General Public License
 * (>=v2). Read the file COPYING that comes with GRASS for details.
 *
 * \author GRASS GIS Development Team
 *
 * \date 1999-2006
 */

#include <grass/gis.h>
#include "G.h"


/**
 * \fn int G_maskfd (void)
 *
 * \brief Test for MASK.
 *
 * \return -1 if no MASK
 * \return file descriptor if MASK
 */

int G_maskfd ()
{
	G__check_for_auto_masking () ;

	return G__.auto_mask > 0 ? G__.mask_fd : -1;
}
