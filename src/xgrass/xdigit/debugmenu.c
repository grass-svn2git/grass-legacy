#include "digit.h"

#define DB_LINE 0
#define DB_AREA 1
#define DB_NODE 2
#define DB_ISLE 3

    static char *name[] = {
	"line",
	"area",
	"node",
	"island",
    };
    static int dpytype=0, findtype=0;
    static int m_line_prev = 0, prev=0, curr= 0;

make_debug_menu (parent)
    Widget parent;
{
    Widget findbutton, elem, elemarea, elemlabel;
    Widget display;
    static Widget dialog[10], button[3];

    int    x, y;
    int    n = 0;
    Arg    wargs[10];
    Pixel  fg, bg;
    x = 20;
    y = 5;


   
   
    n = 0;
    XtSetArg (wargs[n], XtNforeground, &fg); n++;
    XtSetArg (wargs[n], XtNbackground, &bg); n++;
    XtGetValues (parent, wargs, n);
 
    elem = 
    make_button (parent, "", elcb, &findtype, "line", fg, bg, "Change element type");
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (elem, wargs, n); 

    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x+1); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, elem); n++;
    XtSetArg (wargs[n], XmNmarginHeight, 0); n++;
    XtCreateManagedWidget("Type", xmLabelGadgetClass, parent, wargs, n);

    x += 10;
    findbutton = 
    make_button (parent, "find", findcb, &findtype, NULL, fg, bg, 
                                                 "Find element");
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (findbutton, wargs, n); 

    x += 25;
    elem = 
    make_button (parent, "", el2cb, &dpytype, "line", fg, bg, 
                                         "Change element type");
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (elem, wargs, n); 

    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x+1); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, elem); n++;
    XtSetArg (wargs[n], XmNmarginHeight, 0); n++;
    XtCreateManagedWidget("Type", xmLabelGadgetClass, parent, wargs, n);

    x += 10;
    display = 
    make_button (parent, "show\ninfo", infocb, &dpytype, NULL, fg, bg, 
					   "Display information on an element");
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (display, wargs, n); 


}
void
elcb(w, type, call_data)
    Widget w;
    int *type;
    caddr_t call_data;
{
    static int i = 0;
    char   str[50];

    
    *type = i = (i+1)%4;
    sprintf (str, "Change element type to %s", name[i]);
    showtext (w, str, NULL);
    change_pix (w, name[i], w); 

}


void
el2cb(w, type, call_data)
    Widget w;
    int    *type;
    caddr_t call_data;
{
    static int i = 0;
    char   str[50];

    
    *type = i = (i+1)%3;
    sprintf (str, "change element type to %s", name[i]);
    showtext (w, str, NULL);
    change_pix (w, name[i], w); 
}


void
findcb(w, type, call_data)
    Widget w;
    int *type;
    caddr_t call_data;
{
    char   str[50];

    sprintf (str, "Find  %s", name[findtype]);
    showtext (w, str, NULL);
    start_find(w);

}

void
infocb(w, type, call_data)
    Widget w;
    int *type;
    caddr_t call_data;
{
    char   str[50];
    int    elements = 0;

    sprintf (str, "Display information on  %s", name[*type]);
    showtext (w, str, NULL);

    switch (dpytype) {
	case DB_LINE:
	    if (elements = CM->n_lines)
	        d_line_info(CM);
            break;
	case DB_AREA:
	    if (elements = CM->n_areas)
	        d_area_info(CM);
            break;
	case DB_NODE:
	    if (elements = CM->n_nodes)
	        d_node_info(CM);
        default:
            break;
    }
    if (!elements)
    {
       sprintf(str, "There are no %ss", name[*type]);
       make_monolog (1, str);
    }
}

Widget
make_find_dialog (parent, str, button, text, cb, data)
    Widget parent;
    char *str;
    Widget button[3];
    int text;
    void (*cb)();
    int data;
{
    XmString next = XmStringCreateSimple ("next");
    XmString prev = XmStringCreateSimple ("previous");
    XmString cancel = XmStringCreateSimple ("cancel");
    XmString message= XmStringCreateSimple (str);
    Widget dialog, label, rc, frame, npform, sep, top;
    Arg wargs[2];
    int n=0;

/*	    XtSetArg (wargs[n], XtNx, Winx + Wdth/2); n++;
    XtSetArg (wargs[n], XtNy, Winy); n++;
    */
    dialog = XtCreatePopupShell ("Next", transientShellWidgetClass, toplevel,
		      NULL, 0);
   
   frame = XtVaCreateManagedWidget ("frame", xmFrameWidgetClass, dialog, 
		      XmNmarginWidth, 3,
		      XmNmarginHeight, 3,
                      XmNshadowThickness, 3, 
                      XmNshadowType, XmSHADOW_OUT, 
		      NULL);

    npform = XtVaCreateManagedWidget ("npform", xmFormWidgetClass, frame, 
		      NULL);
    label = XtVaCreateManagedWidget ("nplabel", xmLabelWidgetClass, npform,
		      XmNlabelString, message, 
		      XmNtopAttachment,  XmATTACH_FORM,
		      XmNrightAttachment,  XmATTACH_FORM,
		      XmNleftAttachment,  XmATTACH_FORM,
		      NULL);
    top = label;
    if (text)
    top = XtVaCreateManagedWidget ("nptext", xmTextFieldWidgetClass, npform,
		      XmNtopAttachment,  XmATTACH_WIDGET,
		      XmNtopWidget, label,
		      XmNrightAttachment,  XmATTACH_FORM,
		      XmNleftAttachment,  XmATTACH_FORM,
		      NULL);
    sep = XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, npform,
		      XmNtopAttachment,  XmATTACH_WIDGET,
		      XmNtopWidget, top,
		      XmNrightAttachment,  XmATTACH_FORM,
		      XmNleftAttachment,  XmATTACH_FORM,
		      NULL);
    rc = XtVaCreateManagedWidget ("nprc", xmRowColumnWidgetClass, npform,
		      XmNtopAttachment,  XmATTACH_WIDGET,
		      XmNorientation, XmHORIZONTAL,
		      XmNtopWidget, sep,
		      XmNrightAttachment,  XmATTACH_FORM,
		      XmNleftAttachment,  XmATTACH_FORM,
		      XmNpacking, XmPACK_COLUMN,
		      XmNentryAlignment, XmALIGNMENT_CENTER,
		      NULL);
    
    button[0] = XtVaCreateManagedWidget ("next", xmPushButtonWidgetClass, rc,
		      XmNlabelString, next, 
		      NULL);
    button[1] = XtVaCreateManagedWidget ("prev", xmPushButtonWidgetClass, rc,
		      XmNlabelString, prev, 
		      NULL);
    button[2] = XtVaCreateManagedWidget ("cancel", xmPushButtonWidgetClass, rc,
		      XmNlabelString, cancel, 
		      NULL);
    if (text)
    XtAddCallback (top, XmNactivateCallback, cb, data);


    XmStringFree (next);
    XmStringFree (message);
    XmStringFree (prev);
    XmStringFree (cancel);
    return dialog;
}

void
start_find (w)
    Widget w;
{
    char text[100];
    int elements;
    Widget button[3];
    static Widget dialog;

    switch (findtype) {
	case DB_LINE:
	    elements = CM->n_lines;
	    break;
	case DB_AREA:
	    elements = CM->n_areas;
	    break;
	case DB_NODE:
	    elements = CM->n_nodes;
	    break;
	case DB_ISLE:
	    elements = CM->n_isles;
	    break;
	default:
	    break;
	}

    sprintf(text, "Total %ss: %d", name[findtype], elements); 
    write_info (1, text);
    prev = 0;
    if (elements)
    {
	if (dialog == NULL)
        {
	    dialog = make_find_dialog (w, 
	       "Enter number of desired element:", button, 1, find, NUM);
    	    XtAddCallback (button[0], XmNactivateCallback, find, NEXT);
    	    XtAddCallback (button[1], XmNactivateCallback, find, PREV);
            XtAddCallback (button[2], XmNactivateCallback, downcb, dialog);
            XtAddCallback (button[2], XmNactivateCallback, find, DONE);
	}
	    showcb (w, dialog, NULL);
	    find (w, NEXT, NULL);
    }
    else
    {
        sprintf(text, "There are no %ss", name[findtype]);
	make_monolog (1, text);
    }
}
void
find(w, direction, call_data)
    Widget w;
    int direction;
    caddr_t call_data;
{
    int i;
    char *str;
    XmString result;
    
    switch (direction) {
	case PREV:
		i = prev - 1;
	    break;
	case NEXT:
		i = prev + 1;
	    break;
	case NUM:

	    str = XmTextGetString (w);
    	    i = atoi(str);
	    debugf ("NUM = %d\n", i);
	    XtFree (str);
	    break;
	case DONE:
	    i = -1;
	    break;
	default:
	    i = prev;
	    break;
    }
    switch (findtype) {
	case DB_LINE:
            show_lines (i, CM);
	    break;
	case DB_AREA:
            show_areas (i, CM);
	    break;
	case DB_NODE:
            show_nodes (i, CM);
	    break;
	case DB_ISLE:
            show_isles (i, CM);
	    break;
	default:
	    break;
	}
}

void
quit_find (w, client_data, call_data)
    Widget w;
    caddr_t client_data, call_data;
{
    if (prev)
    switch (findtype) {
	case DB_LINE:
            display_line (CM->Line[prev].type, &Gpoints, prev, CM);
            if (Disp_llabels)
            	display_llabel (CM, prev);
	    break;
	case DB_AREA:
            reset_area (prev, CM);
	    break;
	case DB_NODE:
            display_node (prev, CM);
	    break;
	case DB_ISLE:
            reset_isle (prev, CM);
	    break;
	default:
	    break;
    }
}

show_lines (i, map)
    int i;
    struct Map_info *map;
{
    
    if (prev == i)
	return (0);
    if (prev)       /* reset the previously highlit */
    {
        display_line (map->Line[prev].type, &Gpoints, prev, map);
        if (Disp_llabels)
            display_llabel (map, prev);
    }
    if (i == -1)
	return (0);
    if (i < 1) i = map->n_lines;
    if (i > map->n_lines) i = 1;

    prev = i;
    if (LINE_ALIVE (&(map->Line[i])))
    {
	m_line_info (map, i);   /* show line info on lines */
        if(0 > V1_read_line (map, &Gpoints, map->Line[i].offset))
        {
/*ERRORMSG*/            write_info (1, "Error reading line information");
                return (0);
        }

        if (Auto_Window && line_outside_window (&(map->Line[i])))
        {
            P_LINE *Line;

            Line = &(map->Line[i]);
            move_window (Line->N, Line->S, Line->E, Line->W, 1);
        }

        highlight_line (map->Line[i].type, &Gpoints,i, map);
    }
    else
	write_info (1, "Line is dead");
}

m_line_info (map, line)
    struct Map_info *map;
    int line;
{
    char buf[500];
    P_LINE *Line;

    if (m_line_prev)
        if (Disp_llabels)
            display_llabel (map, m_line_prev);
    if (line > 0)
    {
        Line = &(map->Line[line]);
        sprintf(buf, "Total lines %d  #%4d:  N1 %3d N2 %3d  Left %3d Right %3d",
            CM->n_lines, line, Line->N1, Line->N2, Line->left, Line->right);
        write_info (1, buf);
        if (Line->att)
        {
            P_ATT *Att;

            if (Disp_llabels)
            {
                highlight_llabel (map, line);
                m_line_prev = line;
            }
            Att = &(map->Att[Line->att]);
            sprintf (buf, 
	    "  Att %3d (index %3d)   Category %3d  X %10.3lf Y %10.3lf",
                Line->att, Att->index, Att->cat, Att->x, Att->y);
        }
        else
            sprintf (buf, " Line is NOT labeled");
        add_info (1, buf);
    }
}


show_areas (i, map)
    int i;
    struct Map_info *map;
{
    int x;
    char buf[1024], text[2048];
    if (prev == i)
	return (0);
    
    if (prev)
    {
        reset_area (prev, map);
    }
    if (i == -1)
	return (0);
    if (i < 1) i = map->n_areas;
    if (i > map->n_areas) i = 1;
    prev = i;
    if (AREA_ALIVE (&(map->Area[i])))
    {

        if (Auto_Window && area_outside_window (&(map->Area[i])))
        {
            P_AREA *Area;

            Area = &(map->Area[i]);
            move_window (Area->N, Area->S, Area->E, Area->W, 1);
        }

        highlight_area (i, map);

        sprintf (text, "Total areas: %d\nArea %d  Att: %d  N_lines: %d", 
	    CM->n_areas, i, 
	    map->Area[i].att ? map->Att[map->Area[i].att].cat : 1, 
	    map->Area[i].n_lines);
        write_info (1, text);

        strcpy (text, "  ");
        for (x = 0 ; x < map->Area[i].n_lines ; x++)
        {
            sprintf (buf, "%d ", map->Area[i].lines[x]);
            strcat (text, buf);
        }
        add_info (1, text);
    }
    else
	write_info (1, "Area is dead");
}

show_isles (i, map)
    int i;
    struct Map_info *map;
{
    int x;
    char buf[1024], text[2048];
    if (prev == i)
	return (0);
        
    if (prev)
    {
        reset_isle (prev, map);
    }
    if (i == -1)
	return (0);
    if (i < 1) i = map->n_isles;
    if (i > map->n_isles) i = 1;
    prev = i;
    if (ISLE_ALIVE (&(map->Isle[i])))
    {
        highlight_isle (i, map);

        sprintf (text, "Total islands: %d\nIsle %d  Area: %d  N_lines: %d", 
                     CM->n_isles, i, map->Isle[i].area, map->Isle[i].n_lines);
        write_info (1, text);

        strcpy (text, "  ");
        for (x = 0 ; x < map->Isle[i].n_lines ; x++)
        {
            sprintf (buf, "%d ", map->Isle[i].lines[x]);
            strcat (text, buf);
        }
        add_info (1, text);
    }
    else
	write_info (1, "Isle is dead");

}

show_nodes (i, map)
    int i;
    struct Map_info *map;
{
    
    if (prev == i)
	return (0);
    if (prev)       /* reset the previously highlit */
    {
        display_node (prev, map);
    }
    if (i == -1)
	return (0);
    if (i < 1) i = map->n_nodes;
    if (i > map->n_nodes) i = 1;

    prev = i;
    if (NODE_ALIVE (&(map->Node[i])))
    {
	m_node_info (map, i);   /* show node info*/
        highlight_node (i, map);
    }
    else
	write_info (1, "Node is dead");
}
m_node_info (map, node)
    struct Map_info *map;
    int node;
{
    char buf[500];
    char text[500];
    P_NODE *Node;
    register int x;

    if (node > 0)
    {
        Node = &(map->Node[node]);
        sprintf (buf, "#%4d:  Num lines: %d  X: %12.2f  Y: %12.2f",
            node, Node->n_lines, Node->x, Node->y);
        write_info (1, buf);

        strcpy (text, "  ");
      /*  sprintf (text, "HEX: %x%x %x%x  Lines:  ", Node->x, Node->y);*/
        for (x = 0 ; x < map->Node[node].n_lines ; x++)
        {
            sprintf (buf, "Lines: %d ", map->Node[node].lines[x]);
            strcat (text, buf);
        }
        add_info (1, text);
    }
}
d_area_info (map)
    struct Map_info *map;
{
    int area;
    double x, y;
    char buf[100];

    area = 0;
    while (1)
    {
        if (!new_point_with_mouse (&x, &y, "Choose a point in Area:"))
	{
	    break;
	  }  
	if (area)
        {

            if (Disp_labels)
                display_area_label (area, map);
            else
                undisplay_area_label (area, map);
            if (Disp_outline)
                display_area (area, map);
            else
                reset_area (area, map);
        }
        if (x == 0.0 && y == 0.0)
        {
            make_monolog (1, "No Area Found");
            return (0);
        }
        if (area = dig_point_to_area (map, x, y))
        {
            if (Disp_labels)
                highlight_area_label (area, map);
            highlight_area (area, map);
            sprintf (buf, "Area# %d", area);
            write_info (1, buf);
        }
        else
        {
            make_monolog (1, "No Area Found");
        }
    }
}

d_line_info (map)
    struct Map_info *map;
{
    int line;

    line = 0;
    m_line_prev = 0;    /* for m_line_info -  display_llabel() */
    set_accept(0);
    while (1)
    {
        if (line)
        {
            if (Disp_llabels)
                display_llabel (map, line);
            display_line (map->Line[line].type, &Gpoints, line, map);
        }
        line = find_line_with_mouse (-1, "Choose Line:", m_line_info);
        if (line <= 0)
        {
	    set_accept(1);
            return (0);
        }
    }
}

d_node_info (map)
    struct Map_info *map;
{
    double thresh ;
    double fabs() ;
    double ux1, uy1 ;
    double ux2, uy2 ;

    int node;
    double x, y;
    node = 0;

    screen_to_utm (0, 0, &ux1, &uy1) ;
    screen_to_utm (5, 0, &ux2, &uy2) ;
    thresh = fabs ( ux1 - ux2);

    set_accept (0);
    while (1)
    {
        node = find_node_with_mouse (&x, &y, thresh, "Choose Node:");
        if (node <= 0)
        {
            display_node (prev, CM);
    	    set_accept (1);
            return (0);
        }
	    show_nodes (node, CM);
    }
}
void 
start_info (w, client_data, call_data)
    Widget w;
    caddr_t client_data, call_data;
{
    switch (dpytype) {
	case DB_LINE:
	    d_line_info(CM);
            break;
	case DB_AREA:
	    d_area_info(CM);
            break;
	case DB_NODE:
	    d_node_info(CM);
            break;
        default:
            break;
    }
}
