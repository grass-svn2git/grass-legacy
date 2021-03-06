
/*
** Written Summer, 1992 by Bill Brown
** US Army Construction Engineering Research Lab
** Modified for X, Terry Baker Spring 1993
*/

/*
** Copyright USA CERL 1992. All rights reserved.
*/

#include "digit.h"
#include "gis.h"
#include <math.h>
#include "wind.h"
#include "color.h"

double G_adjust_easting ();
double distance();
void display_vect();
/**********************************************************************/ 

draw_circle(x, y, rad, curr_gc)
int x, y, rad;
GC curr_gc;
{

    XDrawArc (dpy, XtWindow (canvas), curr_gc, 
		     x - rad, y - rad,
		     rad * 2, rad *2, 0, 360*64);
}

/**********************************************************************/

draw_disc(x, y, rad)
int x, y, rad;
{
float v[2];
int i;
    
    XFillArc (dpy, XtWindow (canvas), gc, 
		     x - rad, y - rad,
		     rad * 2, rad *2, 0, 360*64);
}
/**********************************************************************/


display_pullpoint(P)
double P[2];
{
    standard_color (dcolors[CLR_2_NODE]);
    Blot (&P[0], &P[1]);
}
erase_pullpoint(P)
double P[2];
{
    standard_color (dcolors[CLR_ERASE]);
    Blot (&P[0], &P[1]);

}


/**********************************************************************/
display_sculptool(e, n, rad, grav)
double e, n;
int rad;
double grav;
{
double dv[2];
int sx, sy;

    utm_to_screen (e, n, &sx, &sy);
    standard_color (dcolors[XD_RED]);
    draw_disc(sx, sy, rad);

    if(grav > 1.0)
	draw_circle(sx, sy, (int)(rad * grav), gc);

}


/**********************************************************************/

display_blot_tool(e, n, rad)
double e, n;
int rad;
{
double dv[2];
int x, y;

    standard_color (dcolors[XD_RED]);
    utm_to_screen (e, n, &x, &y);
    draw_circle(x,y, rad, gc);

}
/**********************************************************************/

erase_anchors(A)
double A[2][2];
{
  
    standard_color (dcolors[CLR_ERASE]);
    if (A[0][0] || A[0][1])
        Blot (&A[0][0], &A[0][1]);
    if (A[1][0] || A[1][1])
        Blot (&A[1][0], &A[1][1]);

}

erase_anchor(A)
double A[2];
{
  
    standard_color (dcolors[CLR_ERASE]);
    if (A[0] || A[1])
        Blot (&A[0], &A[1]);

}

/**********************************************************************/

display_anchor(A)
double A[2];
{
    standard_color (dcolors[CLR_HIGHLIGHT]);
    if (A[0] || A[1])
        Blot (&A[0], &A[1]);

}


display_anchors (A)
double A[2][2];
{
    standard_color (dcolors[CLR_2_NODE]);
    if (A[0][0] || A[0][1])
        Blot (&A[0][0], &A[0][1]);
    if (A[1][0] || A[1][1])
        Blot (&A[1][0], &A[1][1]);

}

/**********************************************************************/
/**********************************************************************/

recalc_dist(dist)
double dist[MAX_PULL_PTS];
{
int i; 
double sumdist, tleft, tright;
    
    tleft = tright = 0.0;

    for (i = 0; i < PullIndex; i++)
	tleft += distance(Pbuf[i][0], Pbuf[i][1], Pbuf[i+1][0], Pbuf[i+1][1]);

    for (i = PullIndex; i < (Numppts-1); i++)
	tright += distance(Pbuf[i][0], Pbuf[i][1], Pbuf[i+1][0], Pbuf[i+1][1]);
    
    dist[PullIndex] = 0.0;
    sumdist = 0.0;
    for (i = PullIndex; i > 0; i--){
	sumdist += distance(Pbuf[i][0], Pbuf[i][1], Pbuf[i-1][0], Pbuf[i-1][1]);
	dist[i-1] = sumdist/tleft; 
    }
    sumdist = 0.0;
    for (i = PullIndex; i < (Numppts-1); i++){
	sumdist += distance(Pbuf[i][0], Pbuf[i][1], Pbuf[i+1][0], Pbuf[i+1][1]);
	dist[i+1] = sumdist/tright; 
    }
}
    
/**********************************************************************/


static double Dist[MAX_PULL_PTS];

/**********************************************************************/
display_pullseg(vect, pullgc, pt)
    double vect[2];
    GC pullgc;
    XPoint *pt;
{
    double mult, dx, dy, x, y;
    int i, j;
    int screen_x, screen_y; 
    if(Newseg){
	recalc_dist(Dist);
	Newseg = 0;
    }
    for(i = 0; i < Numppts; i++){
	mult = pullfunc(Dist[i]);
	dx = vect[0] * mult;
	dy = vect[1] * mult;
	y = Pbuf[i][1] + dy ;
	x = Pbuf[i][0] + dx ;
        utm_to_screen (x, y, &screen_x, &screen_y);
	pt[i].x = screen_x;
        pt[i].y = screen_y;
    }
    XSetForeground (dpy, pullgc, dcolors[XD_WHITE]);

    XDrawLines (dpy, XtWindow (canvas), pullgc, pt, i, CoordModeOrigin);
    /* return npoints */
    return (i);
}
/**********************************************************************/

recalc_pullbuf(vect)
double vect[2];
{
double mult, dx, dy;
float pt[2];
int i;

    if(Newseg){
	recalc_dist(Dist);
	Newseg = 0;
    }


    for(i = 0; i < Numppts; i++){
	mult = pullfunc(Dist[i]);
	dx = vect[0] * mult;
	dy = vect[1] * mult;
	Pbuf[i][0] += dx;
	Pbuf[i][1] += dy;
    }

}
/**********************************************************************/

redisplay_current_edit()
{
    {
        if (Editing)
	    copy_pix();
        else if (EditChanges)
            display_plseg (PLtop, CLR_ERASE);
	   
	if (Cur_Line > 0)
	    highlight_line( CM->Line[Cur_Line].type, Cur_Points, Cur_Line, CM); 
        fill_pix();
        if (EditChanges || Editing)
            display_plseg (PLtop, XD_WHITE);
    }
}
