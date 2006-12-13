/* plot1() - Level One vector reading */

#include <string.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/display.h>
#include <grass/raster.h>
#include "plot.h"
#include "local_proto.h"
#include <grass/symbol.h>
#include <grass/glocale.h>
#include <grass/dbmi.h>

int plot1 (
    struct Map_info *Map, int type, int area, 
    struct cat_list *Clist, int color, int fcolor, int chcat, SYMBOL *Symb, int size, int id_flag,
    int table_colors_flag, int cats_color_flag, char *rgb_column, int default_width, char *width_column, double width_scale)
{
    int i, j, k, ltype, nlines = 0, line, cat = -1;
    double *x, *y, xd, yd, xd0 = 0, yd0 = 0;
    struct line_pnts *Points, *PPoints;
    struct line_cats *Cats;
    double msize;
    SYMBPART *part;
    SYMBCHAIN *chain;
    int x0, y0, xp, yp;

    struct field_info *fi = NULL;
    dbDriver *driver = NULL;
    dbCatValArray cvarr_rgb, cvarr_width;
    dbCatVal *cv_rgb = NULL, *cv_width = NULL;
    int nrec_rgb = 0, nrec_width = 0;

    int open_db;
    int rgb = 0; /* 0|1 */
    char colorstring[12]; /* RRR:GGG:BBB */
    int red, grn, blu;
    unsigned char which;
    int width;

    msize = size * ( D_d_to_u_col(2.0) - D_d_to_u_col(1.0) ); /* do it better */
    
    Points = Vect_new_line_struct ();
    PPoints = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();

    open_db = table_colors_flag || width_column;

    if(open_db){
      fi = Vect_get_field (Map, (Clist->field > 0 ? Clist->field : 1));
      if (fi == NULL) {
	G_fatal_error (_("Cannot read field info"));
      }
      
      driver = db_start_driver_open_database(fi->driver, fi->database);
      if (driver == NULL)
	G_fatal_error (_("Cannot open database %s by driver %s"), fi->database, fi->driver);
    }

    if( table_colors_flag ) {
      /* for reading RRR:GGG:BBB color strings from table */

      if ( rgb_column == NULL || *rgb_column == '\0' )
	G_fatal_error(_("Color definition column not specified."));

      db_CatValArray_init (&cvarr_rgb);     

      nrec_rgb = db_select_CatValArray(driver, fi->table, fi->key, 
				   rgb_column, NULL, &cvarr_rgb);

      G_debug (3, "nrec_rgb (%s) = %d", rgb_column, nrec_rgb);

      if (cvarr_rgb.ctype != DB_C_TYPE_STRING)
	G_fatal_error (_("Color definition column (%s) not a string. "
	    "Column must be of form RRR:GGG:BBB where RGB values range 0-255."), rgb_column);

      if ( nrec_rgb < 0 )
	G_fatal_error (_("Cannot select data (%s) from table"), rgb_column);

      G_debug (2, "\n%d records selected from table", nrec_rgb);

      for ( i = 0; i < cvarr_rgb.n_values; i++ ) {
	G_debug (4, "cat = %d  %s = %s", cvarr_rgb.value[i].cat, rgb_column,
		 db_get_string(cvarr_rgb.value[i].val.s));

	/* test for background color */
	if (test_bg_color (db_get_string(cvarr_rgb.value[i].val.s))) {
	  G_warning (_("Category <%d>: Line color and background color are the same!"),
		     cvarr_rgb.value[i].cat);
	}
      }
    }

    if ( width_column ) {
      if ( *width_column == '\0' )
	G_fatal_error(_("Line width column not specified."));

      db_CatValArray_init (&cvarr_width);     

      nrec_width = db_select_CatValArray(driver, fi->table, fi->key, 
				   width_column, NULL, &cvarr_width);

      G_debug (3, "nrec_width (%s) = %d", width_column, nrec_width);

      if (cvarr_width.ctype != DB_C_TYPE_INT && cvarr_width.ctype != DB_C_TYPE_DOUBLE)
	G_fatal_error (_("Line width column (%s) not a number."), width_column);

      if ( nrec_width < 0 )
	G_fatal_error (_("Cannot select data (%s) from table"), width_column);

      G_debug (2, "\n%d records selected from table", nrec_width);

      for ( i = 0; i < cvarr_width.n_values; i++ ) {
	G_debug (4, "cat = %d  %s = %d", cvarr_width.value[i].cat, width_column,
		 (cvarr_width.ctype==DB_C_TYPE_INT?cvarr_width.value[i].val.i:
		  (int)cvarr_width.value[i].val.d));
      }
    }

    if (open_db)
      db_close_database_shutdown_driver(driver);

    Vect_rewind ( Map );
    
    /* Is it necessary to reset line/label color in each loop ? */

    if ( color > -1 && !table_colors_flag && !cats_color_flag) R_color(color) ;

    if ( Vect_level ( Map ) >= 2 )
	nlines = Vect_get_num_lines ( Map );

    line = 0;
    while (1) {
	if ( Vect_level ( Map ) >= 2 ) { 
	    line++;
	    if ( line > nlines ) return 0;
	    if ( !Vect_line_alive (Map, line) ) continue;
	    ltype =  Vect_read_line (Map, Points, Cats, line);   
	} else {
	    ltype =  Vect_read_next_line (Map, Points, Cats);   
	    switch ( ltype )
	    {
	    case -1:
		fprintf (stderr, _("\nERROR: vector map - can't read\n" ));
		return -1;
	    case -2: /* EOF */
		return  0;
	    }
	}

	if ( !(type & ltype) ) continue;

	if ( chcat ) {
	     int found = 0;

	     if ( id_flag ) { /* use line id */
		 if ( !(Vect_cat_in_cat_list ( line, Clist)) )
		     continue;
	     } else {
		 for ( i = 0; i < Cats->n_cats; i++ ) {
		     if ( Cats->field[i] == Clist->field && Vect_cat_in_cat_list ( Cats->cat[i], Clist) ) {
			 found = 1;
                         break;
                     }
                 }
                 if (!found) continue;
	     }
	} else
	if ( Clist->field > 0 ) {
		int found = 0;

		for ( i = 0; i < Cats->n_cats; i++ ) {
			if ( Cats->field[i] == Clist->field ) {
				found = 1;
				break;
			}
		}
	        /* lines with no category will be displayed */
	        if (Cats->n_cats > 0 && !found) continue;
	}

	if( table_colors_flag ) {

	  /* only first category */
	  cat = Vect_get_line_cat ( Map, line,
			  (Clist->field > 0 ? Clist->field :
			   (Cats->n_cats > 0 ? Cats->field[0] : 1)));
	  
	  if (cat >= 0) {
	    G_debug (3, "display element %d, cat %d", line, cat);
	    
	    /* Read RGB colors from db for current area # */
	   
	    if (db_CatValArray_get_value (&cvarr_rgb, cat, &cv_rgb) != DB_OK) {
	      rgb = 0;
	    }
	    else {
	      sprintf (colorstring, "%s", db_get_string(cv_rgb -> val.s));
	    
	      if (*colorstring != '\0') {
		G_debug (3, "element %d: colorstring: %s", line, colorstring);
		
		if ( G_str_to_color(colorstring, &red, &grn, &blu) == 1) {
		  rgb = 1;
		  G_debug (3, "element:%d  cat %d r:%d g:%d b:%d", line, cat, red, grn, blu);
		} 
		else { 
		  rgb = 0;
		  G_warning(_("Error in color definition column (%s), element %d "
		    "with cat %d: colorstring [%s]"), rgb_column, line, cat, colorstring);
		}
	      }
	      else {
		rgb = 0;
		G_warning (_("Error in color definition column (%s), element %d with cat %d"),
		    rgb_column, line, cat);
	      }
	    }
	  } /* end if cat */
	  else {
	    rgb = 0;
	  } 
	} /* end if table_colors_flag */

	/* random colors */
	if( cats_color_flag ) {
	  rgb = 0;
	  if(Clist->field > 0){
	    cat = Vect_get_line_cat ( Map, line, Clist->field );
	    if( cat >= 0 ) {
	      G_debug (3, "display element %d, cat %d", line, cat);
	      /* fetch color number from category */
	      which = (cat % palette_ncolors);
	      G_debug (3,"cat:%d which color:%d r:%d g:%d b:%d", cat, which, 
		    palette[which].R, palette[which].G, palette[which].B);

	      rgb = 1;
	      red = palette[which].R;
	      grn = palette[which].G;
	      blu = palette[which].B;
	    }
	  } else
	  if(Cats->n_cats > 0){
	    /* fetch color number from layer */
	    which = (Cats->field[0] % palette_ncolors);
	    G_debug (3,"layer:%d which color:%d r:%d g:%d b:%d", Cats->field[0],
		   which, palette[which].R, palette[which].G, palette[which].B);

	    rgb = 1;
	    red = palette[which].R;
	    grn = palette[which].G;
	    blu = palette[which].B;
	  }
	}

	if( nrec_width ) {

	  /* only first category */
	  cat = Vect_get_line_cat ( Map, line,
			  (Clist->field > 0 ? Clist->field :
			   (Cats->n_cats > 0 ? Cats->field[0] : 1)));
	  
	  if (cat >= 0) {
	    G_debug (3, "display element %d, cat %d", line, cat);
	    
	    /* Read line width from db for current area # */
	   
	    if (db_CatValArray_get_value (&cvarr_width, cat, &cv_width) != DB_OK) {
	      width = default_width;
	    }
	    else {
	      width = width_scale * (cvarr_width.ctype==DB_C_TYPE_INT?
			      cv_width->val.i:(int)cv_width->val.d);
	      if (width < 0) {
		  G_warning(_("Error in line width column (%s), element %d "
		    "with cat %d: line width [%d]"), width_column, line, cat, width);
		  width = default_width;
	      }
	    }
	  } /* end if cat */
	  else {
	    width = default_width;
	  } 

	  R_line_width(width);
	} /* end if nrec_width */

	x = Points->x;
	y = Points->y;

        if ( (ltype & GV_POINTS) && Symb != NULL ) {
	  /* Note: this could/should be updated to use the new D_symbol() library function */
	  if ((color != -1 || fcolor != -1) || rgb) {
	    G_plot_where_xy(x[0], y[0], &x0, &y0);
	  }  
 
            for ( i = 0; i < Symb->count; i++ ) {
                part = Symb->part[i];

	        switch ( part->type ) {
		    case S_POLYGON:
			/* Note: it may seem to be strange to calculate coor in pixels, then convert
			 *       to E-N and plot. I hope that we get some D_polygon later. */
			if ( (part->fcolor.color == S_COL_DEFAULT && fcolor > -1) ||
			      part->fcolor.color == S_COL_DEFINED || rgb) 
			{
			    if (!table_colors_flag && !cats_color_flag) {
			      if ( part->fcolor.color == S_COL_DEFAULT )
				R_color(fcolor);
			      else
				R_RGB_color ( part->fcolor.r, part->fcolor.g, part->fcolor.b );
			  }
			  else {
			    if (rgb) {
			      R_RGB_color ((unsigned char) red, (unsigned char) grn, (unsigned char) blu);
			    }
			    else {
			      R_color (fcolor);
			    }
			  }

			    Vect_reset_line ( PPoints );

			    for ( j = 0; j < part->count; j++ ) { /* Construct polygon */
				chain = part->chain[j];
				for ( k = 0; k < chain->scount; k++ ) { 
				    xp  = x0 + chain->sx[k];
				    yp  = y0 - chain->sy[k];
				    G_plot_where_en ( xp, yp, &xd, &yd );
				    Vect_append_point ( PPoints, xd, yd, 0.0);
				}
				if ( j == 0 ) {
				    xd0 = PPoints->x[0];
				    yd0 = PPoints->y[0];
				} else {
				    Vect_append_point ( PPoints, xd0, yd0, 0.0);
				}
			    }
			    
			    G_plot_polygon ( PPoints->x, PPoints->y, PPoints->n_points);

			}
			if ( (part->color.color == S_COL_DEFAULT && color > -1 ) ||
			      part->color.color == S_COL_DEFINED  ) 
			{
			    if ( part->color.color == S_COL_DEFAULT ) {
				R_color(color);
			    } else {
			        R_RGB_color ( part->color.r, part->color.g, part->color.b );
			    }

			    for ( j = 0; j < part->count; j++ ) { 
				chain = part->chain[j];
				for ( k = 0; k < chain->scount; k++ ) { 
				    xp  = x0 + chain->sx[k];
				    yp  = y0 - chain->sy[k];
				    if ( k == 0 ) D_move_abs ( xp, yp );
				    else D_cont_abs ( xp, yp );

				}
			    }
			    
			}
			
                        break;
                    case S_STRING: 
			if ( part->color.color == S_COL_NONE ) break;
			else {
			  if (!table_colors_flag && !cats_color_flag) {
			    if ( part->color.color == S_COL_DEFAULT ) R_color(color) ;
			    else R_RGB_color ( part->color.r, part->color.g, part->color.b );
			  }
			  else {
			    if (ltype == GV_CENTROID) {
			      R_color (color);
			    }
			    else {
			      if (rgb) {
				R_RGB_color ((unsigned char) red, (unsigned char) grn, (unsigned char) blu);
			      }
			      else {
				R_color (color);
			      }
			    }
			  }
			}
			    
			chain = part->chain[0];

                        for ( j = 0; j < chain->scount; j++ ) { 
			    xp  = x0 + chain->sx[j];
			    yp  = y0 - chain->sy[j];
			    if ( j == 0 ) D_move_abs ( xp, yp );
			    else D_cont_abs ( xp, yp );

                        }
                        break;
                }
            }
	} else if (color > -1 || rgb) {
	  if (!table_colors_flag && !cats_color_flag) {
	    R_color (color);
	  }
	  else {
	    if (rgb) {
	      R_RGB_color ((unsigned char) red, (unsigned char) grn, (unsigned char) blu);
	    }
	    else {
	      R_color (color);
	    }
	  }
	    if ( Points->n_points == 1 ) { /* line with one coor */
	        G_plot_line(x[0], y[0], x[0], y[0]);
	    } else {
		for(i=1; i < Points->n_points; i++) {
		    G_plot_line(x[0], y[0], x[1], y[1]);
		    x++;
		    y++;
		  }
	    }
	}
    }

    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);
    
    return 0; /* not reached */
}


