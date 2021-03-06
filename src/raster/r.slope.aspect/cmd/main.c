#include "gis.h"
#define abs(x) ((x)<0?-(x):(x))

/**************************************************************
 * input is from command line.
 * arguments are elevation file, slope file and aspect file
 * elevation filename required
 * either slope filename or aspect filename required
 * usage: r.slope.aspect [-av] elevation=input slope=output1 aspect=output2
 * -a don't align window
 * -q quiet
 **************************************************************/

/*  some changes made to code to retrieve correct distances when using
    lat/lon projection.  changes involve recalculating H and V. see
    comments within code.                                           */


main (argc, argv) char *argv[];
{
    struct Colors colr;
    struct Categories cats;
    int verbose;
    int align;
    int zero_is_nodata;
    int elevation_fd;
    int aspect_fd ;
    int slope_fd ;
    CELL *elev_cell[3], *temp;
    CELL *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8, *c9;
    CELL *aspect_cell, *ac ;
    CELL *slope_cell, *sc ;
    int i;
    int Wrap;  /* global wraparound */
    struct Cell_head window, cellhd;

    char *elev_name;
    char *aspect_name;
    char *slope_name;
    char buf[300];
    char *mapset;
    int nrows, row;
    int ncols, col;

    double G_distance();
    double G_row_to_northing();
    double G_col_to_easting();
    double north, east, south, west, ns_med;

    double radians_to_degrees;
    double degrees_to_radians;
    double sqrt(), tan(), atan2();
    double H,V;
    double dx;              /* slope in ew direction */
    double dy;              /* slope in ns direction */
    double zfactor;
    double aspect;

    double answer[92];
    double degrees;
    double tan_ans;
    double key;
    double slp_in_perc;
    double min_slp;
    int low, hi, test;
    int deg=0;
    int perc=0;
    char *slope_fmt;
    struct
    {
	struct Option *elevation, *slope_fmt, *slope, *aspect, *zfactor, *min_slp;
    } parm;
    struct
    {
	struct Flag *a,*q,*z;
    } flag;


    parm.elevation = G_define_option() ;
    parm.elevation->key        = "elevation" ;
    parm.elevation->type       = TYPE_STRING ;
    parm.elevation->required   = YES ;
    parm.elevation->gisprompt  = "old,cell,raster" ;
    parm.elevation->description= "Raster elevation file name";

    parm.slope = G_define_option() ;
    parm.slope->key        = "slope" ;
    parm.slope->type       = TYPE_STRING ;
    parm.slope->required   = NO ;
    parm.slope->answer     = NULL ;
    parm.slope->gisprompt  = "any,cell,raster" ;
    parm.slope->description= "Output slope filename" ;

    parm.slope_fmt = G_define_option() ;
    parm.slope_fmt->key        = "format" ;
    parm.slope_fmt->type       = TYPE_STRING ;
    parm.slope_fmt->required   = NO ;
    parm.slope_fmt->answer     = "degrees";
    parm.slope_fmt->options  = "degrees,percent";
    parm.slope_fmt->description= "format for reporting the slope" ;

    parm.aspect = G_define_option() ;
    parm.aspect->key        = "aspect" ;
    parm.aspect->type       = TYPE_STRING ;
    parm.aspect->required   = NO ;
    parm.aspect->answer     = NULL ;
    parm.aspect->gisprompt  = "any,cell,raster" ;
    parm.aspect->description= "Output aspect filename" ;

    parm.zfactor = G_define_option();
    parm.zfactor->key         = "zfactor";
    parm.zfactor->description = "Multiplicative factor to convert elevation units to meters";
    parm.zfactor->type        = TYPE_DOUBLE;
    parm.zfactor->required    = NO;
    parm.zfactor->answer      = "1.0";

    parm.min_slp = G_define_option();
    parm.min_slp->key         = "min_slp";
    parm.min_slp->description = "Minimum slope value (in percent) for which aspect is computed.";
    parm.min_slp->type        = TYPE_DOUBLE;
    parm.min_slp->required    = NO;
    parm.min_slp->answer      = "0.0";

    flag.a = G_define_flag() ;
    flag.a->key         = 'a' ;
    flag.a->description = "Do not align the current region to the elevation layer" ;

    flag.q = G_define_flag() ;
    flag.q->key         = 'q' ;
    flag.q->description = "Quiet" ;

    flag.z = G_define_flag() ;
    flag.z->key         = 'z' ;
    flag.z->description = "Zero values in elevation layer are true elevations" ;

    G_gisinit (argv[0]);

    radians_to_degrees = 180.0 / 3.14159 ;
    degrees_to_radians = 3.14159 / 180.0 ;

    answer[0] = 0.0;
    answer[91] = 15000.0;

    for (i = 1; i < 91; i++)
    {
        degrees = i - .5;
        tan_ans = tan ( degrees  / radians_to_degrees );
        answer[i] = tan_ans * tan_ans;
    }


    if (G_parser(argc, argv))
        exit(-1);

    verbose = (!flag.q->answer);
    align   = (!flag.a->answer);
    zero_is_nodata =(!flag.z->answer);

    elev_name = parm.elevation->answer;
    slope_name = parm.slope->answer;
    aspect_name = parm.aspect->answer;
    if (sscanf (parm.zfactor->answer, "%lf", &zfactor) != 1 || zfactor <= 0.0)
    {
        fprintf (stderr, "ERROR: %s=%s - must be a postive number\n",
                       parm.zfactor->key, parm.zfactor->answer);
        G_usage();
        exit(1);
    }

    if (sscanf (parm.min_slp->answer, "%lf", &min_slp) != 1 || min_slp < 0.0)
    {
        fprintf (stderr, "ERROR: %s=%s - must be a non_negative number\n",
                       parm.min_slp->key, parm.min_slp->answer);
        G_usage();
        exit(1);
    }

    slope_fmt = parm.slope_fmt->answer;
    if(strcmp(slope_fmt,"percent")==0)perc=1;
    else if(strcmp(slope_fmt,"degrees")==0)deg=1;

    if (slope_name == NULL && aspect_name == NULL)
    {
	fprintf (stderr, "\nYou must specify at least one of the parameters <%s> or <%s>\n",
	    parm.slope->key, parm.aspect->key);
	G_usage();
	exit(1);
    }

    /* check elevation file existence */
    mapset = G_find_cell2(elev_name, "");
    if (!mapset)
    {
        sprintf (buf, "elevation file [%s] not found\n", elev_name);
        G_fatal_error (buf);
        exit(1);
    }
/* set the window from the header for the elevation file */
    if (align)
    {
	G_get_window (&window);
	if (G_get_cellhd (elev_name, mapset, &cellhd) >= 0)
	{
	    G_align_window (&window, &cellhd);
	    G_set_window (&window);
	}
    }
    G_get_set_window (&window);

    nrows = G_window_rows();
    ncols = G_window_cols();

    if (((window.west==(window.east-360.)) 
          ||(window.east==(window.west-360.)))&&
	  (G_projection()==PROJECTION_LL))
    {
       Wrap = 1;
       ncols+=2; 
    }
    else Wrap = 0;

    /* H = window.ew_res * 4 * 2/ zfactor;  /* horizontal (east-west) run 
                                   times 4 for weighted difference */
    /* V = window.ns_res * 4 * 2/ zfactor;  /* vertical (north-south) run 
                                   times 4 for weighted difference */

    G_begin_distance_calculations();
    north = G_row_to_northing(0.5, &window);
    ns_med = G_row_to_northing(1.5, &window);
    south = G_row_to_northing(2.5, &window);
    east =  G_col_to_easting(2.5, &window);
    west =  G_col_to_easting(0.5, &window);
    V = G_distance(east, north, east, south) * 4 / zfactor;
    H = G_distance(east, ns_med, west, ns_med) * 4 / zfactor;
    /*    ____________________________
	  |c1      |c2      |c3      |
	  |        |        |        |
	  |        |  north |        |        
	  |        |        |        |
	  |________|________|________|          
	  |c4      |c5      |c6      |
	  |        |        |        |
	  |  east  | ns_med |  west  |
	  |        |        |        |
	  |________|________|________|
	  |c7      |c8      |c9      |
	  |        |        |        |
	  |        |  south |        |
	  |        |        |        |
	  |________|________|________|
    */

    /* open the elevation file for reading */
    elevation_fd = G_open_cell_old (elev_name, mapset);
    if (elevation_fd < 0) exit(1);
    elev_cell[0] = (CELL *) G_calloc (ncols + 1, sizeof(CELL));
    elev_cell[1] = (CELL *) G_calloc (ncols + 1, sizeof(CELL));
    elev_cell[2] = (CELL *) G_calloc (ncols + 1, sizeof(CELL));

    if (slope_name != NULL)
    {
        slope_fd = opennew (slope_name);
        slope_cell = G_allocate_cell_buf();
        G_zero_cell_buf(slope_cell);
        G_put_map_row (slope_fd, slope_cell);
    }
    else
    {
	slope_cell = NULL;
	slope_fd = -1;
    }

    if (aspect_name != NULL)
    {
        aspect_fd = opennew (aspect_name);
        aspect_cell = G_allocate_cell_buf();
        G_zero_cell_buf (aspect_cell);
        G_put_map_row (aspect_fd, aspect_cell);
    }
    else
    {
	aspect_cell = NULL;
	aspect_fd = -1;
    }

    if (aspect_fd < 0 && slope_fd < 0)
        exit(1);

    if(Wrap)
    {
       G_get_map_row_nomask (elevation_fd, elev_cell[1]+1,0);
       elev_cell[1][0] = elev_cell[1][G_window_cols()-1];
       elev_cell[1][G_window_cols()+1]=elev_cell[1][2];
    }
    else G_get_map_row_nomask (elevation_fd, elev_cell[1],0);

    if(Wrap)
    {
       G_get_map_row_nomask (elevation_fd, elev_cell[2]+1,1);
       elev_cell[2][0] = elev_cell[2][G_window_cols()-1];
       elev_cell[2][G_window_cols()+1]=elev_cell[2][2];
    }
    else G_get_map_row_nomask (elevation_fd, elev_cell[2],1);

    if (verbose) fprintf (stderr, "percent complete: ");
    for (row = 2; row < nrows; row++)
    {

/*  if projection is Lat/Lon, recalculate  V and H   */

	if (G_projection()==PROJECTION_LL)
	{
          north = G_row_to_northing((row-2 + 0.5), &window);
          ns_med = G_row_to_northing((row-1 + 0.5), &window);
          south = G_row_to_northing((row + 0.5), &window);
          east =  G_col_to_easting(2.5, &window);
          west =  G_col_to_easting(0.5, &window);
          V = G_distance(east, north, east, south) * 4 / zfactor;
          H = G_distance(east, ns_med, west, ns_med) * 4 / zfactor;
/*        ____________________________
	  |c1      |c2      |c3      |
	  |        |        |        |
	  |        |  north |        |        
	  |        |        |        |
	  |________|________|________|          
	  |c4      |c5      |c6      |
	  |        |        |        |
	  |  east  | ns_med |  west  |
	  |        |        |        |
	  |________|________|________|
	  |c7      |c8      |c9      |
	  |        |        |        |
	  |        |  south |        |
	  |        |        |        |
	  |________|________|________|
*/
	}

        if (verbose) G_percent (row, nrows, 2);
        temp = elev_cell[0];
        elev_cell[0] = elev_cell[1];
        elev_cell[1] = elev_cell[2];
        if(Wrap)
        {
           G_get_map_row_nomask (elevation_fd, (elev_cell[2]= temp) +1, row);
           elev_cell[2][0] = elev_cell[2][G_window_cols()-1];
           elev_cell[2][G_window_cols()+1]=elev_cell[2][2];
        }
        else G_get_map_row_nomask (elevation_fd, elev_cell[2] = temp, row);

        c1 = elev_cell[0];
        c2 = c1+1;
        c3 = c1+2;
        c4 = elev_cell[1];
        c5 = c4+1;
        c6 = c4+2;
        c7 = elev_cell[2];
        c8 = c7+1;
        c9 = c7+2;

	if (aspect_fd >= 0)
	{
	    if(Wrap)
	       ac = aspect_cell;
            else 
	       ac = aspect_cell + 1;
        }
	if (slope_fd >= 0)
	{
	    if(Wrap)
	       sc = slope_cell;
            else
	       sc = slope_cell + 1;
        }
        /*skip first cell of the row*/

        for (col = ncols-2; col-- > 0; c1++,c2++,c3++,c4++,c5++,c6++,c7++,c8++,c9++)
        {
            /*  DEBUG:
            fprintf(stdout,"%d   %d   %d\n%d   %d   %d\n%d   %d   %d\n\n",
	         *c1,*c2,*c3,*c4,*c5,*c6,*c7,*c8,*c9);
            */

	    if(zero_is_nodata && (*c1==0 || *c2==0 || *c3==0 ||
		                  *c4==0 || *c5==0 || *c6==0 ||
		                  *c7==0 || *c8==0 || *c9==0))
	    {
		if(slope_fd > 0)
		    *sc++ = 0;
		if (aspect_fd > 0)
		    *ac++ = 0;
		continue;
	    }

	    dx = ((*c1 + *c4 + *c4 + *c7) - (*c3 + *c6 + *c6 + *c9)) / H;
	    dy = ((*c7 + *c8 + *c8 + *c9) - (*c1 + *c2 + *c2 + *c3)) / V;

            key = dx*dx + dy*dy;
	    slp_in_perc = 100*sqrt(key);  
	    if(slp_in_perc < min_slp) slp_in_perc = 0.;
	    if(deg)
	    {
               low = 1;
               hi = 91;
               test = 20;

               while (hi >= low)
               {
                   if ( key >= answer[test] )
                       low = test + 1;
                   else if ( key < answer[test-1] )
                       hi = test - 1;
                   else
                       break;
                   test = (low + hi) / 2;
               }
            }
	    else if(perc) test = slp_in_perc + 1.5;  /* All the categories are
						        incremented by 1 */

               if (slope_fd > 0)
                   *sc++ = (CELL) test;

            if (aspect_fd > 0)
            {
                if (key == 0) aspect = 0.;  
                else if (dx == 0)
                {
                    if (dy > 0) aspect = 90.;  
                    else aspect = 270.;   
                }
                else 
		{
		   aspect = (atan2(dy,dx)/degrees_to_radians);
		   if((aspect<=0.5)&&(aspect>0)) aspect=360.;
		   if(aspect<=0.)aspect=360.+aspect;
                }

                if(!((slope_fd > 0)&&(slp_in_perc < min_slp)))
		/* if it's not the case that the slope for this cell 
		is below specified minimum */
                    *ac++ = (CELL) (aspect + .5);
                else
		    *ac++ = 0;
             }
        }
        if (aspect_fd > 0)
            G_put_map_row (aspect_fd, aspect_cell);
        if (slope_fd > 0)
            G_put_map_row (slope_fd, slope_cell);
    }
    if (verbose) G_percent (row, nrows, 2);

    G_close_cell (elevation_fd);
    if (verbose)
        fprintf (stderr,"CREATING SUPPORT FILES\n");

    printf ("ELEVATION PRODUCTS for mapset [%s] in [%s]\n",
        G_mapset(), G_location());

    if (aspect_fd >= 0)
    {
        G_zero_cell_buf (aspect_cell);
        G_put_map_row (aspect_fd, aspect_cell);
        G_close_cell (aspect_fd);

        G_read_cats (aspect_name, G_mapset(), &cats);
        G_set_cats_title ("aspect in degrees from east", &cats);

	printf("%d categor%s of aspect\n", cats.num, cats.num==1?"y":"ies");
        G_set_cat ((CELL)0, "no aspect", &cats);
	    for(i=1;i<=cats.num;i++)
	    {
	       if(i==360)sprintf(buf,"east");
	       else if(i==360)sprintf(buf,"east");
	       else if(i==45)sprintf(buf,"north of east");
	       else if(i==90)sprintf(buf,"north");
	       else if(i==135)sprintf(buf,"north of west");
	       else if(i==180)sprintf(buf,"west");
	       else if(i==225)sprintf(buf,"south of west");
	       else if(i==270)sprintf(buf,"south");
	       else if(i==315)sprintf(buf,"south of east");
               else sprintf (buf, "%d degree%s from east", i, i==1?"":"s");
               G_set_cat ((CELL)(i), buf, &cats);
	    }
        G_write_cats (aspect_name, &cats);
        G_free_cats (&cats);

        sprintf(buf, "r.colors map='%s' c=aspect",
		G_fully_qualified_name (aspect_name, G_mapset()));
	system(buf);

        printf ("ASPECT [%s] COMPLETE\n", aspect_name);
    }

    if (slope_fd >= 0)
    {
        G_zero_cell_buf (slope_cell);
        G_put_map_row (slope_fd, slope_cell);
        G_close_cell (slope_fd);

        G_read_cats (slope_name, G_mapset(), &cats);
        if(deg) G_set_cats_title ("slope in degrees", &cats);
        else if(perc) G_set_cats_title ("percent slope", &cats);
        G_set_cat ((CELL)0, "no data", &cats);
	printf("%d categor%s of slope\n", cats.num, cats.num==1?"y":"ies");
        for (i = 0; i <cats.num; i++)
        {
            if(deg)sprintf (buf, "%d degree%s", i, i==1?"":"s");
            else if(perc)sprintf (buf, "%d percent", i);
            G_set_cat ((CELL)(i+1), buf, &cats);
        }
        G_write_cats (slope_name, &cats);
        printf ("SLOPE [%s] COMPLETE\n", slope_name);
    }

    exit(0);
}


