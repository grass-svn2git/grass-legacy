/**********************************************************************
 *
 *      This is the main program for simulating ellptical spread.
 *                  
 *      It
 *      1) determines the earliest time a phenomenon REACHES to a
 *         map cell, NOT the time that cell is EXHAUSTED.
 *      3) If a cell is spread barrier, a no-data value is assigned
 *         to it.
 *      
 *      Author:
 *         Jianping Xu 
 *         Center for Remote Sensing and Spatial Analysis (CRSSA)
 *         Rutgers University.
 *
 *********************************************************************/

#include "gis.h"
#define MAIN
#include "cmd_line.h"
#include "costHa.h"
#include "cell_ptrHa.h"
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
 
#define DATA(map, r, c)		(map)[(r) * ncols + (c)]

/*#define DEBUG*/

CELL	range_min, range_max;
CELL 	*cell;
CELL	*x_cell; 
CELL	*y_cell;

CELL	*map_max;
CELL	*map_dir;
CELL	*map_base;
CELL	*map_spotdist;
CELL	*map_velocity;
CELL	*map_mois;
float	*map_out;
CELL	*map_x_out;
CELL	*map_y_out;
CELL	*map_visit; 

char	buf[400];

float 	zero = 0.0;
float	neg = -2.0; 
/*float	PI = 3.14159; */

int	BARRIER = 0;
int	max_fd, dir_fd, base_fd, start_fd;
int	spotdist_fd, velocity_fd, mois_fd;
int	cum_fd, x_fd, y_fd;  
int	nrows, ncols;

long	heap_len;

struct Cell_head window;
struct Range 	range;

struct costHa 	*heap;


main(argc,argv)
int argc;
char *argv[];
{
	int col, row, srows, scols;
         
	float ros, cost,
        xstep_len, rrow, rcol, rstart_row, rstart_col, angle, 
	dir_angle, cos_dir_angle, sin_dir_angle, cos_angle, sin_angle;

	struct Cell_head cum_cellhd;            

        struct cell_ptrHa *to_cell, *old_to_cell, 
        *front_cell = NULL, *rear_cell = NULL, *insert2Ha();

	struct Colors colors;

	struct start_pt *pres_start_pt, *new_start_pt;


	double west, north;	
	double G_row_to_northing(), G_col_to_easting();

	struct {
		struct Option 	*max, *dir, *base, *start, 
				*spotdist, *velocity, *mois, 
				*least, *comp_dens, *init_time, 
				*time_lag, *backdrop, 
				*out, *x_out, *y_out;
	} parm;
        struct {
		struct Flag 	*display, *spotting, *verbose;
	} flag;


	parm.max = G_define_option() ;        
	parm.max->key        = "max" ;        
	parm.max->type       = TYPE_STRING ;  
	parm.max->required   = YES ;
	parm.max->gisprompt  = "old,cell,raster" ;
	parm.max->description= "Name of raster map containing MAX rate of spread (ROS) (cm/min)" ;

        parm.dir = G_define_option() ;
	parm.dir->key        = "dir" ;
	parm.dir->type       = TYPE_STRING ;
	parm.dir->required   = YES ;
	parm.dir->gisprompt  = "old,cell,raster" ;
	parm.dir->description= "Name of raster map containing DIRections of max ROS (degree)";

        parm.base = G_define_option() ;
	parm.base->key        = "base" ;
	parm.base->type       = TYPE_STRING ;
	parm.base->required   = YES ;
	parm.base->gisprompt  = "old,cell,raster" ;
	parm.base->description= "Name of raster map containing BASE ROS (cm/min)" ;

	parm.start = G_define_option() ;
	parm.start->key        = "start" ;
	parm.start->type       = TYPE_STRING ;
	parm.start->required   = YES ;
	parm.start->gisprompt  = "old,cell,raster" ;
	parm.start->description= "Name of raster map containing STARTing sources" ;

	parm.spotdist = G_define_option() ;
	parm.spotdist->key        = "spot_dist" ;
	parm.spotdist->type       = TYPE_STRING ;
	parm.spotdist->gisprompt  = "old,cell,raster" ;
	parm.spotdist->description= "Name of raster map containing max SPOTting DISTance (m) (required w/ -s)" ;

        parm.velocity = G_define_option() ;
        parm.velocity->key        = "w_speed" ;
        parm.velocity->type       = TYPE_STRING ;
        parm.velocity->gisprompt  = "old,cell,raster" ;
        parm.velocity->description= "Name of raster map containing midflame Wind SPEED (ft/min)\n\t\t\b(required w/ -s)";

        parm.mois = G_define_option() ;
        parm.mois->key        = "f_mois" ;
        parm.mois->type       = TYPE_STRING ;
        parm.mois->gisprompt  = "old,cell,raster" ;
        parm.mois->description= "Name of raster map containing fine Fuel MOISture of the \n\t\t\bcell receiving a spotting firebrand (%) (required w/ -s)";

        parm.least = G_define_option() ;
        parm.least->key        = "least_size" ;
        parm.least->type       = TYPE_STRING ;
        parm.least->key_desc   = "odd int" ;
        parm.least->options 	  = "3,5,7,9,11,13,15" ;
        parm.least->description= "Basic sampling window SIZE needed to meet certain accuracy (3)" ;

        parm.comp_dens = G_define_option() ;
        parm.comp_dens->key        = "comp_dens" ;
        parm.comp_dens->type       = TYPE_STRING ;
        parm.comp_dens->key_desc   = "decimal" ;
        parm.comp_dens->description= "Sampling DENSity for additional COMPuting \n\t\t\brange: 0.0 - 1.0 (0.5)" ;

        parm.init_time = G_define_option() ;
        parm.init_time->key        = "init_time" ;
        parm.init_time->type       = TYPE_STRING ;
        parm.init_time->key_desc   = "int (>= 0)" ;
        parm.init_time->description= "INITial TIME for current simulation (0) (min)" ;

        parm.time_lag = G_define_option() ;
        parm.time_lag->key        = "lag" ;
        parm.time_lag->type       = TYPE_STRING ;
        parm.time_lag->key_desc   = "int (>= 0)" ;
        parm.time_lag->description= "Simulating time duration LAG (fill the region) (min)" ;

        parm.backdrop = G_define_option() ;
        parm.backdrop->key        = "backdrop" ;
        parm.backdrop->type       = TYPE_STRING ;
        parm.backdrop->gisprompt  = "old,cell,raster" ;
        parm.backdrop->description= "Name of raster map as a display backdrop";

	parm.out = G_define_option() ;
	parm.out->key        = "output" ;
	parm.out->type       = TYPE_STRING ;
	parm.out->required   = YES ;
	parm.out->gisprompt  = "new,cell,raster" ;
	parm.out->description= "Name of raster map to contain OUTPUT spread time (min)";

	parm.x_out = G_define_option() ;
	parm.x_out->key        = "x_output" ;
	parm.x_out->type       = TYPE_STRING ;
	parm.x_out->gisprompt  = "new,cell,raster" ;
	parm.x_out->description= "Name of raster map to contain X_BACK coordiates";

	parm.y_out = G_define_option() ;
	parm.y_out->key        = "y_output" ;
	parm.y_out->type       = TYPE_STRING ;
	parm.y_out->gisprompt  = "new,cell,raster" ;
	parm.y_out->description= "Name of raster map to contain Y_BACK coordiates";

	flag.verbose = G_define_flag();
	flag.verbose->key = 'v';
	flag.verbose->description = "Run VERBOSELY";

	flag.display = G_define_flag();
	flag.display->key = 'd';
	flag.display->description = "DISPLAY 'live' spread process on screen";

	flag.spotting = G_define_flag();
	flag.spotting->key = 's';
	flag.spotting->description = "For wildfires: consider SPOTTING effect";


	/* initialize access to database and create temporary files */

	G_gisinit (argv[0]);

	srand(getpid()); 

	/*   Parse command line */
	if (G_parser(argc, argv))
		exit(-1);

	verbose = flag.verbose->answer;
	display = flag.display->answer;
	spotting = flag.spotting->answer;
	
	max_layer = parm.max->answer;
	dir_layer = parm.dir->answer;
	base_layer = parm.base->answer;
	start_layer = parm.start->answer;
	backdrop_layer = parm.backdrop->answer;
	out_layer = parm.out->answer;
	if (parm.x_out->answer) {x_out = 1; x_out_layer = parm.x_out->answer;}
	if (parm.y_out->answer)	{y_out = 1; y_out_layer = parm.y_out->answer;}
	if (spotting) {
		if (!(parm.spotdist->answer && parm.velocity->answer && parm.mois->answer)) {
			printf ("Error: SPOTTING DISTANCE, fuel MOISTURE, or wind VELOCITY map not given w/ -s\n");
			G_usage();
			exit(1);
		} else {
			spotdist_layer = parm.spotdist->answer;
			velocity_layer = parm.velocity->answer;
			mois_layer = parm.mois->answer;
		}
	}
       /*Check the given the least sampling size, assign the default if needed*/
        if (parm.least->answer)
		least = atoi (parm.least->answer);
	else 	least = 3;
        /*Check the given computing density, assign the default if needed*/
        if (parm.comp_dens->answer)
        {       comp_dens = atof (parm.comp_dens->answer);
                if (comp_dens < 0.0 || comp_dens > 1.0)
                {       fprintf(stderr, "Illegal computing density <%s>\n",                                     parm.comp_dens->answer);
                        G_usage();
                        exit(1);
                } 
        } else
        {       comp_dens = 0.5;
        } 
        /*Check the given initial time and simulation time lag, assign the default if needed*/
        if (parm.init_time->answer)
        {       init_time = atoi (parm.init_time->answer);
                if (init_time < 0)
                {       fprintf(stderr, "Illegal initial time <%s>\n",
                            parm.init_time->answer);
                        G_usage();
                        exit(1);
                }
        } else
        {       time_lag = 0;
        }
        if (parm.time_lag->answer)
        {       time_lag = atoi (parm.time_lag->answer);
                if (time_lag < 0)
                {       fprintf(stderr, "Illegal simulating time lag <%s>\n",                               parm.time_lag->answer);
                        G_usage();
                        exit(1);
                } 
        } else
        {       time_lag = 99999;
        }       
            
	/*  Get database window parameters  */

	if(G_get_window (&window) < 0) {
		sprintf (buf,"can't read current window parameters");
		G_fatal_error (buf);
		exit(1);
	} 

	/*  find number of rows and columns in window    */

	nrows = G_window_rows();
	ncols = G_window_cols();

	/*transfor measurement unit from meters to centimeters due to ROS unit
	 *if the input ROSs are in m/min units, cancell the following*/
	window.ns_res = 100*window.ns_res;
	window.ew_res = 100*window.ew_res;

	/* Initialize display screens */
	if (display)
		display_init ();	

	/*  Check if input layers exists in data base  */   
	
        if (G_find_cell2 (max_layer, "")  == NULL) {
		sprintf(buf, "%s - not found", max_layer);
		G_fatal_error (buf);
		exit(1);
	}  

        if (G_find_cell2 (dir_layer, "")  == NULL) {
		sprintf(buf, "%s - not found", dir_layer);
		G_fatal_error (buf);
		exit(1);
        }

        if (G_find_cell2 (base_layer, "")  == NULL) {
		sprintf(buf, "%s - not found", base_layer);
		G_fatal_error (buf);
		exit(1);
        }	

        if (G_find_cell2 (start_layer, "")  == NULL) {
		sprintf(buf, "%s - not found", start_layer);
		G_fatal_error (buf);
		exit(1);
        }	

	if (spotting) {
        	if (G_find_cell2 (spotdist_layer, "")  == NULL) {
			sprintf(buf, "%s - not found", spotdist_layer);
			G_fatal_error (buf);
			exit(1);
		}	
        	if (G_find_cell2 (velocity_layer, "")  == NULL) {
			sprintf(buf, "%s - not found", velocity_layer);
			G_fatal_error (buf);
			exit(1);
		}	
        	if (G_find_cell2 (mois_layer, "")  == NULL) {
			sprintf(buf, "%s - not found", mois_layer);
			G_fatal_error (buf);
			exit(1);
		}	
        }	


	/*  Check if specified output layer names ARE LEGAL or EXISTS   */

	if (G_legal_filename (out_layer) < 0) {
		sprintf(buf, "%s - illegal name", out_layer);
		G_fatal_error (buf);
		exit(1);
	}
        if (G_find_cell2 (out_layer, G_mapset())) {
                sprintf(buf, "%s - exits in Mapset <%s>, select another name", out_layer, G_mapset());
                G_fatal_error (buf);
                exit(1);
        }

	if (x_out) {
		if (G_legal_filename (x_out_layer) < 0) {
			sprintf(buf, "%s - illegal name", x_out_layer);
			G_fatal_error (buf);
			exit(1);
		}
        	if (G_find_cell2 (x_out_layer, G_mapset())) {
                	sprintf(buf, "%s - exits in Mapset <%s>, select another name", x_out_layer, G_mapset());
                	G_fatal_error (buf);
                	exit(1);
        	}
	}

	if (y_out) {
		if (G_legal_filename (y_out_layer) < 0) {
			sprintf(buf, "%s - illegal name", y_out_layer);
			G_fatal_error (buf);
			exit(1);
		}
        	if (G_find_cell2 (y_out_layer, G_mapset())) {
            	    sprintf(buf, "%s - exits in Mapset <%s>, select another name", y_out_layer, G_mapset());
             	   G_fatal_error (buf);
              	  exit(1);
        	}
	}

	/*  Open input cell layers for reading  */

	max_fd = G_open_cell_old(max_layer, G_find_cell2 (max_layer,""));
	if (max_fd < 0) {
		sprintf (buf, "%s - can't open raster file", max_layer);
		G_fatal_error (buf);
		exit(1);
	}

        dir_fd = G_open_cell_old(dir_layer, G_find_cell2 (dir_layer,""));
	if (dir_fd < 0) {
		sprintf (buf, "%s - can't open raster file", dir_layer);
		G_fatal_error (buf);
		exit(1);
	}

        base_fd = G_open_cell_old(base_layer, G_find_cell2 (base_layer,""));
	if (base_fd < 0) {
		sprintf (buf, "%s - can't open raster file", base_layer);
		G_fatal_error (buf);
		exit(1);
	}

	if (spotting) {
        	spotdist_fd = G_open_cell_old(spotdist_layer, G_find_cell2 (spotdist_layer,""));
		if (spotdist_fd < 0) {
			sprintf (buf, "%s - can't open raster file", spotdist_layer);
			G_fatal_error (buf);
			exit(1);
		}
        	velocity_fd = G_open_cell_old(velocity_layer, G_find_cell2 (velocity_layer,""));
		if (velocity_fd < 0) {
			sprintf (buf, "%s - can't open raster file", velocity_layer);
			G_fatal_error (buf);
			exit(1);
		}
        	mois_fd = G_open_cell_old(mois_layer, G_find_cell2 (mois_layer,""));
		if (mois_fd < 0) {
			sprintf (buf, "%s - can't open raster file", mois_layer);
			G_fatal_error (buf);
			exit(1);
		}
	}

	/*  Allocate memories for a row  */
	cell = G_allocate_cell_buf();
	if (x_out)	x_cell = G_allocate_cell_buf();
	if (y_out)	y_cell = G_allocate_cell_buf();

	/*  Allocate memories for a map  */
	map_max = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	map_dir = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	map_base = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	map_visit = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	map_out = (float *) calloc (nrows*ncols + 1, sizeof(float));
	if (spotting) {
		map_spotdist = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
		map_velocity = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
		map_mois = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	}
	if (x_out) map_x_out = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	if (y_out) map_y_out = (CELL *) calloc (nrows*ncols + 1, sizeof(CELL));
	

	/*   Write the input layers in the map "arrays"  */

	if (verbose)
	        fprintf (stderr, "\nReading inputs ...");

	for( row=0 ; row<nrows ; row++ ) {
		if (verbose)
		        G_percent (row, nrows, 2);
		if( G_get_map_row(max_fd, cell, row)<0)
			exit(1);
		for (col=0; col<ncols; col++)
			DATA(map_max, row, col) = cell[col];
		if( G_get_map_row(dir_fd, cell, row)<0)
			exit(1);
		for (col=0; col<ncols; col++)
			DATA(map_dir, row, col) = cell[col];
		if( G_get_map_row(base_fd, cell, row)<0)
			exit(1);
		for (col=0; col<ncols; col++)
			DATA(map_base, row, col) = cell[col];
		if (spotting) {
			if( G_get_map_row(spotdist_fd, cell, row)<0)
				exit(1);
			for (col=0; col<ncols; col++)
				DATA(map_spotdist, row, col) = cell[col];
			if( G_get_map_row(velocity_fd, cell, row)<0)
				exit(1);
			for (col=0; col<ncols; col++)
				DATA(map_velocity, row, col) = cell[col];
			if( G_get_map_row(mois_fd, cell, row)<0)
				exit(1);
			for (col=0; col<ncols; col++)
				DATA(map_mois, row, col) = cell[col];
		}
	}
	if (verbose)
	    G_percent (row, nrows, 2);


	/*   Scan the START layer searching for starting points.
 	 *   Create an array of starting points (min_heap) ordered by costs.
	 */

        start_fd = G_open_cell_old(start_layer, G_find_cell2 (start_layer,""));
	if (start_fd < 0) {
		sprintf (buf, "%s - can't open raster file", start_layer);
		G_fatal_error (buf);
		exit(1);
	}

	G_read_range(start_layer, G_find_file("cell",start_layer,""), &range);
	G_get_range_min_max (&range, &range_min, &range_max);

        /*  Initialize the heap  */
        heap = (struct costHa *) calloc(nrows*ncols + 1, sizeof(struct costHa));
        heap_len = 0;

	if (verbose)
		fprintf (stderr, "Reading %s ... ", start_layer);
#ifdef DEBUG
printf ("Collecting origins ...");
#endif
	collect_ori (start_fd);
#ifdef DEBUG
printf ("Done\n");
#endif


	/* Major computation of spread time */
#ifdef DEBUG
printf ("Spreading ...");
#endif
	spread ();      
#ifdef DEBUG
printf ("Done\n");
#endif


	/*  Open cumulative cost layer (and x, y direction layers) for writing*/

	cum_fd = G_open_cell_new(out_layer);
	if (x_out)	x_fd = G_open_cell_new(x_out_layer);
	if (y_out)	y_fd = G_open_cell_new(y_out_layer);

	/* prepare output -- adjust from cm to m */
	window.ew_res = window.ew_res/100;
	window.ns_res = window.ns_res/100;

	/* copy maps in ram to output maps */
	ram2out ();

	free(map_max); 
	free(map_dir); 
	free(map_base); 
	free(map_out); 
	free(map_visit);
	if (x_out)	free(map_x_out);
	if (y_out)	free(map_y_out);
	if (spotting) { free(map_spotdist); 
			free(map_mois); 
			free(map_velocity); 
	}
	
	G_close_cell(max_fd);	
        G_close_cell(dir_fd);
        G_close_cell(base_fd);
	G_close_cell(start_fd);
	G_close_cell(cum_fd);
	if (x_out)	G_close_cell(x_fd);
	if (y_out)	G_close_cell(y_fd);
	if (spotting) { G_close_cell(spotdist_fd); 
			G_close_cell(velocity_fd);
			G_close_cell(mois_fd); 
	} 

	/* close graphics */
	if (display)
		display_close ();

        exit(0);
} 
