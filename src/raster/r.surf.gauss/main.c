/*
** Code Compiled by Jo Wood [JWO] 24th October 1991
** Midlands Regional Research Laboratory (ASSIST)
**
**
*/

#include "gis.h"

main(argc,argv) 
    int argc;
    char *argv[];
{

    /****** INITIALISE ******/

    double 		gauss_mean,gauss_sigma;


    struct Option 	*out;		/* Structures required for the G_parser()	*/
   					/* call. These can be filled with the		*/
    struct Option	*mean;		/* various defaults, mandatory paramters	*/
					/* etc. for the GRASS user interface.		*/
    struct Option	*sigma;

    G_gisinit (argv[0]);	/*	This GRASS library function MUST
					be called first to check for valid
					database and mapset. As usual argv[0]
					is the program name. This can be
					recalled using G_program_name(). */

    /****** SET PARSER OPTIONS ******/

    out   = G_define_option(); 	/*	Request pointer to memory for each option.	*/
    mean  = G_define_option();	/* 	Mean of the distribution			*/
    sigma = G_define_option();	/* 	Standard deviation of the distribution		*/

    out->key		= "out";
    out->description	= "Name of the random surface to be produced";
    out->type		= TYPE_STRING;
    out->required	= YES;

    mean->key		= "mean";
    mean->description	= "Distribution mean";
    mean->type		=  TYPE_DOUBLE;
    mean->answer	= "0.0";

    sigma->key		= "sigma";
    sigma->description	= "Standard deviation";
    sigma->type		= TYPE_DOUBLE;
    sigma->answer	= "1.0";


    if (G_parser(argc,argv))
	exit(-1);		/*	Returns a 0 if sucessful		*/

    sscanf(mean->answer,"%lf",&gauss_mean);
    sscanf(sigma->answer,"%lf",&gauss_sigma);


    /****** CHECK THE CELL FILE (OUT) DOES NOT ALREADY EXIST******/

    if (G_legal_filename(out->answer)==NULL)
    {
	char err[256];
	sprintf(err,"Illegal file name. Please try another.");
	G_fatal_error(err);
    }
    else
    {
	if (G_find_cell(out->answer,"") !=NULL)
	{
	    char err[256];
	    sprintf(err,"Raster map [%s] already exists.\nPlease try another.",out);
	    G_fatal_error(err);
	}

    }

    /****** CREATE THE RANDOM CELL FILE  ******/

    gaussurf(out->answer,gauss_mean,gauss_sigma);


}
