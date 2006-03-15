/*
* $Id$
*/

/*  Gp.c 
    Bill Brown, USACERL  
    January 1994
    Uses GRASS routines!
*/

#include <stdlib.h>
#include <grass/gis.h>
#include <grass/site.h>
#include <grass/Vect.h>

#include <grass/gstypes.h>

/* used when site attribute mode is ST_ATT_COLOR */
/* Gets color structure for grass file, goes through points and
   uses fattr as CAT, putting rgb color in iattr. */
int Gp_set_color(char *grassname, geopoint * gp)
{
    char *col_map;
    struct Colors sc;
    CELL cat;
    geopoint *tp;
    int r, g, b, color;

    /* TODO: handle error messages */

    if (grassname) {
	col_map = G_find_file2("cell", grassname, "");

	if (col_map == NULL) {
	    fprintf(stderr, "Could not find file '%s'", grassname);
	    return (0);
	}
	else {
	    G_read_colors(grassname, col_map, &sc);
	}

	for (tp = gp; tp; tp = tp->next) {
	    cat = (int) tp->fattr;
	    color = NULL_COLOR;

	    if (G_get_color(cat, &r, &g, &b, &sc)) {
		color = r & 0xff | ((g & 0xff) << 8) | ((b & 0xff) << 16);
	    }

	    tp->iattr = color;
	}

	return (1);
    }

    return (0);
}

/*##############################################################*/
/* This loads to memory.  
The other alternative may be to load to a tmp file. */
geopoint *Gp_load_sites(char *grassname, int *nsites, int *has_z,
			int *has_att)
{
    FILE *sfd;
    char *mapset;
    geopoint *top, *gpt, *prev;
    int np;
    struct Cell_head wind;
    double e_ing, n_ing;
    Site *nextsite;
    RASTER_MAP_TYPE rtype;
    int ndim, nstr, ndec;


    /* TODO: handle error messages */

    np = 0;
    *has_z = *has_att = 0;

    if (NULL == (mapset = G_find_sites(grassname, ""))) {
	fprintf(stderr, "Can't find sites file %s.\n", grassname);
	return (NULL);
    }

    if (NULL == (sfd = G_sites_open_old(grassname, mapset))) {
	fprintf(stderr, "Can't open sites file %s.\n", grassname);
	return (NULL);
    }

    if (NULL == (top = gpt = (geopoint *) malloc(sizeof(geopoint)))) {
	fprintf(stderr, "Can't malloc.\n");
	return (NULL);
    }

    G_get_set_window(&wind);

    G_site_describe(sfd, &ndim, &rtype, &nstr, &ndec);

    /* use these for allocation */
    nextsite = G_site_new_struct(rtype, ndim, nstr, ndec);
    fprintf(stdout, "Site dim: %d\n", ndim);

    while (G_site_get(sfd, nextsite) != -1) {
	n_ing = nextsite->north;
	e_ing = G_adjust_easting(nextsite->east, &wind);

	if (G_site_in_region(nextsite, &wind)) {
	    np++;
	    gpt->p3[X] = e_ing;
	    gpt->p3[Y] = n_ing;

	    if (ndim > 2) {
		/* enables 3d site display */
		*has_z = 1;
		gpt->dims = 3;
		gpt->p3[Z] = nextsite->dim[0];
	    }
	    else {
		gpt->dims = 2;
		*has_z = 0;
	    }

	    if (ndec > 0) {
		*has_att = 1;
		gpt->fattr = nextsite->dbl_att[0];
/* ACS_MODIFY_OneLine site_attr management - new */
		gpt->cat = nextsite->ccat;

/* ACS_MODIFY_OneLine highlight management - new */
		gpt->highlight_color = gpt->highlight_size = gpt->highlight_marker = FALSE;
	    }
	    else {
		gpt->fattr = 0;
		*has_att = 0;
	    }

	    gpt->iattr = gpt->fattr;

	    /* TODO: use leftover text as cattr */
	    gpt->cattr = NULL;

	    if (NULL == (gpt->next = (geopoint *) malloc(sizeof(geopoint)))) {
		fprintf(stderr, "Can't malloc.\n");	/* CLEAN UP */
		return (NULL);
	    }

	    prev = gpt;
	    gpt = gpt->next;
	}
    }

    G_site_free_struct(nextsite);
    prev->next = NULL;
    free(gpt);

    G_sites_close(sfd);

    fprintf(stderr, "Sites file %s loaded.\n", grassname);
    if (!np) {
	    fprintf(stderr, "Error: No points from %s fall within current region\n", grassname);
	    return(NULL);
    }
    *nsites = np;

    return (top);
}
