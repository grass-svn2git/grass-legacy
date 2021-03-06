

#include "digit.h"

Widget monolog;
static Widget message_pop;
static Widget yes_no;
static int answer;

void
On (w, item)
    Widget w;
    int *item;
{
    *item = 1;
}

void
Off (w, item)
    Widget w;
    int *item;
{
    *item = 0;
}

make_yes_no(parent, msg)
    Widget parent;
    char  *msg;
{
    int n, x, y;
    XmString  message, yes, no, title;
    message = XmStringCreateSimple (msg);
    title = XmStringCreateSimple ("Yes/No");
    yes = XmStringCreateSimple ("Yes");
    no = XmStringCreateSimple ("No");

    n = 0;
    yes_no = XmCreateMessageDialog (toplevel, "", NULL, 0);
    XtVaSetValues (yes_no, XmNautoUnmanage, TRUE,
    XmNmessageString, message,
    XmNokLabelString, yes,
    XmNcancelLabelString, no,
    XmNdialogTitle, title, 
     NULL);
    XtVaSetValues (XtParent(yes_no), 
		XmNsaveUnder, True,
		NULL);
   
    XtAddCallback (yes_no, XmNcancelCallback, Off, &answer);
    XtAddCallback (yes_no, XmNokCallback, On, &answer);


    XmStringFree (message);
    XmStringFree (title);
    XmStringFree (yes);
    XmStringFree (no);


    XtUnmanageChild (XmMessageBoxGetChild(yes_no, XmDIALOG_HELP_BUTTON));
}
mouse_yes_no ( header)
        char    *header ;
{
    extern Widget toplevel;
    Window win;
    XEvent event;
    XmString msg = XmStringCreateSimple (header);

    XFlush (dpy);
    answer = 2;


    XtVaSetValues (yes_no, XmNmessageString, msg, NULL);
    XtManageChild (yes_no);
    win = XtWindow(yes_no);
    while (answer > 1)
    {
    XmUpdateDisplay (toplevel);
        while (XCheckMaskEvent (dpy,
            ButtonPressMask | ButtonReleaseMask |
            KeyPressMask | KeyReleaseMask, &event))
        {
        if (event.xany.window == win)
          XtDispatchEvent (&event);
        else
            XBell (dpy,50);
        }
    }
    XmStringFree (msg);

        return (answer) ;
}


void
make_monolog (button, msg)
    int button;
    char *msg;
{
    int n, x, y;
    Arg wargs[10];
    XmString  message;
    Widget w;
    extern Widget toplevel;

    if (button) 
        XtManageChild (XmMessageBoxGetChild(monolog, XmDIALOG_CANCEL_BUTTON));
    else
        XtUnmanageChild (XmMessageBoxGetChild(monolog, XmDIALOG_CANCEL_BUTTON));
    message = XmStringCreateSimple (msg);
    if(XtIsManaged (monolog))
        XtUnmanageChild (monolog);

    n = 0;
    XtVaSetValues (monolog, XmNmessageString, message, NULL); 
    XtManageChild (monolog);
    XmUpdateDisplay (toplevel);

}

void
make_1st_monolog (parent, msg)
    Widget parent;
    char *msg;
{
    int n, x, y;
    Arg wargs[10];
    XmString  cancel, message, title;
    
    message = XmStringCreateSimple (msg);
    title = XmStringCreateSimple ("Info");
    cancel = XmStringCreateSimple ("ok");

    n = 0;
    XtSetArg (wargs[n], XmNautoUnmanage, TRUE); n++;
    XtSetArg (wargs[n], XmNmessageString, message); n++;
    XtSetArg (wargs[n], XmNcancelLabelString, cancel); n++;
    XtSetArg (wargs[n], XmNdialogTitle, title); n++;
    monolog = XmCreateMessageDialog (parent, "", wargs, n);
    XtVaSetValues (XtParent(monolog), 
		XmNsaveUnder, True,
		NULL);


    XtAddCallback (monolog, XmNcancelCallback, downcb, monolog);
    XmStringFree (message);
    XmStringFree (title);

    XtUnmanageChild (XmMessageBoxGetChild(monolog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild(monolog, XmDIALOG_OK_BUTTON));
}
void 
message_callback (w, str)
    Widget w;
    char *str;
{
    make_monolog (1, str);
}
/*
void
show_message(msg)
    char *msg;
{
    int n, x, y;
    Arg wargs[10];
    XmString  cancel, message, title;
    extern Widget toplevel;
    
    message = XmStringCreateSimple (msg);
    title = XmStringCreateSimple ("Info");

    n = 0;
    XtSetArg (wargs[n], XmNautoUnmanage, TRUE); n++;
    XtSetArg (wargs[n], XmNmessageString, message); n++;
    XtSetArg (wargs[n], XmNdialogTitle, title); n++;
    message_pop= XmCreateMessageDialog (canvas, "", wargs, n);
    XtVaSetValues (XtParent(message_pop), 
		XmNsaveUnder, True,
		NULL);


    XmStringFree (message);
    XmStringFree (title);

    XtUnmanageChild (XmMessageBoxGetChild(message_pop, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild(message_pop, XmDIALOG_OK_BUTTON));
    XtUnmanageChild (XmMessageBoxGetChild(message_pop, XmDIALOG_CANCEL_BUTTON));

    XtManageChild (message_pop);
    XFlush (dpy);
    XmUpdateDisplay (toplevel);
}
*/
void
end_message()
{
    
    if (XtIsManaged (monolog))
        XtUnmanageChild (monolog);
}
