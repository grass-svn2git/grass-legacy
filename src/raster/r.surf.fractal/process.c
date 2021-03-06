/***************************************************************************/
/***                                                                     ***/
/***                             process()                               ***/
/***          Reads in a raster files row by row for processing          ***/
/***                  Jo Wood, V1.0, 13th September, 1994		 ***/
/***                                                                     ***/
/***************************************************************************/

#include "frac.h"

process()
{

    /*-------------------------------------------------------------------*/
    /*                              INITIALISE                         	 */
    /*-------------------------------------------------------------------*/ 

    int         nrows,          /* Will store the current number of     */
                ncols,          /* rows and columns in the raster.      */

		nn,		/* Size of raster to nearest power of 2.*/

                row,col;        /* Counts through each row and column   */
                                /* of the input raster.                 */

    double	*data[2];	/* Array holding complex data.		*/
                           

    /*------------------------------------------------------------------*/
    /*                       GET DETAILS OF INPUT RASTER                */
    /*------------------------------------------------------------------*/ 
                                        
    nrows = G_window_rows();         /* Find out the number of rows and	*/
    ncols = G_window_cols();         /* columns of the raster view.	*/

    nn = max_pow2(MAX(nrows,ncols)); /* Find smallest power of 2 that	*/
				     /* largest side of raster will fit.*/

    /*------------------------------------------------------------------*/
    /*                      CREATE SQUARE ARRAY OF SIDE 2^n             */
    /*------------------------------------------------------------------*/ 

    data[0] = (double *) G_malloc(nn*nn*sizeof(double));
    data[1] = (double *) G_malloc(nn*nn*sizeof(double));

    /*------------------------------------------------------------------*/
    /*                   Apply spectral synthesis algorithm.            */
    /*------------------------------------------------------------------*/ 

    specsyn(data,nn);

    free(data[0]);
    free(data[1]);
}

/***************************************************/
/*  Initialize real & complex components to zero   */
/***************************************************/

data_reset(data,nn)
    double *data[2];
    int	   nn;
{
    register double *dptr0=data[0], *dptr1=data[1];
   	     int    total_size = nn*nn,
		    count;

    for (count=0; count<total_size; count++) 
        *dptr0++ = *dptr1++ =0.0;
}
