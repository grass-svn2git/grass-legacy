#include "gis.h"

struct stats
{
    int nalloc;
    int n;
    long *cat;
    double *area;
};

o_median(basemap,covermap,outputmap,usecats, cats)
char *basemap,*covermap,*outputmap;
int usecats;
struct Categories *cats;
{
  char command[1024];
  FILE *popen(), *stats_fd, *reclass_fd;
  int first;
  long basecat, covercat, catb, catc;
  double area, max;
  struct stats stats;
   
          
    sprintf(command, "r.stats -a input='%s,%s' fs=space", basemap, covermap);
    stats_fd = popen (command, "r");
         

    sprintf (command, "r.reclass i='%s' o='%s'",basemap, outputmap);

    reclass_fd = popen (command, "w");

    first = 1;
    while (read_stats(stats_fd, &basecat, &covercat, &area))
    {
	if (first)
	{
	    stats.n = 0;
	    stats.nalloc = 16;
	    stats.cat = (long *) 
		G_calloc (stats.nalloc, sizeof(long));
	    stats.area = (double *) 
		G_calloc (stats.nalloc, sizeof(double));
	    first = 0;
	    catb = basecat;
	}
	if (basecat != catb)
	{
	    catc = median (&stats);
	    write_reclass (reclass_fd, catb, catc, G_get_cat (catc, cats),usecats);
	    catb = basecat;
	    stats.n = 0;
	}
	stats.n++;
	if (stats.n > stats.nalloc)
	{
	    stats.nalloc *= 2;
	    stats.cat = (long *)
		G_realloc (stats.cat, stats.nalloc * sizeof(long));
	    stats.area = (double *)
		G_realloc (stats.area, stats.nalloc * sizeof(double));
	}
	stats.cat[stats.n-1] = covercat;
	stats.area[stats.n-1] = area;
    }
    if (!first)
    {
	catc = median (&stats);
	write_reclass (reclass_fd, catb, catc, G_get_cat (catc, cats), usecats);
    }

    pclose (stats_fd);
    pclose (reclass_fd);

    exit(0);
}


long
median (stats)
struct stats *stats;
{
    double total, sum;
    int i;

    total = 0;
    for (i = 0; i < stats->n; i++)
	total += stats->area[i];
    
    total /= 2.0;

    sum = 0;
    for (i = 0; i < stats->n; i++)
    {
	sum += stats->area[i];
	if (sum > total) break;
    }
    if (i == stats->n) i--;
    if (i < 0) return((long)0);
    return(stats->cat[i]);
}


