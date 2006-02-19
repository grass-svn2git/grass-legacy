/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:       GRASS 5 gis library, list.c
 * AUTHOR(S):    unknown
 * PURPOSE:      list elements
 * COPYRIGHT:    (C) 2000 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *   	    	 License (>=v2). Read the file COPYING that comes with GRASS
 *   	    	 for details.
 *
 *****************************************************************************/

/**********************************************************************
 *  G_list_element (element, desc, mapset, lister)
 *
 *      char *element    database element (eg, "cell", "cellhd", etc)
 *      char *desc       desc for element (if NULL, element is used)
 *      char *mapset     mapset to be listed.
 *                       "" to list all mapsets in mapset search list
 *                       "." will list current mapset
 *
 *      int (*lister)()  if given will call this routine to get a list
 *                       title. NULL if no titles desired.
 *
 *                       lister (name, mapset, buf)
 *                       char *name, *mapset, *buf
 *
 *                       given file 'name', and 'mapset',
 *                       lister() should copy a string into 'buf'
 *
 *                       when called with name == "", should set buf
 *                       to general title for mapset list.
 *
 *   General purpose list function. Will list files from all mapsets
 *   in the mapset list for a specified database element.
 *
 *   note:
 *      output is to stdout piped thru the more utility
 *********************************************************************/

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <grass/gis.h>
#include <grass/glocale.h>

static int broken_pipe;
static int hit_return = 0;
static int list_element(FILE *,char *,char *,char *,int (*)());
static void sigpipe_catch(int);

int G_set_list_hit_return(int flag)
{
    hit_return = flag;
    return 0;
}

int G_list_element (
    char *element,
    char *desc,
    char *mapset,
    int (*lister)())
{
    int n;
    FILE *more;
    int count;
#ifdef SIGPIPE
    void (*sigpipe)();
#endif

/* must catch broken pipe in case "more" quits */
    broken_pipe = 0;
#ifdef SIGPIPE
    sigpipe = signal (SIGPIPE, sigpipe_catch);
#endif

    count = 0;
    if (desc == 0 || *desc == 0)
	desc = element;
/*
 * G_popen() the more command to page the output
 */
    if (isatty(1))
    {
	more = G_popen ("$GRASS_PAGER","w");
	if (!more) more = stdout;
    }
    else
	more = stdout;
    fprintf (more,"----------------------------------------------\n");

/*
 * if no specific mapset is requested, list the mapsets
 * from the mapset search list
 * otherwise just list the specified mapset
 */
    if (mapset == 0 || *mapset == 0)
	for (n = 0; !broken_pipe && (mapset = G__mapset_name (n)); n++)
	    count += list_element (more, element, desc, mapset, lister);
    else
	count += list_element (more, element, desc, mapset, lister);

    if (!broken_pipe)
    {
	if (count == 0){
	   if (mapset == 0 || *mapset == 0)
	    fprintf (more,_("no %s files available in current mapset\n"), desc);
	   else
	    fprintf (more,_("no %s files available in mapset %s\n"), desc, mapset);
	}

	fprintf (more,"----------------------------------------------\n");
    }
/*
 * close the more
 */
    if (more != stdout) G_pclose (more);
#ifdef SIGPIPE
    signal (SIGPIPE, sigpipe);
#endif
if (hit_return && isatty(1))
    {
	fprintf (stderr, _("hit RETURN to continue -->"));
	while (getchar() != '\n')
	    ;
    }

    return 0;
}

static void sigpipe_catch(int n)
{
    broken_pipe = 1;
    signal (n,sigpipe_catch);
}

static int list_element( FILE *out, char *element,
    char *desc, char *mapset, int (*lister)())
{
    char path[1000];
    DIR *dirp;
    struct dirent *dp;
    int count = 0, maxlen = 0, num_cols = 1;

/*
 * convert . to current mapset
 */
    if (strcmp (mapset,".") == 0)
	mapset = G_mapset();


/*
 * get the full name of the GIS directory within the mapset
 * and list its contents (if it exists)
 *
 * if lister() routine is given, the ls command must give 1 name
 */
    G__file_name (path, element, "", mapset);
    if(access(path, 0) == 0)
    {
/*
 * if a title so that we can call lister() with the names
 * otherwise the ls must be forced into columnar form.
 */

	if((dirp = opendir(path)) == NULL)
		G_fatal_error("ERROR: %s: open failed.", path);

	if(!lister)
	{
		int i;

		while((dp = readdir(dirp)) != NULL)
		{
			if(dp->d_name[0] == '.')
				continue;
			if(maxlen < (i = strlen(dp->d_name)))
				maxlen = i;
		}
		rewinddir(dirp);

		num_cols = 80 / (maxlen + 1); /* + 1: column separator */
	}

	while ((dp = readdir(dirp)) != NULL)
	{
	    if(dp->d_name[0] == '.')
		    continue;

	    if (count++ == 0)
	    {
	        fprintf(out, _("%s files available in mapset %s:\n"), desc, mapset);
	        if (lister)
	        {
	    	char title[400];
	    	char name[GNAME_MAX];

	    	*name = *title = 0;
	    	lister (name, mapset, title);
	    	if (*title)
	    	    fprintf(out,"\n%-18s %-.60s\n",name,title);
	        }
	    }
	    if (lister)
	    {
	        char *b;
	        char title[400];

	    /* remove the trailing newline */
	        for (b = dp->d_name; *b; b++)
	    	if (*b == '\n')
	    	    *b = 0;

	        lister (dp->d_name, mapset, title);
	        fprintf(out,"%-18s %-.60s\n",dp->d_name,title);
	    }
	    else
	        fprintf(out,"%-*s", maxlen+1, dp->d_name);
	    if(!lister && !(count % num_cols))
		fprintf(out, "\n");
	}
	closedir(dirp);
    }
    if (!lister && (count % num_cols))
	fprintf(out,"\n");

    return count;
}
