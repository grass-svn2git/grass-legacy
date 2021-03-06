
#include "globals.h"
/* This routine completes the region by adding a line from the last
   point to the first point. */
complete_region()
{
  if (Region.npoints < 3) 
    G_warning("Too few points for region.  Must have at least 3 points.");
  else if (Region.area.completed) 
    G_warning("Area already completed.");
  else {
    add_point(Region.point[0].x, Region.point[0].y);
    Region.area.completed = 1;
    save_region();
  }
  Menu_msg("");
  return(0);
}


  
