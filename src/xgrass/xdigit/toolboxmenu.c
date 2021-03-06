/*
** Written Fall 1992 by Terry Baker
** US Army Construction Engineering Research Lab
*/
#include "gis.h"
#include "digit.h"
#include "dig_head.h"

extern Widget reg, neat;
Widget threshpop;

make_toolbox_menu (parent)
    Widget parent;
{
    Widget unlabarea, openarea, dup;
    Widget nodeline, islands, select, lines;
    Widget write, nodes, thresh;
    static Widget dialog[5];
    int    x, y;
    int    n = 0;
    Arg    wargs[10];
    Pixel  fg, bg;
    Pixmap pix;
    XmString next, prev;
    int i;

    i = 0;
    x = 15;
    y = 5;
   
    n = 0;
    XtSetArg (wargs[n], XtNforeground, &fg); n++;
    XtSetArg (wargs[n], XtNbackground, &bg); n++;
    XtGetValues (parent, wargs, n);

    unlabarea= 
    make_button (parent, "", NULL, NULL,  "unlabarea", fg, bg,
						  "Display unlabeled areas");
    XtAddCallback (unlabarea, XmNactivateCallback, win_men, MTC_ULAREAS);
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (unlabarea, wargs, n);

   
    x += 8;
 
    openarea = 
    make_button (parent, "", NULL, NULL, "openarea", fg, bg,
						  "Display open area lines");
    XtAddCallback (openarea, XmNactivateCallback, win_men, MTC_BADAREAS);
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (openarea,  wargs, n);
   
    x += 8;
 
    dup = 
    make_button (parent, "", NULL, NULL, "dup", fg, bg,
						  "Display duplicate lines");
    XtAddCallback (dup, XmNactivateCallback, win_men, MTC_DUPLICATE);
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (dup, wargs, n);
   

    x += 8;

    nodeline = 
    make_button (parent, "", NULL, NULL, "nodeline", fg, bg,
						  "Display node lines");
    XtAddCallback (nodeline, XmNactivateCallback, tbox, MTC_NODELINES);
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (nodeline, wargs, n);
   
    x += 8; 
    nodes = 
    make_button (parent, "", NULL, "", "nodeinthresh", fg, bg,
						  "Display nodes in threshold");
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (nodes, wargs, n);

    XtAddCallback (nodes, XmNactivateCallback, showcb, threshpop);

    x += 8; 
    islands = 
    make_button (parent, "", NULL, NULL, "island", fg, bg, "Display islands");
    XtAddCallback (islands, XmNactivateCallback, win_men, MTC_ISLES);
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNspacing, 1); n++;
    XtSetValues (islands, wargs, n);

    x += 20;
   
    reg =
       
    make_button (parent, "", NULL, NULL, "reg", fg, bg, "Register map");
    XtAddCallback (reg, XmNactivateCallback, tbox, MTC_REGIST);
    add_inverse_map (reg,  "reg", fg, bg);


    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (reg, wargs, n); 
   
    x += 8;
    neat =
    make_button (parent, "", NULL, NULL, "neat", fg, bg,
						   "Build neat line");
    add_inverse_map (neat,  "neat", fg, bg);
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetValues (neat, wargs, n); 
    XtAddCallback (neat, XmNactivateCallback, tbox, MTC_NEAT);

    dialog[i] = NULL;
    add_dlog_list (dialog);
}

Widget
make_thresh_popup (parent)
    Widget parent;
{
    XmString t = XmStringCreateSimple ("Enter temporary threshold:");
    XmString title = XmStringCreateSimple ("nodes in threshold");
    XmString accept = XmStringCreateSimple ("accept");
    Arg wargs[5];
    int n=0;

    threshpop = XmCreatePromptDialog (toplevel, "threshpop", NULL, 0);
    XtVaSetValues(threshpop, XmNselectionLabelString,t,
    	    XmNokLabelString, accept,
    		XmNdialogTitle, title, 
		 NULL);
    XtVaSetValues (XtParent(threshpop), 
		XmNsaveUnder, True,
		NULL);

    XmStringFree (t);
    XmStringFree (title);
    XmStringFree (accept);

    XtAddCallback (threshpop, XmNokCallback, win_men, MWC_THRESH);

    XtUnmanageChild(XmSelectionBoxGetChild(threshpop, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(threshpop);

    return threshpop;
}

void
tbox (w, i)
    Widget w;
    int i;
{
    switch (i) {
       case MTC_NODELINES:
	    node_lines (w, CM); 
            break;
       case MTC_NEAT:
            build_neat (CM);
            break;
       case MTC_REGIST:
            {
                if (reset_map(w, CM, CM->coor_file) >= 0)
		{
		    dig_on_off();
                    set_window();
                    standard_color( dcolors[CLR_ERASE]);
                    redraw();
		}
           }
            break;
       default:
            break;
    }
}
