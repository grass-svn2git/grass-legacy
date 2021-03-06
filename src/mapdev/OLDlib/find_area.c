/*  @(#)find_area.c    2.1  6/26/87  */
#include "digit.h"
#include <math.h>


dig_find_area (map, Area, totalarea, cent_x, cent_y, south)
    struct Map_info *map;
    P_AREA *Area;
    double *totalarea;
    double *cent_x, *cent_y;
    double south;
{
    int cur_line;
    int ab_line;
    int n_points;
    int i;
    double area;
    double *xptr1, *yptr1;
    double *xptr2, *yptr2;
    double cent_weight_x, cent_weight_y;
    double len, tot_len;
    static struct line_pnts points;
    static int first_time;	/* 0 on startup */

    if (! Area->n_lines)
	return(-1);

    *totalarea = 0.0;
    tot_len = 0.0;
    cent_weight_x = 0.0;
    cent_weight_y = 0.0;

    if (first_time == 0)
    {
	points.alloc_points = 0;
	first_time = -1;
    }

    for(cur_line = 0; cur_line < Area->n_lines ; cur_line++)
    {
	ab_line = ABS(Area->lines[cur_line]);
	dig__Read_line (&points, map->digit, map->Line[ab_line].offset);
	
	area = 0.0;

	xptr1 = points.x;
	yptr1 = points.y;
	xptr2 = points.x + 1;
	yptr2 = points.y + 1;

	for(i=1; i<points.n_points; i++)
	{
	    area += (*xptr2 - *xptr1) * ((*yptr2 + *yptr1) / 2.0 - south);
	    len = hypot(*xptr1-*xptr2, *yptr1-*yptr2);
	    cent_weight_x += len * ((*xptr1 + *xptr2) / 2.);
	    cent_weight_y += len * ((*yptr1 + *yptr2) / 2.);
	    tot_len += len;
	    xptr1++ ; xptr2++ ; yptr1++; yptr2++;
	}

	if (Area->lines[cur_line] > 0)
	    *totalarea += area;
	else
	    *totalarea -= area;
    }

    if (tot_len != 0.0)
    {
	*cent_x = cent_weight_x / tot_len;
	*cent_y = cent_weight_y / tot_len;
    }
    
    return(0);
}

/*
**  old code was terrible overkill.  This routine is an efficient 
**  fast area find replacement for above.  
*/

dig_find_area2 (map, Area, totalarea)
    struct Map_info *map;
    P_AREA *Area;
    double *totalarea;
{
    int cur_line;
    int ab_line;
    int i;
    double *x, *y;
    static struct line_pnts points;
    static int first_time;	/* 0 on startup */
    double tot_area, sum_area;


    *totalarea = 0.;

    if (! Area->n_lines)
	return(-1);

    tot_area = 0.0;

    if (first_time == 0)
    {
	points.alloc_points = 0;
	first_time = -1;
    }

    tot_area = 0.0;
    for (cur_line = 0; cur_line < Area->n_lines ; cur_line++)
    {
	ab_line = ABS(Area->lines[cur_line]);

	dig__Read_line (&points, map->digit, map->Line[ab_line].offset);
	if (points.n_points < 2)
	    continue;
	
	x= points.x;
	y= points.y;

	sum_area = 0.0;
	for (i=1; i < points.n_points; i++) 
	{
	    sum_area += (x[i]-x[i-1]) * (y[i] + y[i-1]);
	}
	if (Area->lines[cur_line] > 0)
	    tot_area += sum_area;
	else
	    tot_area -= sum_area;
    }
    *totalarea =  0.5 * tot_area;
    
    return(0);
}
