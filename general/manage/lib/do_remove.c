#include "list.h"
#include "gis.h"
#include "Vect.h"
#include <string.h>

int do_remove (int n, char *old)
{
    int i, ret;
    int len;
    char *mapset;

    fprintf (stdout,"REMOVE [%s]\n", old);

    len = get_description_len(n);

    hold_signals(1);
    if ( strcmp(list[n].alias, "vect") == 0 ) {
	if ((mapset = G_find_vector2 (old, "")) == NULL)
		G_fatal_error ("Vector map <%s> not found\n", old);
	ret = Vect_delete ( old );
	if ( ret == -1 ) {
	    G_warning ("Cannot delete vector %s", old );
	}
    } else {
	for (i = 0; i < list[n].nelem; i++)
	{
	    fprintf (stdout," %-*s ", len, list[n].desc[i]);
	    fflush (stdout);

	    switch (G_remove (list[n].element[i], old))
	    {
	    case -1: fprintf (stdout,"COULD NOT REMOVE"); break;
	    case  0: fprintf (stdout,"MISSING"); break;
	    }
	    fprintf (stdout,"\n");
	}
    }
    if (strcmp (list[n].element[0], "cell") == 0)
    {
	char colr2[50];
	sprintf (colr2, "colr2/%s", G_mapset());
	G_remove (colr2, old);
    }
    hold_signals(0);

    return 0;
}
