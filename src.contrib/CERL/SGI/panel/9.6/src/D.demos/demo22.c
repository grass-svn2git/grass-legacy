/*
 *	This software is in the public domain, it may not be resold
 *	or relicensed.  Modified and enhanced versions of this software
 *	are likewise to be made freely available.  Sites using this
 *	software are requested to register with NASA at the address below.  
 *	Send modifications and requests for most recent version to:
 *
 *	Author:  David A. Tristram,  ATTN: Panel Library
 *		 M/S T045-1
 *		 Ames Research Center
 *		 National Aeronautics and Space Administration
 *		 Moffett Field, CA  94035-4000
 *
 *		 415-694-4404
 *		 dat@nas.nasa.gov
 */
#include <gl.h>
#include <device.h>
#include "panel.h"

Actuator *s1, *s2, *b1, *b2, *b3;

Panel
*defpanel();

main() 
{
Actuator *a;
Panel *panel;

    foreground();
    winopen("demo");

    doublebuffer();
    gconfig();

    ortho2(-1.0,1.0,-1.0,1.0);

    panel=defpanel();

    for (;;) {
        a=pnl_dopanel();
	if (a==b1) {
	    exit(0);
	}

	if (a==b2) {
	    b3->val=b2->val;
	    b3->dirtycnt=2;
	}

	pushmatrix();
	translate(s1->val,s2->val,0.0);
	color(BLACK);
	clear();
	color(WHITE);
	drawit();
	swapbuffers();
	popmatrix();
    }
}


drawit()
{
    rectf(-.10,-.10,.10,.10);
}


Panel
*defpanel()
{
Panel *panel;

    panel=pnl_mkpanel();
    panel->label="demo";

    s1=pnl_mkact(pnl_slideroid);
    s1->label="x position";
    s1->x=0.0;
    s1->y=0.0;
    s1->minval= -1.0;
    s1->maxval=1.0;
    pnl_addact(s1, panel);

    s2=pnl_mkact(pnl_slideroid);
    s2->label="y position";
    s2->x=2.0;
    s2->y=0.0;
    s2->minval= -1.0;
    s2->maxval=1.0;
    pnl_addact(s2, panel);

    b1=pnl_mkact(pnl_button);
    b1->label="exit";
    b1->x=1.0;
    b1->y=3.0;
    pnl_addact(b1, panel);

    b2=pnl_mkact(pnl_toggle_button);
    b2->label="push me";
    b2->x=1.0;
    b2->y=3.5;
    pnl_addact(b2, panel);

    b3=pnl_mkact(pnl_toggle_button);
    b3->label="I push myself";
    b3->x=1.0;
    b3->y=4.0;
    pnl_addact(b3, panel);

    return panel;
}

