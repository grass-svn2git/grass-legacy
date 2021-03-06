/**********************************************************************
 *  G_read_histogram (name, mapset, histogram)
 *      char *name                   name of map
 *      char *mapset                 mapset that map belongs to
 *      struct Histogram *histogram  struct for histogram
 *
 *  Reads the histogram information associated with map layer "map"
 *  in mapset "mapset" into the structure "histogram".
 *
 *   returns:    1  if successful
 *               0  if no histogram file
 *              -1  on fail
 *
 *  note:   a warning message is printed if the file is missing or incorrect
 **********************************************************************
 *  G_init_histogram (histogram)
 *      struct Histogram *histogram
 *
 *  initializes the histogram structure for calls to G_set_histogram()
 *  and G_add_histogram()
 **********************************************************************
 *  G_add_histogram (cat, count, histogram);
 *      CELL cat
 *      long count
 *      struct Histogram *histogram
 *
 *  adds count to the histogram value for cat
 **********************************************************************
 *  G_set_histogram (cat, count, histogram);
 *      CELL cat
 *      long count
 *      struct Histogram *histogram
 *
 *  sets the histogram value for cat to count
 **********************************************************************
 *  G_sort_histogram (histogram)
 *      struct Histogram *histogram
 *
 *  Sorts the histogram in ascending order by category,
 *  combining (by adding) elements that have the same category.
 **********************************************************************
 *  G_sort_histogram_by_count (histogram)
 *      struct Histogram *histogram
 *
 *  Sorts the histogram in ascending order by counts then category.
 *  No combining is done.
 **********************************************************************
 *  G_get_histogram_num (histogram)
 *
 *  returns the number of elements in the histogram
 **********************************************************************
 *  CELL
 *  G_get_histogram_cat (n, histogram)
 *      int n
 *      struct Histogram *histogram
 *
 *  Returns cat for the nth element in the histogram
 **********************************************************************
 *  long
 *  G_get_histogram_count (n, histogram)
 *      int n
 *      struct Histogram *histogram
 *
 *  Returns count for the nth element in the histogram
 **********************************************************************
 *  G_write_histogram (name, &histogram)
 *      char *name
 *      struct Histogram *histogram
 *
 *  Writes the histogram information associated with map layer "map"
 *
 *   returns:    1  if successful
 *              -1  on fail
 *
 **********************************************************************
 * G_free_histogram (histogram)
 *      struct Histogram *histogram
 *
 * frees the memory allocated for the histogram
 **********************************************************************/

#include "gis.h"

#define LIST struct Histogram_list

FILE *fopen_histogram_new();

G_init_histogram (histogram)
    struct Histogram *histogram;
{
    histogram->num = 0;
    histogram->list = NULL;
}

G_read_histogram (name, mapset, histogram)
    char *name, *mapset;
    struct Histogram *histogram;
{
    FILE *fd;
    long cat;
    long count;
    char buf[200];

    fd = NULL;

    G_init_histogram (histogram);

    sprintf (buf,"cell_misc/%s", name);
    if (G_find_file (buf, "histogram", mapset) == NULL)
    {
	sprintf (buf, "Histogram for [%s in %s] missing", name, mapset);
	G_warning (buf);
	return 0;
    }
    fd = G_fopen_old (buf, "histogram", mapset);
    if (!fd)
    {
	sprintf (buf, "Can't read histogram for [%s in %s]", name, mapset);
	G_warning (buf);
	return -1;
    }

    while (fgets (buf, sizeof buf, fd))
    {
	if (sscanf (buf, "%ld:%ld", &cat, &count) != 2)
	{
	    G_free_histogram (histogram);
	    fclose (fd);
	    sprintf (buf,"Invalid histogram file for [%s in %s]", name, mapset);
	    G_warning (buf);
	    return -1;
	}
	G_extend_histogram ((CELL)cat, count, histogram);
    }
    fclose (fd);
    if (histogram->num == 0)
    {
	sprintf (buf,"Invalid histogram file for [%s in %s]", name, mapset);
	G_warning (buf);
	return -1;
    }

    G_sort_histogram  (histogram);
    return 1;
}

G_write_histogram (name, histogram)
    char *name;
    struct Histogram *histogram;
{
    FILE *fd;
    int n;
    LIST *list;

    fd = fopen_histogram_new (name);
    if (fd == NULL)
	return -1;

    list = histogram->list;
    for (n = 0; n < histogram->num; n++)
    {
	if (list[n].count)
	    fprintf (fd, "%ld:%ld\n", (long)list[n].cat, list[n].count);
    }
    fclose (fd);
    return 1;
}

G_write_histogram_cs (name, statf)
    char *name;
    struct Cell_stats *statf;
{
    FILE *fd;
    CELL cat;
    long count;

    fd = fopen_histogram_new (name);
    if (fd == NULL)
	return -1;
    G_rewind_cell_stats (statf);
    while (G_next_cell_stat (&cat, &count, statf))
    {
	if (count > 0)
	    fprintf (fd, "%ld:%ld\n", (long)cat, count);
    }
    fclose (fd);
    return 1;
}

G_make_histogram_cs (statf, histogram)
    struct Cell_stats *statf;
    struct Histogram *histogram;
{
    CELL cat;
    long count;

    G_init_histogram (histogram);
    G_rewind_cell_stats (statf);
    while (G_next_cell_stat (&cat, &count, statf))
	G_add_histogram (cat, count, histogram);
    G_sort_histogram (histogram);
}

G_get_histogram_num (histogram)
    struct Histogram *histogram;
{
    return histogram->num;
}

CELL
G_get_histogram_cat (n, histogram)
    struct Histogram *histogram;
{
    if (n < 0 || n >= histogram->num)
	return 0;
    return histogram->list[n].cat;
}

long
G_get_histogram_count (n, histogram)
    struct Histogram *histogram;
{
    if (n < 0 || n >= histogram->num)
	return 0;
    return histogram->list[n].count;
}

G_free_histogram (histogram)
    struct Histogram *histogram;
{
    if (histogram->num > 0)
	free (histogram->list);
    histogram->num = 0;
    histogram->list = NULL;
    return 1;
}

G_sort_histogram (histogram)
    struct Histogram *histogram;
{
    int cmp();
    int a,b,n;
    LIST *list;

/* if histogram only has 1 entry, nothing to do */
    if ((n = histogram->num) <= 1) return;

    list=histogram->list;

/* quick check to see if sorting needed */
    for (a = 1; a < n; a++)
	if (list[a-1].cat >= list[a].cat)
	    break;
    if (a >= n) return;

/* sort */
    qsort (list, n, sizeof(LIST), cmp);

/* sum duplicate entries */
    for (a = 0, b = 1; b < n; b++)
	if (list[a].cat != list[b].cat)
	{
	    a++;
	    list[a].count = list[b].count;
	    list[a].cat   = list[b].cat;
	}
	else
	{
	    list[a].count += list[b].count;
	}
    histogram->num = a + 1;
}

static
cmp (a,b)
    LIST *a, *b;
{
    if (a->cat < b->cat)
	return -1;
    if (a->cat > b->cat)
	return 1;
    return 0;
}

G_sort_histogram_by_count (histogram)
    struct Histogram *histogram;
{
    int cmp_count();
    int n;
    LIST *list;

/* if histogram only has 1 entry, nothing to do */
    if ((n = histogram->num) <= 1) return;

    list=histogram->list;

/* sort */
    qsort (list, n, sizeof(LIST), cmp_count);
}

static
cmp_count (a,b)
    LIST *a, *b;
{
    if(a->count < b->count)
	return -1;
    if(a->count > b->count)
	return 1;
    if(a->cat < b->cat)
	return -1;
    if(a->cat > b->cat)
	return 1;
    return 0;
}

static
FILE *
fopen_histogram_new (name)
    char *name;
{
    char buf[100];
    FILE *fd;

    sprintf (buf,"cell_misc/%s", name);
    fd = G_fopen_new (buf, "histogram");
    if (fd == NULL)
    {
	sprintf (buf,"can't create histogram for [%s in %s]", name, G_mapset());
	G_warning (buf);
    }
    return fd;
}

G_add_histogram (cat, count, histogram)
    CELL cat;
    long count;
    struct Histogram *histogram;
{
    int i;

    for (i = 0 ; i < histogram->num; i++)
	if (histogram->list[i].cat == cat)
	{
	    histogram->list[i].count += count;
	    return;
	}

    G_extend_histogram (cat, count, histogram);
}

G_set_histogram (cat, count, histogram)
    CELL cat;
    long count;
    struct Histogram *histogram;
{
    int i;

    for (i = 0 ; i < histogram->num; i++)
	if (histogram->list[i].cat == cat)
	{
	    histogram->list[i].count = count;
	    return;
	}

    G_extend_histogram (cat, count, histogram);
}

G_extend_histogram (cat, count, histogram)
    CELL cat;
    long count;
    struct Histogram *histogram;
{
    histogram->num++;
    histogram->list =
	(LIST *) G_realloc (histogram->list, histogram->num*sizeof (LIST));
    histogram->list[histogram->num-1].cat = cat;
    histogram->list[histogram->num-1].count = count;
}

G_zero_histogram (histogram)
    struct Histogram *histogram;
{
    int i;

    for (i = 0; i < histogram->num; i++)
	histogram->list[i].count = 0;
}
