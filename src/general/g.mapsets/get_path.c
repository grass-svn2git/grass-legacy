#include <string.h>
#include "externs.h"
#include "gis.h"
get_mapset_path ()
{
    static int first = 1;
    char buf[1024];
    char *b;
    char name[50];
    int n;
    int action;	/* defines action to be taken in set_path function */

    printf("Hit RETURN to keep current list, or ");
    printf("enter a new list of mapsets\n");
    if (first)
    {
	printf("(enter the numbers before the mapset name, ");
	printf("or the names themselves)\n");
	printf ("    (to ADD to end of list, use + . . .)\n");
	printf ("    (to DELETE from list, use - . . .)\n");
	printf ("    ('+' or '-' must be followed by space)\n");
	first = 0;
    }

    nchoices = 0;

    printf ("\nnew list> ");
    if (!gets (b = buf)) goto same;
    if (sscanf (buf,"%s",name) != 1)
	    goto same;

    if (strlen (name) == 1) 
	switch (name[0]) {
	case '+':	/* preload existing path into choice array */
	    action = ADD;
            for (n = 0; n < ncurr_mapsets;
	     choice[nchoices++] = curr_mapset[n++]);
	    goto next;
	    break;
	case '-':
	    action = DELETE;
	    goto next;
	}
    action = REPLACE;
	
    while (1)
    {
	if (sscanf (b, "%s", name) != 1)
	{
	    if (action == DELETE)
		return delete_choices ();
	    return 1;
	}

	for (n = 0; n < nmapsets; n++)
	    if (strcmp (name, mapset_name[n]) == 0)
	    {
		choice[nchoices++] = n;
		goto next;
	    }
	
	if (scan_int (name, &n))
	{
	    if (n > 0 && n <= nmapsets)
	    {
		choice[nchoices++] = n-1;
		goto next;
	    }
	}

	printf("\n<%s> not found\n\n", name);
	return -1;

next:
	while (*b == ' ' || *b == '\t')
		b++;
	while (*b && *b != ' ' && *b != '\t')
		b++;
    }
same:
    for (n = 0; b = G__mapset_name (n); n++)
    {
	int i;
	for (i = 0; i < nmapsets; i++)
	    if (strcmp (b, mapset_name[i]) == 0)
	    {
		choice[nchoices++] = i;
		break;
	    }
    }
    return 1;
}


delete_choices ()
{
    int i, n;
    int deletion;	/* map number to be deleted */

/* action is delete:  modify previous mapset list and write to choice array */

    for (i = 0; i < nchoices; i++)
    {
	deletion = choice[i];
        for (n = 0; n < ncurr_mapsets; n++)
            if (curr_mapset[n] == deletion)  /* delete mapset from path */
            {
                curr_mapset[n] = -1;
                break;
            }
	if (n == ncurr_mapsets)
	{
	    printf("\n<%s> not found in search list\n\n",
	     mapset_name[deletion]);
            return -1;
        }
    }

/* prepare choice array list for final processing */
    nchoices = 0;
    for (n = 0; n < ncurr_mapsets; n++)
	if (curr_mapset[n] >= 0)	/* i.e., not deleted */
             choice[nchoices++] = curr_mapset[n];
    return 1;
}
