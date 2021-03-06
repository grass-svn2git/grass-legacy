/*
**  Last modified by Dave Gerdes  5/1988
**	added find_line, other changes for 3.0
**  US Army Construction Engineering Research Lab
**  Last, Last modified by Terry Baker Spring 1993
**  Add compatibility w/ X 
*/

#include "digit.h"


Zoomcb (w, choice)
    Widget w;
    int choice;
{
    scal_window_w_mouse (choice);
}

Pancb ()
{
    slid_window_w_mouse ();
}

Widget
make_zoom(parent)
    Widget parent;
{
    
    Widget zoomshell;
    Widget form, rc, tmp;
    XmString ttl = XmStringCreateSimple ("");
    
    zoomshell = XtCreatePopupShell ("Zoom", transientShellWidgetClass,
                                    toplevel, NULL, 0);
    XtVaSetValues (zoomshell, 
	       XmNsaveUnder, True, XtNx, Winx, XtNy, Winy + 100, NULL);
                      
    form = XtCreateManagedWidget 
              ("zoomform", xmFormWidgetClass, zoomshell, NULL, 0);
    rc = XtCreateManagedWidget 
              ("zoomrc", xmRowColumnWidgetClass, form, NULL, 0);
    tmp = XtCreateManagedWidget 
              ("Select with box", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, set_window_w_box, NULL);
    
    tmp = XtCreateManagedWidget 
              ("Widen", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, Widen, NULL);
    
    tmp = XtCreateManagedWidget 
              ("Zoom in", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, Zoomcb, 1);
    tmp = XtCreateManagedWidget 
              ("Zoom out", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, Zoomcb, 3);
    tmp = XtCreateManagedWidget 
              ("Pan", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, Pancb, NULL );
    tmp = XtCreateManagedWidget 
              ("Done", xmPushButtonWidgetClass, rc, NULL, 0);
    XtAddCallback (tmp, XmNactivateCallback, downcb, zoomshell);
  
    return (zoomshell);
    
}
void 
Zoom (w, pop)
    Widget w, pop;
{
    if (Window_Device == MOUSE)
    {
       if (!XtIsManaged (pop))
            XtManageChild (pop);
    }
    else
    {
	set_window();
	redraw();
    }
}
