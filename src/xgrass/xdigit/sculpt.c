
/*
** Written by Bill Brown, Fall 1992
** Modified for X, Terry Baker Spring 1993
*/

/*
** Copyright USA CERL 1992. All rights reserved.
*/
#include <math.h>
#include "digit.h"
#include "wind.h"
#include "gis.h"
#include "linkm.h"
#include "color.h"
#include "ginput.h"

#define X 0
#define Y 1


/***********************************************************************/

Pntlist *
copy_pbuf_tolist(token)
VOID_T * token;
{
Pntlist *top, *prev, *p;
int i;
if (Numppts == 0)
    return (NULL);
    top = (Pntlist *)link_new(token); 
    top->prior = NULL;

    p = top; 
    for(i = 0; i< Numppts; i++){
        p->pnt[X] = Pbuf[i][X];
        p->pnt[Y] = Pbuf[i][Y];
        p->next = (Pntlist *)link_new(token); 
        p->next->prior = prev = p;
        p = p->next;
    }
    prev->next = NULL;
    link_dispose(token, p);

    return(top);
}
/***********************************************************************/

Pntlist *
copy_pl(token, old)
VOID_T * token;
Pntlist *old;
{
Pntlist *top, *prev, *p, *tmp;
int i = 0;
    if (!token)
    {
	return ((Pntlist *)0);
    }
    if (!old)
    {
	return ((Pntlist *)0);
    }

    top = (Pntlist *)link_new(token); 
    top->prior = NULL;

    p = top; 
    for(tmp = old; tmp != NULL; ){
        p->pnt[X] = tmp->pnt[X];
        p->pnt[Y] = tmp->pnt[Y];
        p->next = (Pntlist *)link_new(token); 
        p->next->prior = prev = p;
        p = p->next;
	if (tmp)
	    tmp = tmp->next;
    }
    if (prev != NULL)
        prev->next = NULL;
    link_dispose(token, p);
    return(top);
}
/***********************************************************************/

void
copy_list_to_buf (old)
Pntlist *old;
{
Pntlist  *p, *tmp;
int i;

    for(tmp = old, i = 0; tmp != NULL; tmp = tmp->next, i++){
        Pbuf[i][X] = tmp->pnt[X];
        Pbuf[i][Y] = tmp->pnt[Y];
    }
    Numppts = i - 1;

}
/***********************************************************************/
void
empty_pl(token, old)
VOID_T *token;
Pntlist *old;
{
Pntlist  *p;
Pntlist  *tmp;
    
    if (token == NULL || old == NULL)
    {
	return ;
    }
    for(p = old ; p != NULL; p = tmp)
    {
	tmp = p->next;
        link_dispose(token, p);
    }
}
/***********************************************************************/

Pntlist *
copy_line_tolist(line, token)
struct line_pnts *line;
VOID_T *token;
{
Pntlist *top, *prev, *p;
int i;

    top = (Pntlist *)link_new(token); 
    top->prior = NULL;

    p = top; 
    for(i = 0; i< line->n_points; i++){
        p->pnt[X] = line->x[i];
        p->pnt[Y] = line->y[i];
        p->next = (Pntlist *)link_new(token); 
        p->next->prior = prev = p;
        p = p->next;
    }
    prev->next = NULL;
    link_dispose(token, p);

    return(top);
}


/***********************************************************************/
    

    
sculpt_it()
{
short val;
long dev;
double  prevpx, prevpy, tx, ty;
double px, py, d, dincr, norm, vect[2];
double dscale, dgrav;
int i, repeat; 
int scale, whole, antigrav, grav = 100;
int action, screen_x, screen_y;
int copyrad, rad, diam;
int first = 1;
int t;
int top, left, w, h;
    redisplay_current_edit();

    whole = XmToggleButtonGetState (Awhole);
    if(antigrav = XmToggleButtonGetState (Aantigrav))
    {  
        XtVaGetValues (Agravextent, XmNvalue, &grav, NULL);
    }
    XtVaGetValues (Asculptsiz, XmNvalue, &scale, NULL);
    dgrav = grav/100.0;
    dscale = scale/100.0;
    rad = (int)(25*dscale);
    copyrad = (int)(rad*dgrav + 1);
    diam = (int)(rad *dgrav* 2 + 2);
    SculptRad = (double)rad * ONEPIX;
    prevpx = prevpy = 0.0;
    show_select_dialog("accept", "abort", "Sculpt line", 1);


    vect[X] = vect[Y] = 0.0;    
    while(TRUE){
            action = Check_for_action (&screen_x, &screen_y);
	    switch (action) {
		case DRAW:
		case 0:
			if (!action && (!antigrav ||  first))
			    break;
			first = 0;
			if (action)
			    screen_to_utm (screen_x, screen_y, &px, &py);
			if(NewPL)
			{
			    tx = px;
			    ty = py;
			    display_sculptool(tx, ty, rad, dgrav);
			    if(!Pulled)
			    {
				SegIndex[1] =
				    get_closest_point(Cur_Points, px, py, vect);
				fill_pbuf_orig();
			    
				Lastmove[X] = Lastmove[Y] = 0;
				Pulled = 1;
			    }
			    if(whole)
			    {
				
			        PLtop = copy_line_tolist(Cur_Points, PLtoken);
			    }
			    else
			    {
				PLtop = copy_pbuf_tolist(PLtoken);
			    }
			    CurrPL = copy_pl (PLtoken, PLtop);
			    NewPL = 0;
			    display_plseg(PLtop, XD_WHITE); 
			}
			Editing = 1;
			if(!prevpx)
			{
			    prevpx = px;
			    prevpy = py;
			}
			/*limit movement speed to prevent jumping over lines*/
			d = distance(prevpx, prevpy, px, py);
			repeat = 1 + (int)(d / (SculptRad/2.0));
			dincr = 1.0  / repeat;

			for(i = 0 ; i < repeat; i++)
			{
			    /* erase previous */
			    utm_to_screen (tx, ty, &screen_x, &screen_y);

			    tx = prevpx + (i+1)*dincr*(px - prevpx);
			    ty = prevpy + (i+1)*dincr*(py - prevpy);
			    if(antigrav)
				nudge_outside_pts(tx, ty, 
					    SculptRad, dgrav, PLtop);
			    else
				nudge_pts(tx, ty, SculptRad, PLtop);
			    /*erase previous */
			     left = (screen_x - 2*copyrad >= 0) ? 
					     screen_x - 2*copyrad : 0; 
			     top = (screen_y - 2*copyrad >= 0) ? 
					     screen_y - 2*copyrad : 0; 
			    XCopyArea (dpy, pix, XtWindow (canvas), gc, 
					     left, top,
					     2*diam , 2*diam,
					     left, top);
					     
			    display_sculptool(tx, ty, rad, dgrav);
			    display_plseg(PLtop, XD_WHITE);
			}
		    prevpx = px;
		    prevpy = py;
		    break;
		case FIND:
		    clean_plist(PLtop);
		    first = 1;
		    prevpx = prevpy = 0.0;
		    copy_pix();
		    display_sculptool(tx, ty, rad, dgrav);
		    display_plseg(PLtop, XD_WHITE);
		    break;

		case ACCEPT:
		    first = 1;
		    EditChanges = 1;
		    Editing = 0;
		    
		    /* refresh canvas */
		    copy_pix();
		    
		    /* empty list of old  changes */
		    empty_pl (PLtoken, CurrPL);
		    
		    /* make new changes current */
		    CurrPL= copy_pl (PLtoken, PLtop);
		    
		    /* show new line */
		    display_plseg(PLtop, XD_WHITE);
		    return(0);
		    break;
		case DONE:
		    Editing = 0;
		    first = 1;
		    /* refresh canvas */
		    copy_pix();
		    
		    /* erase line and empty list of current changes */
		    empty_pl (PLtoken, PLtop);

		    /* copy previous line back to top */
		    PLtop = copy_pl (PLtoken, CurrPL);
		    
		    /* draw old line */
		    display_plseg(PLtop, XD_WHITE);
		    return(0);
		    break;
	    }
    }
}
/***********************************************************************/
    
blot_it()
{
long dev;
double  blotrad, px, py, vect[2];
int i; 
int scale, rad, whole;
double dscale;
int action, screen_x, screen_y, prevx, prevy;
int left, top, bottom, right;

    redisplay_current_edit();

    prevx = 0;
    prevy = 0;
    left = top = bottom = right = 0;
			
    whole = XmToggleButtonGetState (Awhole);
    XtVaGetValues (Ablotsiz, XmNvalue, &scale, NULL);
    dscale = scale/100.0;

    rad = (int)(25.0 * dscale);    /* just for now */
    blotrad = rad * ONEPIX;
   
    if (CurrPL)
	get_corners (CurrPL, &left, &top, &right, &bottom);
   
    show_select_dialog("accept", "abort", "Blot line", 1);
    while(TRUE){
            action = Check_for_action (&screen_x, &screen_y);
	    switch (action) {
		case DRAW:
		    screen_to_utm (screen_x, screen_y, &px, &py);
		    if(NewPL){
			if(!Pulled){
			    SegIndex[1] =
				get_closest_point(Cur_Points, px, py, vect);
			    fill_pbuf_orig();
			    Lastmove[X] = Lastmove[Y] = 0;
			    Pulled = 1;
			}
			if(whole)
			{
			    PLtop = copy_line_tolist(Cur_Points, PLtoken);
			}
			else
			{
			    PLtop = copy_pbuf_tolist(PLtoken);
			}
			CurrPL = copy_pl (PLtoken, PLtop);
			NewPL = 0;
			get_corners (CurrPL, &left, &top, &right, &bottom);
		    }	
	    	    Editing = 1;
		    if (!prevx)
		    {
			prevx = screen_x;
			prevy = screen_y;
		    }
		    
		    /* erase previous */
			left = 
			    left < prevx - rad  - 1? left : prevx - rad - 1;
			top = 
			    top < prevy - rad  - 1? top : prevy - rad - 1;
			right = 
			    right >  prevx + rad + 1? right : prevx + rad + 1;
			bottom  = 
			    bottom > prevy + rad + 1? bottom : prevy + rad + 1;
			XCopyArea (dpy, pix, XtWindow (canvas), gc, 
					     left, top, right - left, 
					     bottom - top, left, top);
		    prevx = screen_x;
		    prevy = screen_y;

		    display_blot_tool(px, py, rad);
		    blot_pts(px, py, blotrad, PLtop);
		    display_plseg(PLtop, XD_WHITE);
		    break;

		case ACCEPT:
		    Editing = 0;
		    EditChanges = 1;
		    copy_pix();
		    empty_pl (PLtoken, CurrPL);
		    CurrPL= copy_pl (PLtoken, PLtop);
		    display_plseg(PLtop, XD_WHITE);
		    return(0);
		    break;
		case DONE:
		    Editing = 0;
		    copy_pix();
		    empty_pl (PLtoken, PLtop);
		    PLtop = copy_pl (PLtoken, CurrPL);
		    display_plseg(PLtop, XD_WHITE);
		    return(0);
		    break;
	    }
    }

}

/***********************************************************************/

count_pts(top)
Pntlist *top;
{
Pntlist *p;
int cnt=0;

    for (p = top; p != NULL; ++cnt, p = p->next);

    return(cnt);
}

/***********************************************************************/

do_adjust(px, py, rad, new, mag)
double px, py, rad, mag;
Pntlist *new;
{
double uvect[2];

    uvect[X] = (new->pnt[X] - px)/mag;
    uvect[Y] = (new->pnt[Y] - py)/mag;
    new->pnt[X] = px + uvect[X] * rad; 
    new->pnt[Y] = py + uvect[Y] * rad; 

}

/***********************************************************************/

do_outside_adjust(px, py, rad, grav, mag, new)
double px, py, rad, grav, mag;
Pntlist *new;
{
double  mag2, velocity, diff, extent, uvect[2];
    
    extent = grav * rad;
    /* TODO: still working on this */
    mag2 = mag*mag;
    if(!mag2){
	new->pnt[X] += EPSILON;
	new->pnt[Y] += EPSILON;
	mag = EPSILON * sqrt(2);
    }
    velocity =  (rad*rad)/(mag*mag); 

    uvect[X] = (new->pnt[X] - px)/mag;
    uvect[Y] = (new->pnt[Y] - py)/mag;
    if(rad > mag){
	new->pnt[X] = px + uvect[X] * rad * (1. + EPSILON);
	new->pnt[Y] = py + uvect[Y] * rad * (1. + EPSILON);
    }
    else{
	new->pnt[X] = px + uvect[X] * (mag + (extent - mag)*velocity);
	new->pnt[Y] = py + uvect[Y] * (mag + (extent - mag)*velocity);
    }


}

/***********************************************************************/
adjust_pt(px, py, rad, new)
double px, py, rad;
Pntlist *new;
{
double mag;

    if(rad > (mag = distance(px, py, new->pnt[X], new->pnt[Y]))){
	do_adjust(px, py, rad, new, mag);
    }

}

/***********************************************************************/

Pntlist *
add_midpt(prev, p)
Pntlist *prev, *p;
{
Pntlist *new;

    new = (Pntlist *)link_new(PLtoken); 
    new->prior = prev;
    p->prior = new;
    prev->next = new;
    new->next = p;
    new->pnt[X] = (prev->pnt[X] + p->pnt[X]) / 2.0;
    new->pnt[Y] = (prev->pnt[Y] + p->pnt[Y]) / 2.0;

    return(new);
}

/***********************************************************************/
/* adds a point thresh distance along segment pn */
/* returns NULL if length  < thresh */

Pntlist *
add_pt_early(p, n, len, thresh)
Pntlist *p, *n;
double len, thresh;
{
Pntlist *new;
double ux, uy;
    if(len < thresh){
	return(NULL);
    }
    new = (Pntlist *)link_new(PLtoken); 
    new->prior = p;
    n->prior = new;
    p->next = new;
    new->next = n;
    ux = (n->pnt[X] - p->pnt[X]) / len;
    uy = (n->pnt[Y] - p->pnt[Y]) / len;
    new->pnt[X] = p->pnt[X] + ux * thresh;
    new->pnt[Y] = p->pnt[Y] + uy * thresh;

    return(new);

}

/***********************************************************************/
/* adds a point len - thresh distance along segment pn */
/* returns NULL if length  < thresh */

Pntlist *
add_pt_late(p, n, len, thresh)
Pntlist *p, *n;
double len, thresh;
{
Pntlist *new;
double dist, ux, uy;

    if(len < thresh){
	return(NULL);
    }
    dist = len - thresh;
    new = (Pntlist *)link_new(PLtoken); 
    new->prior = p;
    n->prior = new;
    p->next = new;
    new->next = n;
    ux = (n->pnt[X] - p->pnt[X]) / len;
    uy = (n->pnt[Y] - p->pnt[Y]) / len;
    new->pnt[X] = p->pnt[X] + ux * dist;
    new->pnt[Y] = p->pnt[Y] + uy * dist;

    return(new);
}

/***********************************************************************/
Pntlist *
add_thispt(p, n, nx, ny)
Pntlist *p, *n;
double nx, ny;
{
Pntlist *new;

    new = (Pntlist *)link_new(PLtoken); 
    new->prior = p;
    n->prior = new;
    p->next = new;
    new->next = n;
    new->pnt[X] = nx;
    new->pnt[Y] = ny;

    return(new);
}

/***********************************************************************/

del_pt(prev, p)
Pntlist *prev, *p;
{

    if (p == NULL)
	return;
    prev->next = p->next;
    if(p->next != NULL)
	p->next->prior = prev;
    link_dispose(PLtoken, p);

}

/**************************************************************************/
/* This one allows full stretching  */

nudge_pts(px, py, rad, top)
double px, py, rad;
Pntlist *top;
{
double lowthresh, midthresh, thresh, lowthresh2, thresh2;
double sqrtlen, len, mag, mag2, mag2a, rad2, norm, uvect[2];
Pntlist *p, *prev, *new, *tmp;
double nx, ny;
    rad2 = rad*rad;

    thresh = rad / 2.0;
    lowthresh = thresh / 2.0;
    midthresh = (thresh + lowthresh)/2.0;
    thresh2 = thresh*thresh;
    lowthresh2 = lowthresh*lowthresh;
    prev = NULL;
    for (p = top->next; p->next != NULL; p = p->next) /* leave anchors alone! */
    {
	if(p->next->next == NULL)
	{
	    return(0);
        }
	nx = px;
	ny = py;
	if(rad2 > (mag2a = dig_xy_distance2_point_to_line(&nx, &ny,
		    p->pnt[X], p->pnt[Y],p->next->pnt[X], p->next->pnt[Y]))){

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(rad2 > (mag2 = distance2(px, py,p->pnt[X], p->pnt[Y]))){
	  
	    mag = sqrt(mag2);
	    do_adjust(px, py, rad, p, mag);
	  }
	  else if(rad2>(mag2=distance2(px,py,p->next->pnt[X],p->next->pnt[Y]))){
	    mag = sqrt(mag2);
	    do_adjust(px, py, rad, p->next, mag);
	    if(p->next->next->next != NULL){   /* not last */
		prev = p;
		p = p->next;
	    }
	  }
	  else if(mag2a && len > thresh2){  /* seg intersects, but not ends */
		new = add_thispt(p, p->next, nx, ny);
		adjust_pt(px, py, rad, new);
		prev = p;
		p = p->next;
	  }

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(len < lowthresh2){
		del_pt(p, p->next);
	  }
	  else if(len > thresh2){
	  sqrtlen = sqrt(len);
		new = add_pt_early(p, p->next, sqrtlen, midthresh);
                if (new != NULL) 
		adjust_pt(px, py, rad, new);
	  }

	  if(prev != NULL){
		len = distance2(p->pnt[X],p->pnt[Y],prev->pnt[X],prev->pnt[Y]);
		if(len < lowthresh2){
		    del_pt(prev, p);
		    p = prev; 
		}
		else if(len > thresh2){
		    new = add_pt_late(prev, p, sqrt(len), midthresh);
		    adjust_pt(px, py, rad, new);
		}
	  }
	}
	prev = p;
    }
}
    

/**************************************************************************/
/* allows full stretching  */

nudge_outside_pts1(px, py, rad, top)
double px, py, rad;
Pntlist *top;
{
double  mag, mag2, rad2;
Pntlist *p;
int grav;
double dgrav;	/* dpg */
    
    XtVaGetValues (Agravextent, XmNvalue, &grav, NULL);
    /* grav /= 100; dpg */
    dgrav = grav / 100.;
  
    rad2 = dgrav* dgrav*rad*rad; 
    /* GRAVrad squared  - extent of anti-gravity */

    for (p = top->next; p->next != NULL; p = p->next) /* leave anchors alone! */
    {

        if(rad2 > (mag2 = distance2(px, py,p->pnt[X], p->pnt[Y]))){
  	  mag = sqrt(mag2);
	  do_outside_adjust(px, py, rad, dgrav, mag, p);
        }
    }
}

/**************************************************************************/
/* This one allows full stretching  */

nudge_outside_pts(px, py, rad, grav, top)
double px, py, rad, grav;
Pntlist *top;
{
double lowthresh, midthresh, thresh, lowthresh2, thresh2;
double len, mag, mag2, mag2a, rad2, norm, uvect[2];
Pntlist *p, *prev, *new, *tmp;
double nx, ny;
int change = 0;

    rad2 = grav*grav*rad*rad; 
    /* GRAVrad squared  - extent of anti-gravity */

    thresh = grav * rad / 2.0;
    lowthresh = thresh / 2.0;
    midthresh = (thresh + lowthresh)/2.0;
    thresh2 = thresh*thresh;
    lowthresh2 = lowthresh*lowthresh;

    prev = NULL;
    for (p = top->next; p->next != NULL; p = p->next) /* leave anchors alone! */
    {
	if(p->next->next == NULL)
	{
	    return(change);
	}
	nx = px;
	ny = py;
	if(rad2 > (mag2a = dig_xy_distance2_point_to_line(&nx, &ny,
		    p->pnt[X], p->pnt[Y],p->next->pnt[X], p->next->pnt[Y])))
	{

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(rad2 > (mag2 = distance2(px, py,p->pnt[X], p->pnt[Y]))){
	    mag = sqrt(mag2);
	    do_outside_adjust(px, py, rad, grav, mag, p);
	    change = 1;
	  }
	  else if(rad2>(mag2=distance2(px,py,p->next->pnt[X],p->next->pnt[Y]))){
	    mag = sqrt(mag2);
	    do_outside_adjust(px, py, rad, grav, mag, p->next);
	    if(p->next->next->next != NULL){   /* not last */
		prev = p;
		p = p->next;
	    }
	    change = 1;
	  }
	  else if(mag2a && len > thresh2){  /* seg intersects, but not ends */
                new = add_thispt(p, p->next, nx, ny);
		adjust_pt(px, py, rad, new);
		prev = p;
		p = p->next;
	        change = 1;
	  }

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(len < lowthresh2){
		del_pt(p, p->next);
	        change = 1;
	  }
	  else if(len > thresh2){
		new = add_pt_early(p, p->next, sqrt(len), midthresh);
		adjust_pt(px, py, rad, new);
	        change = 1;
		/**/
		prev = p;
		p = p->next;
		/**/
	  }

	  if(prev != NULL){
		len = distance2(p->pnt[X],p->pnt[Y],prev->pnt[X],prev->pnt[Y]);
		if(len < lowthresh2){
	            change = 1;
		    del_pt(prev, p);
		    p = prev; 
		}
		else if(len > thresh2){
	            change = 1;
		    new = add_pt_late(prev, p, sqrt(len), midthresh);
		    adjust_pt(px, py, rad, new);
		}
	  }
	}
	prev = p;
    }

}

/**************************************************************************/
/* This one allows no stretching   NOT YET IMPLEMENTED!  */

rigid_nudge_pts(px, py, rad, top)
double px, py, rad;
Pntlist *top;
{
double lowthresh, midthresh, thresh, lowthresh2, thresh2;
double len, mag, mag2, mag2a, rad2, norm, uvect[2];
Pntlist *p, *prev, *new, *tmp;
double nx, ny;
   
    thresh = rad / 2.0;
    lowthresh = thresh / 2.0;
    midthresh = (thresh + lowthresh)/2.0;
    thresh2 = thresh*thresh;
    lowthresh2 = lowthresh*lowthresh;
    rad2 = rad*rad;
    prev = NULL;
    for (p = top->next; p->next != NULL; p = p->next){ /* leave anchors alone! */
	if(p->next->next == NULL)
	    return(0);
	nx = px;
	ny = py;
	if(rad2 > (mag2a = dig_xy_distance2_point_to_line(&nx, &ny,
		    p->pnt[X], p->pnt[Y],p->next->pnt[X], p->next->pnt[Y]))){

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(rad2 > (mag2 = distance2(px, py,p->pnt[X], p->pnt[Y]))){
	    mag = sqrt(mag2);
	    do_outside_adjust(px, py, rad, 1., mag, p);
	  }
	  else if(rad2>(mag2=distance2(px,py,p->next->pnt[X],p->next->pnt[Y]))){
	    mag = sqrt(mag2);
	    do_outside_adjust(px, py, rad, 1., mag, p->next);
	    if(p->next->next->next != NULL){   /* not last */
		prev = p;
		p = p->next;
	    }
	  }
	  else if(mag2a && len > thresh2){  /* seg intersects, but not ends */
		new = add_thispt(p, p->next, nx, ny);
		adjust_pt(px, py, rad, new);
		prev = p;
		p = p->next;
	  }

       	  len = distance2p(p->pnt, p->next->pnt);

	  if(len < lowthresh2){
		del_pt(p, p->next);
	  }
	  else if(len > thresh2){
		new = add_pt_early(p, p->next, sqrt(len), midthresh);
		adjust_pt(px, py, rad, new);
	  }

	  if(prev != NULL){
		len = distance2(p->pnt[X],p->pnt[Y],prev->pnt[X],prev->pnt[Y]);
		if(len < lowthresh2){
		    del_pt(prev, p);
		    p = prev; 
		}
		else if(len > thresh2){
		    new = add_pt_late(prev, p, sqrt(len), midthresh);
		    adjust_pt(px, py, rad, new);
		}
	  }
	}
	prev = p;
    }
}
    
/***********************************************************************/

point_on_seg(new, p, n, len)
Pntlist *new, *p, *n;
double len;
{
double a, b, diff;

    a = distancep(new->pnt, p->pnt);
    b = distancep(new->pnt, n->pnt);
    diff = len - (a+b);
    return(diff < EPSILON && diff > -EPSILON);

}

/***********************************************************************/
/* Adds point between p & n that is just outside the circle */

Pntlist *
add_pt_on_circle(p, n, cx, cy, rad)
Pntlist *p, *n;
double cx, cy, rad; 
{
Pntlist *new, *bgn, *end;
double t, dir[2], len;
   
    t = ONEPIX;
    len = distancep(p->pnt, n->pnt);
    if(len < t){
	return(NULL);
    }

    if(rad >  distance(cx, cy, n->pnt[X], n->pnt[Y])){
	bgn = n;
	end = p;
    }
    else if (rad >  distance(cx, cy, p->pnt[X], p->pnt[Y])){
	bgn = p;
	end = n;
    }
    else
	return (NULL);

    new = (Pntlist *)link_new(PLtoken); 
    p->next = new;
    new->next = n;
    
    get_direction(dir, bgn->pnt, end->pnt, len);
    new->pnt[X] = bgn->pnt[X]; 
    new->pnt[Y] = bgn->pnt[Y];

    while(rad > distance(cx, cy, new->pnt[X], new->pnt[Y])){
	new->pnt[X] = new->pnt[X] + t * dir[X];
	new->pnt[Y] = new->pnt[Y] + t * dir[Y];
    }

    return(new);
}

/***********************************************************************/

blot_pts(px, py, rad, top)
double px, py, rad;
Pntlist *top;
{
Pntlist  *p, *prev, *new1, *new2;
   
    prev = top;
    for (p = top->next; p->next != NULL; p = p->next){ /* leave anchors alone! */
	new1 = new2 = NULL;
	if(rad > distance(px, py, p->pnt[X], p->pnt[Y])){
	    new1 = add_pt_on_circle(prev, p, px, py, rad);
	    if(new1 != NULL)
	        prev = new1;
            
	    while(rad > distance(px,py,p->next->pnt[X],p->next->pnt[Y])){
		del_pt(prev, p);
		p = prev->next;
		if(p->next == NULL) return 0;
	    }
	    new2 = add_pt_on_circle(p, p->next, px, py, rad);
	    del_pt(prev, p);
	    p = prev->next;
	    if(p == NULL) return 0;
	}
	prev = p;

	if (p->next == NULL)	/* added cuz stepped over p->next */
	    break;
    }
}

/***********************************************************************/

display_plseg(top, co)
Pntlist *top;
int co;
{
Pntlist *p;
int i, show;
int x, y;
int n;
int xmin, ymin, xmax, ymax;

    if (!top)
	return (0);


    show = XmToggleButtonGetState (Ashowpts);
    XSetForeground(dpy, gc, dcolors[co]);
    for(i = 0, p = top; p != NULL; p= p->next, i++){
	if (i >= MAX_PULL_PTS)
	{
            XDrawLines (dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);
    	    if(show)
	    {
		if (co != CLR_ERASE)
    		    XSetForeground(dpy, gc, dcolors[XD_MAGENTA]);
		XDrawPoints 
		(dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);
    		XSetForeground(dpy, gc, dcolors[co]);
	    }
	    i = 0;
	}
	utm_to_screen (p->pnt[X], p->pnt[Y], &x, &y);
	xbuf[i].x = x;
	xbuf[i].y = y;
    }
    XDrawLines (dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);
    if(show)
    {
	if (co != CLR_ERASE)
	    XSetForeground(dpy, gc, dcolors[XD_MAGENTA]);
	XDrawPoints (dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);
    }
}
get_corners (top, x1, y1, x2, y2)
    Pntlist *top;
    int *x1, *y1, *x2, *y2;
{
   int x, y, i; 
   int xmax, xmin, ymax, ymin;
    Pntlist *p;
   
    if ( !top)
    {
	*x1 = *x2 = *y1 = *y2;
	return (0);
    }
    xmax = ymax = 0;
    xmin = Wdth; 
    ymin = Hght;
    
    for(i = 0, p = top; p != NULL; p= p->next, i++){
	utm_to_screen (p->pnt[X], p->pnt[Y], &x, &y);
	xmin = x < xmin ? x : xmin;
	xmax = x > xmax ? x : xmax;
	ymin = y < ymin ? y : ymin;
	ymax = y > ymax ? y : ymax;
   } 
    *x1 = xmin > 0 ? xmin : 0;
    *y1 = ymin > 0 ? ymin : 0;

    *x2 = xmax < Wdth? xmax : Wdth;
    *y2 = ymax < Hght ? ymax : Hght;
}

/***********************************************************************/

display_plseg_scaled(top, co, scale, centroid)
Pntlist *top;
int co;
double scale, centroid[2];
{
Pntlist *p;
float v[2];
double dir[2], dv[2], rad;
int i;
int x, y;
    if(XmToggleButtonGetState (Awhole))
    {

	for(p = top, i = 0; p != NULL ; p= p->next, i++){
	    dir[X] = p->pnt[X] - centroid[X];
	    dir[Y] = p->pnt[Y] - centroid[Y];
	    dv[0] = (centroid[X] + scale * dir[X]);
	    dv[1] = (centroid[Y] + scale * dir[Y]);
	    utm_to_screen (dv[0], dv[1], &x, &y);
	    xbuf[i].x = x;
	    xbuf[i].y = y;
	}
    }

    else{              /* don't scale anchors */
	dv[0] = top->pnt[X];
	dv[1] = top->pnt[Y];
	utm_to_screen (dv[0], dv[1], &x, &y);
	xbuf[0].x = x;
	xbuf[0].y = y;
	for(p = top->next, i = 1; p->next != NULL ; p= p->next, i++){
	    dir[X] = p->pnt[X] - centroid[X];
	    dir[Y] = p->pnt[Y] - centroid[Y];
	    dv[0] = (centroid[X] + scale * dir[X]);
	    dv[1] = (centroid[Y] + scale * dir[Y]);
	    utm_to_screen (dv[0], dv[1], &x, &y);
	    xbuf[i].x = x;
	    xbuf[i].y = y;
	}
	dv[0] = p->pnt[X];
	dv[1] = p->pnt[Y];
	utm_to_screen (dv[0], dv[1], &x, &y);
	xbuf[i].x = x;
	xbuf[i].y = y;
	i++;
    }
    XSetForeground(dpy, gc, dcolors[co]);
    XDrawLines (dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);

    if(XmToggleButtonGetState (Ashowpts))
    {
        XSetForeground(dpy, gc, dcolors[XD_BLUE]);
        XDrawPoints(dpy, XtWindow(canvas), gc, xbuf, i, CoordModeOrigin);
    }
}

/***********************************************************************/

clean_plist(top)
Pntlist *top;
{
int dirty=1;
    
    if(XmToggleButtonGetState (Aclean))
	while (dirty){
	    dirty = thin_out(top);
	}

}

/***********************************************************************/
/* eliminate midpoint of triples whose combined length - end-to-end length
is < .5% * Acleanres->val of the combined length */


thin_out(t)
Pntlist *t;
{
Pntlist *p, *nxt;
double tol, lena, lenb, dista, distb;
int cnt=0;
int val;
double dval;

    XtVaGetValues (Acleanres, XtNvalue, &val, NULL);
    dval = (double)val/100.0;

    for(p = t; p->next != NULL; p = p->next){
	if(p->next->next == NULL)
	    return(cnt);
	nxt = p->next;
	lena = distancep(p->pnt,nxt->pnt);
	lenb = distancep(nxt->pnt,nxt->next->pnt);
	dista = lena + lenb;
	distb = distancep(p->pnt,nxt->next->pnt); 
	tol = dval * dista / 200.;
	if(dista - distb < tol){
	    del_pt(p, nxt);
	    cnt++;
	}
	else if(fold(p, lena, lenb)){
	    del_pt(p, nxt);
	    cnt++;
	}
    }
    return(cnt);

}

/***********************************************************************/
/* adds the two vectors, normalizes them to the length of the smallest,
   returns 1 if resulting vect is shorter than tol, zero otherwise.
   Should end up eliminating angles of 0 - 60 degrees, depending on
   the value of Acleanres. */

fold(p, lena, lenb)
Pntlist *p;
double lena, lenb;
{
double tol, len, diffmag, diff[2], nvecta[2], nvectb[2];
int val;
double dval;
   
    get_direction(nvecta, p->pnt, p->next->pnt, lena);
    get_direction(nvectb, p->next->pnt, p->next->next->pnt, lenb);
    len = lena < lenb? lena: lenb;
    diff[X] = len * (nvecta[X] + nvectb[X]);
    diff[Y] = len * (nvecta[Y] + nvectb[Y]);
    diffmag = sqrt(diff[X] * diff[X] + diff[Y] * diff[Y]);
    
    XtVaGetValues (Acleanres, XtNvalue, &val, NULL);
    dval = val / 100;
    tol = dval * len;

    return(diffmag < tol);

}

/***********************************************************************/



