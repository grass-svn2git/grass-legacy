
#include "globals.h"

#define SPT Region.saved_point

restore_region()
{
  int i;
  
  if (!Region.area.saved || Region.saved_view==NULL || Region.saved_npoints<=0)
    G_warning("No region is saved, can not restore!");
  else {
    if (Region.area.define)
      erase_region();
    Region.view = Region.saved_view;
    Region.area.define = 1;
    for(i=0; i<Region.saved_npoints; i++)
      add_point(SPT[i].x, SPT[i].y);
    Region.area.completed = 1;
  }
  Menu_msg("");

  return(0);
}
