/* modified 1998-OCT-06 Benjamin Horner-Johnson - 80->256 char dxf_line */
/* written by J Moorman
 * 7/23/90
 */

#include <stdlib.h>
#include "global.h"

/* DECLARING SUBROUTINES */

int add_line(FILE * dxf_file)
{
    int layer_flag = 0;		/* INDICATES IF A LAYER NAME HAS BEEN FOUND */
    int xflag = 0;		/* INDICATES IF A x VALUE HAS BEEN FOUND */
    int yflag = 0;		/* INDICATES IF A y VALUE HAS BEEN FOUND */
    int zflag = 0;		/* INDICATES IF A z VALUE HAS BEEN FOUND */
    char *nolayername = "UNIDENTIFIED";
    struct dxf_dig *layer_fd = NULL;	/* POINTER TO LAYER NAME */
    int code;			/* VARIABLE THAT HOLDS VALUE RETURNED BY readcode() */
    int arr_size = 0;

    /* READS IN LINES AND PROCESSES INFORMATION UNTIL A 0 IS READ IN */
    while ((code = dxf_readcode(dxf_file)) != 0) {
	if (code == -2)		/* EOF */
	    return 0;
	dxf_fgets(dxf_line, 256, dxf_file);
	if (feof(dxf_file) != 0)	/* EOF */
	    return 0;

	switch (code) {
	case 8:
	    if (!layer_flag) {
		layer_fd = which_layer(dxf_line, DXF_ASCII);
		if (layer_fd == NULL)
		    return 0;
		layer_flag = 1;
	    }
	    break;
	case 10:		/* START POINT x COORDINATE */
	    xinfo[arr_size] = atof(dxf_line);
	    xflag = 1;
	    break;
	case 20:		/* START POINT y COORDINATE */
	    yinfo[arr_size] = atof(dxf_line);
	    yflag = 1;
	    break;
	case 30:		/* START POINT z COORDINATE */
	    zinfo[arr_size] = atof(dxf_line);
	    zflag = 1;
	    break;
	case 11:		/* END POINT x COORDINATE */
	    xinfo[arr_size] = atof(dxf_line);
	    xflag = 1;
	    break;
	case 21:		/* END POINT y COORDINATE */
	    yinfo[arr_size] = atof(dxf_line);
	    yflag = 1;
	    break;
	case 31:		/* END POINT z COORDINATE */
	    zinfo[arr_size] = atof(dxf_line);
	    zflag = 1;
	    break;

	    /* THE FOLLOWING GROUPS USED ONLY IF DIFFERENT THAN DEFAULTS */
	case 6:		/* LINETYPE NAME */
	case 38:		/* ELEVATION IF NONZERO */
	case 39:		/* THICKNESS IF NONZERO */
	case 62:		/* COLOR NUMBER (IF NOT "BYLAYER") */
	case 210:		/* X EXTRUSION IF NOT PARALLEL TO THE WORLD Z AXIS */
	case 220:		/* Y EXTRUSION IF NOT PARALLEL TO THE WORLD Z AXIS */
	case 230:		/* Z EXTRUSION IF NOT PARALLEL TO THE WORLD Z AXIS */

	default:
	    break;
	}
	if (xflag == 1 && yflag == 1) {
	    check_ext(xinfo[arr_size], yinfo[arr_size]);
	    if (arr_size == ARR_MAX) {
		ARR_MAX += ARR_INCR;
		xinfo = (double *)G_realloc(xinfo, ARR_MAX * sizeof(double));
		yinfo = (double *)G_realloc(yinfo, ARR_MAX * sizeof(double));
		zinfo = (double *)G_realloc(zinfo, ARR_MAX * sizeof(double));
	    }
	    arr_size++;
	    xflag = 0;
	    yflag = 0;
	}
    }

    if (!layer_flag) {		/* NO LAYER DESIGNATED */
	layer_fd = which_layer(nolayername, DXF_ASCII);
	if (layer_fd == NULL)
	    return 0;
    }
    if (arr_size == 2) {	/* had both starts and stops */
	/* PRINTS OUT THE POLYLINE VERTEX DATA TO FILE DESIGNATED AS layer_fd */
	if (!zflag)
	    zinfo[0] = zinfo[1] = 0.0;
	write_polylines(layer_fd, arr_size);
    }
    return 1;
}
