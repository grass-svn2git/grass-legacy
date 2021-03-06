

/*
**  Written by Bill Brown   Fall, 1992 
**  US Army Construction Engineering Research Lab
** Modified for X, Terry Baker Spring 1993
*/


/*
** Copyright USA CERL 1992. All rights reserved.
*/

#include "digit.h"
#include "dig_head.h"
#include <math.h>
#include "wind.h"

#define X 0
#define Y 1
/**********************************************************************/


get_closest_point(P, e, n, closest)
struct line_pnts *P;
double e, n, closest[2];
{
int i, index = -1;
double dist, dist2;
int first = 1;
   
    for(i=0; i < P->n_points; i++){
	dist2 = distance(P->x[i], P->y[i], e, n);
	if (first)
	{
	    first = 0;
	    dist = dist2;
	}
	if(dist2 < dist){
	    dist = dist2;
	    index = i;
	}
    }
    if(index >= 0){
	closest[X] = P->x[index];
	closest[Y] = P->y[index];
    }
    return(index);
}
/**********************************************************************/
/* fill point buffer with original points */
/************************************************************************/

fill_pbuf_orig()
{
int i, numpts, id, startid, endid;
struct line_pnts *Points;

     Points =Cur_Points;

    if(!Closed && (SegIndex[1] < SegIndex[0] || SegIndex[1] > SegIndex[2]))
	return (0);  /* pullpoint outside of anchors */

    /* end = Points->n_points - 1; */
    startid = SegIndex[0];
    endid = SegIndex[2];

    if(Closed && (SegIndex[1] < SegIndex[0] || SegIndex[1] > SegIndex[2])){
	startid = SegIndex[2];
	endid = SegIndex[0];
    }

    /* for (i=0, id=startid; id != endid; i++, id = (id+1)%end){ dpg */

    for (i=0, id=startid; id <= endid; i++, id++)
    {
	Pbuf[i][X] = Points->x[id];
	Pbuf[i][Y] = Points->y[id];
	if(id == SegIndex[1]) PullIndex = i;
    }

    Newseg = 1;
    Numppts = i;
    return(Numppts);  /* num of points in Pbuf */
}


/************************************************************************/
/* fill point buffer with a line between anchors */

fill_pbuf_line()
{
int i, end, numpts, id, startid, endid;
double seglength, dist, linevect[2];
struct line_pnts *Points;

     Points = Cur_Points;

    if(!Closed && (SegIndex[1] < SegIndex[0] || SegIndex[1] > SegIndex[2]))
	return (0);  /* pullpoint outside of anchors */

    end = Points->n_points - 1;
    startid = SegIndex[0];
    endid = SegIndex[2];

    if(Closed && (SegIndex[1] < SegIndex[0] || SegIndex[1] > SegIndex[2])){
	startid = SegIndex[2];
	endid = SegIndex[0];
    }
    
    linevect[X] = Points->x[endid] - Points->x[startid];
    linevect[Y] = Points->y[endid] - Points->y[startid];
    
    seglength = 0.0;
    for (id=startid; id != endid; id++)
	seglength += distance(Points->x[id],Points->y[id],
			Points->x[(id+1)],Points->y[(id+1)]);

    dist = 0.0;
    Pbuf[0][X] = Points->x[startid];
    Pbuf[0][Y] = Points->y[startid];
    for (i=0, id=startid; id != endid; i++, id++){
	dist += distance(Points->x[id],Points->y[id],
			Points->x[(id+1)],Points->y[(id+1)]);
	Pbuf[i+1][X] = Points->x[startid] + dist*linevect[X]/seglength;
	Pbuf[i+1][Y] = Points->y[startid] + dist*linevect[Y]/seglength;
	if(id == SegIndex[1]) PullIndex = i;
    }

    Newseg = 1;
    Numppts = i+1;
    
    return(Numppts);  /* num of points in Pbuf */
}



/************************************************************************/
/* actually takes the place of set_pullpoint when using arc not on line */

fill_pbuf_arc2(e, n)
double e, n;
{
int i, end, numpts, id, startid, endid;
double  anchor_arc, begarc, midarc, endarc, tmparc, arcincr, theta;
double arcpts[3][2], center[2], rad;
struct line_pnts *Points;

     Points = Cur_Points;


    arcpts[0][X] = Points->x[SegIndex[0]]; 
    arcpts[0][Y] = Points->y[SegIndex[0]]; 
    arcpts[1][X] = Pullpoint[X] = e; 
    arcpts[1][Y] = Pullpoint[Y] = n;
    arcpts[2][X] = Points->x[SegIndex[2]]; 
    arcpts[2][Y] = Points->y[SegIndex[2]]; 

    get_circle(arcpts, center, &rad);

    endarc = clockangle(arcpts[0], center, rad);
    midarc = clockangle(arcpts[1], center, rad);
    begarc = clockangle(arcpts[2], center, rad);
   
    /* ok, so I'm getting a brain spasm here, I'll just do it 
       step by step */

    /* make endarc > begarc */
    if(begarc > endarc){
	tmparc = begarc;
	begarc = endarc;
	endarc = tmparc;
    }

    if(endarc > midarc && midarc > begarc)
	anchor_arc = endarc - begarc;

    else{                  /* arc crosses zero */
	tmparc = begarc;
	begarc = endarc;
	endarc = tmparc;
	anchor_arc = endarc - begarc + 2.0 * Pi;
    }

    arcincr = anchor_arc / (ARC_RES - 1);
    Numppts = ARC_RES;
    PullIndex = Numppts / 2; /* actually would be better to use arc length */

    for(i = 0; i < Numppts; i++){
	theta = endarc - i * arcincr;
	Pbuf[i][X] = rad * sin(theta) + center[X];
	Pbuf[i][Y] = rad * cos(theta) + center[Y];
    }

    Newseg = 1;
    return(Numppts);  

}
/************************************************************************/
void 
ringbell()
{
    XBell (XtDisplay(toplevel), 25);
}
/************************************************************************/

set_pull_point(e, n)
    double e, n;
{
    int ok=0, id;

    if(Cur_Line > 0 && Anchors_set){
        if(XmToggleButtonGetState (Afillfunc2) || Cur_Line == dig_point_by_line
		(CM, e-TRIPIX,n+TRIPIX,e+TRIPIX,n-TRIPIX,LINE | AREA)){ 
		id = get_closest_point(Cur_Points, e, n, Pullpoint);
		if (id == SegIndex[0] || id == SegIndex[1])
		{
		    ok = 0;
		}
		else
		    if(0 <= id)
		    {
			display_pullpoint(Pullpoint);
			ok=1;
		    }
	    }
    }
    else{   /* no current line selected */
	ringbell();
	make_monolog(1,"First select a line to edit and set anchors!");
	return(0);
    }

    if(ok){
	SegIndex[1] = id;
	ok = fillfunc(); /*fill pullpoint buffer with points between anchors*/
    }
    if(!ok)
	return(0);
    
    return(1);
}


/************************************************************************/

double
shape_func_pow(x)
double x;
{
    int value;
    double val, power;

    XtVaGetValues (Afunchshape, XmNvalue, &value, NULL);
    val = (double)value/100.0;
   
    power = (val > .5 ? 1.0 + 8.0 * (val-.5): 
			    2.0 * val);
    return(pow(x, power));

}

/************************************************************************/

double pull_cos(dist)
double dist;
{
double x;

    x = shape_func_pow(dist); 

    return(cos(Pi*x/2.0));

}

/************************************************************************/
/* coef for spline through (0,1) , midpoint, (1,0) */

recalc_spline(mid, s)
double mid[2];
CubSpline *s;
{
double h[2], b[2], u2, v2, z[3];
int seg;

    s->x[0] = 0.0;
    s->y[0] = 1.0;
    s->x[1] = mid[X];
    s->y[1] = mid[Y];
    s->x[2] = 1.0;
    s->y[2] = 0.0;

    for(seg=0; seg < 2; seg++){
	h[seg] = s->x[seg+1] - s->x[seg];
	b[seg] = (s->y[seg+1] - s->y[seg])/h[seg];
    }

    u2 = 2. * (h[0] + h[1]);
    v2 = 6. * (b[1] - b[0]);
    z[0] = 0.0;
    z[2] = 0.0;
    z[1] = v2/u2;

    for(seg=0; seg < 2; seg++){
	s->A[seg] = s->y[seg];
	s->B[seg] = (-h[seg] * z[seg+1])/6. - (h[seg] * z[seg])/3. + b[seg];
	s->C[seg] = z[seg]/2.;
	s->D[seg] = (z[seg+1] - z[seg]) / (6.*h[seg]);
    }

}

/************************************************************************/
/* returns y = A + B*x + C*x*x + D*x*x*x  */

double
cubic_spline(s, seg, mid, val)
CubSpline *s;
int seg;
double mid[2], val;
{
double dx;

    dx = seg? val-mid[X]: val;
    val = s->A[seg] + dx*(s->B[seg] + dx*(s->C[seg] + dx*s->D[seg]));
    return (val);
}

/************************************************************************/
/* spline */

double 
pull_spline(dist)
    double dist;
{
    static double midpt[2];
    static int newspline;
    static CubSpline s;
    int seg;
    int value;
    double val;

    XtVaGetValues (Afunchshape, XmNvalue, &value, NULL);
    val = (double)value/100.0;
    
    if(midpt[X] != val){
	midpt[X] = val;
	newspline = 1;
    }
    if(midpt[Y] != val){
	midpt[Y] = val;
	newspline = 1;
    }
    if(newspline){
	recalc_spline(midpt, &s);
	newspline = 0;
    }
    
    seg = (dist > midpt[X]);
    return ( cubic_spline(&s, seg, midpt, dist) );

}

/************************************************************************/
/* line */

double pull_line(dist)
double dist;
{
double x;

    x = shape_func_pow(dist); 

    return(1.0 - x);

}

/************************************************************************/
/* first derivative = 0 */

double pull_spl1(dist)
double dist;
{
double x, x2, x3;

    x = shape_func_pow(dist); 
    x2 = x * x;
    x3 = x2 * x;

    return(1.0 - 3.0*x2 + 2.0*x3);

}

/************************************************************************/
/* first & second derivatives = 0 */

double pull_spl2(dist)
double dist;
{
double x, x2, x3;

    x = shape_func_pow(dist); 
    x2 = x * x;
    x3 = x2 * x;

    return(1.0 - 6.0*x2*x3 + 15.0*x2*x2 - 10.0*x3);

}

/************************************************************************/


double pull_spl3(dist)
double dist;
{
double x, x2, x3;

    x = shape_func_pow(dist); 
    x2 = x * x;
    x3 = x2 * x;
    return(1.0 + 3.*x2*x2 - 4.*x3);

}


/************************************************************************/
/* semi-circle */

double pull_semi(dist)
double dist;
{
double x;

    x = shape_func_pow(dist); 
    if(x > 1.0 || x < 0.0) return(0.0);

    return (sqrt(1.0 - x * x));

}

mono_out_of_memory ()
{
    make_monolog (1, "Out of memory!"); 
    return (-1);
}

/************************************************************************/
fill_new_line (points, plist)
    struct line_pnts *points;
    Pntlist *plist;
{
    int i, j;
    int npoints;
    Pntlist *p;
    int whole;
    int id1, id2;

    whole = XmToggleButtonGetState (Awhole);

    if (0 >= (Numppts = count_pts (plist)))
	return (0);
    p = plist;

    id1 = whole ?  0 : SegIndex[0];
    id2 = whole ? Cur_Points->n_points -1 :  SegIndex[2];
     npoints = id1 + Numppts + Cur_Points->n_points - id2 - 1;

    Vect_reset_line (points);

    if (whole) /* plist becomes new line */
    {
         for ( i = 0 ; i < Numppts; i++, p = p->next)
         {
	    if (0 >  Vect_append_point (points, p->pnt[X], p->pnt[Y]))
		return mono_out_of_memory ();
         }
	 SegIndex[0] = 0;
	 SegIndex[2] = i -1;
	 return (i);
    }
	

    /* else fill up seperate parts of line */
    /* fill up to first anchor with old line points */
     for ( i = 0 ; i < id1;  )
     {
	  if(0 > Vect_append_point (points, Cur_Points->x[i], Cur_Points->y[i]))
	    return mono_out_of_memory ();
	  i++;
     }
    /* fill mid section with new altered points */
     for ( j = 0 ; j < Numppts; j++, p = p->next)
     {
	  if (0 > Vect_append_point (points, p->pnt[X], p->pnt[Y]))
	    return mono_out_of_memory ();
	  i++;
     }
    /* fill after second anchor with old line points */
     for ( j = id2 +1;  j < Cur_Points->n_points; j++)
     {
	  if(0 > Vect_append_point (points, Cur_Points->x[j],Cur_Points->y[j]))
	    return mono_out_of_memory ();
	i++;
     }
     SegIndex[2] = id1 + Numppts -1;
     return (i+1);
}
/************************************************************************/
save_line (map, points, line)
    struct Map_info *map;
    struct line_pnts *points;
    int line;
{
    int i ;
    char type;
    struct new_node node;
    int is_site;
    int att;
  
     erase_line( map->Line[line].type, Cur_Points, Cur_Line, map);
     erase_line( map->Line[line].type, points, line, map);
     erase_anchors (Anchor);
     erase_pullpoint (Pullpoint);
        {
            if (map->Line[line].att)
                att = map->Att[map->Line[line].att].cat;
            else
                att = 0;

            type = map->Line[line].type;
        /* points loaded here */
            _remove_line (map, line);


            dig_check_nodes (map, &node, points);

            Cur_Line = line = new_line (map, type, &node, points);
            if (line < 0)
            {
                BEEP;
                make_monolog (1, "Error creating new line.");
                return (-1);
            }

            if (att)
            {
                if (map->Line[line].type == DOT)	/* dpg */
                    map->Line[line].att = dig_new_att (map, 
                       points->x[0], points->y[0], type, line, att);
                else
                {
                    double x, y;

                    get_line_center (&x, &y, points);
                    map->Line[line].att = 
			      dig_new_att (map, x, y, type, line, att);
                }
            }
        }

        Changes_Made = 1;
        highlight_line( map->Line[line].type, points, line, map);

        EditChanges = 0;
}
/**********************************************************************/

keep_line ()
{
    struct line_pnts *points;

    points = Vect_new_line_struct ();

    if (0 >= fill_new_line (points, CurrPL))
	return (0);
    save_line (CM, points, Cur_Line);
    Vect_destroy_line_struct (points);
    V2_read_line (CM, Cur_Points, Cur_Line);
}
