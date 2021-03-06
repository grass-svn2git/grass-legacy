/****************************************************************************/
/***      Function to get input from user and check files can be opened   ***/
/***                                                                      ***/
/***                    Jo Wood,  V1.0, 13th September 1994               ***/
/****************************************************************************/

#include "frac.h"

interface(argc,argv) 

    int      argc;              /* Number of command line arguments     */
    char    *argv[];            /* Contents of command line arguments.  */

{
    /*---------------------------------------------------------------------*/
    /*                               INITIALISE				   */
    /*---------------------------------------------------------------------*/ 

    struct Option       *rast_out;      /* Structure for output raster     */           
    struct Option	*frac_dim;	/* Fractal dimension of surface.   */	
    struct Option	*num_images;	/* Number of images to produce.	   */

    G_gisinit (argv[0]);                /* Link with GRASS interface.	   */


    /*---------------------------------------------------------------------*/
    /*                              SET PARSER OPTIONS                     */
    /*---------------------------------------------------------------------*/

    rast_out = G_define_option();
    frac_dim = G_define_option();
    num_images = G_define_option();

    /* Each option needs a 'key' (short description),
         		 a 'description` (a longer one),
         		 a 'type' (eg intiger, or string),
         	     and an indication whether manditory or not */

    rast_out->key         = "out";
    rast_out->description = "Name of fractal surface raster layer";
    rast_out->type        = TYPE_STRING;
    rast_out->required    = YES;
    
    frac_dim->key         = "d";
    frac_dim->description = "Fractal dimension of surface (2 < D < 3)";
    frac_dim->type        = TYPE_DOUBLE;
    frac_dim->required    = NO;
    frac_dim->answer	  = "2.05";

    num_images->key         = "n";
    num_images->description = "Number of intermediate images to produce";
    num_images->type        = TYPE_INTEGER;
    num_images->required    = NO;
    num_images->answer	    = "0";

    if (G_parser(argc,argv))            /* Performs the prompting for      */
        exit(-1);                       /* keyboard input.                 */

    rast_out_name  = rast_out->answer;
    sscanf(frac_dim->answer,"%lf",&H);
    H = 3.0 - H;
    Steps = atoi(num_images->answer) + 1;

printf("Steps=%d\n",Steps);
    /*--------------------------------------------------------------------*/
    /*                  CHECK OUTPUT RASTER FILE DOES NOT EXIST           */
    /*--------------------------------------------------------------------*/

    mapset_out = G_mapset();            /* Set output to current mapset.  */

    if (G_legal_filename(rast_out_name)==NULL)
    {
        char err[256];
        sprintf(err,"Illegal output file name. Please try another.");
        G_fatal_error(err);
    }
    else
    {
        if (G_find_cell2(rast_out_name,mapset_out) !=NULL)
        {
            char err[256];
            sprintf(err,"Raster map [%s] exists.\nPlease try another\n",rast_out_name);
            G_fatal_error(err);
        }
    }

    /*--------------------------------------------------------------------*/
    /*               CHECK FRACTAL DIMENSION IS WITHIN LIMITS             */
    /*--------------------------------------------------------------------*/

    if ( (H <= 0) || (H >= 1))
    {
        char err[256];
        sprintf(err,"Fractal dimension of [%.2lf] must be between 2 and 3.", 3.0-H);
        G_fatal_error(err);
    }


    /*--------------------------------------------------------------------*/
    /*        MAKE SURE NUMBER OF IMAGES <= NUMBER OF FOURIER COEFF.      */
    /*--------------------------------------------------------------------*/

    /* TODO */
}
