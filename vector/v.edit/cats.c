/***************************************************************
 *
 * MODULE:     v.edit
 * 
 * AUTHOR(S):  GRASS Development Team
 *             Jachym Cepicky <jachym  les-ejk cz>
 *             Radim Blazek, Martin Landa
 *               
 * PURPOSE:    This module edits vector maps.
 *
 * COPYRIGHT:  (C) 2001-2007 The GRASS Development Team
 *
 *             This program is free software under the 
 *             GNU General Public License (>=v2). 
 *             Read the file COPYING that comes with GRASS
 *             for details.
 *
 **************************************************************/

#include "global.h"

/**
   \brief Edit category numbers of selected vector features

   \param[in] Map vector map
   \param[in] List list of selected features
   \param[in] layer layer number
   \param[in] del action (add/delete)
   \param[in] cats_list list of category numbers

   \return number of modified features
   \return -1 on error
*/
int cats (struct Map_info *Map, struct ilist *List,
	  int layer, int del, char *cats_list)
{
    int i, j;
    struct line_cats *Cats;
    struct line_pnts *Points;
    struct cat_list *Clist;
    int line, type, cat;
    int nlines_modified, rewrite;
    
    nlines_modified = 0;

    /* get list of categories */
    Clist = Vect_new_cat_list();
    if (Vect_str_to_cat_list(cats_list, Clist)) {
	G_warning (_("Unable to get category list <%s>, editing terminated"), cats_list);
	return -1;
    }
    
    /* features defined by cats */
    if(Clist->n_ranges > 0) {
	Cats   = Vect_new_cats_struct (); 
	Points = Vect_new_line_struct();

	/* for each line, set new category */
	for (i = 0; i < List->n_values; i++) {
	    line = List->value[i];
            type = Vect_read_line(Map, Points, Cats, line);

	    if (!Vect_line_alive (Map, line))
		continue;

	    rewrite = 0;
	    for (j = 0; j < Clist -> n_ranges; j++) {
		for (cat = Clist -> min[j]; cat <= Clist -> max[j]; cat++) {
		    /* add new category */
		    if (!del) {
			if(Vect_cat_set (Cats, layer, cat) < 1) {
			    G_warning (_("Unable to set category %d line %d"),
				       cat, line);
			}
			else {
			    rewrite = 1;
			}
		    }
		    else { /* delete old category */
			if(Vect_field_cat_del (Cats, layer, cat) == 0) {
			    G_warning (_("Unable to delete layer/category [%d/%d] line %d"), 
				       layer, cat, line);
			}
			else {
			    rewrite = 1;
			}
		    }
		}
	    }

	    if (rewrite == 0)
		continue;

	    if (Vect_rewrite_line (Map, line, type, Points, Cats) < 0)  {
		G_warning (_("Unable to rewrite line %d"), line);
		return -1;
	    }

	    nlines_modified++;

	}

	/* destroy structures */
	Vect_destroy_line_struct(Points);
	Vect_destroy_cats_struct(Cats);
    }

    return nlines_modified;
}
