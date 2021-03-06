/***************************************************************************/
/***                                                                     ***/
/***                            write_rast()                             ***/
/***  Extracts real component from complex array and writes as raster.   ***/
/***                  Jo Wood, V1.0, 20th October, 1994  		 ***/
/***                                                                     ***/
/***************************************************************************/

#include "frac.h"

write_rast(data,nn,step)
    double *data[2];		/* Array holding complex data.		*/
    int    nn,			/* Size of side of array.		*/
           step;		/* Version of file to send.		*/
{

    /*------------------------------------------------------------------*/
    /*                              INITIALISE                         	*/
    /*------------------------------------------------------------------*/ 

    CELL        *row_out;   	/* Buffers to hold raster rows.		*/

    char	file_name[128];	/* Name of each file to be written	*/

    int         nrows,          /* Will store the current number of     */
                ncols,          /* rows and columns in the raster.      */

                row,col;        /* Counts through each row and column   */
                                /* of the input raster.                 */

    nrows = G_window_rows();    /* Find out the number of rows and 	*/
    ncols = G_window_cols();    /* columns of the raster view.     	*/

    row_out = G_allocate_cell_buf();

    /*------------------------------------------------------------------*/
    /*         Open new file and set the output file descriptor.        */
    /*------------------------------------------------------------------*/

    if (Steps != step)
	sprintf(file_name,"%s.%d",rast_out_name,step);
    else
	strcpy(file_name,rast_out_name);

    if ( (fd_out=G_open_cell_new(file_name)) <0)
    {
        char err[256];
        sprintf(err,"ERROR: Problem opening output file.");
        G_fatal_error(err);
    }

    /*------------------------------------------------------------------*/
    /*  Extract real component of transform and save as a GRASS raster. */
    /*------------------------------------------------------------------*/ 

    for(row=0; row<nrows; row++)
    {
        for(col = 0; col < ncols; col++)
            *(row_out + col) = (CELL) (*(data[0] + row*nn + col) *100000);
        
        G_put_map_row(fd_out,row_out);
    }

    G_close_cell(fd_out);
}
