#include <stdlib.h>
#include "global.h"

#ifndef DBL_MAX
#define DBL_MAX 9999999999999999999999.9
#define DBL_MIN -99999999999999999999.9
#endif

static void make_head(struct Map_info *);

static BOUND_BOX ext, dxf_ext;

int dxf_to_vect(struct dxf_file *dxf, struct Map_info *Map)
{
    int code;
    int bounds = 0;

    if (dxf_find_header(dxf)) {
	/* code == 0: end of the header section */
	code = dxf_get_code(dxf);
	while (code != 0) {
	    if (code == -2)	/* EOF */
		return -1;

	    /* only looking for header groups (code == 9) */
	    if (code != 9)
		continue;

	    if (strcmp(dxf_buf, "$EXTMAX") == 0) {
		/* READS IN LINES AND PROCESSES INFORMATION UNTIL A 9
		 * OR A 0 IS READ IN */
		while ((code = dxf_get_code(dxf)) != 9) {
		    if (code == -2)	/* EOF */
			return -1;

		    switch (code) {
		    case 10:
			dxf_ext.E = atof(dxf_buf);
			bounds++;
			break;
		    case 20:
			dxf_ext.N = atof(dxf_buf);
			bounds++;
			break;
		    case 30:
			dxf_ext.T = atof(dxf_buf);
			bounds++;
			break;
		    default:
			break;
		    }
		}
	    }
	    else if (strcmp(dxf_buf, "$EXTMIN") == 0) {
		/* READS IN LINES AND PROCESSES INFORMATION UNTIL A 9
		 * OR A 0 IS READ IN */
		while ((code = dxf_get_code(dxf)) != 9) {
		    if (code == -2)	/* EOF */
			return -1;

		    switch (code) {
		    case 10:
			dxf_ext.W = atof(dxf_buf);
			bounds++;
			break;
		    case 20:
			dxf_ext.S = atof(dxf_buf);
			bounds++;
			break;
		    case 30:
			dxf_ext.B = atof(dxf_buf);
			bounds++;
		    default:
			break;
		    }
		}
	    }
	    else {
		while ((code = dxf_get_code(dxf)) != 9) {
		    if (code == -2)	/* EOF */
			return -1;
		}
	    }

	    if (bounds == 6)
		break;
	}
    }
    else
	code = dxf_get_code(dxf);

    ARR_MAX = ARR_INCR;
    ext.E = ext.N = ext.T = DBL_MIN;
    ext.W = ext.S = ext.B = DBL_MAX;

    xpnts = (double *)G_malloc(ARR_MAX * sizeof(double));
    ypnts = (double *)G_malloc(ARR_MAX * sizeof(double));
    zpnts = (double *)G_malloc(ARR_MAX * sizeof(double));

    if (!flag_list)
	Points = Vect_new_line_struct();

    while (!feof(dxf->fp)) {
	set_entity(dxf_buf);

	/* avoid TEXT having object names: '0' should be followed by objects */
	if (code != 0)
	    code = dxf_get_code(dxf);

	else if (strcmp(dxf_buf, "POINT") == 0)
	    add_point(dxf, Map);

	else if (strcmp(dxf_buf, "LINE") == 0)
	    add_line(dxf, Map);

	else if (strcmp(dxf_buf, "POLYLINE") == 0)
	    add_polyline(dxf, Map);

	else if (strcmp(dxf_buf, "LWPOLYLINE") == 0)
	    add_lwpolyline(dxf, Map);

	else if (strcmp(dxf_buf, "ARC") == 0)
	    add_arc(dxf, Map);

	else if (strcmp(dxf_buf, "CIRCLE") == 0)
	    add_circle(dxf, Map);

	else if (strcmp(dxf_buf, "TEXT") == 0)
	    add_text(dxf, Map);

	else
	    code = dxf_get_code(dxf);
    }

    G_free(xpnts);
    G_free(ypnts);
    G_free(zpnts);

    if (!flag_list) {
	Vect_destroy_line_struct(Points);
	write_done(Map);
	if (found_layers)
	    make_head(Map);
    }

    return 0;
}

int check_ext(double x, double y)
{
    if (y < ext.S)
	ext.S = y;
    if (y > ext.N)
	ext.N = y;
    if (x < ext.W)
	ext.W = x;
    if (x > ext.E)
	ext.E = x;

    return 0;
}

void set_entity(char *str)
{
    strcpy(entity, str);
    for (str = entity; *str; str++)
	*str = tolower(*str);

    return;
}

static void make_head(struct Map_info *Map)
{
    char *organization;

    if ((organization = getenv("GRASS_ORGANIZATION")))	/* added MN 5/2001 */
	Vect_set_organization(Map, organization);
    else
	Vect_set_organization(Map, "GRASS Development Team");
    Vect_set_date(Map, G_date());
    Vect_set_person(Map, G_whoami());
    Vect_set_map_date(Map, "");
    Vect_set_scale(Map, 2400);
    Vect_set_comment(Map, "");
    Vect_set_zone(Map, 0);
    Vect_set_thresh(Map, 0.0);

    if (!flag_extent)
	Vect_box_copy(&(Map->plus.box), &dxf_ext);

    return;
}
