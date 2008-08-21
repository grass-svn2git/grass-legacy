#include <stdlib.h>
#include <string.h>
#include "global.h"

int add_arc(struct dxf_file *dxf, struct Map_info *Map)
{
    int code;
    char layer[DXF_BUF_SIZE];
    int layer_flag = 0;		/* indicates if a layer name has been found */
    int xflag = 0;		/* indicates if a x value has been found */
    int yflag = 0;		/* indicates if a y value has been found */
    int rflag = 0;		/* indicates if a radius has been found */
    int sflag = 0;		/* indicates if a start_angle has been found */
    int fflag = 0;		/* indicates if a finish_angle has been found */
    double centerx = 0;		/* read in from dxf file */
    double centery = 0;		/* read in from dxf file */
    double radius = 0;		/* read in from dxf file */
    double zcoor = 0;		/* read in from dxf file */
    float start_angle = 0;	/* read in from dxf file */
    float finish_angle = 0;	/* read in from dxf file */
    int arr_size = 0;

    strcpy(layer, UNIDENTIFIED_LAYER);

    /* read in lines and processes information until a 0 is read in */
    while ((code = dxf_get_code(dxf)) != 0) {
	if (code == -2)
	    return -1;

	switch (code) {
	case 8:		/* layer name */
	    if (!layer_flag && *dxf_buf) {
		if (flag_list) {
		    if (!is_layer_in_list(dxf_buf)) {
			add_layer_to_list(dxf_buf);
			fprintf(stdout, _("Layer %d: %s\n"), num_layers,
				dxf_buf);
		    }
		    return 0;
		}
		/* skip if layers != NULL && (
		 * (flag_invert == 0 && is_layer_in_list == 0) ||
		 * (flag_invert == 1 && is_layer_in_list == 1)
		 * )
		 */
		if (layers && flag_invert == is_layer_in_list(dxf_buf))
		    return 0;
		strcpy(layer, dxf_buf);
		layer_flag = 1;
	    }
	    break;
	case 10:		/* x coordinate */
	    centerx = atof(dxf_buf);
	    xflag = 1;
	    break;
	case 20:		/* y coordinate */
	    centery = atof(dxf_buf);
	    yflag = 1;
	    break;
	case 30:		/* z coordinate */
	    zcoor = atof(dxf_buf);
	    break;
	case 40:		/* radius */
	    radius = atof(dxf_buf);
	    rflag = 1;
	    break;
	case 50:		/* start angle */
	    start_angle = atof(dxf_buf);
	    sflag = 1;
	    break;
	case 51:		/* end angle */
	    finish_angle = atof(dxf_buf);
	    fflag = 1;
	    break;
	}
    }

    if (xflag && yflag && rflag && sflag && fflag) {
	arr_size =
	    make_arc(0, centerx, centery, radius, start_angle, finish_angle,
		     zcoor);
	write_line(Map, layer, arr_size);
    }

    return 0;
}
