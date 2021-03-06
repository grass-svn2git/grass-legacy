#include "xgdisp.h"


void
#ifdef _NO_PROTO
BarStyleToggleCallBack(w, cld, cbs)
    Widget      w;
    XtPointer       cld;
    XmToggleButtonCallbackStruct *cbs;
#else
BarStyleToggleCallBack(Widget w, XtPointer cld, XmToggleButtonCallbackStruct * cbs)
#endif
{
    Global.barattr.style = (int) cld;
    if (Global.selectedObjects == NULL)
        return;
    if (Global.selectedObjects->object &&
        SelectedObjectCount() == 1 &&
        Global.selectedObjects->object->type == XGD_BARSCALE) {
        if (Global.barattr.style != Global.selectedObjects->object->Obj.Barscale.style) {
            XgdSetBarscaleStyle(Global.selectedObjects->object, Global.barattr.style);
            XgdDrawResizeHandles(Global.selectedObjects->object, Global.xorGC);
            XClearArea(Global.selectedObjects->object->display,
                   Global.selectedObjects->object->window,
                   Global.selectedObjects->object->x,
                   Global.selectedObjects->object->y,
                   Global.selectedObjects->object->width,
                   Global.selectedObjects->object->height,
                   False);
            XgdDrawBarscale(Global.selectedObjects->object, True, NULL);
            XgdDrawResizeHandles(Global.selectedObjects->object, Global.xorGC);
            XgdUpdateBoxForObject(Global.selectedObjects->object,
                          Global.selectedObjects->object->y, Global.selectedObjects->object->y + Global.selectedObjects->object->height,
                          Global.selectedObjects->object->x, Global.selectedObjects->object->x + Global.selectedObjects->object->width);
        }
    }
}

static XtCallbackRec lengthModCB[] = {
    {(XtCallbackProc) BarLengthTextVerifyCallBack, (XtPointer) NULL},
    {(XtCallbackProc) NULL, NULL}
};

static XtCallbackRec lengthValCB[] = {
    {(XtCallbackProc) BarLengthTextCallBack, (XtPointer) NULL},
    {(XtCallbackProc) NULL, NULL}
};

Widget
#ifdef _NO_PROTO
CreateBarStyle(parent, string)
    Widget      parent;
    char       *string;
#else
CreateBarStyle(Widget parent, char *string)
#endif
{
    Widget      caption;
    Widget      frame;
    Widget      form;
    Widget      rc;
    Widget      tbutton;
    XmString    xms;
    int         offset = 7;

    xms = XmStringCreateSimple(string);
    caption = XtVaCreateManagedWidget("barstyle_caption",
                      xbaeCaptionWidgetClass, parent,
                      XmNlabelPosition, XbaePositionTop,
                     XmNlabelAlignment, XbaeAlignmentCenter,
                      XmNlabelString, xms,
                      XmNlabelOffset, -offset,
                      NULL);
    XmStringFree(xms);

    frame = XtVaCreateManagedWidget("barstyle_frame",
                    xmFrameWidgetClass, caption,
                    XmNmarginWidth, offset,
                    XmNmarginHeight, offset, NULL);

    form = XtVaCreateManagedWidget("linewidth_form",
                       xmFormWidgetClass, frame, NULL);


    rc = XtVaCreateManagedWidget("rc",
                     xmRowColumnWidgetClass, form,
                     XmNorientation, XmHORIZONTAL,
                     XmNtopAttachment, XmATTACH_FORM,
                     XmNleftAttachment, XmATTACH_FORM,
                     XmNspacing, 5,
                     XmNradioBehavior, XmONE_OF_MANY,
                     XmNradioAlwaysOne, True,
                     NULL);

    tbutton = XtVaCreateManagedWidget("dashed_button",
                      xmToggleButtonWidgetClass, rc,
                 XmNlabelString, XmStringCreateSimple("dashed"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarStyleToggleCallBack, (XtPointer) DASHED);
    bardashedw = tbutton;

    tbutton = XtVaCreateManagedWidget("ticked_button",
                      xmToggleButtonWidgetClass, rc,
                 XmNlabelString, XmStringCreateSimple("ticked"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarStyleToggleCallBack, (XtPointer) TICKED);
    bartickedw = tbutton;

    return caption;
}


double
XgdGetBarscaleLength(obj)
 XgdObject *obj;
{
    if ( obj->type != XGD_BARSCALE ) return 0.0;
    return obj->Obj.Barscale.length;
}

void
#ifdef _NO_PROTO
BarUnitToggleCallBack(w, cld, cbs)
    Widget      w;
    XtPointer       cld;
    XmToggleButtonCallbackStruct *cbs;
#else
BarUnitToggleCallBack(Widget w, XtPointer cld, XmToggleButtonCallbackStruct * cbs)
#endif
{
    Global.barattr.unit = (int) cld;
    if (Global.selectedObjects == NULL)
        return;
    if (Global.selectedObjects->object &&
        SelectedObjectCount() == 1 &&
        Global.selectedObjects->object->type == XGD_BARSCALE) {
        if (Global.barattr.unit != Global.selectedObjects->object->Obj.Barscale.unit) {
            XgdSetBarscaleUnit(Global.selectedObjects->object, Global.barattr.unit);
            XgdDrawResizeHandles(Global.selectedObjects->object, Global.xorGC);
            RedrawArea(Global.selectedObjects->object);
            XgdDrawBarscale(Global.selectedObjects->object, True, NULL);
            XgdDrawResizeHandles(Global.selectedObjects->object, Global.xorGC);
            XgdUpdateBoxForObject(Global.selectedObjects->object,
                          Global.selectedObjects->object->y, Global.selectedObjects->object->y + Global.selectedObjects->object->height,
                          Global.selectedObjects->object->x, Global.selectedObjects->object->x + Global.selectedObjects->object->width);
            if ( barlengthw != NULL && XtIsManaged(barlengthw) ) {
                char text[256];

                XtRemoveCallbacks(barlengthw, 
                    XmNvalueChangedCallback, lengthValCB);
                XtRemoveCallbacks(barlengthw, 
                    XmNmodifyVerifyCallback, lengthModCB);
                sprintf(text, "%8.2f", 
                    XgdGetBarscaleLength(Global.selectedObjects->object));
                XmTextSetString(barlengthw, text);
                XtAddCallback(barlengthw, XmNvalueChangedCallback, 
                    BarLengthTextCallBack, (XtPointer)NULL);
                XtAddCallback(barlengthw, XmNmodifyVerifyCallback, 
                    BarLengthTextVerifyCallBack, (XtPointer)NULL);
            }
        }
    }
}




Widget
#ifdef _NO_PROTO
CreateBarUnit(parent, string)
    Widget      parent;
    char       *string;
#else
CreateBarUnit(Widget parent, char *string)
#endif
{
    Widget      caption;
    Widget      frame;
    Widget      form;
    Widget      rc;
    Widget      tbutton;
    XmString    xms;
    int         offset = 7;

    xms = XmStringCreateSimple(string);
    caption = XtVaCreateManagedWidget("barunit_caption",
                      xbaeCaptionWidgetClass,
                      parent,
                      XmNlabelPosition, XbaePositionTop,
                     XmNlabelAlignment, XbaeAlignmentCenter,
                      XmNlabelString, xms,
                      XmNlabelOffset, -offset,
                      NULL);
    XmStringFree(xms);

    frame = XtVaCreateManagedWidget("barunit_frame",
                    xmFrameWidgetClass, caption,
                    XmNmarginWidth, offset,
                    XmNmarginHeight, offset, NULL);

    form = XtVaCreateManagedWidget("linewidth_form",
                       xmFormWidgetClass, frame, NULL);

    rc = XtVaCreateManagedWidget("rc",
                     xmRowColumnWidgetClass, form,
                     XmNorientation, XmHORIZONTAL,
                     XmNtopAttachment, XmATTACH_FORM,
                     XmNleftAttachment, XmATTACH_FORM,
                     XmNradioBehavior, XmONE_OF_MANY,
                     XmNradioAlwaysOne, True,
                     XmNnumColumns, 2,
                     NULL);

    tbutton = XtVaCreateManagedWidget("meter_button",
                      xmToggleButtonWidgetClass, rc,
                 XmNlabelString, XmStringCreateSimple("meters"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarUnitToggleCallBack, XGD_METERS);
    barmw = tbutton;

    tbutton = XtVaCreateManagedWidget("km_button",
                      xmToggleButtonWidgetClass, rc,
             XmNlabelString, XmStringCreateSimple("kilometers"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarUnitToggleCallBack, XGD_KILOMETERS);
    barkmw = tbutton;

    tbutton = XtVaCreateManagedWidget("mi_button",
                      xmToggleButtonWidgetClass, rc,
                  XmNlabelString, XmStringCreateSimple("miles"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarUnitToggleCallBack, XGD_MILES);
    barmiw = tbutton;

    tbutton = XtVaCreateManagedWidget("ft_button",
                      xmToggleButtonWidgetClass, rc,
                   XmNlabelString, XmStringCreateSimple("feet"),
                      NULL);
    XtAddCallback(tbutton, XmNvalueChangedCallback,
              BarUnitToggleCallBack, XGD_FEET);
    barftw = tbutton;

    return caption;
}
