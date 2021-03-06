/*
**  Written by Terry Baker Winter 1993
**  US Army Construction Engineering Research Lab
*/

#include <stdio.h>
#include "digit.h"
#include "xgrass_lib.h"
#include "Browser.h"
#include "Interact.h"
#include "Help.h"
#include "buttonbm.h"
#include "gis.h"


static Widget info, menu, mapinfo;
static Widget new, ope;
Widget dignamelabel, menubar;

String back[] = {
    "XDig*background:            		cadetblue",
    "XDig*fontList: -adobe-helvetica-medium-r-normal--14-*-75-75-*-*-iso8859-*",
    NULL
    };
start_panel (c, v)
    int    c;
    char   *v[];
{
    XtAppContext app;
    Widget top;

    if (c >= 2)
        if (!strcmp(v[1],"text"))
	    Text = 1;
    toplevel = top = 
	XtVaAppInitialize(&app, "XDig", NULL, 0, &c, v, back, NULL, 0);
    make_display (top);
    
    

    XtRealizeWidget (top);

    XtAppMainLoop (app); 
}

void
make_display (parent)
    Widget parent;
{
    int        i, n;
    Arg        wargs[20];
    Widget     form, winmenu, custmenu, digmenu;
    Widget     frame, frame2, sep, sep1, sep2, sep3;
    Widget     debugmenu, toolmenu, editmenu, labelmenu;
    Widget     tmp, save, saveas,  quit, namelabel;
    Widget     lineinfo, line2, newdig, saveaspop;
    Widget     text, overview, context, helpmenu;
    Widget     optmenu, optbutton, filemenu, filebutton, helpbutton;
    int        xy[2]; 
    Pixel      fg, bg;
    Display    *dpy = XtDisplay (parent);
    int        scr = DefaultScreen (dpy);
    Colormap   cmap;
    XVisualInfo     *vis;
    XgHelpStruct Help;
    int        fp, lp, lw;
    int w, h, x, y;


    fp = 0;
    lp = 1;
    lw = 1;

  /*  saveaspop =  make_saveas_pop (parent); */
    
    h = DisplayHeight(dpy, scr);
    x = h*.05;
    h *= .9;
    w = h * 1.2;
    y = (DisplayWidth(dpy, scr) - w) *.5;
    
    Winx = x; 
    Winy = y;

    Wdth = w;
    Hght = h;

    n = 0;
    XtSetArg(wargs[n],XmNheight, h); n++;
    XtSetArg(wargs[n],XmNwidth, w); n++;
    XtSetArg(wargs[n],XmNx, x); n++;
    XtSetArg(wargs[n],XmNy, y); n++;

    Mainform = form = 
    XtCreateManagedWidget("form", xmFormWidgetClass, parent , wargs, n);
    n = 0;
    XtSetArg(wargs[n],XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNrowColumnType, XmMENU_BAR); n++;
    menubar = XtCreateManagedWidget ("menubar", 
				   xmRowColumnWidgetClass, form, wargs, n);
    filebutton = XtVaCreateManagedWidget("File",
			  xmCascadeButtonGadgetClass, menubar, NULL);
    filemenu = XmCreatePulldownMenu (menubar, "filemenu", NULL, 0);

    new = XtCreateManagedWidget ("new", xmPushButtonWidgetClass, 
			filemenu, NULL, 0);
    XtAddCallback (new, XmNactivateCallback, new_map, NULL);

    make_coor_popup (parent);
    make_reg_popup (parent);
    make_dig_popup (parent);
    make_check_popup (parent);
    ope = XtCreateManagedWidget ("open", xmPushButtonWidgetClass, 
			filemenu, NULL, 0);
    XtAddCallback (ope, XmNactivateCallback, make_file_browser, NULL);


    save = XtCreateManagedWidget ("save", xmPushButtonWidgetClass, 
			filemenu, NULL, 0);
    XtAddCallback (save, XmNactivateCallback, savefile, NULL);
/*    saveas = XtCreateManagedWidget ("save as", xmPushButtonWidgetClass, 
			filemenu, NULL, 0);
    XtAddCallback (saveas, XmNactivateCallback, showinfo, saveaspop);*/
    quit = XtCreateManagedWidget ("quit", xmPushButtonWidgetClass, 
			filemenu, NULL, 0);
    XtVaSetValues (filebutton, XmNsubMenuId, filemenu, NULL);
    XtAddCallback (quit, XmNactivateCallback, Quit, NULL);

    optbutton = XtVaCreateManagedWidget("Options",
			  xmCascadeButtonGadgetClass, menubar, NULL);
    optmenu = XmCreatePulldownMenu (menubar, "optmenu", NULL, 0);
    mapinfo = XtCreateManagedWidget ("mapinfo", xmPushButtonWidgetClass, 
			optmenu, NULL, 0);
    XtSetSensitive (mapinfo, False);
    XtAddCallback (mapinfo, XmNactivateCallback, showintro, NULL); 

    newdig = XtCreateManagedWidget ("digitizer", xmPushButtonWidgetClass, 
			optmenu, NULL, 0);
    XtVaSetValues (optbutton, XmNsubMenuId, optmenu, NULL);
    Help.widget = form;
    Help.text = XtNewString("$");

    /*helpbutton = XtVaCreateManagedWidget("Help",
			  xmCascadeButtonGadgetClass, menubar, NULL);
    helpmenu = XmCreatePulldownMenu (menubar, "helpmenu", NULL, 0);
    XtVaSetValues (helpbutton, XmNsubMenuId, helpmenu, NULL);
    overview = XtCreateManagedWidget ("overview", xmPushButtonWidgetClass, 
			helpmenu, NULL, 0);
    context = XtCreateManagedWidget ("on context", xmPushButtonWidgetClass, 
			helpmenu, NULL, 0);
    XtAddCallback(context, XmNactivateCallback,
			_XgInteractorHelp, (XtPointer)(&Help));
			 */

    n = 0;
    XtSetArg(wargs[n],XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n], XmNtopWidget, menubar); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_FORM); n++;
    sep = XtCreateManagedWidget ("sep", xmSeparatorGadgetClass,
                                                      form, wargs, n);
    n = 0;
    XtSetArg(wargs[n],XtNy, 94); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg (wargs[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg (wargs[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg (wargs[n], XmNnumColumns, 1); n++;
    menu = XtCreateManagedWidget ("menu", xmRowColumnWidgetClass,
                                                      form, wargs, n);
    if (!Text)						      
        make_pixmaps (menu);
    n = 0;
    XtSetArg (wargs[n], XtNforeground, &fg); n++;
    XtSetArg (wargs[n], XtNbackground, &bg); n++;
    XtGetValues (menu, wargs, n);


    info = make_info_area2 (form, menu, fg, bg);
    make_menus_active(NULL, False);
    make_1st_monolog(parent, "");
    make_yes_no(parent, "xxx");



    n = 0;
    XtSetArg(wargs[n],XmNorientation, XmVERTICAL); n++;
    XtSetArg(wargs[n],XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n], XmNtopWidget, sep); n++;
    XtSetArg(wargs[n],XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_WIDGET); n++;
    if (Text)
        XtSetArg(wargs[n], XmNleftWidget, menu); 
    else
        XtSetArg(wargs[n], XmNleftWidget, info);
    n++; 
    sep1 = XtCreateManagedWidget ("sep1", xmSeparatorGadgetClass,
                                                      form, wargs, n);
    n = 0;
    XtSetArg(wargs[n],XmNorientation, XmVERTICAL); n++;
    XtSetArg(wargs[n],XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n], XmNtopWidget, sep); n++;
    XtSetArg(wargs[n],XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_FORM); n++;
    sep2 = XtCreateManagedWidget ("sep2", xmSeparatorGadgetClass,
                                                      form, wargs, n);
    n = 0;
    XtSetArg(wargs[n],XtNy, 115); n++;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNleftWidget, sep1); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNrightWidget, sep2); n++;
    sep3 = XtCreateManagedWidget ("sep3", xmSeparatorGadgetClass,
                                                      form, wargs, n);
   
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNleftWidget, sep1); n++;
    XtSetArg (wargs[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNrightPosition, 45); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (wargs[n], XmNeditable, FALSE); n++;
    XtSetArg (wargs[n], XmNcursorPositionVisible, FALSE); n++;
    XtSetArg (wargs[n], XmNcolumns, 50); n++;
    
    text = XmCreateTextField (form, "text", wargs, n);
    XtManageChild (text);
    
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNleftWidget, sep1); n++;
    XtSetArg (wargs[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNrightPosition, 85); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNbottomWidget, text); n++;
    XtSetArg (wargs[n], XmNeditable, FALSE); n++;
    XtSetArg (wargs[n], XmNcursorPositionVisible, FALSE); n++;
    XtSetArg (wargs[n], XmNrows, 2); n++;
    XtSetArg (wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg (wargs[n], XmNshadowThickness, 0); n++;
    
    lineinfo = XmCreateText (form, "lineinfo", wargs, n);
    XtManageChild (lineinfo);

    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNleftWidget, lineinfo); n++;
    XtSetArg (wargs[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNrightWidget, sep2); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNbottomWidget, text); n++;
    XtSetArg (wargs[n], XmNeditable, FALSE); n++;
    XtSetArg (wargs[n], XmNcursorPositionVisible, FALSE); n++;
    XtSetArg (wargs[n], XmNrows, 2); n++;
    XtSetArg (wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg (wargs[n], XmNshadowThickness, 0); n++;
    
    line2 = XmCreateText (form, "lineinfo", wargs, n);
    XtManageChild (line2);

    n = 0;
    XtSetArg(wargs[n],XmNbackground,BlackPixel(dpy, scr)); n++;
    XtSetArg(wargs[n],XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNtopWidget, sep3); n++;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNleftWidget, sep1); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNrightWidget, sep2); n++;
    XtSetArg(wargs[n],XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n],XmNbottomWidget, lineinfo); n++;
    canvas = XtCreateManagedWidget ("drawing", xmDrawingAreaWidgetClass,
                                                      form, wargs, n);
    set_canvas (canvas);
    XtAddCallback (canvas, XmNexposeCallback,expose, NULL);
    XtAddCallback (canvas, XmNresizeCallback, resize, NULL);

    /*make_next_prev (parent); */
    make_thresh_popup(parent);


    n = 0;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n], XmNleftWidget, text); n++;
    XtSetArg(wargs[n],XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(wargs[n],XmNrightPosition, 75); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    frame = XtCreateManagedWidget ("frame", xmFrameWidgetClass,
                                                      form, wargs, n);
    n = 0;
    XtSetArg(wargs[n],XmNalignment, XmALIGNMENT_BEGINNING); n++;
    XtSetArg(wargs[n],XmNmarginTop, 7); n++;
    namelabel = XtCreateManagedWidget("  Map name:    ", xmLabelWidgetClass, 
					 frame, wargs, n);
    make_info_pop (parent, namelabel);

    n = 0;
    XtSetArg(wargs[n],XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(wargs[n], XmNleftWidget, frame); n++;
    XtSetArg (wargs[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNrightWidget, sep2); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    frame2 = XtCreateManagedWidget ("frame2", xmFrameWidgetClass,
                                                      form, wargs, n);
    n = 0;
    XtSetArg(wargs[n],XmNalignment, XmALIGNMENT_BEGINNING); n++;
    XtSetArg(wargs[n],XmNmarginTop, 7); n++;
    dignamelabel = XtCreateManagedWidget("Digitizer: none  ", 
	       xmLabelWidgetClass, frame2, wargs, n);
    XtAddCallback (newdig, XmNactivateCallback, make_dig_select, dignamelabel);

    init_dlogs();
    set_text_widget (text);
    set_info_widget (lineinfo);
    set_info2 (line2);
    custmenu = make_menu (form, sep, sep3, sep1, sep2, 
					     "Customize Menu", "custmenu");
    make_custom_menu (custmenu, parent); 
    winmenu = make_menu (form, sep, sep3, sep1, sep2, 
					      "Window Menu", "winmenu");
    make_win_menu (winmenu);
    digmenu = make_menu (form, sep, sep3, sep1, sep2, 
					     "Digitizing Menu", "digmenu");
    make_digit_menu (digmenu);
    editmenu = make_menu (form, sep, sep3, sep1, sep2, 
					      "Edit Menu", "editmenu");
    make_edit_menu (editmenu); 
    labelmenu = make_menu (form, sep, sep3, sep1, sep2, 
					      "Label Menu", "labelmenu");
    make_label_menu (labelmenu); 
    toolmenu = make_menu (form, sep, sep3, sep1, sep2, 
					      "Toolbox Menu", "toolmenu");
    make_toolbox_menu (toolmenu);
    debugmenu = make_menu (form, sep, sep3, sep1, sep2, 
					      "Debug Menu", "debugmenu");
    make_debug_menu (debugmenu);
     

    tmp = 
    make_button (menu, "digitize", boardchange, digmenu, "digitizer", fg, bg,
					      "Display digitizing menu");
    XtAddCallback (tmp, XmNactivateCallback, write_type_info, NULL);

    add_inverse_map (tmp, "digitizer", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "edit", boardchange, editmenu, "edit", fg, bg,
					      "Display editing menu");
    add_inverse_map (tmp, "edit", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "label", boardchange, labelmenu, "label", fg, bg,
					      "Display labeling menu");
    add_inverse_map (tmp, "label", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "window", boardchange, winmenu, "win", fg, bg,
					      "Display window menu");
    add_inverse_map (tmp, "win", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "custom", boardchange, custmenu, "options", fg, bg,
					      "Display customize menu");
    add_inverse_map (tmp, "options", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "toolbox", boardchange, toolmenu, "tool", fg, bg,
					      "Display toolbox menu");
    add_inverse_map (tmp, "tool", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    tmp = 
    make_button (menu, "debug", boardchange, debugmenu, "debug", fg, bg,
					      "Display debugging menu");
    add_inverse_map (tmp, "debug", fg, bg);
    XtAddCallback (tmp, XmNactivateCallback, buttonchange, tmp);
    XtAddCallback (tmp, XmNactivateCallback, XmProcessTraversal,
					   XmTRAVERSE_NEXT_TAB_GROUP); 
    
    dig_on_off();
    showtext (tmp, " ");
   
}


Widget
make_button (parent, label,  cb, data, map, fore, back, str)
    Widget parent;
    char   *label;
    void   (*cb)();
    void    *data;
    char    *map;
    Pixel   fore, back;
    char    *str;
    
{
    Widget w;
    Pixmap  pix1, pix2;
    Arg     wargs[10];
    int     n = 0;

    if (map != NULL && !Text)
    {
	pix1 = XmGetPixmap (XtScreen (parent), map, fore, back);
	if (pix1 != XmUNSPECIFIED_PIXMAP)
	{
            XtSetArg (wargs[n], XmNlabelType, XmPIXMAP); n++;
            XtSetArg (wargs[n], XmNlabelPixmap, pix1); n++;
            XtSetArg (wargs[n], XmNlabelInsensitivePixmap, pix1); n++;
	}
    }

    XtSetArg (wargs[n], XmNshowAsDefault, False);  n++;

    w  = XtCreateManagedWidget (label, xmPushButtonWidgetClass,
                                                      parent, wargs, n);
    if (cb != NULL)
        XtAddCallback(w, XmNactivateCallback, cb, data);
    if (str != NULL)
	XtAddCallback(w, XmNarmCallback, showtext, str);

    return (w);
}

add_inverse_map (w, map, fore, back)
    Widget w;
    char *map;
    Pixel   fore, back;
{
    Pixmap  pix;

    if (Text)
	return (0);

    XtVaGetValues (w, XmNbottomShadowColor, &fore, NULL);

    pix = XmGetPixmap (XtScreen (w), map, back, fore);
    XtVaSetValues (w, XmNlabelInsensitivePixmap, pix, NULL); 
}

void
boardchange (w, board, call_data)
    Widget w;
    Widget board;
    caddr_t  call_data;
{
    static Widget old = NULL;
    int i;

    if (old != board)
    {
	if (old != NULL)
            XtUnmanageChild (old);
        clear_dlogs();
        XtManageChild (board);
        old = board;
    }
    
}

void
showinfo (w, w2, call_data)
    Widget w;
    Widget w2;
    caddr_t  call_data;
{
    if (!XtIsManaged (w2))
       XtManageChild ( w2);
}

void
buttonchange (w, button, call_data)
    Widget w;
    Widget button;
    caddr_t  call_data;
{
    static Widget old;
    int i;

    if (old != button)
    {
	if (old != NULL)
            XtSetSensitive (old, True);
        XtSetSensitive (button, False);
        old = button;
    }
    
}


void 
savefile()
{
    if (Changes_Made)
    {
        if (write_out(1) <  0)
	    make_monolog (NULL, "Could not save changes");
        else 
            Changes_Made = 0;
    }
}
Widget
make_menu (parent, top, bottom, left, right, title, name)
     Widget parent;
     Widget top, bottom;
     Widget left, right;
     char   *title, *name;
{
    int n;
    Arg wargs[10];
    Widget menuform, frame, label;

    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNleftWidget, left); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, top); n++;
    XtSetArg (wargs[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNrightWidget, right); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNbottomWidget, bottom); n++;
    menuform = XtCreateWidget(name, xmFormWidgetClass, parent, wargs, n);
    frame = XtCreateManagedWidget ("frame", xmFrameWidgetClass,
                                                      menuform, NULL, 0);
    n = 0;
    XtSetArg(wargs[n],XmNlabelString,XmStringCreateSimple(title)); n++;
    label = XtCreateManagedWidget (title, xmLabelGadgetClass, 
							  frame, wargs, n);

    return menuform;
}
Widget
make_info_area2 (parent, top, fg, bg)
    Widget parent, top;
    Pixel  fg, bg;
{
    Widget rc;
    Widget zoompop, zoom, rdraw, interupt, bigform;
    Widget frame, amt, lines, areas, sites, total;


    int    n = 0;
    Arg    wargs[10];

    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, top); n++;
    XtSetArg (wargs[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_FORM); n++;
    bigform = XtCreateManagedWidget ("bigform", xmFormWidgetClass,
                                                      parent, wargs, n);
    n = 0;
    XtSetArg (wargs[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, 20); n++;
    rc = XtCreateManagedWidget ("rc", xmRowColumnWidgetClass,
                                                      bigform, wargs, n);
    zoom = make_button (rc, "zoom",  NULL, "zoom", NULL, fg, bg,
						   "Define new window");
    zoompop = make_zoom (zoom);
    XtAddCallback (zoom, XmNactivateCallback, Zoom, zoompop);
    
    rdraw = make_button (rc, "redraw", redraw, NULL, NULL, fg, bg,
						    "Redraw screen");
    interupt = make_button (rc, "cancel", NULL, NULL, NULL, fg, bg,
						    "Cancel redraw of screen");
						    /*
    XtAddCallback (interupt, XmNarmCallback, stop, NULL);
    */
    XtAddCallback (interupt, XmNarmCallback, clear, NULL);
    XtAddCallback (rdraw, XmNactivateCallback, zero_display_list, NULL);
    Cancel = interupt;
    return bigform;
}
clear()
{
    XEvent event;
    
    while ( XCheckMaskEvent (dpy, ButtonPressMask | ButtonReleaseMask |
                KeyPressMask | KeyReleaseMask, &event))
		;
}


void
make_menus_active(w, yes_no)
    Widget w;
    int yes_no;
{
    XtSetSensitive (info, yes_no);
    XtSetSensitive (menu, yes_no);
    XtSetSensitive (mapinfo, yes_no);
    XmUpdateDisplay (toplevel);
}
   

void 
no_new_map()
{
    XtSetSensitive (ope, False);
    XtSetSensitive (new, False);
    close_infopop(); 

}
void
make_file_browser(w)
    Widget w;
{
    make_browser_popup (w, "open", XG_VECTOR, 
				  old_map, NULL, NULL, NULL, TRUE);
}
