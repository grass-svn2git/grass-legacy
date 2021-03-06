
/*  gsd.c
    Bill Brown, USACERL  
    January 1993
*/
	
#include "gstypes.h"
#include "gsget.h"
#include <stdio.h>
#include "rowcol.h"

/*
#define TRACE_DFUNCS
*/

/************************************************************************/
/* Notes on exageration:
    vertical exageration is of two forms: 
    1) global exageration (from geoview struct) 
    2) vertical exageration for each surface
*/
/************************************************************************/
/* may need to add more parameters to tell it which window or off_screen
 * pixmap to draw into.
*/

int
gsd_wire_surf(surf)
geosurf *surf;
{
int desc, ret;

#ifdef TRACE_DFUNCS
Gs_status("gsd_wire_surf");
#endif

    desc = ATT_TOPO;

    switch (gs_get_att_src(surf, desc)) {
	case NOTSET_ATT:
	    ret =  (-1); 
	    break;

	case MAP_ATT:
	    ret =  (gsd_wire_surf_map(surf));
	    break;

	case CONST_ATT:
	    ret =  (gsd_wire_surf_const(surf, surf->att[desc].constant));
	    break;

	case FUNC_ATT:
	    ret =  (gsd_wire_surf_func(surf, surf->att[desc].user_func));
	    break;

	default:
	    ret =  (-1);
	    break;
    }

    return(ret);

}

/************************************************************************/

int
gsd_wire_surf_map(surf)
geosurf *surf;
{
int check_mask, check_color;
typbuff *buff, *cobuff;
int xmod, ymod, row, col, cnt, xcnt, ycnt, x1off;
long offset, y1off;
float pt[4], xres, yres, ymax, zexag;
int col_src, curcolor;
gsurf_att *coloratt;

#ifdef TRACE_DFUNCS
Gs_status("gsd_wire_surf_map");
#endif
   
    buff = gs_get_att_typbuff(surf, ATT_TOPO, 0);
    cobuff = gs_get_att_typbuff(surf, ATT_COLOR, 0);

    gs_update_curmask(surf);
    check_mask = surf->curmask ? 1 : 0;

    /*
    checks ATT_TOPO & ATT_COLOR no_zero flags, make a mask from each,
    combine it/them with any current mask, put in typbuff:
    if(surf->att[ATT_TOPO].constant)
    */


    xmod = surf->x_modw;
    ymod = surf->y_modw;
    xres = xmod * surf->xres;
    yres = ymod * surf->yres;
    ymax = (surf->rows - 1) * surf->yres;
    xcnt =  1 + (surf->cols - 1) / xmod;
    ycnt =  1 + (surf->rows - 1) / ymod;
/*
    xcnt =  surf->cols / xmod;
    ycnt =  surf->rows / ymod;
*/


    gsd_pushmatrix();
    gsd_do_scale(1);
    gsd_translate(surf->x_trans, surf->y_trans, surf->z_trans);
/*
    gsd_translate(surf->x_trans, surf->y_trans, surf->z_trans + surf->zmin);
*/
    zexag = surf->z_exag;

    gsd_colormode(CM_COLOR);

   
/* will need to check for color source of FUNC_ATT & NOTSET_ATT, 
    or else use more general and inefficient gets */

    check_color = (surf->wire_color == WC_COLOR_ATT);
    if(check_color){
	coloratt = &(surf->att[ATT_COLOR]);
	col_src = surf->att[ATT_COLOR].att_src;
	if(col_src != MAP_ATT){
	    if(col_src == CONST_ATT){
		gsd_color_func(surf->att[ATT_COLOR].constant);
	    }
	    else
		gsd_color_func(surf->wire_color);
	    check_color = 0;
	}
    }
    else
	gsd_color_func(surf->wire_color);
    
    /* would also be good to check if colormap == surfmap, to increase speed */


    for(row = 0; row < ycnt; row++) {
	

	pt[Y] = ymax - row*yres;
	y1off = row * ymod * surf->cols;

	gsd_bgnline ();
	cnt = 0;

	for(col = 0; col < xcnt; col++) {
	    
	    pt[X] = col*xres;
	    x1off = col * xmod;
	    offset = x1off + y1off;
	    
	    if(check_mask){
		if(BM_get(surf->curmask, col*xmod, row*ymod)){
		    gsd_endline();
		    gsd_bgnline();
		    cnt = 0;
		    continue;
		}
	    }

            GET_MAPATT(buff, offset, pt[Z]); 

	    if(check_color){
		curcolor = gs_mapcolor(cobuff, coloratt, offset);
		gsd_color_func(curcolor);
/* could use this & skip the GET if colordata == elevdata
	gsd_color_func(gs_fastmapcolor(cobuff, coloratt, offset, (int)pt[Z]));
*/
	    }
/*
	    pt[Z] = (pt[Z] - surf->zmin)*zexag; 
*/
	    pt[Z] = pt[Z] * zexag; 

	    gsd_vert_func(pt); 

	    if(cnt == 255){
		gsd_endline();
		gsd_bgnline();
		cnt = 0;
		gsd_vert_func(pt);
	    }

	    cnt++;
	}
	gsd_endline();
    }


    for(col = 0; col < xcnt; col++) {
	
	pt[X] = col*xres;
	x1off = col * xmod;

	gsd_bgnline ();
	cnt = 0;

	for(row = 0; row < ycnt; row++) {
	    
	    pt[Y] = ymax - row*yres;
	    y1off = row * ymod * surf->cols;
	    offset = x1off + y1off;
	    
	    if(check_mask){
		if(BM_get(surf->curmask, col*xmod, row*ymod)){
		    gsd_endline();
		    gsd_bgnline();
		    cnt = 0;
		    continue;
		}
	    }
            GET_MAPATT(buff, offset, pt[Z]); 

	    if(check_color){
		curcolor = gs_mapcolor(cobuff, coloratt, offset);
		gsd_color_func(curcolor);
/* could use this & skip the GET if colordata == elevdata
		gsd_color_func(gs_fastmapcolor(coloratt, offset, (int)pt[Z]));
*/
	    }
/*
	    pt[Z] = (pt[Z] - surf->zmin)*zexag; 
*/
	    pt[Z] = pt[Z] * zexag; 

	    gsd_vert_func(pt); 

	    if(cnt == 255){
		gsd_endline();
		gsd_bgnline();
		cnt = 0;
		gsd_vert_func(pt);
	    }

	    cnt++;
	}
	gsd_endline();
    }
    gsd_popmatrix();
    
}

/************************************************************************/

int
gsd_wire_surf_const(surf, k)
geosurf *surf;
int k;
{
int check_mask, check_color;
int xmod, ymod, row, col, cnt, xcnt, ycnt, x1off;
long offset, y1off;
float pt[4], xres, yres, ymax, zexag;
int col_src;
gsurf_att *coloratt;
typbuff *cobuff;

#ifdef TRACE_DFUNCS
Gs_status("gsd_wire_surf_map");
#endif

    cobuff = gs_get_att_typbuff(surf, ATT_COLOR, 0);

    gs_update_curmask(surf);
    check_mask = surf->curmask ? 1 : 0;

    xmod = surf->x_modw;
    ymod = surf->y_modw;
    xres = xmod * surf->xres;
    yres = ymod * surf->yres;
/*
    xcnt =  surf->cols / xmod;
    ycnt =  surf->rows / ymod;
*/
    xcnt =  1 + (surf->cols - 1) / xmod;
    ycnt =  1 + (surf->rows - 1) / ymod;
    ymax = (surf->rows - 1) * surf->yres;

    gsd_pushmatrix();
    gsd_do_scale(1);
    gsd_translate(surf->x_trans, surf->y_trans, surf->z_trans);

    zexag = surf->z_exag;

    gsd_colormode(CM_COLOR);

/* will need to check for color source of FUNC_ATT & NOTSET_ATT, 
    or else use more general and inefficient gets */

    check_color = (surf->wire_color == WC_COLOR_ATT);
    if(check_color){
	coloratt = &(surf->att[ATT_COLOR]);
	col_src = surf->att[ATT_COLOR].att_src;
	if(col_src != MAP_ATT){
	    if(col_src == CONST_ATT){
		gsd_color_func(surf->att[ATT_COLOR].constant);
	    }
	    else
		gsd_color_func(surf->wire_color);
	    check_color = 0;
	}
    }
    else
	gsd_color_func(surf->wire_color);

    pt[Z] = k * zexag; 

    for(row = 0; row < ycnt; row++) {
	

	pt[Y] = ymax - row*yres;
	y1off = row * ymod * surf->cols;

	gsd_bgnline ();
	cnt = 0;

	for(col = 0; col < xcnt; col++) {
	    
	    pt[X] = col*xres;
	    x1off = col * xmod;
	    offset = x1off + y1off;
	    
	    if(check_mask){
		if(BM_get(surf->curmask, col*xmod, row*ymod)){
		    gsd_endline();
		    gsd_bgnline();
		    cnt = 0;
		    continue;
		}
	    }
	    if(check_color)
		gsd_color_func(gs_mapcolor(cobuff, coloratt, offset));

	    gsd_vert_func(pt); 

	    if(cnt == 255){
		gsd_endline();
		gsd_bgnline();
		cnt = 0;
		gsd_vert_func(pt);
	    }

	    cnt++;
	}
	gsd_endline();
    }


    for(col = 0; col < xcnt; col++) {
	
	pt[X] = col*xres;
	x1off = col * xmod;

	gsd_bgnline ();
	cnt = 0;

	for(row = 0; row < ycnt; row++) {
	    
	    pt[Y] = ymax - row*yres;
	    y1off = row * ymod * surf->cols;
	    offset = x1off + y1off;
	    
	    if(check_mask){
		if(BM_get(surf->curmask, col*xmod, row*ymod)){
		    gsd_endline();
		    gsd_bgnline();
		    cnt = 0;
		    continue;
		}
	    }
	    if(check_color)
		gsd_color_func(gs_mapcolor(cobuff, coloratt, offset));

	    gsd_vert_func(pt); 

	    if(cnt == 255){
		gsd_endline();
		gsd_bgnline();
		cnt = 0;
		gsd_vert_func(pt);
	    }

	    cnt++;
	}
	gsd_endline();
    }

    gsd_popmatrix();

}

/************************************************************************/

gsd_wire_surf_func(gs, user_func)
geosurf *gs;
int (*user_func)();
{

/*
Gs_status("Not yet supported");
*/

}
/************************************************************************/
/************************************************************************/
