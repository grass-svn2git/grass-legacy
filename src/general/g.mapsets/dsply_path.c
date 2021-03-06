#include <string.h>
#include "externs.h"
#include "gis.h"
display_mapset_path(verbose)
{
    int n;
    int map;	/* pointer into list of available mapsets */
    int offset = 6;	/* accounts for " <x>, " */
    char *name;
    int len;
    int nleft;

    if (verbose)
    {
    	/* account for largest mapset number in offset value */
    	for (n = nmapsets; n /= 10; offset++);

	printf("Your mapset search list:\n");
	ncurr_mapsets = 0;
    }

    nleft = 78;
    for (n = 0; name = G__mapset_name(n); n++)
    {
        /* match each mapset to its numeric equivalent */
        if (verbose) 
	{
	    for (map = 0; map < nmapsets && strcmp(mapset_name[map], name);
	     map++);
	    if (map == nmapsets)
	    {
	    	fprintf (stderr, "\n%s not found in mapset list:  call greg\n",
	     	 name);
	    	exit (-1);
	    }
	}

        len = strlen (name);
        if (len > nleft)
        {
            printf("\n");
            nleft = 78;
        }

	if (verbose)
	{
	    if (n)
		printf(", ");
            printf("%s <%d>", name, map + 1);
            nleft -= (len + offset);
	    curr_mapset[n] = map;
	    ++ncurr_mapsets;
	}
	else
	{
            printf("%s ", name);
            nleft -= (len + 1);
	}
    }
    printf("\n");
    if (verbose)
	printf("\n");
}
