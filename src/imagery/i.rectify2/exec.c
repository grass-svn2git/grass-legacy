#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "global.h"

int exec_rectify (int order)          /* ADDED WITH CRS MODIFICATIONS */
{
    char *name;
    char *mapset;
    char *result;
    char *type;
    int i,n;
    struct Colors colr;
    struct Categories cats;
    struct History hist;
    int colr_ok, hist_ok, cats_ok;
    long start_time, rectify_time, compress_time;
    char *mailfile;

/* allocate the output cell matrix */
    cell_buf = (void **) G_calloc (NROWS, sizeof(void *));
    n = NCOLS * G_raster_size(map_type);
    for (i=0; i < NROWS; i++)
    {
	cell_buf[i] = (void *) G_malloc (n);
	G_set_null_value(cell_buf[i], NCOLS, map_type);
    }


/* go into background */
    fprintf (stderr, "\nYou will receive mail when %s is complete\n",
	G_program_name());
    if (G_fork()) exit(0);

/* note: all calls to G_tempfile() should happen after the fork */

/* create a mailfile */
    mailfile = G_tempfile();
    unlink (mailfile);
    close(creat(mailfile,0666));

/* open stderr to /dev/null so all GRASS error messages will be
 * mailed to the user
 */
    freopen ("/dev/null","w",stderr);
    freopen ("/dev/null","w",stdout);

/* rectify each file */
    for (n = 0; n < ref.nfiles; n++)
    {
	if ((i = ref_list[n]) < 0)
	    continue;
	name   = ref.file[i].name;
	mapset = ref.file[i].mapset;
	result = new_name[n];

	select_current_env();

	G_suppress_warnings(1);
	cats_ok = G_read_cats (name, mapset, &cats) >= 0;
	colr_ok = G_read_colors (name, mapset, &colr) > 0;
/*
	hist_ok = G_read_history (name, mapset, &hist) >= 0;
*/
	/* Initialze History */
        type = "raster";
        G_short_history(name, type, &hist);

	G_suppress_warnings(0);

	time (&start_time);

        if (rectify (name, mapset, result,order))
	{
	    select_target_env();
          G_put_cellhd (result,&target_window);
	    if(cats_ok)
	    {
		G_write_cats (result, &cats);
		G_free_cats (&cats);
	    }
	    if(colr_ok)
	    {
		G_write_colors (result, G_mapset(), &colr) ;
		G_free_colors (&colr);
	    }

                /* Write out History Structure History */
                sprintf(hist.title, "%s", result);
                sprintf(hist.datsrc_1, "%s", name);
                sprintf(hist.edhist[0], "Created from: i.rectify2");
		sprintf(hist.edhist[1], "Transformation order = %d", order);
                hist.edlinecnt = 2;
                G_write_history (result, &hist) ;

	    select_current_env();
	    time (&rectify_time);
	    if (compress(result))
		time (&compress_time);
	    else
		compress_time = rectify_time;
	    report (mailfile, name, mapset, result, rectify_time-start_time, compress_time-rectify_time, 1);
	}
	else
	    report (mailfile, name, mapset, result, (long)0, (long)0, 0);
    }
    mail (mailfile);
    unlink (mailfile);
    G_done_msg ("Check your mail");

    return 0;
}
