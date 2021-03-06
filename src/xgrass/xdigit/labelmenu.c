#include "digit.h"

#define LAB_LINE 0
#define LAB_AREA 1
#define LAB_AREA_B 6
#define LAB_SITE 2
#define LAB_BULK  3
#define LAB_CONTOUR  4
#define LAB_CONTOUR_B  5


int Cat;
static int type;

make_label_menu (parent)
    Widget parent;
{
    Widget label, unlabel, elem;
    Widget autoinc, catw, interval, highlight;
    int    x, y;
    int    n = 0;
    Arg    wargs[10];
    Pixel  fg, bg;
    x = 10;
    y = 5;

    Cat = 0;
    type = 0;
    n = 0;
    XtSetArg (wargs[n], XtNforeground, &fg); n++;
    XtSetArg (wargs[n], XtNbackground, &bg); n++;
    XtGetValues (parent, wargs, n);
 
    elem = 
    make_button (parent, "", elemcb, NULL, "line", fg, bg, 
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
    XtCreateManagedWidget ("Type", xmLabelGadgetClass, parent, wargs, n);
/*
    x += 12;
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y + 5); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    autoinc = XtCreateManagedWidget ("Auto\nIncrement", xmToggleButtonGadgetClass, 
					       parent, wargs, n);
    XtAddCallback (autoinc, XmNarmCallback, togglecb, 
						"auto increment");
						*/
   
    x += 12;
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNcolumns, 7); n++;
    catw = XtCreateManagedWidget ("cat#", xmTextFieldWidgetClass, 
						     parent, wargs, n);
    XtAddCallback (catw, XmNactivateCallback, textcb, "category");
    XtAddCallback (catw, XmNvalueChangedCallback, catcb, NULL);

    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, catw); n++;
    XtSetArg (wargs[n], XmNmarginHeight, 0); n++;
    XtCreateManagedWidget ("Category\nNumber", xmLabelGadgetClass, parent, wargs, n);
    
    x += 20;
    
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    label = XtCreateManagedWidget ("Label", xmPushButtonWidgetClass, 
						      parent, wargs, n);
    XtAddCallback (label, XmNarmCallback, showtext,
						  "Label element");
						  

    x += 6;
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    unlabel = XtCreateManagedWidget ("Unlabel", xmPushButtonWidgetClass, 
					      parent, wargs, n);
    XtAddCallback (unlabel, XmNarmCallback, showtext,
						  "Unlabel element");

    x += 7;
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    highlight = XtCreateManagedWidget ("Highlight", xmPushButtonWidgetClass, 
					      parent, wargs, n);
    XtAddCallback (highlight, XmNarmCallback, hilgtcb,
						  NULL);
    x += 17;
    n = 0;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNtopPosition, y); n++;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x); n++;
    XtSetArg (wargs[n], XmNcolumns, 7); n++;
    XtSetArg (wargs[n], XmNvalue, "5"); n++;
    interval = XtCreateManagedWidget ("interval", xmTextFieldWidgetClass, 
				      parent, wargs, n);
    XtAddCallback (interval,  XmNactivateCallback, textcb,
						  "contour interval");
    XtAddCallback (interval,  XmNvalueChangedCallback, intervalcb, NULL);
    n = 0;
    XtSetArg (wargs[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg (wargs[n], XmNleftPosition, x+1); n++;
    XtSetArg (wargs[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg (wargs[n], XmNtopWidget, interval); n++;
    XtSetArg (wargs[n], XmNmarginHeight, 0); n++;
    XtCreateManagedWidget ("Contour\nInterval", xmLabelGadgetClass, 
						   parent, wargs, n);

    XtAddCallback (label, XmNactivateCallback, labelcb, NULL);
    XtAddCallback (unlabel, XmNactivateCallback, unlabelcb, NULL);

    

}
void
catcb (w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    Cat = atoi (XmTextGetString (w));
}

void
intervalcb (w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    Contour_Interval = atoi (XmTextGetString (w));
}

void
hilgtcb(w, client_data, call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    char str[80];

    Label (w, MLC_SLINES);
    sprintf (str, "Highlight elements of category %d", Cat);
    showtext (w, str, NULL);

}

void
labelcb(w)
    Widget w;
{
    char str[80]; 
    Arg warg;

    switch (type) { 
	case LAB_LINE:
	    sprintf(str, "Label lines with category %d", Cat);
	    Label (w, MLC_LLINE);
	    break;
	case LAB_AREA:
	    sprintf(str, "Label areas with category %d", Cat);
	    Label (w, MLC_LAREA);
	    break;
	case LAB_SITE:
	    sprintf(str, "Label sites with category %d", Cat);
	    Label (w, MLC_LSITE);
	    break;
	case LAB_BULK:
	    sprintf(str, "Bulk label remaining lines with category %d", Cat);
	    Label (w, MLC_LLINES);
	    break;
	case LAB_CONTOUR:
	    sprintf(str, "Label contours with interval %d", Contour_Interval);
	    Label (w, MLC_CONTOUR);
	    break;
	default:
	    break;
	}
	showtext(w, str, NULL);
}

void
unlabelcb(w)
    Widget w;
{
    char str[80]; 
    Arg warg;

    switch (type) { 
	case LAB_LINE:
	    sprintf(str, "Unlabel lines");
	    Label (w, MLC_ULLINE);
	    break;
	case LAB_AREA:
	    sprintf(str, "Unlabel areas");
	    Label (w, MLC_ULAREA);
            break;
	case LAB_SITE:
	    sprintf(str, "Unlabel sites");
	    Label (w, MLC_ULSITE);
	    break;
	case LAB_BULK:
	    sprintf(str, "Bulk unlabel remaining lines");
	    Label (w, MLC_UMLINES);
	    break;
	case LAB_CONTOUR:
	    make_monolog (1, "Contour lines must be unlabeled seperately");
	    break;
	default:
	    break;
	}
	showtext(w, str, NULL);
}
void
elemcb(w, client_data,  call_data)
    Widget w;
    caddr_t client_data;
    caddr_t call_data;
{
    char   str[50];

    extern int Text;
    static char *name[] = {
	"line",
	"area",
	"site",
	"bulk lines",
	"contour lines"
    };
    static char *pix[] = {
	"line",
	"area",
	"site",
	"bulk",
	"contour"
    };

    
    type = (type+1)%5;
    sprintf (str, "Change element type to %s", name[type]);
    showtext (w, str, NULL);
    if (!Text)
	change_pix (w, pix[type], w); 
    else
	XtVaSetValues (w, XtNlabel, name[type], NULL);
    
}


void 
change_pix (w, map, parent)
    Widget w;
    char *map;
    Widget parent;
{
    Arg    wargs[2];
    int    n;
    Pixmap pix;
    Pixel  fg, bg;
    int q = 0;


    n = 0;
    XtSetArg (wargs[n], XtNforeground, &fg); n++;
    XtSetArg (wargs[n], XtNbackground, &bg); n++;
    XtGetValues (parent, wargs, n);


    pix = XmGetPixmap (XtScreen(parent), map, fg, bg);

    n = 0;
    XtSetArg (wargs[n], XmNlabelPixmap, pix); n++;
    XtSetValues (w, wargs, n);
}
