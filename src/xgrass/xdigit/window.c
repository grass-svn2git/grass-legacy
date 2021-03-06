/*
**  Written by Dave Gerdes  4/1988
**  US Army Construction Engineering Research Lab
*/
#include "digit.h"
#include "wind.h"
#include "gis.h"

/*
**  Window menu 
**
**  driver routine for Window menu
**
**  also includes a mess of routines for displaying various things on 
**  video monitor.  This needs to be organized.
**
**  Written by Dave Gerdes 4 1988
*/
int
win_men (w, command)
    Widget w;
    int command;		/* command user enters */
{
    int ret;			/* get return values from subrs */

    if (w)
    {
	add_to_display_list (command);
    } 
    ret = 0;

    Save_Disp_settings ();
    {
	    switch(command) {
		case MWC_CENT:
		    Zero_Disp_settings();
		    Disp_markers = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_LINES:
		    Zero_Disp_settings();
		    Disp_lines = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_SITES:
		    Zero_Disp_settings();
		    Disp_sites = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_NODES:
		    Zero_Disp_settings();
		    Disp_nodes = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_THRESH:
		    if (0 >=get_thresh (w))
			remove_from_list (MWC_THRESH);
		    break;
		case MWC_ISLES:
		    if (0 >= (ret = display_islands (CM)))
			remove_from_list (MWC_ISLES);
		    break;
		case MWC_LABELS:
		    Zero_Disp_settings();
		    Disp_labels = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_LLABELS:
		    Zero_Disp_settings();
		    Disp_llabels = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_SLABELS:
		    Zero_Disp_settings();
		    Disp_slabels = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_LLINES:
		    Zero_Disp_settings();
		    Disp_llines = 1;
		    replot (CM);
		    Restore_Disp_settings ();
		    break;
		case MWC_CLEAR:
		    standard_color (dcolors[CLR_ERASE]);
		    erase_window();
		    outline_window();
		    break;
		case MWC_WHERE:
		    where_am_i (CM);
		    break;
		case MWC_SCALE:
		    add_scale ();
		    break;
		case MWC_BACKDROP:
		    if (strcmp(N_backdrop, "None"))
		        display_backdrop ();
		    else
			if (w)
			    make_rbrowse (w);
		    break;
		case MWC_OVERLAY:
		    if (strcmp(N_overlay, "None"))
		    {
		        Zero_Disp_settings();
		        Disp_overlay = 1;
		        replot (CM);
		        Restore_Disp_settings ();
		    }
		    else
		    {
			if (w)
			    make_vbrowse (w);
		    }
		    break;
		case MWC_ULAREAS:
		    if (0 >= (ret = display_unlabeled_areas (CM)))
			remove_from_list ( MWC_ULAREAS);
		    break;
               case MTC_BADAREAS:
                    if (0 >= (ret = unfinished_areas (CM)))
			remove_from_list ( MTC_BADAREAS);
                    break;
               case MTC_DUPLICATE:
                    if (0 >= (ret = display_duplicate_lines (CM)))
			remove_from_list ( MTC_DUPLICATE);
                    break;
		default:
		    break;
	    }
	}
    Restore_Disp_settings ();
}

display_cents (map)
    struct Map_info *map;
{
    register int i;
    int ret = 0;
    char buf[100];
    
    standard_color (dcolors[CLR_AMARK]);
    for (i = 1 ; i <= map->n_areas ; i++)
    {
	if (AREA_LABELED (&(map->Area[i])))
	{
	    _Blot (&(map->Att[map->Area[i].att].x), &(map->Att[map->Area[i].att].y)); 
	}
    }
    return (ret);
}

display_alabels (map)
    struct Map_info *map;
{
    register int i;
    register P_AREA *Area;
    int ret = 0;
    char buf[100];

    standard_color (dcolors[CLR_ALABEL]);

    for (i = 1 ; i <= map->n_areas ; i++)
    {
        if (Check_for_interrupt())
           break;

	Area = &(map->Area[i]);
	if (AREA_LABELED (Area))
	{
	    sprintf (buf, "%d", map->Att[Area->att].cat);
	    Adot (&(map->Att[Area->att].x), &(map->Att[Area->att].y), buf); 
	}
    }

    return (ret);
}

highlight_llabel (map, i)
    struct Map_info *map;
{
    register P_LINE *Line;
    char buf[100];

    standard_color (dcolors[CLR_HIGHLIGHT]);
	Line = &(map->Line[i]);
	if (LINE_ALIVE (Line) && Line->att && map->Att[Line->att].cat)
	{
	    sprintf (buf, "%d", map->Att[Line->att].cat);
	    Adot (&(map->Att[Line->att].x), &(map->Att[Line->att].y), buf); 
	}
}


display_llabel (map, i)
    struct Map_info *map;
{
    register P_LINE *Line;
    char buf[100];

    standard_color (dcolors[CLR_LLABEL]);
	Line = &(map->Line[i]);
	if (LINE_ALIVE (Line) && Line->att && map->Att[Line->att].cat)
	{
	    sprintf (buf, "%d", map->Att[Line->att].cat);
	    Adot (&(map->Att[Line->att].x), &(map->Att[Line->att].y), buf); 
	}
}

display_llabels (map)
    struct Map_info *map;
{
    register int i;
    register P_LINE *Line;
    char buf[100];

    standard_color (dcolors[CLR_LLABEL]);
    for (i = 1 ; i <= map->n_lines ; i++)
    {
	Line = &(map->Line[i]);
	if (LINE_ALIVE (Line) && Line->att && map->Att[Line->att].cat)
	{
	    sprintf (buf, "%d", map->Att[Line->att].cat);
	    Adot (&(map->Att[Line->att].x), &(map->Att[Line->att].y), buf); 
	}
    }
}

/* highlight all lines of category */
display_llines (map, cat)
    struct Map_info *map;
    int cat;
{
    register int i;
    char input[1024];

    if (cat <= 0)
	return (-1);
    for (i = 1 ; i <= map->n_lines ; i++)
    {
	if (LINE_ALIVE (&(map->Line[i])) && map->Line[i].att && 
		map->Att[map->Line[i].att].cat == cat)
	{
	    if (0 > V1_read_line (map, &Gpoints, map->Line[i].offset))
		return (-1);
	    _highlight_line (map->Line[i].type, &Gpoints, i, map);
	}
    }

    return 0;
}

/* highlight all areas of category */
display_lareas (map, cat)
    struct Map_info *map;
    int cat;
{
    register int i;
    char input[1024];
    P_AREA *Area;

    if (cat <= 0)
	return (-1);

    for (i = 1 ; i <= map->n_areas ; i++)
    {
	Area = &(map->Area[i]);
	if (AREA_LABELED (Area) && map->Att[Area->att].cat == cat)
	{
	    _highlight_area (Area, map);
	}
    }

    return 0;
}

display_unlabeled_areas (map)
    struct Map_info *map;
{
    register int i;
    P_AREA *Area;
    char buf[100];
    int ret;

    ret = 0;

    for (i = 1 ; i <= map->n_areas ; i++)
    {
        if (Check_for_interrupt())
           return (ret);
 
	Area = &(map->Area[i]);
	if (AREA_ALIVE (Area) && !AREA_LABELED (Area))
	{
	    _highlight_area (Area, map);
	    ret++;
	}
    }
    return (ret);
}


display_islands (map)
    struct Map_info *map;
{
    register int i;
    P_LINE *Line;
    char buf[60];
    int ret = 0;
    for (i = 1 ; i <= map->n_lines ; i++)
    {
        if (Check_for_interrupt())
           return (ret);
 
	Line = &(map->Line[i]);
	if (!LINE_ALIVE (Line))
	    continue;
	if (Line->right < 0 || Line->left < 0)
	{
	    if (0 > V1_read_line (map, &Gpoints, Line->offset))
		break;
	    _highlight_line (Line->type, &Gpoints, i, map);
	    ret++;
	}
    }
    return (ret);
}

/* expand current window to include new boundries */
/* window only expands, does not shrink */
expand_window (N, S, E, W, plus)
    double N, S, E, W;
    int plus;
{
    double diff;

    standard_color (dcolors[CLR_ERASE]);
    /*
    erase_window();
    */

    if (N < U_north)
	N = U_north;
    if (S > U_south)
	S = U_south;
    if (E < U_east)
	E = U_east;
    if (W > U_west)
	W = U_west;
    /*
    window_conversions (N, S, E, W);
    */
    if (plus)
    {
	diff = (N - S) * .05;
	N += diff;
	S -= diff;
	diff = (E - W) * .05;
	E += diff;
	W -= diff;
    }
    window_rout (N, S, E, W);
    outline_window();

    redraw();
    /*
    replot(CM);
    */
}

/* change current window to new boundries */
move_window (N, S, E, W, plus)
    double N, S, E, W;
    int plus;
{
    double diff;

/*
    standard_color (dcolors[CLR_ERASE]);
    erase_window();
    */

    /*
    if (N < U_north)
	N = U_north;
    if (S > U_south)
	S = U_south;
    if (E < U_east)
	E = U_east;
    if (W > U_west)
	W = U_west;
    */
    /*
    window_conversions (N, S, E, W);
    */
    if (plus)
    {
	diff = (N - S) * .05;
	N += diff;
	S -= diff;
	diff = (E - W) * .05;
	E += diff;
	W -= diff;
    }
    window_rout (N, S, E, W);
    outline_window();

    redraw();
    /*
    replot(CM);
    */
}

/* 
** area_outside_window    return 1 if  any part of Area extends outside the
** Screen window.  0  if it is completely within 
*/
area_outside_window (Area)
    P_AREA *Area;
{
/*DEBUG fprintf (stderr, "Checking (%lf,%lf,%lf,%lf) - \n	(%lf,%lf,%lf,%lf), Area->N, Area->S, Area->E, Area->W, U_north, U_south, U_east, U_west); */

    if (Area->N > U_north)
	return (1);
    if (Area->S < U_south)
	return (1);
    if (Area->E > U_east)
	return (1);
    if (Area->W < U_west)
	return (1);
    return (0);
}

line_outside_window (Line)
    P_LINE *Line;
{
    if (Line->N > U_north)
	return (1);
    if (Line->S < U_south)
	return (1);
    if (Line->E > U_east)
	return (1);
    if (Line->W < U_west)
	return (1);
    return (0);
}

clear_window ()
{
    erase_window ();
    outline_window ();
}


static char S_Disp_overlay;
static char S_Disp_backdrop;
static char S_Disp_lines;
static char S_Disp_points;
static char S_Disp_nodes;
static char S_Disp_labels;
static char S_Disp_outline;
static char S_Disp_markers;
static char S_Disp_llines;
static char S_Disp_llabels;
static char S_Disp_sites;
static char S_Disp_slabels;

Save_Disp_settings ()
{
    S_Disp_overlay = Disp_overlay;
    S_Disp_backdrop = Disp_backdrop;
    S_Disp_lines = Disp_lines;
    S_Disp_points = Disp_points;
    S_Disp_nodes = Disp_nodes;
    S_Disp_labels = Disp_labels;
    S_Disp_outline = Disp_outline;
    S_Disp_markers = Disp_markers;
    S_Disp_llines = Disp_llines;
    S_Disp_llabels = Disp_llabels;
    S_Disp_sites = Disp_sites;
    S_Disp_slabels = Disp_slabels;
}

Restore_Disp_settings ()
{
    Disp_overlay = S_Disp_overlay;
    Disp_backdrop = S_Disp_backdrop;
    Disp_lines = S_Disp_lines;
    Disp_points = S_Disp_points;
    Disp_nodes = S_Disp_nodes;
    Disp_labels = S_Disp_labels;
    Disp_outline = S_Disp_outline;
    Disp_markers = S_Disp_markers;
    Disp_llines = S_Disp_llines;
    Disp_llabels = S_Disp_llabels;
    Disp_sites = S_Disp_sites;
    Disp_slabels = S_Disp_slabels;
}

Zero_Disp_settings ()
{
    Disp_overlay = 0;
    Disp_backdrop = 0;
    Disp_lines = 0;
    Disp_points = 0;
    Disp_nodes = 0;
    Disp_labels = 0;
    Disp_outline = 0;
    Disp_markers = 0;
    Disp_llines = 0;
    Disp_llabels = 0;
    Disp_sites = 0;
    Disp_slabels = 0;
}



#ifdef FOO
slid_window ()
{
    slid_window_w_mouse ();
}

scal_window ()
{
    scal_window_w_mouse ();
}
#endif
