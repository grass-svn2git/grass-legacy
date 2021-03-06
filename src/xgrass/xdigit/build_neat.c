/*
**  Written by Dave Gerdes  9/1988
**  US Army Construction Engineering Research Lab
*/

#include "digit.h"
#include "map.h"

static int order_cmp ();

/*
**  build_neat ()
**
**    Function requested by SCS to automatically create a (neat) line from
**    the first 4 map registration points.  Obviously the first 4 points
**    should be something line the four corners of a USGS quad sheet for this
**    function to create useful data.
*/

build_neat (map)
    struct Map_info *map;
{
    int order[4];
    char type;
    int line;
    int area;
    struct new_node node;
    int greatest, next_greatest;

    type = AREA;
    if (!Dig_Enabled)
	return (-1);
    write_info (1,"You have selected to automatically build a neat line from");
    add_info(1, "the 1st four current registration points.");
    if (!ask_driver_yes_no ( "Build rectangle from registration points?"))
	return (0);

    
    /* this block orders the x/y pairs in clockwise order starting a
    ** the southwest corner
    **  this assumes that there is a center point where a horizontal 
    **  will pass thru both vertical segments and a vertical line will pass
    **  thru both horzonal segments
    **
    ** ie. the points below would probably fail
    **
    **
    **		*-------*  |
    **		  \       \|
    **		    \      |\
    **		      \    |  \
    **		        \  |    \
    **		          \|      \
    **		           |*-------*
    **			   |
    **
    */

    {	
	register int i;
	/* by/bx are defined in map.h */
	/* they are the arrays of user entered map registration points */

	for (i = 0 ; i < 4 ; i++)
	    order[i] = i;	/* init the order array */

	qsort (order, 4, sizeof (int), order_cmp);

	/* now adjust Ys */
	if (by[order[1]] < by[order[0]])
	{
	    swap (&(order[0]), &(order[1]));
	}
	if (by[order[2]] < by[order[3]])
	{
	    swap (&(order[2]), &(order[3]));
	}
    }

    if (0 > dig_alloc_points (&Gpoints, 5))
	return (dig_out_of_memory ());

    Gpoints.x[0] = bx[order[0]];
    Gpoints.y[0] = by[order[0]];
    Gpoints.x[1] = bx[order[1]];
    Gpoints.y[1] = by[order[1]];
    Gpoints.x[2] = bx[order[2]];
    Gpoints.y[2] = by[order[2]];
    Gpoints.x[3] = bx[order[3]];
    Gpoints.y[3] = by[order[3]];
    Gpoints.x[4] = bx[order[0]];
    Gpoints.y[4] = by[order[0]];
    Gpoints.n_points = 5;


    /* force new node not to snap to any existing nodes */
    node.N1 = map->n_nodes+1;
    node.N2 = node.N1;
    line = new_line (map, type, &node, &Gpoints);
    if (line < 0)
    {
	BEEP;
	write_info (1, "Error creating new line.");
	sleep (4);
	return (-1);
    }
    Changes_Made = 1;
	display_line (type, &Gpoints, line, map);

    /* is this an area boundary that will affect neighbor areas? */
    /* if (type == AREA) */
    {  /* this shouldn't be needed */
	if (area = check_next (map, line, RIGHT))
	    Del_area (map, area);
	if (area = check_next (map, line, LEFT))
	    Del_area (map, area);
	if (area = check_next (map, -line, RIGHT))
	    Del_area (map, area);
	if (area = check_next (map, -line, LEFT))
	    Del_area (map, area);
    }

    return (0);
}

static 
swap (a, b)
    register int *a, *b;
{
    register int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

static int 
order_cmp (a, b)
    int *a, *b;
{
    if (bx[*a] < bx[*b])
	return (-1);
    if (bx[*a] > bx[*b])
	return (1);
    return (0);
}
