/****************************************************************
 *
 * MODULE:     v.edit
 *
 * AUTHOR(S):  GRASS Development Team
 *             Jachym Cepicky <jachym  les-ejk cz>
 *             Martin Landa
 *
 * PURPOSE:    This module edits vector maps. 
 *             Vertex operations.
 *
 * COPYRIGHT:  (C) 2006-2007 by the GRASS Development Team
 *
 *             This program is free software under the
 *             GNU General Public License (>=v2).
 *             Read the file COPYING that comes with GRASS
 *             for details.
 *
 ****************************************************************/

#include "global.h"

/* 
 * move all vertices in the given thresh
 *
 * return number of modified lines
 * return -1 on error
 */
int do_move_vertex(struct Map_info *Map, struct ilist *List, int print,
		   struct Option *coord, double thresh,
		   double move_x, double move_y)
{
    int nvertices_moved, nlines_modified;

    int i, j, k;
    int line, type, rewrite;
    double east, north, dist;
    double *x, *y, *z;
    char *moved;

    struct line_pnts *Points;
    struct line_cats *Cats;
    
    nlines_modified = 0;
    nvertices_moved = 0;
    moved = NULL;

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    for (i = 0; i < List -> n_values; i++) {
	line = List -> value[i];
	
	if (!Vect_line_alive (Map, line))
	    continue;
	
	type = Vect_read_line(Map, Points, Cats, line);

	if (!(type & GV_LINES))
	    continue;

	x = Points -> x;
	y = Points -> y;
	z = Points -> z;

	/* vertex moved ? */
	moved = (char *) G_realloc ((void *) moved, Points -> n_points * sizeof (char));
	G_zero ((void *) moved, sizeof (char));
	
	rewrite = 0;
	for (j = 0; coord -> answers[j]; j += 2) {
	    east  = atof (coord -> answers[j]);
	    north = atof (coord -> answers[j+1]);
	    
	    /* move all vertices in the bounding box */
	    for (k = 0; k < Points -> n_points; k++) {
		if (!moved[k]) {
		    dist = Vect_points_distance (east, north, 0.0,
						 x[k], y[k], z[k],
						 WITHOUT_Z);
		    if (dist <= thresh) {
			G_debug (3, "do_move_vertex(): line=%d; x=%f, y=%f -> x=%f, y=%f",
				 line, x[k], y[k], x[k] + move_x, y[k] + move_y);
			x[k] += move_x;
			y[k] += move_y;
			
			moved[k] = 1;
			rewrite = 1;
			nvertices_moved++;
		    }
		}
	    } /* for each point */
	} /* for each coord */
	
	if (rewrite) {
	    if (Vect_rewrite_line (Map, line, type, Points, Cats) < 0)  {
		G_warning(_("Cannot rewrite line [%d]"), line);
		return -1;
	    }
	    
	    if (print) {
		fprintf(stdout, "%d%s",
			line,
			i < List->n_values -1 ? "," : "");
		fflush (stdout);
	    }
	    nlines_modified++;
	}
    } /* for each selected line */

    /* destroy structures */
    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);
/*     G_free ((void *) moved); */

    G_message(_("[%d] vertices moved"), nvertices_moved);
    G_message(_("[%d] lines modified"), nlines_modified);
    
    return nlines_modified;
}

/* 
 * add new verteces to the line on location given by coords (check for threshold)
 * shape of line is not changed
 *
 * return number of modified lines
 * return -1 on error
 */
int do_add_vertex (struct Map_info *Map, struct ilist *List, int print,
		   struct Option* coord, double thresh)
{
    int i, j;
    int type, line, seg;
    int nlines_modified, nvertices_added, rewrite;
    double east, north, dist;
    double *x, *y, *z;
    double px, py;

    struct line_pnts *Points;
    struct line_cats *Cats;

    nlines_modified = 0;
    nvertices_added = 0;
    Points  = Vect_new_line_struct();
    Cats    = Vect_new_cats_struct();

    for (i = 0; i < List -> n_values; i++) {
	line = List -> value[i];

	if (!Vect_line_alive (Map, line))
	    continue;

        type = Vect_read_line(Map, Points, Cats, line);

	if (!(type & GV_LINES))
	    continue;

	x = Points -> x;
	y = Points -> y;
	z = Points -> z;
	rewrite = 0;
	for (j = 0; coord -> answers[j]; j += 2) {
	    east  = atof (coord -> answers[j]);
	    north = atof (coord -> answers[j+1]);

	    seg = Vect_line_distance (Points,
				      east, north, 0.0,    /* standpoint */
				      WITHOUT_Z,
				      &px, &py, NULL,      /* point on line */
				      &dist,               /* distance to line */
				      NULL, NULL);
	    
	    if (dist <= thresh &&
		Vect_points_distance (px, py, 0.0, x[seg], y[seg], z[seg], WITHOUT_Z) > 0 &&
		Vect_points_distance (px, py, 0.0, x[seg-1], y[seg-1], z[seg-1], WITHOUT_Z) > 0) {
		/* add new vertex */
		Vect_line_insert_point (Points, seg, px, py, 0.0);
		G_debug (3, "do_add_vertex(): line=%d; x=%f, y=%f, index=%d", line, px, py, seg);
		rewrite = 1;
		nvertices_added++;
	    }
	} /* for each point */

	/* rewrite the line */
	if (rewrite) {
	    Vect_line_prune (Points);
	    if (Vect_rewrite_line (Map, line, type, Points, Cats) < 0) {
		G_warning(_("Cannot rewrite line [%d]"), line);
		return -1;
	    }
		
	    if (print) {
		fprintf(stdout, "%d%s",
			line,
			i < List->n_values -1 ? "," : "");
		fflush (stdout);
	    }
	    nlines_modified++;
	}
    } /* for each line */

    /* destroy structures */
    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);

    G_message(_("[%d] vertices added"), nvertices_added);    
    G_message(_("[%d] lines modified"), nlines_modified);

    return nlines_modified;
}

/*
 * remove vertex from line in the given bounading box(es)
 *
 * return number of removed vertices
 * return -1 on error
 */
int do_remove_vertex(struct Map_info *Map, struct ilist *List, int print,
		     struct Option *coord, double thresh)
{
    int i, j, k;
    int type, line;
    int nvertices_removed, rewrite, nlines_modified;
    double east, north;
    double dist;
    double *x, *y, *z;

    struct line_pnts *Points;
    struct line_cats *Cats;

    nvertices_removed = nlines_modified = 0;

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();
    
    for (i = 0; i < List -> n_values; i++) {
	line = List -> value[i];
	
	if (!Vect_line_alive (Map, line))
	    continue;
	
        type = Vect_read_line(Map, Points, Cats, line);

	if (!(type & GV_LINES))
	    continue;

	x = Points -> x;
	y = Points -> y;
	z = Points -> z;
	rewrite = 0;
	for (j = 0; coord -> answers[j]; j += 2) {
	    east  = atof (coord -> answers[j]);
	    north = atof (coord -> answers[j+1]);
	    
	    for (k = 0; k < Points -> n_points; k++) {
		dist = Vect_points_distance (east, north, 0.0,
					     x[k], y[k], z[k],
					     WITHOUT_Z);
		if (dist <= thresh) {
		    /* remove vertex */
		    Vect_line_delete_point (Points, k);
		    G_debug (3, "do_remove_vertex(): line=%d; x=%f, y=%f, index=%d", line, x[k], y[k], k);
		    k--;
		    nvertices_removed++;
		    rewrite = 1;
		}
	    } /* for each point */
	} /* for each bounding box */
	
	if (rewrite) {
	    /* rewrite the line */
	    if (Vect_rewrite_line (Map, line, type, Points, Cats) < 0) {
		G_warning (_("Cannot rewrite line [%d]"), line);
		return -1;
	    }
	    
	    if (print) {
		fprintf(stdout, "%d%s",
			line,
			i < List->n_values -1 ? "," : "");
		fflush (stdout);
	    }
	    nlines_modified++;
	}
    } /* for each line */

    /* destroy structures */
    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);
    
    G_message(_("[%d] vertices removed"), nvertices_removed);
    G_message(_("[%d] lines modified"), nlines_modified);

    return nlines_modified;
}
