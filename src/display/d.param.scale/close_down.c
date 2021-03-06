/*****************************************************************************/
/***                                                                       ***/
/***                             close_down()                              ***/
/***   	   Closes all input and output raster files and frees memory.	   ***/
/***               Jo Wood, Project ASSIST, 7th February 1993              ***/
/***                                                                       ***/
/*****************************************************************************/

#include "param.h"


close_down()
{
    /* Close connection with existing input raster. */

    G_unopen_cell(fd_in);

    R_close_driver();

}
