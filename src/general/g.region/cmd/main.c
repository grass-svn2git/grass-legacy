/***************************************************************************
 * $Id$
 *
 * MODULE: 	g.region (commandline)
 * AUTHOR(S):	Michael Shapiro, CERL
 *              datum added by Andreas Lange <andreas.lange@rhein-main.de>
 * PURPOSE: 	Program to manage and print the boundary definitions for the
 *              geographic region.
 * 
 * COPYRIGHT:  	(C) 2000 by the GRASS Development Team
 *
 *   	    	This program is free software under the GPL (>=v2)
 *   	    	Read the file COPYING that comes with GRASS for details.
 ****************************************************************************
 *
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gis.h"
#include "site.h"
#include "Vect.h"
#include "local_proto.h"

static int nsew(char *,char *,char *,char *);
static void die(struct Option *);
static char *llinfo(char *,char *,int);

int main (int argc, char *argv[])
{
	int i;
	int print_flag, dist_flag;
	int set_flag;
	double x;
	struct Cell_head window, temp_window;
	char msg[200];
	char *value;
	char *name;
	char *mapset;
	char *err;
	int projection;

	struct GModule *module;
	struct
	    {
		struct Flag
		*update,
		*print,
		*gprint,
		*lprint,
		*eprint,
		*center,
		*res_set,
		*dist_res,
		*dflt;
	} flag;
	struct
	    {
		struct Option
		*north,*south,*east,*west,
		*res, *nsres, *ewres,
		*save, *region, *view,
		*raster, *align, *zoom, *vect, *sites;
	} parm;

	G_gisinit (argv[0]);

	module = G_define_module();
	module->description =
		"Program to manage the boundary definitions for the "
		"geographic region.";

	/* flags */

	flag.dflt = G_define_flag();
	flag.dflt->key         = 'd';
	flag.dflt->description = "Set from default region";

	flag.gprint = G_define_flag();
	flag.gprint->key         = 'g';
	flag.gprint->description = "Print the current region (shell script style)";

	flag.print = G_define_flag();
	flag.print->key         = 'p';
	flag.print->description = "Print the current region";

	flag.lprint = G_define_flag();
	flag.lprint->key         = 'l';
	flag.lprint->description = "Print the current region in lat/long";

	flag.eprint = G_define_flag();
	flag.eprint->key         = 'e';
	flag.eprint->description = "Print the current region extent";

	flag.center = G_define_flag();
	flag.center->key         = 'c';
	flag.center->description = "Print the current region map center coordinates";

        flag.dist_res= G_define_flag();
        flag.dist_res->key         = 'm';
        flag.dist_res->description = "Print region resolution in meters (geodesic)";

        flag.res_set= G_define_flag();
        flag.res_set->key         = 'a';
        flag.res_set->description = "Align region to resolution [default = align to bounds]";

	flag.update = G_define_flag();
	flag.update->key         = 'u';
	flag.update->description = "Do not update the current region";

	/* parameters */

	parm.region = G_define_option();
	parm.region->key         = "region";
	parm.region->key_desc    = "name";
	parm.region->required    = NO;
	parm.region->multiple    = NO;
	parm.region->type        = TYPE_STRING;
	parm.region->description = "Set current region from named region";

	parm.raster = G_define_option();
	parm.raster->key         = "raster";
	parm.raster->key_desc    = "name";
	parm.raster->required    = NO;
	parm.raster->multiple    = NO;
	parm.raster->type        = TYPE_STRING;
	parm.raster->description = "Set region to match this raster map";

	parm.vect = G_define_option();
	parm.vect->key         = "vector";
	parm.vect->key_desc    = "name";
	parm.vect->required    = NO;
	parm.vect->multiple    = NO;
	parm.vect->type        = TYPE_STRING;
	parm.vect->description = "Set region to match this vector map";

	parm.sites = G_define_option();
	parm.sites->key         = "sites";
	parm.sites->key_desc    = "name";
	parm.sites->required    = NO;
	parm.sites->multiple    = NO;
	parm.sites->type        = TYPE_STRING;
	parm.sites->description = "Set region to match this sites map";

	parm.view = G_define_option();
	parm.view->key         = "3dview";
	parm.view->key_desc    = "name";
	parm.view->required    = NO;
	parm.view->multiple    = NO;
	parm.view->type        = TYPE_STRING;
	parm.view->description = "Set region to match this 3dview file";

	parm.north = G_define_option();
	parm.north->key         = "n";
	parm.north->key_desc    = "value";
	parm.north->required    = NO;
	parm.north->multiple    = NO;
	parm.north->type        = TYPE_STRING;
	parm.north->description = llinfo("Value for the northern edge", G_lat_format_string(), window.proj);

	parm.south = G_define_option();
	parm.south->key         = "s";
	parm.south->key_desc    = "value";
	parm.south->required    = NO;
	parm.south->multiple    = NO;
	parm.south->type        = TYPE_STRING;
	parm.south->description = llinfo("Value for the southern edge", G_lat_format_string(), window.proj);

	parm.east = G_define_option();
	parm.east->key         = "e";
	parm.east->key_desc    = "value";
	parm.east->required    = NO;
	parm.east->multiple    = NO;
	parm.east->type        = TYPE_STRING;
	parm.east->description = llinfo("Value for the eastern edge ", G_lon_format_string(), window.proj);

	parm.west = G_define_option();
	parm.west->key         = "w";
	parm.west->key_desc    = "value";
	parm.west->required    = NO;
	parm.west->multiple    = NO;
	parm.west->type        = TYPE_STRING;
	parm.west->description = llinfo("Value for the western edge ", G_lon_format_string(), window.proj);

	parm.res = G_define_option();
	parm.res->key         = "res";
	parm.res->key_desc    = "value";
	parm.res->required    = NO;
	parm.res->multiple    = NO;
	parm.res->type        = TYPE_STRING;
	parm.res->description = "Grid resolution (both north-south and east-west)";

	parm.nsres = G_define_option();
	parm.nsres->key         = "nsres";
	parm.nsres->key_desc    = "value";
	parm.nsres->required    = NO;
	parm.nsres->multiple    = NO;
	parm.nsres->type        = TYPE_STRING;
	parm.nsres->description = llinfo("North-south grid resolution", G_llres_format_string(), window.proj);

	parm.ewres = G_define_option();
	parm.ewres->key         = "ewres";
	parm.ewres->key_desc    = "value";
	parm.ewres->required    = NO;
	parm.ewres->multiple    = NO;
	parm.ewres->type        = TYPE_STRING;
	parm.ewres->description = llinfo("East-west grid resolution  ", G_llres_format_string(), window.proj);

	parm.zoom = G_define_option();
	parm.zoom->key         = "zoom";
	parm.zoom->key_desc    = "name";
	parm.zoom->required    = NO;
	parm.zoom->multiple    = NO;
	parm.zoom->type        = TYPE_STRING;
	parm.zoom->description = "Raster map to zoom into";

	parm.align = G_define_option();
	parm.align->key         = "align";
	parm.align->key_desc    = "name";
	parm.align->required    = NO;
	parm.align->multiple    = NO;
	parm.align->type        = TYPE_STRING;
	parm.align->description = "Raster map to align to";

	parm.save = G_define_option();
	parm.save->key         = "save";
	parm.save->key_desc    = "name";
	parm.save->required    = NO;
	parm.save->multiple    = NO;
	parm.save->type        = TYPE_STRING;
	parm.save->description = "Name the current region";

	if (G_parser(argc,argv))
		exit(1);

	/* get current region.
	 * if current region not valid, set it from default
	 * note: G_get_default_window() dies upon error
	 */
	if (G__get_window (&window, "", "WIND", G_mapset()) != NULL)
	{
		G_get_default_window (&window);
		G_put_window (&window);
	}
	projection = window.proj;

	set_flag = ! flag.update->answer;
	if (flag.eprint->answer)
		print_flag = 5;
	else if (flag.center->answer)
		print_flag = 4;
	else if (flag.lprint->answer)
		print_flag = 3;
	else if (flag.gprint->answer)
		print_flag = 2;
	else if (flag.print->answer)
		print_flag = 1;
	else
		print_flag = 0;

	/* Flag for reporting distance in meters */
	if (flag.dist_res->answer) {
		dist_flag = 1;
		/* Set -g default output */
		if ( print_flag == 0)
		print_flag = 2;
	} else
		dist_flag = 0;

	if (flag.dflt->answer)
		G_get_default_window (&window);

	/* region= */
	if (name = parm.region->answer)
	{
		mapset = G_find_file ("windows", name, "");
		if (!mapset)
			G_fatal_error ("region <%s> not found", name);
		if (G__get_window (&window, "windows", name, mapset) != NULL)
			G_fatal_error ("can't read region <%s> in <%s>", name, mapset);
	}

	/* 3dview= */
	if (name = parm.view->answer)
	{
		struct G_3dview v;
		FILE *fp;
		int ret;
		
		mapset = G_find_file2 ("3d.view", name, "");
		if (!mapset)
			G_fatal_error ("3dview file <%s> not found", name);

		G_3dview_warning(0); /* suppress boundary mismatch warning */

		if(NULL == (fp = G_fopen_old("3d.view",name,mapset)))
		    G_fatal_error ("can't open 3dview file <%s> in <%s>", name, mapset);

		G_copy (&temp_window, &window, sizeof(window));

		if(0 > (ret = G_get_3dview(name, mapset, &v)))
		    G_fatal_error ("can't read 3dview file <%s> in <%s>", name, mapset);
		if (ret == 0)
		    G_fatal_error ("Old 3dview file. Region not found in <%s> in <%s>", name, mapset);

                 
		window.north = v.vwin.north;
		window.south = v.vwin.south;
		window.west  = v.vwin.west;
		window.east  = v.vwin.east;

		window.rows = v.vwin.rows;
		window.cols = v.vwin.cols;
		window.ns_res = v.vwin.ns_res;
		window.ew_res = v.vwin.ew_res;

		fclose (fp);

	}

	/* raster= */
	if (name = parm.raster->answer)
	{
		mapset = G_find_cell2 (name, "");
		if (!mapset)
		{
			sprintf (msg, "raster map <%s> not found", name);
			G_fatal_error (msg);
		}
		if (G_get_cellhd (name, mapset, &window) < 0)
		{
			sprintf (msg, "can't read header for <%s> in <%s>",
			    name, mapset);
			G_fatal_error (msg);
		}
	}

	/* vect= */
	if (name = parm.vect->answer)
	{
		struct Map_info Map;
		mapset = G_find_vector2 (name, "");
		if (!mapset)
		{
			sprintf (msg, "vector map <%s> not found", name);
			G_fatal_error (msg);
		}

		G_copy (&temp_window, &window, sizeof(window));

		Vect_set_open_level (1);
		if (1 != Vect_open_old (&Map, name, mapset))
		{
			sprintf (msg, "can't open vector file <%s> in <%s>", name, mapset);
			G_fatal_error (msg);
		}

		window.north = Map.head.N;
		window.south = Map.head.S;
		window.west  = Map.head.W;
		window.east  = Map.head.E;

       	        if(window.north == window.south)
       	        {
       	              window.north = window.north + 0.5 * temp_window.ns_res;
                      window.south = window.south - 0.5 * temp_window.ns_res;
                }
                if(window.east==window.west)
                {
                      window.west = window.west - 0.5 * temp_window.ew_res;
                      window.east = window.east + 0.5 * temp_window.ew_res;
                }

		G_align_window (&window, &temp_window);

		Vect_close (&Map);
	}

	/* sites= */
	if (name = parm.sites->answer)
	{
		FILE *fp;
		int i, rtype, ndim, nstr, ndec;
                Site *mysite;

		mapset = G_find_sites2 (name, "");
		if (!mapset)
		{
			sprintf (msg, "sites map <%s> not found", name);
			G_fatal_error (msg);
		}
		if (NULL == (fp = G_fopen_sites_old (name, mapset)))
		{
			sprintf (msg, "Could not open sites map <%s>", name);
			G_fatal_error (msg);
		}

		rtype = -1;
		G_site_describe(fp, &ndim, &rtype, &nstr, &ndec);
		mysite = G_site_new_struct(rtype, ndim, nstr, ndec);

		for (i = 0; G_site_get (fp, mysite) == 0; i++)
		{
			if (i==0)
			{
				G_copy (&temp_window, &window, sizeof(window));
				window.east = window.west = mysite->east;
				window.north = window.south = mysite->north;
			}
			else
			{
				if (mysite->east > window.east) 
					window.east = mysite->east;
				if (mysite->east < window.west) 
					window.west = mysite->east;
				if (mysite->north > window.north) 
					window.north = mysite->north;
				if (mysite->north < window.south) 
					window.south = mysite->north;
			}
		}
		G_free(mysite);
		fclose (fp);
		if (i)
		{
		     window.east += 100;
		     window.west -= 100;
		     window.south -= 100;
		     window.north += 100;

       	             if(window.north == window.south)
       	             {
       	                   window.north = window.north + 0.5 * temp_window.ns_res;
                           window.south = window.south - 0.5 * temp_window.ns_res;
                     }
                     if(window.east==window.west)
                     {
                           window.west = window.west - 0.5 * temp_window.ew_res;
                           window.east = window.east + 0.5 * temp_window.ew_res;
                     }

		     G_align_window (&window, &temp_window);
                }
	}



	/* n= */
	if (value = parm.north->answer)
	{
		if(i = nsew(value, "n+", "n-", "s+"))
		{
			if (!G_scan_resolution (value+2, &x, window.proj))
				die(parm.north);
			switch(i)
			{
			case 1:
				window.north += x;
				break;
			case 2:
				window.north -= x;
				break;
			case 3:
				window.north = window.south + x;
				break;
			}
		}
		else if (G_scan_northing (value, &x, window.proj))
			window.north = x;
		else
			die(parm.north);
	}

	/* s= */
	if (value = parm.south->answer)
	{
		if(i = nsew(value, "s+", "s-", "n-"))
		{
			if (!G_scan_resolution (value+2, &x, window.proj))
				die(parm.south);
			switch(i)
			{
			case 1:
				window.south += x;
				break;
			case 2:
				window.south -= x;
				break;
			case 3:
				window.south = window.north - x;
				break;
			}
		}
		else if (G_scan_northing (value, &x, window.proj))
			window.south = x;
		else
			die(parm.south);
	}

	/* e= */
	if (value = parm.east->answer)
	{
		if(i = nsew(value, "e+", "e-", "w+"))
		{
			if (!G_scan_resolution (value+2, &x, window.proj))
				die(parm.east);
			switch(i)
			{
			case 1:
				window.east += x;
				break;
			case 2:
				window.east -= x;
				break;
			case 3:
				window.east = window.west + x;
				break;
			}
		}
		else if (G_scan_easting (value, &x, window.proj))
			window.east = x;
		else
			die(parm.east);
	}

	/* w= */
	if (value = parm.west->answer)
	{
		if(i = nsew(value, "w+", "w-", "e-"))
		{
			if (!G_scan_resolution (value+2, &x, window.proj))
				die(parm.west);
			switch(i)
			{
			case 1:
				window.west += x;
				break;
			case 2:
				window.west -= x;
				break;
			case 3:
				window.west = window.east - x;
				break;
			}
		}
		else if (G_scan_easting (value, &x, window.proj))
			window.west = x;
		else
			die(parm.west);
	}

	/* res= */
	if (value = parm.res->answer)
	{
		if (!G_scan_resolution (value, &x, window.proj))
			die(parm.res);
		window.ns_res = x;
		window.ew_res = x;
	if (flag.res_set->answer) {
		window.north =  ceil(window.north/x) * x ;
		window.south = floor(window.south/x) * x ;
		window.east = ceil(window.east/x) * x ;
		window.west = floor(window.west/x) * x ;
                }
	}

	/* nsres= */
	if (value = parm.nsres->answer)
	{
		if (!G_scan_resolution (value, &x, window.proj))
			die(parm.nsres);
		window.ns_res = x;
	if (flag.res_set->answer) {
		window.north = 2 * x * ( (int)(window.north/2/x));
                window.south = 2 * x * ( (int)(window.south/2/x));
		}
	}

	/* ewres= */
	if (value = parm.ewres->answer)
	{
		if (!G_scan_resolution (value, &x, window.proj))
			die(parm.ewres);
		window.ew_res = x;
	if (flag.res_set->answer) {
		window.east =  2 * x * ( (int)(window.east/2/x));
                window.west =  2 * x * ( (int)(window.west/2/x));
		}
	}

	/* zoom= */
	if (name = parm.zoom->answer)
	{
		mapset = G_find_cell2 (name, "");
		if (!mapset)
		{
			sprintf (msg, "raster map <%s> not found", name);
			G_fatal_error (msg);
		}
		zoom (&window, name, mapset);
	}

	/* align= */
	if (name = parm.align->answer)
	{
		mapset = G_find_cell2 (name, "");
		if (!mapset)
		{
			sprintf (msg, "raster map <%s> not found", name);
			G_fatal_error (msg);
		}
		if (G_get_cellhd (name, mapset, &temp_window) < 0)
		{
			sprintf (msg, "can't read header for <%s> in <%s>",
			    name, mapset);
			G_fatal_error (msg);
		}
		if (err = G_align_window (&window, &temp_window))
		{
			sprintf (msg, "%s in %s: %s", name, mapset, err);
			G_fatal_error (msg);
		}
	}

	/* save= */
	if (name = parm.save->answer)
	{
		if (G_legal_filename (name) < 0)
		{
			sprintf (msg, "<%s> - illegal region name", name);
			G_fatal_error (msg);
		}
		G_copy (&temp_window, &window, sizeof(window));
		adjust_window (&temp_window);
		if (G__put_window (&temp_window, "windows", name) < 0)
		{
			sprintf (msg, "can't write region <%s>", name);
			G_fatal_error (msg);
		}
	}

	adjust_window (&window);
	if (set_flag)
	{
		if (G_put_window (&window) < 0)
			G_fatal_error ("unable to update current region");
	}
	if (print_flag)
	{
		print_window (&window, print_flag, dist_flag);
	}

	exit(0);
}

static void die(struct Option *parm)
{
    /*
    G_usage();
    */
    G_fatal_error("<%s=%s> ** invalid input **", parm->key, parm->answer);
}

static int nsew(char *value,char *a,char *b,char *c)
{
	if (strncmp (value, a, strlen(a)) == 0 ) return 1;
	if (strncmp (value, b, strlen(b)) == 0 ) return 2;
	if (strncmp (value, c, strlen(c)) == 0 ) return 3;
	return 0;
}

static char *llinfo(char *msg,char *llformat,int proj)
{
	char buf[256];
	if (proj != PROJECTION_LL)
		return msg;

	sprintf (buf, "%s (format %s)", msg, llformat);
	return G_store(buf);
}
