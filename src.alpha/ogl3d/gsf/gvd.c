
/*  gvd.c
    Bill Brown, USACERL  
    December 1993
*/
	
#include "gstypes.h"
#include "rowcol.h"
#include <stdio.h>
#include <stdlib.h>

Point3 *gsdrape_get_segments();

#define CHK_FREQ 5
/* check for cancel every CHK_FREQ lines */

/* FIX to use fast clipping and move to gs.c */
gs_clip_segment(gs, bgn, end, region)
geosurf *gs;
float *bgn, *end, *region;
{
float top, bottom, left, right;
    
    if(!region){
	top = gs->yrange;
	bottom = VROW2Y(gs,VROWS(gs)); 
	left = 0.0;
	right = VCOL2X(gs,VCOLS(gs));
    }
    else{
	top = region[0];
	bottom = region[1]; 
	left = region[2];
	right = region[3];
    }

    /* for now, ignore any segments with an end outside */
    return(bgn[X] >= left && bgn[X] <= right &&
	   end[X] >= left && end[X] <= right &&
	   bgn[Y] >= bottom && bgn[Y] <= top &&
	   end[Y] >= bottom && end[Y] <= top);

}

/* need to think about translations - If user translates surface,
vector should automatically go with it, but translating vector should
translate it relative to surface on which it's displayed? */

/* handling mask checking here, but may be more appropriate to
handle in get_drape_segments? */

gvd_vect(gv, gs, do_fast)
geovect *gv;
geosurf *gs;
int do_fast;
{
int i,j,k;
float bgn[3], end[3], tx, ty, tz, konst;
float zmin, zmax, fudge;
Point3 *points;
int npts, src, check; 
geoline *gln;
    
    if(GS_check_cancel()) return(0);

    gs_update_curmask(gs);

    src = gs_get_att_src(gs, ATT_TOPO);
    GS_get_scale(&tx, &ty, &tz, 1);
    gs_get_zrange(&zmin, &zmax);
    fudge = (zmax - zmin)/500.;

    if (src == CONST_ATT){
	konst = gs->att[ATT_TOPO].constant;
	bgn[Z] = end[Z] = konst;
    }

    gsd_pushmatrix();

    /* avoid scaling by zero */
    if(tz == 0.0){
	src = CONST_ATT;
	konst = 0.0;
	bgn[Z] = end[Z] = konst;
	gsd_do_scale(0);
    }
    else
	gsd_do_scale(1);

    gsd_translate(gs->x_trans, gs->y_trans, gs->z_trans+fudge);

    gsd_colormode(CM_COLOR);
    gsd_color_func(gv->color);
    gsd_linewidth(gv->width);
/*
fprintf(stderr,"::%d::", gv->n_lines);
fprintf(stderr,"%d::\n", gv_num_points(gv));
*/

    check = 0;
    if(do_fast){
	if(!gv->fastlines) gv_decimate_lines(gv);
	gln = gv->fastlines;
    }
    else
	gln = gv->lines;

    for (; gln; gln=gln->next){
	if(!(++check % CHK_FREQ)){
	    if(GS_check_cancel()){
		gsd_linewidth(1);
		gsd_popmatrix();
		return(0);
	    }
	}
/*
fprintf(stderr,"%d(%d) ", check, gln->npts);
*/

	for (k=0; k < gln->npts - 1; k++){
	    bgn[X] = gln->p2[k][X] + gv->x_trans - gs->ox;
	    bgn[Y] = gln->p2[k][Y] + gv->y_trans - gs->oy;
	    end[X] = gln->p2[k+1][X] + gv->x_trans - gs->ox;
	    end[Y] = gln->p2[k+1][Y] + gv->y_trans -  gs->oy;

	    if(src == MAP_ATT){
		points = gsdrape_get_segments(gs, bgn, end, &npts);
		gsd_bgnline();
		for(i=0, j=0 ; i < npts; i++){ 
		    if(gs_point_is_masked(gs, points[i])){
			if(j){
			    gsd_endline();
			    gsd_bgnline();
			    j = 0;
			}
			continue;
		    }
		    points[i][Z] += gv->z_trans;
		    gsd_vert_func(points[i]);
		    j++;
		    if(j > 250){
			gsd_endline();
			gsd_bgnline();
			gsd_vert_func(points[i]);
			j = 1;
		    }
		}
		gsd_endline();
	    }
	    
	    /* need to handle MASK! */
	    else if (src == CONST_ATT){
		/* for now - but later, do seg intersect maskedge */
		if(gs_point_is_masked(gs,bgn) || gs_point_is_masked(gs,end))
		    continue;
		if(gs_clip_segment(gs, bgn, end, NULL)){
		    gsd_bgnline();
		    gsd_vert_func(bgn);
		    gsd_vert_func(end);
		    gsd_endline();
		}
	    }

	}
    }

    gsd_linewidth(1);
    gsd_popmatrix();
    return(1);

}

/*****************************************************************/
gvd_draw_lineonsurf(gs, bgn, end, color)
geosurf *gs;
float *bgn, *end;
int color;
{
Point3 *points;
int npts, i, j;

	gsd_color_func(color);
	points = gsdrape_get_segments(gs, bgn, end, &npts);
	gsd_bgnline();
	for(i=0, j=0 ; i < npts; i++){ 
	    if(gs_point_is_masked(gs, points[i])){
		if(j){
		    gsd_endline();
		    gsd_bgnline();
		    j = 0;
		}
		continue;
	    }
	    gsd_vert_func(points[i]);
	    j++;
	    if(j > 250){
		gsd_endline();
		gsd_bgnline();
		gsd_vert_func(points[i]);
		j = 1;
	    }
	}
	gsd_endline();


}
