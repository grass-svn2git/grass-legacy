#include "coin.h"

make_coin(verbose)

{
    FILE *fd, *popen();
    FILE *statfd;
    struct stats
    {
	long cat1;
	long cat2;
	long count;
	double area;
    } stats;
    int n, n1,n2;
    int reversed;
    char buf[512];
    int count;
    int cmp();

    if (verbose)
    {
	fprintf(stderr,"\n\nTabulating Coincidence between '%s' and '%s'\n",
				    map1name,map2name);
	fflush (stderr);
    }
    sprintf (buf, "r.stats -ac%s fs=: input='", verbose?"":"q");
    strcat(buf, G_fully_qualified_name(map1name, mapset1));
    strcat(buf, ",");
    strcat(buf, G_fully_qualified_name(map2name, mapset2));
    strcat(buf, "'");
    statfd = fopen (statname, "w");
    if (statfd == NULL)
    {
	G_fatal_error ("can't create any tempfiles");
	exit(1);
    }
    /*G_fatal_error(buf);*/
    
    fd = popen (buf, "r");
    if (fd == NULL)
    {
	G_fatal_error ("can't run r.stats");
	exit(1);
    }

/* need to find the number of cats in each file */
    count = 0;
    while (fgets (buf, sizeof buf, fd))
    {
	if (sscanf (buf, "%ld:%ld:%lf:%ld",
	    &stats.cat1, &stats.cat2, &stats.area, &stats.count) != 4)
	{
	    pclose (fd);
	    G_fatal_error ("Unexpected output from r.stats");
	    exit(1);
	}
	fwrite (&stats, sizeof (stats), 1, statfd);
	count++;
    }

    pclose (fd);
    fclose (statfd);

    statfd = fopen (statname, "r");
    if (statfd == NULL)
    {
	G_fatal_error ("can't open tempfile");
	exit(1);
    }

/* build a sorted list of cats in both maps */
    catlist1 = (long *) G_calloc (count*2, sizeof(long));
    catlist2 = catlist1 + count;

/* read the statsfile to get the cat lists */
    count = 0;
    while (fread (&stats, sizeof (stats), 1, statfd))
    {
	catlist1[count] = stats.cat1;
	catlist2[count++] = stats.cat2;
    }

/* sort both lists */
    qsort (catlist1, count, sizeof(long), cmp);
    qsort (catlist2, count, sizeof(long), cmp);

/* collapse the lists so each cat appears only once */
    ncat1 = collapse (catlist1, count);
    ncat2 = collapse (catlist2, count);

/* copy catlist2 to end of catlist1, then free the unused memory */
    for (count = 0; count < ncat2; count++)
	catlist1[ncat1+count] = catlist2[count];
    catlist1 = (long *) G_realloc (catlist1, (ncat1+ncat2) * sizeof(long));
    catlist2 = catlist1 + ncat1;

/* allocate the table */
    n = ncat1 * ncat2;
    table = (struct stats_table *) G_malloc (n * sizeof(struct stats_table));
    while (--n >= 0)
    {
	table[n].count = 0;
	table[n].area = 0.0;
    }

/* want the smaller number across, larger number down */
    reversed = 0;
    if (ncat1 > ncat2)
    {
	char name[40];
	char *mp;
	long *list;
	int n;

	n = ncat1;
	ncat1 = ncat2;
	ncat2 = n;

	strcpy (name, map1name);
	strcpy (map1name, map2name);
	strcpy (map2name, name);
	mp = mapset1;
	mapset1 = mapset2;
	mapset2 = mp;

	list = catlist1;
	catlist1 = catlist2;
	catlist2 = list;

	reversed = 1;
    }

    title1 = G_get_cell_title (map1name, mapset1);
    title2 = G_get_cell_title (map2name, mapset2);

/* determine where no data (cat 0) is */
    for (no_data1 = ncat1-1; no_data1 >= 0; no_data1--)
	if (catlist1[no_data1] == 0)
	    break;
    for (no_data2 = ncat2-1; no_data2 >= 0; no_data2--)
	if (catlist2[no_data2] == 0)
	    break;

/* now read the statsfile and insert into the table */
    fseek (statfd, 0L, 0);
    while (fread (&stats, sizeof (stats), 1, statfd))
    {
	long z;
	if (reversed)
	{
	    z = stats.cat1;
	    stats.cat1 = stats.cat2;
	    stats.cat2 = z;
	}

/*
 * search the catlists to index these cats in their respective list
 * (this could be sped up by doing a binary search)
 */
	for (n1 = 0; n1 < ncat1; n1++)
	    if (catlist1[n1] == stats.cat1)
		break;
	for (n2 = 0; n2 < ncat2; n2++)
	    if (catlist2[n2] == stats.cat2)
		break;
/*
 * insert the coincidence count, area into the table
 */
	n = n2*ncat1+n1;
	table[n].count = stats.count;
	table[n].area  = stats.area ;
    }
    fclose (statfd);
}

static
cmp (a,b)
    long *a, *b;
{
    if (*a < *b)
	return -1;
    if (*a > *b)
	return 1;
    return 0;
}

collapse (list,n)
    long *list;
{
    long *cur;
    int count;

    cur = list;
    count = 1;
    while (n-- > 0)
    {
	if (*cur != *list)
	{
	    cur++;
	    *cur = *list;
	    count++;
	}
	list++;
    }
    return count;
}
