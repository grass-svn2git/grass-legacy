/*
* Name
* 	s.proj -- convert a sites file to a new geographic projection.
*
* Usage:
*	s.proj input=name location=name [output=name] [mapset=name]
*		  [dbase=name] 
*
*
* Description:
*	s.proj converts a sites list to a new geographic projection. 
*       It reads a sites list from a different location, projects it
*	and writes it out to the current location. 
*
*
* Parameters:
*
*	input   	input sites list
*	location   	location of input 
*	output		output sites list 
*	mapset		mapset of input 
*	dbase		database of input 
*
* Notes: G_projection() is used by G_site_put and G_site_get for formating
* reads and writes, so have to cache or switch env for each read/write.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gis.h"
#include "site.h"
#include "local_proto.h"
#include "gprojects.h"

FILE *infile, *outfile;

int
main(int argc, char **argv)
{
	char      *setname,		 /* ptr to name of input mapset	 */
	          errbuf[256];		 /* buffer for error messages	 */

	int       permissions,		 /* mapset permissions		 */
		  oldproj, newproj;


	double    xcoord,	 /* temporary x coordinates 	 */
	          ycoord;	 /* temporary y coordinates	 */

	struct pj_info iproj,		 /* input map proj parameters	 */
	          oproj;		 /* output map proj parameters	 */

        Site *si;                         /* Site * to pass to next_point */

	struct Key_Value *in_proj_info,	 /* projection information of 	 */
	         *in_unit_info,		 /* input and output mapsets	 */
	         *out_proj_info, 
	         *out_unit_info;

	struct Option *imapset,		 /* name of input mapset	 */
	         *inmap,		 /* name of input layer		 */
	         *outmap,		 /* name of output layer	 */
	         *inlocation,		 /* name of input location	 */
	         *indbase;		 /* name of input database	 */
	
	struct Flag *list;               /* list files in source location */
	
        struct GModule *module;
        

	G_gisinit(argv[0]);
        
        module = G_define_module();
        module->description =        
                        "Allows the user to re-project a sites file from one "
                        "location to the current location.";
                        
	newproj = G_projection();

	inmap = G_define_option();
	inmap->key = "input";
	inmap->type = TYPE_STRING;
	inmap->required = YES;
	inmap->description = "input sites list";

	inlocation = G_define_option();
	inlocation->key = "location";
	inlocation->type = TYPE_STRING;
	inlocation->required = YES;
	inlocation->description = "location of input sites list";

	imapset = G_define_option();
	imapset->key = "mapset";
	imapset->type = TYPE_STRING;
	imapset->required = NO;
	imapset->description = "mapset of input sites list";

	indbase = G_define_option();
	indbase->key = "dbase";
	indbase->type = TYPE_STRING;
	indbase->required = NO;
	indbase->description = "path to GRASS database of input location";

	outmap = G_define_option();
	outmap->key = "output";
	outmap->type = TYPE_STRING;
	outmap->required = NO;
	outmap->description = "output sites list";

	list = G_define_flag();
	list->key = 'l';
	list->description = "List sites files in input location and exit";
                        

	if (G_parser(argc, argv))
		exit(-1);


	if (imapset->answer)
		setname = imapset->answer;
	else
		setname = G_store(G_mapset());

	if (strcmp(inlocation->answer, G_location()) == 0)
		G_fatal_error("You have to use a different location for input than the current");

	if(outmap->answer)
	    outfile = G_sites_open_new (outmap->answer);
	else 
	    outfile = G_sites_open_new (inmap->answer);


   /* Get projection info for output mapset */
	if ((out_proj_info = G_get_projinfo()) == NULL)
		G_fatal_error("Can't get projection info of output map");

	if ((out_unit_info = G_get_projunits()) == NULL)
		G_fatal_error("Can't get projection units of output map");

	if (pj_get_kv(&oproj, out_proj_info, out_unit_info) < 0)
		G_fatal_error("Can't get projection key values of output map");

   /* Change the location 		 */
	G__create_alt_env();
	G__setenv("GISDBASE", indbase->answer == NULL
		  ? G_gisdbase()
		  : indbase->answer);
	G__setenv("LOCATION_NAME", inlocation->answer);
	permissions = G__mapset_permissions(setname);

	if (permissions >= 0) {

          /* if requested, list the raster files in source location - MN 5/2001*/
		if (list->answer)
		{
		  if(isatty(0))  /* check if on command line */
		  {
		   fprintf(stderr, "Checking location %s, mapset %s:\n", inlocation->answer, setname);
		   G_list_element ("site_lists", "sites", setname, 0);
		   exit(0); /* leave s.proj after listing*/
		  }
		}

		if (!G_find_file ("site_lists", inmap->answer, setname)) {
			sprintf(errbuf, "Input list [%s] in location [%s] in mapset [%s] not found.",
				inmap->answer, inlocation->answer, setname);
			G_fatal_error(errbuf);
		}
   /* Get projection info for input mapset */
		if ((in_proj_info = G_get_projinfo()) == NULL)
			G_fatal_error("Can't get projection info of input map");

		if ((in_unit_info = G_get_projunits()) == NULL)
			G_fatal_error("Can't get projection units of input map");

		if (pj_get_kv(&iproj, in_proj_info, in_unit_info) < 0)
			G_fatal_error("Can't get projection key values of input map");


		infile = G_sites_open_old (inmap->answer, setname);
		if (infile == NULL)
		{
			sprintf (errbuf, "can't open sites file [%s]", inmap->answer);
			G_fatal_error (errbuf);
		}


		if (!G_projection())	/* XY data 		 */
			G_fatal_error("Can't work with xy data");


	} else {		/* can't access mapset 	 */

		sprintf(errbuf, "Mapset [%s] in input location [%s] - ",
			setname, inlocation->answer);

		strcat(errbuf, permissions == 0
		       ? "permission denied\n"
		       : "not found\n");
		G_fatal_error(errbuf);
	}
	 
        /* save old coord system */
        {
	struct Cell_head dbwindow;

	G__get_window (&dbwindow,"","WIND","PERMANENT");
	oldproj = dbwindow.proj;

	}

	G__switch_env();


	if(outfile){ /* also write G_recreate_command as comment */
	    Site_head sh;
	    char buf0[512];

            G_site_get_head (infile, &sh);

	/* mhhh, how to write this properly 9/2001 ? see s.in.shape */
       /*   sprintf( buf0, "%s", G_recreate_command());
            sh->desc = (char *)malloc( strlen(buf0) + 1 );
            strcpy(sh->desc, buf0 );
        */

            G_site_put_head (outfile, &sh);
        }

	{
	int nstr, ndim, ndec, err;
	RASTER_MAP_TYPE rtype;

	    rtype = -1;
	    err=G_site_describe (infile, &ndim, &rtype, &nstr, &ndec);
	    if (err == -1)
	    	G_fatal_error("Sites map is empty");
	    if (err == -2)
	      	G_fatal_error("Input sites map: Format error.");

	    si = G_site_new_struct(rtype,ndim,nstr,ndec); 
	}

	while (G__site_get (infile, si, oldproj) >= 0) {
        
	    xcoord = si->east;
	    ycoord = si->north;

	    if(pj_do_proj(&xcoord, &ycoord, &iproj, &oproj) < 0 ) {
                fprintf(stderr,"Error in pj_do_proj\n");
                exit(0);
            }


	    si->east = xcoord;
	    si->north = ycoord;
	    G__site_put (outfile, si, newproj);

	}

	fclose(infile);
	fclose(outfile);
	return(0);		/* OK */
}
