/*
**  Written by Dave Gerdes  5/1988
**  US Army Construction Engineering Research Lab
*/
/*
**  Last modified Winter 1993 
**  Terry Baker US Army Construction Engineering Research Lab
*/

#include "digit.h"
#include "dig_head.h"

static Widget next_prev = NULL;
static int next;
static Widget button[3]; 


double fabs ();

/* ask user to choose a node  and then they can look at each
**  line attached to it, individually.   Great for finding double 
**  digitized lines
*/
node_lines (w, map)
    Widget w;
    struct Map_info *map;
{
    register int Next, prev_line, next_line, node_num;
    register int line_num = 0;	/* to shut up saber */	
    int prev_node;
    int rline_num;
    char buf[1000];
    double ux1, uy1;
    double ux2, uy2;
    double thresh;
    float angle;
    int first;

    screen_to_utm (0, 0, &ux1, &uy1);
    screen_to_utm (5, 0, &ux2, &uy2);
    thresh = fabs ( ux1 - ux2);


    node_num = 0;
    while (1)
    {
	prev_node = node_num;
	node_num = find_node_with_mouse (&ux2, &uy2, thresh, "Select a Node:");
	
	if ( node_num <= 0)
	{
	    if (prev_node)
	    {
	    standard_color(dcolors[dig_node_color(map->Node[prev_node].n_lines)]);
	    Blot (&(map->Node[prev_node].x), &(map->Node[prev_node].y));
	    }
    	    if (next_prev)
                downcb(next_prev, next_prev);
	    return (0);
	}
	if (map->Node[node_num].n_lines == 0)
	{
	    write_info (1, "Node has NO lines attached to it.");
	    continue;
	}
	next_line = map->Node[node_num].n_lines - 1;
	Next = NEXT;
	prev_line = 0;
	first = 1;
        show_next_prev(w, "Select Next Line:");
	do {
	    prev_line = next_line;
	    switch (Next) {
		case PREV:		/* prev */
		    next_line = (next_line == 0 ? 
				map->Node[node_num].n_lines -1 : next_line-1);
		    break;
		case NEXT:		/* next */
		    next_line = (next_line == map->Node[node_num].n_lines -1 ?
				    0 : next_line+1);
		    break;
		default:		/* end */
		    continue;
		    /* shouldn't get here */
		    break;
	    }
	    if (!first)
	    {
		display_line(map->Line[line_num].type, &Gpoints, line_num, map);
	    }
	    first = 0;

	    rline_num = map->Node[node_num].lines[next_line];
	    line_num = abs (rline_num);
	    if (0 > V1_read_line (map, &Gpoints, map->Line[line_num].offset))
		continue;
	    highlight_line (map->Line[line_num].type, &Gpoints, line_num, map);

    /* calculate the angle on the fly for verification */
    if (rline_num < 0)
    {
	angle = dig_calc_end_angle (&Gpoints, map->head.map_thresh);
    }
    else
    {
	angle = dig_calc_begin_angle (&Gpoints, map->head.map_thresh);
    }


	    sprintf(buf, "Node %d  Line %d   Type '%s' Angle  %7.5f (%7.5f)\n",                 node_num, rline_num, 
		tell_type(map->Line[line_num].type),
	        map->Node[node_num].angles[next_line], angle);
	    write_info (1, buf);

    } while (DONE != (Next = ask_next_prev ()));
    if (next_prev)
        XtUnmanageChild (next_prev);

	display_line(map->Line[line_num].type, &Gpoints, line_num, map);
    }
}

char *
tell_type (type)
    char type;
{
    char *p;

    switch (type) {
	
	case LINE:
	    p = "Line";
	    break;
	case AREA:
	    p = "Area Border";
	    break;
	case DOT:
	    p = "Site Marker";
	    break;
	case DEAD_LINE:
	    p = "Deleted Line";
	    break;
	case DEAD_AREA:
	    p = "Deleted Area Border";
	    break;
	case DEAD_DOT:
	    p = "Deleted Site Marker";
	    break;
    }
    return (p);
}


next_prevcb(w, i)
    Widget w;
    int i;
{
  next = i; 
}

show_next_prev (w, msg)
    Widget w;
    char *msg;
{
    static int first = 1;

    if (first)
    {
	first = 0;
        next_prev = make_find_dialog (w, msg, button, 0, NULL, NULL);
    }
    
    XtAddCallback (button[0], XmNactivateCallback, next_prevcb, NEXT);
    XtAddCallback (button[1], XmNactivateCallback, next_prevcb, PREV);
    XtAddCallback (button[2], XmNactivateCallback, next_prevcb, DONE);

    XtManageChild (next_prev);
}
ask_next_prev()
{
    extern Widget toplevel;
    int i;
    XEvent event;
	
    XFlush (dpy);

    next  = 0;
    XmUpdateDisplay (toplevel);
    while (XCheckMaskEvent (dpy,
        ButtonPressMask | ButtonReleaseMask |
        KeyPressMask | KeyReleaseMask, &event))
    {
	for (i = 0; i < 3; i++)
            if (event.xany.window == XtWindow(button[i]))
            {
		XtDispatchEvent (&event);
		break;
	    }
        if (i == 3) 
            XBell (dpy,50);
    }
    return (next);
}

