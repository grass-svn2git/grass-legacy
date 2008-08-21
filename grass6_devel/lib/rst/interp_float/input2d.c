
/*-
 * Written by H. Mitasova, I. Kosinovsky, D. Gerdes Fall 1993
 * University of Illinois
 * US Army Construction Engineering Research Lab  
 * Copyright 1993, H. Mitasova (University of Illinois),
 * I. Kosinovsky, (USA-CERL), and D.Gerdes (USA-CERL)   
 *
 * modified by McCauley in August 1995
 * modified by Mitasova in August 1995  
 * modified by Mitasova in November 1996 to include variable smoothing
 * modified by Brown in June 1999 - added elatt & smatt
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <grass/gis.h>
#include <grass/site.h>
#include <grass/bitmap.h>
#include <grass/linkm.h>
#include <grass/interpf.h>
#include <grass/glocale.h>

struct BM *IL_create_bitmask(struct interp_params *params)

/** Creates a bitmap mask from given raster map **/
{
    int i, j, cfmask = 0, irev, MASKfd;
    char *mapsetm;
    CELL *cellmask, *MASK;
    struct BM *bitmask;

    if ((MASKfd = G_maskfd()) >= 0)
	MASK = G_allocate_cell_buf();
    else
	MASK = NULL;

    if (params->maskmap != NULL || MASK != NULL) {
	bitmask = BM_create(params->nsizc, params->nsizr);

	if (params->maskmap != NULL) {
	    mapsetm = G_find_cell2(params->maskmap, "");
	    if (!mapsetm)
		G_fatal_error(_("Mask raster map <%s> not found"),
			      params->maskmap);

	    cellmask = G_allocate_cell_buf();
	    cfmask = G_open_cell_old(params->maskmap, mapsetm);
	}
	else
	    cellmask = NULL;

	for (i = 0; i < params->nsizr; i++) {
	    irev = params->nsizr - i - 1;
	    if (cellmask)
		G_get_map_row(cfmask, cellmask, i);
	    if (MASK)
		G_get_map_row(MASKfd, MASK, i);
	    for (j = 0; j < params->nsizc; j++) {
		if ((cellmask && cellmask[j] == 0) || (MASK && MASK[j] == 0))
		    BM_set(bitmask, j, irev, 0);
		else
		    BM_set(bitmask, j, irev, 1);
	    }
	}
	G_message(_("Bitmap mask created"));
    }
    else
	bitmask = NULL;

    return bitmask;
}

int translate_quad(struct multtree *tree,
		   double numberx,
		   double numbery, double numberz, int n_leafs)
{
    int total = 0, i, ii;

    if (tree == NULL)
	return 0;
    if (tree->data == NULL)
	return 0;

    if (tree->leafs != NULL) {
	((struct quaddata *)(tree->data))->x_orig -= numberx;
	((struct quaddata *)(tree->data))->y_orig -= numbery;
	((struct quaddata *)(tree->data))->xmax -= numberx;
	((struct quaddata *)(tree->data))->ymax -= numbery;
	for (ii = 0; ii < n_leafs; ii++)
	    total +=
		translate_quad(tree->leafs[ii], numberx, numbery, numberz,
			       n_leafs);
    }
    else {
	((struct quaddata *)(tree->data))->x_orig -= numberx;
	((struct quaddata *)(tree->data))->y_orig -= numbery;
	((struct quaddata *)(tree->data))->xmax -= numberx;
	((struct quaddata *)(tree->data))->ymax -= numbery;
	for (i = 0; i < ((struct quaddata *)(tree->data))->n_points; i++) {
	    ((struct quaddata *)(tree->data))->points[i].x -= numberx;
	    ((struct quaddata *)(tree->data))->points[i].y -= numbery;
	    ((struct quaddata *)(tree->data))->points[i].z -= numberz;
	}

	return 1;
    }

    return total;
}
