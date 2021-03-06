#include "defs.h"

void
report(parms)
    struct Parms *parms;
{
    int i1,i2;
    struct Map *map1, *map2;
    char *fs;
    double distance, north1, east1, north2, east2;
    struct Cell_head region;
    struct CatEdgeList *list1, *list2;
    char temp[100];

    extern void find_minimum_distance();
    extern char *get_label();

    G_get_set_window (&region);
    G_begin_distance_calculations();

    map1 = &parms->map1;
    map2 = &parms->map2;
    fs   = parms->fs;

    if (parms->verbose)
	fprintf (stderr, "Processing...\n");

    for (i1 = 0; i1 < map1->edges.ncats; i1++)
    {
	list1 = &map1->edges.catlist[i1];
	for (i2 = 0; i2 < map2->edges.ncats; i2++)
	{
	    list2 = &map2->edges.catlist[i2];
	    find_minimum_distance (list1, list2,
		&east1, &north1, &east2, &north2, &distance, &region);

	/* print cat numbers */
	    printf ("%ld%s%ld", (long)list1->cat, fs, (long)list2->cat) ;

	/* print distance */
	    sprintf (temp, "%.10lf", distance); G_trim_decimal (temp);
	    printf ("%s%s", fs, temp);
	
	/* print coordinates of the closest pair */
	    G_format_easting (east1, temp, -1);
	    printf ("%s%s", fs, temp);
	    G_format_northing (north1, temp, -1);
	    printf ("%s%s", fs, temp);
	    G_format_easting (east2, temp, -1);
	    printf ("%s%s", fs, temp);
	    G_format_northing (north2, temp, -1);
	    printf ("%s%s", fs, temp);

	/* print category labels */
	    if (parms->labels)
	    {
		printf ("%s%s", fs, get_label (map1, list1->cat));
		printf ("%s%s", fs, get_label (map2, list2->cat));
	    }
	    printf ("\n");
	}
    }
}
