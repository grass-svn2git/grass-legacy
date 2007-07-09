/****************************************************************
 *
 * MODULE:     v.edit
 *
 * AUTHOR(S):  GRASS Development Team
 *             Original author Jachym Cepicky <jachym  les-ejk cz>
 *             Updated by Martin Landa <landa.martin@gmail.com> (2007/03)
 *
 * PURPOSE:    This module edits vector maps. It is inteded to be mainly
 * 	       used by the the new v.digit GUI.
 *
 * COPYRIGHT:  (C) 2002-2006 by the GRASS Development Team
 *
 *             This program is free software under the
 *             GNU General Public License (>=v2).
 *             Read the file COPYING that comes with GRASS
 *             for details.
 *
 * TODO:       
 ****************************************************************/

#include "global.h"

/*
 * merge two given lines a,b
 * a : Points1/Cats1
 * b : Points2/Cats2
 * merged line : Points/Cats
 *
 * return 1 on success
 * return 0 on error
 */
static int merge_lines (struct line_pnts *Points1, struct line_cats *Cats1, 
			struct line_pnts *Points2, struct line_cats *Cats2,
			double thresh, struct line_pnts **Points);

/*
 * merge lines in vector map
 *
 * return number of merged lines
 * return -1 on error
 */
int do_merge(struct Map_info *Map, struct ilist *List, int print)
{
    struct ilist *List_in_box;
    
    struct line_pnts *Points1, *Points2, *Points;
    struct line_cats *Cats1, *Cats2;
    
    int line_i, i, j;    
    int line, line1, type1, line2, type2;
    int do_merge;
    /* number of lines (original, selected, merged) */
    int nlines, nlines_selected, nlines_merged;
    
    nlines_merged = 0;
    
    if (List->n_values < 2) {
	G_warning (_("Only %d lines found, at least two needed"),
		   List->n_values);
	return -1;
    }
    
    Points1 = Vect_new_line_struct();
    Cats1   = Vect_new_cats_struct();
    Points2 = Vect_new_line_struct();
    Cats2   = Vect_new_cats_struct();
    Points  = Vect_new_line_struct();
    
    List_in_box = Vect_new_list();
    
    nlines          = Vect_get_num_lines (Map);
    nlines_selected = List -> n_values;
    
    /* merge lines */
    for (line_i = 0; line_i < List -> n_values; line_i++) {
	G_percent (line_i, List -> n_values, 2);
	
	line1 = List -> value[line_i];
	
	if (!Vect_line_alive (Map, line1))
	    continue;
	
	type1 = Vect_read_line (Map, Points1, Cats1, line1);
	
	if (!(type1 & GV_LINES))
	    continue;
	
	Vect_reset_line (Points);
	
	for (i = 0; i < Points1 -> n_points; i += Points1 -> n_points - 1) {
	    Vect_reset_list (List_in_box);
	    
	    /* define searching region */
	    Vect_reset_line (Points2);
	    /*
	      Vect_append_point (Points2, Points1 -> x[i] - thresh,
	      Points1 -> y[i] + thresh, Points1 -> z[i]);
	      Vect_append_point (Points2, Points1 -> x[i] + thresh,
	      Points1 -> y[i] + thresh, Points1 -> z[i]);
	      Vect_append_point (Points2, Points1 -> x[i] + thresh,
	      Points1 -> y[i] - thresh, Points1 -> z[i]);
	      Vect_append_point (Points2, Points1 -> x[i] - thresh,
	      Points1 -> y[i] - thresh, Points1 -> z[i]);
	    */
	    Vect_append_point (Points2, Points1 -> x[i],
			       Points1 -> y[i], Points1 -> z[i]);
	    
	    /* 
	     * merge lines only if two lines found in the region
	     * i.e. the current line and an adjacent line
	     */
	    if (1 < Vect_select_lines_by_polygon (Map, Points2, 0, NULL,
						  GV_LINES, List_in_box)) {
		do_merge = 1;
		line2 = -1;
		for (j = 0; do_merge && j < List -> n_values; j++) {
		    if (List -> value[j] == line1 ||
			!Vect_line_alive(Map, List -> value[j]))
			continue;
		    
		    if (Vect_val_in_list (List_in_box, List -> value[j])) {
			if (line2 > 0) {
			    /* three lines found
			     * selected lines will be not merged
			     */
			    do_merge = 0;
			}
			else {
			    line2 = List -> value[j];
			}
		    }
		}
		
		if (!do_merge || line2 < 0)
		    continue;
		
		type2 = Vect_read_line (Map, Points2, Cats2, line2);
		
		merge_lines (Points1, Cats1,
			     Points2, Cats2,
			     -1.0,  &Points); /* do not use threshold value */
		
		G_debug (3, "merge lines: %d, %d", line1, line2);
		
		if (Points -> n_points > 0) {
		    if (Vect_delete_line(Map, line2) == -1) {
			G_warning (_("Cannot delete line %d"),
				   line2);
			return -1;
		    }
		    
		    if (print) {
			fprintf(stdout, "%d,",
				line2);

			fflush (stdout);
		    }
		    
		    if (line2 <= nlines)
			nlines_merged++;
		}
	    }
	} /* for each node */
	
	if (Points -> n_points > 0) {
	    line = Vect_rewrite_line (Map, line1, type1, Points, Cats1);
	    if (line < 0) {
		G_warning (_("Cannot rewrite line %d"),
			   line1);
		return -1;
	    }
	    
	    if (print) {
		fprintf(stdout, "%d%s",
			line1,
			i < List->n_values -1 ? "," : "");
		fflush (stdout);
	    }
	    
	    if (line1 <= nlines)
		nlines_merged++;
	    
	    /* update number of lines */
	    Vect_list_append (List, line);
	}
    } /* for each line */
    
    /* destroy stuctures */
    Vect_destroy_line_struct(Points1);
    Vect_destroy_line_struct(Points2);
    Vect_destroy_line_struct(Points);
    
    Vect_destroy_cats_struct(Cats1);
    Vect_destroy_cats_struct(Cats2);
    
    G_message (_("%d lines merged"),
	       nlines_merged);
    
    return nlines_merged;
}

static int merge_lines (struct line_pnts *Points1, struct line_cats *Cats1, 
			struct line_pnts *Points2, struct line_cats *Cats2,
			double thresh, struct line_pnts **Points)
{
    struct line_pnts *ps = *Points;
    struct line_cats *cs = Cats1;

    int i, mindistidx;
    double mindist;
    
    /* find mininal distance and its index */
    mindist = min_distance_line (Points1, Points2,
				 &mindistidx);

    G_debug (3, "merge line ? index: %d, mindist: %g, thresh: %g",
	     mindistidx, mindist, thresh);
    
    if (thresh > 0 && mindist > thresh) {
	return 0;
    }
    
    /* set index and other things */
    switch(mindistidx) {
	/* for each mindistidx create new line */
    case 0: 
	Vect_append_points (ps, Points2, GV_BACKWARD);
	if (ps -> n_points == Points2 -> n_points)
	    Vect_append_points (ps, Points1, GV_FORWARD);
	break;
    case 1: 
	Vect_append_points (ps, Points2, GV_FORWARD);
	if (ps -> n_points == Points2 -> n_points)
	    Vect_append_points (ps, Points1, GV_FORWARD);
	break;
    case 2: 
	if (ps -> n_points == 0)
	    Vect_append_points (ps, Points1, GV_FORWARD);
	Vect_append_points (ps, Points2, GV_FORWARD);
	break;
    case 3: 
	if (ps -> n_points == 0)
	    Vect_append_points (ps, Points1, GV_FORWARD);
	Vect_append_points (ps, Points2, GV_BACKWARD);
	break;
    default:
	break;
    }
    
    /* remove duplicate points */
    Vect_line_prune (ps);

    /* copy categories if needed */
    for (i = 0; i < Cats2 -> n_cats; i++) {
	Vect_cat_set (cs, Cats2 -> field[i], Cats2 -> cat[i]);
    }

    return 1;
}
