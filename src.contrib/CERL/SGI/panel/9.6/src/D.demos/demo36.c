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
#include <panel.h>

Actuator *icon, *cycle, *acycleitem, *bcycleitem, *ccycleitem;
  
Panel *defpanel();

main() 
{
  foreground();
  noport();
  winopen("demo");
  
  doublebuffer();
  gconfig();
  
  defpanel();
  
  for (;;) {
    pnl_dopanel();
    swapbuffers();
  }
}

Panel 
*defpanel()
{
  Actuator *a;
  Panel *p;
  Coord x, y, d=1.0, dl=0.5;

  p=pnl_mkpanel();
  p->label="cycles";

  cycle=a=pnl_mkact(pnl_cycle);
  a->label="a cycle";
  pnl_addact(cycle, p);

  acycleitem=a=pnl_mkact(pnl_wide_button);
  a->label="first choice";
  pnl_addsubact(a, cycle);

  bcycleitem=a=pnl_mkact(pnl_wide_button);
  a->label="a big problem";
  pnl_addsubact(a, cycle);

  ccycleitem=a=pnl_mkact(pnl_wide_button);
  a->label="the end";
  pnl_addsubact(a, cycle);

  return p;
}

