
/***************************************************************
 * This routine takes a menu description and creates it.
 **************************************************************/
#include "xgen.h"

#include <X11/StringDefs.h>
#include <Xm/ScrolledWP.h>

void
CreateMenu(s,isDynamic)
    Shell *s;
    Boolean isDynamic;
{
    Resource *resource;    /* pointer to the current resource                 */
    Widget     shell;      /* popup shell widget (parent of all the below) */
    Widget main_window;    /* the MainWindow widget                         */
    Widget menu_bar;       /* the Menubar widget                             */
    Widget help;           /* the Help widget                                 */
    Widget frame;          /* the Frame widget                             */
    Widget swindow;        /* the ScrolledWindow widget                     */
    Widget row_col;        /* the RowColumn widget                         */
    Widget hsb, vsb;       /* the ScrollBar widgets                         */
    Boolean width_specd,   /* has the width been specified ?                 */
            height_specd;  /* has the height been specified ?                 */
    int n;                 /* the Arg counter                                 */
	int popupWidth,
		popupHeight;

    n = 0;
    SetGlobalArgs(&n,NOFONTS);
    SetShellColorArgs(s,&n);
    if ( (resource = IndexResource(s,SHELL,"dx")) != NULL ) {
        int x;

		if ( resource->variable ) ExpandVariable(resource);
        x = (int)((resource->val.dval/100.0) * 
            (double)DisplayWidth(xgenGD.display,xgenGD.screen));
        XtSetArg(args[n],XmNx,x); n++;
    }
    if ( (resource = IndexResource(s,SHELL,"dy")) != NULL ) {
        int y;

		if ( resource->variable ) ExpandVariable(resource);
        y = (int)((resource->val.dval/100.0) *
            (double)DisplayHeight(xgenGD.display,xgenGD.screen));
        XtSetArg(args[n],XmNy,y); n++;
    }
    if ( (resource = IndexResource(s,SHELL,"width")) != NULL ) {
		if ( resource->variable ) ExpandVariable(resource);
        XtSetArg(args[n],XmNwidth,resource->val.ival); n++;
        width_specd = True;
    }
    if ( (resource = IndexResource(s,SHELL,"height")) != NULL ) {
		if ( resource->variable ) ExpandVariable(resource);
        XtSetArg(args[n],XmNheight,resource->val.ival); n++;
        height_specd = True;
    }
    if ( (resource = IndexResource(s,SHELL,"override")) != NULL ) {
        XtSetArg(args[n],XmNoverrideRedirect,resource->val.ival); n++;
    }
    XtSetArg(args[n],XmNallowShellResize,True); n++;
    if ( (resource = IndexResource(s,SHELL,"titlestring")) != NULL ) {
        s->widget = XtCreatePopupShell(resource->val.cval,
             topLevelShellWidgetClass, xgenGD.applShell,args,n);
    } else {
        s->widget = XtCreatePopupShell(s->name,topLevelShellWidgetClass,
                                    xgenGD.applShell,args,n);
    }

    /***************************************************************
     * create the MainWindow : this is used since it allows for a
     * MenuBar, a Frame, and a ScrolledWindow (if necessary).
     * The application designer might choose to place labels, lists,
     * or pushbuttons in a menu.
     **************************************************************/
    n = 0;
    SetGlobalArgs(&n,NOFONTS);
    SetShellColorArgs(s,&n);
    main_window = XmCreateMainWindow(s->widget,s->name,args,n);
    XtManageChild(main_window);

    /***************************************************************
     * create the MenuBar with help button (if help is supplied)
     **************************************************************/
    menu_bar = NULL;
    if ( (resource = IndexResource(s,SHELL,"help")) != NULL ) {
        n = 0;
        SetGlobalArgs(&n,NOFONTS);
        SetShellColorArgs(s,&n);
        menu_bar = XmCreateMenuBar(main_window,"menu_bar",args,n);
        XtManageChild(menu_bar);
        n = 0;
        SetGlobalArgs(&n,FONTS);
        SetShellColorArgs(s,&n);
        help = XmCreateCascadeButton(menu_bar, "Help", args,n);
        XtManageChild(help);
        /***************************************************************
          * add the help callback, passing it the help string
          **************************************************************/
        XtAddCallback(help,XmNactivateCallback,helpCB,
                        (caddr_t)resource->val.cval);

        n = 0;
        XtSetArg(args[n],XmNmenuHelpWidget,help); n++;
        XtSetValues(menu_bar,args,n);
    }

    /***************************************************************
     * create a frame for the objects 
     **************************************************************/
    n = 0;
    SetGlobalArgs(&n,NOFONTS);
    SetShellColorArgs(s,&n);
    frame = XmCreateFrame(main_window,"frame",args,n);
    XtManageChild(frame);

    /***************************************************************
     * create a scrolled window 
     **************************************************************/
    if ( isDynamic ) {
        n = 0;
        SetGlobalArgs(&n,NOFONTS);
        SetShellColorArgs(s,&n);
        XtSetArg(args[n],XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
        XtSetArg(args[n],XmNscrollingPolicy, XmAUTOMATIC); n++;
        XtSetArg(args[n],XmNvisualPolicy, XmCONSTANT); n++;
        swindow = XmCreateScrolledWindow(frame,"swindow",args,n);
        XtManageChild(swindow);
    }

    /***************************************************************
     * create a rowcolumn widget to manage the objects 
     **************************************************************/
    n = 0;
    SetGlobalArgs(&n,NOFONTS);
    SetShellColorArgs(s,&n);
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 1); n++;
    if ( isDynamic )
        row_col = XmCreateRowColumn(swindow,"rc",args,n);
    else 
        row_col = XmCreateRowColumn(frame,"rc",args,n);
    XtManageChild(row_col);

    /***************************************************************
     * set main window areas 
     **************************************************************/
        XmMainWindowSetAreas(main_window,menu_bar,NULL,NULL,NULL,frame);
        XtRealizeWidget(s->widget);
    
        if ( isDynamic ) {
            n = 0;
            XtSetArg(args[n],XmNhorizontalScrollBar, &hsb); n++;
            XtSetArg(args[n], XmNverticalScrollBar, &vsb); n++;
            XtGetValues(swindow, args, n);
        
            n = 0;
            SetShellColorArgs(s,&n);
            XtSetValues(vsb,args,n);
            XtSetValues(hsb,args,n);
        }


    /***************************************************************
     * if the shell has objects....set up the best width/height
     **************************************************************/
    if ((Widget)0 != CreateObject(s,row_col,isDynamic) && isDynamic ) {
        XtWidgetGeometry blank,     /* send in a blank geometry spec */
                         preferred; /* geometry spec returned */
        Dimension vsb_width = 0,    /* the vertical scrollbar width */
                  hsb_height = 0,    /* the horizontal scrollbar width */
                  dpyht,             /* display height */
                  dpywid,            /* display width */
                  width,            /* the width of the ScrolledWindow */
                  height,            /* the height " " " */
                  spacing,            /* the spacing " " " */
                  bwidth;            /* the borderwidth " " " */
        short sthick;                /* the shadow thickness " " " */
        Boolean taller = False,                /* taller then dpyht */
                wider = False;                /* wider then dpyht */

        /***************************************************************
         * get the preferred width and height of the row_col widget
         * if it's taller than DisplayHeight, make it slightly smaller
         * if it's wider than DisplayWidth, make it slightly smaller
         ***************************************************************/
        bzero((char *)&blank,sizeof(XtWidgetGeometry));
        XtQueryGeometry(row_col, &blank, &preferred);
        width = preferred.width;
        height = preferred.height;
        if ( height > (dpyht = DisplayHeight(xgenGD.display,xgenGD.screen)) ) {
            height =  dpyht - 150;
            taller = True;
        }
        if ( width > (dpywid = DisplayWidth(xgenGD.display,xgenGD.screen)) ) {
            width =  dpywid - 100;
            wider = True;
        }

        /***************************************************************
         * if height is spec'd do nothing
          * if maxheight spec'd and is less than preferred, use it 
         * else go with preferred value and nix the scrollbar
          **************************************************************/
        if ( !height_specd ) {
            if ( ((resource = IndexResource(s,SHELL,"maxheight")) != NULL) ) {
		        if ( resource->variable ) ExpandVariable(resource);
                if (resource->val.ival < height)  
                    height = resource->val.ival;
            }
        }

        /***************************************************************
         * if width is spec'd do nothing
          * if maxwidth spec'd and is less than preferred, use it 
         * else go with preferred value and nix the scrollbar
          **************************************************************/
        if ( !width_specd ) {
            if ( ((resource = IndexResource(s,SHELL,"maxwidth")) != NULL) ) {
		        if ( resource->variable ) ExpandVariable(resource);
                if  (resource->val.ival < width) 
                    width = resource->val.ival;
            }
        }
        /***************************************************************
         * do it
         **************************************************************/
        n = 0;
        if ( taller ) {
            XmScrolledWindowWidget sw = (XmScrolledWindowWidget)swindow;
            bzero((char *)&blank,sizeof(XtWidgetGeometry));
            XtQueryGeometry(vsb, &blank, &preferred);
            
            XtSetArg(args[n],XmNwidth,width + sw->swindow.pad*2 + 
                sw->manager.shadow_thickness*2 + preferred.width); n++;
            if ( !wider ) {
                XtSetArg(args[n],XmNheight,height); n++;
            }
        }
        if ( wider ) {
            XmScrolledWindowWidget sw = (XmScrolledWindowWidget)swindow;
            bzero((char *)&blank,sizeof(XtWidgetGeometry));
            XtQueryGeometry(hsb, &blank, &preferred);
            
            XtSetArg(args[n],XmNheight,height + sw->swindow.pad*2 + 
                sw->manager.shadow_thickness*2 + preferred.height); n++;
            if ( !taller ) {
                XtSetArg(args[n],XmNwidth,width); n++;
            }
        }
        if ( !wider && !taller ) {
            XmScrolledWindowWidget sw = (XmScrolledWindowWidget)swindow;
            sthick = sw->manager.shadow_thickness;
            XtSetArg(args[n],XmNheight,height +2*sthick); 
            n++;
            XtSetArg(args[n],XmNwidth,width + 2*sthick); 
            n++;
        }
        XtSetValues(swindow,args,n);
        XmAddTabGroup(vsb);
    }

	n = 0;
	XtSetArg(args[n],XtNwidth,&popupWidth); n++;
	XtSetArg(args[n],XtNheight,&popupHeight); n++;
	XtGetValues(s->widget,args,n);
 
	n = 0;
    if ( (resource = IndexResource(s,SHELL,"x")) != NULL ) {
		if ( resource->variable ) ExpandVariable(resource);
        if ( resource->val.ival < 0 ) {
            XtSetArg(args[n],XmNx,DisplayWidth(xgenGD.display,xgenGD.screen)
                - popupWidth + resource->val.ival); n++;
        } else {
            XtSetArg(args[n],XmNx,resource->val.ival); n++;
        }
    }
    if ( (resource = IndexResource(s,SHELL,"y")) != NULL ) {
		if ( resource->variable ) ExpandVariable(resource);
        if ( resource->val.ival < 0 ) {
            XtSetArg(args[n],XmNx,DisplayWidth(xgenGD.display,xgenGD.screen)
                - popupHeight + resource->val.ival); n++;
        } else {
            XtSetArg(args[n],XmNy,resource->val.ival); n++;
        }
    }
    XtSetValues(s->widget,args,n);
}

