/*
****************************************************************************
*
* MODULE:       v.what  - for GRASS 6.1 - 
*
* AUTHOR(S):    Trevor Wiens - derived from d.what.vect - 15 Jan 2006
*
* PURPOSE:     To select and report attribute information for objects at  a
*                       user specified location. This replaces d.what.vect by removing
*                       the interactive component to enable its use with the new 
*                       gis.m and future GUI.
*
* COPYRIGHT:    (C) 2006 by the GRASS Development Team
*
*              This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/

#define MAIN
#include <stdlib.h>
#include <string.h>
#include "glocale.h"
#include "gis.h"
#include "Vect.h"
#include "raster.h"
#include "display.h"
#include "what.h"
#include "dbmi.h"
#include "glocale.h"
#include "config.h"

/* Vector map grabbing taken from d.zoom */

int
main(int argc, char **argv)
{
  struct Flag *printattributes, *topo_flag;
  struct Option *opt1, *opt4, *maxdistance;
  struct Cell_head window;
  struct GModule *module;
  char *mapset;
  char *str;
  int i, j, level, width = 0, mwidth = 0;
  double xval, yval, xres, yres, maxd, x;
  double EW_DIST1, EW_DIST2, NS_DIST1, NS_DIST2;
  char nsres[30], ewres[30];
  
    /* Initialize the GIS calls */
    G_gisinit (argv[0]) ;

    /* Conditionalize R_open_driver() so "help" works, open quiet as well */
    R__open_quiet();
    if (R_open_driver() == 0)
    {
        if (D_get_dig_list (&vect, &nvects) < 0)
            vect = NULL;
        else
        {
            vect = (char **)G_realloc(vect, (nvects+1)*sizeof(char *));
            vect[nvects] = NULL;
        }

        R_close_driver();
   }

    opt4 = G_define_option() ;
    opt4->key        = "east_north";
    opt4->type       = TYPE_DOUBLE;
    opt4->key_desc   = "east,north";
    opt4->required   = NO;
    opt4->multiple   = YES;
    opt4->description= _("Coordinates for query");
   
    maxdistance = G_define_option();
    maxdistance->type = TYPE_DOUBLE;
    maxdistance->key = "distance";
    maxdistance->answer = "0";
    maxdistance->multiple = NO;
    maxdistance->description = _("Maximum distance");

    opt1 = G_define_option() ;
    opt1->key        = "map" ;
    opt1->type       = TYPE_STRING ;
    opt1->multiple   = YES;
    if (vect)
          opt1->answers = vect;
    opt1->required   = YES;
    opt1->gisprompt  = "old,vector,vector" ;
    opt1->description= _("Name of existing vector map") ;
  
    topo_flag = G_define_flag();
    topo_flag->key = 'd';
    topo_flag->description = _("Print topological information (debugging)");
 
    printattributes = G_define_flag();
    printattributes->key = 'a';
    printattributes->description = _("Print attribute information");
  
    module = G_define_module();
    module->description = 
    _("Allows the user to interactively query a vector map layer "
      "at user-selected locations within the current geographic region");

    if(!vect)
        opt1->required = YES;
    	  	      
    if((argc > 1 || !vect) && G_parser(argc,argv))
        exit(EXIT_FAILURE);

    if (opt1->answers && opt1->answers[0])
        vect = opt1->answers;

   maxd = atof(maxdistance->answer);
   xval = atof(opt4->answers[0]);
   yval = atof(opt4->answers[1]);

/*  
*  fprintf(stdout, maxdistance->answer);
*  fprintf(stdout, "Maxd is %f", maxd);
*  fprintf(stdout, xcoord->answer);
*  fprintf(stdout, "xval is %f", xval);
*  fprintf(stdout, ycoord->answer);
*  fprintf(stdout, "yval is %f", yval);
*/
  
    if (maxd == 0.0) 
    {
        G_get_window (&window);
        x = window.proj;
        G_format_resolution  (window.ew_res,  ewres,  x);
        G_format_resolution  (window.ns_res,  nsres,  x);
        EW_DIST1 = G_distance(window.east, window.north, window.west, window.north);
        /* EW Dist at South Edge */
        EW_DIST2 = G_distance(window.east, window.south, window.west, window.south);
        /* NS Dist at East edge */
        NS_DIST1 = G_distance(window.east, window.north, window.east, window.south);
        /* NS Dist at West edge */
        NS_DIST2 = G_distance(window.west, window.north, window.west, window.south);
        xres = ((EW_DIST1 + EW_DIST2) / 2) / window.cols;
        yres = ((NS_DIST1 + NS_DIST2) / 2) / window.rows;
        if (xres > yres) maxd = xres; else maxd = yres;
    }


  /* Look at maps given on command line */

    if(vect)
    {
        for(i=0; vect[i]; i++);
        nvects = i;

        Map = (struct Map_info *) G_malloc(nvects * sizeof(struct Map_info));
    
        width = mwidth = 0;
        for(i=0; i<nvects; i++)
        {
            str = strchr(vect[i], '@');
            if (str) j = str - vect[i];
            else j = strlen(vect[i]);
            if (j > width) width = j;
    
            mapset = G_find_vector2(vect[i],"");
            j = strlen(mapset);
            if (j > mwidth)
                mwidth = j;
    
            level = Vect_open_old (&Map[i], vect[i], mapset);
            if (level < 0) 
                G_fatal_error ( _("Vector file [%s] not available"), vect[i]);
    
            if (level < 2) 
                G_fatal_error ( _("%s: You must build topology on vector file"), vect[i]);

            /* G_message ("Building spatial index ..."); */
            Vect_build_spatial_index ( &Map[i], NULL );
       }
    }

  what(xval, yval, maxd, width, mwidth, topo_flag->answer,printattributes->answer); 

  for(i=0; i<nvects; i++)
      Vect_close (&Map[i]);

  R_pad_freelist(vect, nvects);

  /* fprintf(stderr, _("\n")); */
  exit(EXIT_SUCCESS);
}



