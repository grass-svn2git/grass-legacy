#include "gis.h"

o_min(basemap,covermap,outputmap,usecats, cats)
char *basemap,*covermap,*outputmap;
int usecats;
struct Categories *cats;
{
    char command[1024];
    FILE *popen(), *stats, *reclass;
    int first;
    long basecat, covercat, catb, catc;

    sprintf(command, "r.stats input='%s,%s' fs=space", basemap, covermap);
    stats = popen (command, "r");

    sprintf (command, "r.reclass i='%s' o='%s'", basemap, outputmap);
    reclass = popen (command, "w");

    first = 1;
    
    
    while (fscanf (stats, "%ld %ld", &basecat, &covercat) == 2)
    {
	if (first)
	{
	    first = 0;
	    catb  = basecat;
	    catc  = covercat;
	}
	
	if (basecat != catb)
	{
            write_reclass (reclass, catb, catc, G_get_cat (catc, cats), usecats);
	    catb = basecat;
	    catc = covercat;
	}
	if (covercat < catc)
	    catc = covercat;

    }
    if (first)
    {
	catb = catc = 0;
    }
    write_reclass (reclass, catb, catc, G_get_cat (catc, cats), usecats);
    
    pclose (stats);
    pclose (reclass);

    return(0);
}
