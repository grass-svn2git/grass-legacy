/* main.c */

#define GLOBAL
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "raster.h"
#include "imagery.h"
#include "ortholib.h"
#include "globals.h"
#include "local_proto.h"

int main (int argc, char *argv[])
{
    char *name, *location, *mapset, *camera, msg[100];
    struct Cell_head cellhd;
    int ok;
    int nfiles;

    if (argc != 2)
    {
	fprintf (stderr, "usage: %s group\n", argv[0]);
	exit(1);
    }

    G_gisinit (argv[0]);
    G_suppress_masking();	/* need to do this for target location */
    name     = (char *) G_malloc (40 * sizeof (char));
    location = (char *) G_malloc (80 * sizeof (char));
    mapset   = (char *) G_malloc (80 * sizeof (char));
    camera   = (char *) G_malloc (40 * sizeof (char));

    interrupt_char = G_intr_char();
    tempfile1 = G_tempfile();
    tempfile2 = G_tempfile();
    tempfile_dot = G_tempfile();
    cell_list = G_tempfile();
    vect_list = G_tempfile();
    group_list = G_tempfile();
    digit_points = G_tempfile();

    if (R_open_driver() != 0)
	G_fatal_error ("No graphics device selected!!!");

    /* get group ref */
    name = argv[1];
    strcpy (group.name, name);
    if (!I_find_group (group.name))
    {
	fprintf (stderr, "Group [%s] not found\n", group.name);
	exit(1);
    }


    /* get the group ref */    
    I_get_group_ref (group.name, &group.group_ref);
    nfiles = group.group_ref.nfiles;

    /* write block files to block list file */
    prepare_group_list();

    /** look for camera info  for this group**/
    G_suppress_warnings(1);
    if (!I_get_group_camera (group.name,camera))
    {   sprintf (msg, "No camera reference file selected for group [%s]\n", 
		group.name);
	G_fatal_error(msg);
    }

    if (!I_get_cam_info (camera, &group.camera_ref))
    {   sprintf (msg, "Bad format in camera file for group [%s]\n",group.name);
	G_fatal_error(msg);
    }
    G_suppress_warnings(0);

    /* get initial camera exposure station, if any*/
    if (! (ok = I_find_initial(group.name)))
    {
       sprintf (msg, "No initial camera exposure station for group[%s]\n",
		group.name);
       G_warning(msg);
    }

    if (ok)
      if (!I_get_init_info (group.name, &group.camera_exp))
      {
         sprintf (msg, "Bad format in initial camera exposure station for group [%s]\n", group.name);
         G_warning (msg);
      }

    /* get target info and enviroment */
    G_suppress_warnings(1);
    get_target();
    find_target_files();
    G_suppress_warnings(0);

    /* read group reference points, if any */
    G_suppress_warnings(1);
    if (!I_get_ref_points (group.name, &group.photo_points))
      {
        G_suppress_warnings(0);
        if (group.photo_points.count == 0)
           sprintf (msg, "No photo points for group [%s].\n", group.name);
        else if (group.ref_equation_stat == 0)
           sprintf (msg, "Poorly placed photo points for group [%s].\n", 
		group.name);
        G_fatal_error (msg);
      }
    G_suppress_warnings(0);

    /* determine tranformation equation */
    Compute_ref_equation(); 

    /* read group control points, format: image x,y,cfl; target E,N,Z*/
    G_suppress_warnings(1);
    if (!I_get_con_points (group.name, &group.control_points))
	group.control_points.count = 0;
    G_suppress_warnings(0);

    /* compute image coordinates of photo control points */
    /********
    I_convert_con_points (group.name, &group.control_points, 
			 &group.control_points, group.E12, group.N12);
    ********/

    /* determine tranformation equation */
    fprintf (stderr,"Computing equations ...\n");
    if (group.control_points.count > 0)
        Compute_ortho_equation();


/*   signal (SIGINT, SIG_IGN); */
/*   signal (SIGQUIT, SIG_IGN); */

    select_current_env();
    Init_graphics();
    display_title (VIEW_MAP1);
    select_target_env ();
    display_title (VIEW_MAP2);
    select_current_env ();

    Begin_curses();
    G_set_error_routine (error);

/*
#ifdef SIGTSTP
    signal (SIGTSTP, SIG_IGN);
#endif
*/


    /* ask user for group file to be displayed */
    do
    {
	if(!choose_groupfile (name, mapset))
	    quit(0);
        /* display this file in "map1" */
    } while (G_get_cellhd (name, mapset, &cellhd) < 0);


    G_adjust_window_to_box (&cellhd, &VIEW_MAP1->cell.head, VIEW_MAP1->nrows, 
			    VIEW_MAP1->ncols);
    Configure_view (VIEW_MAP1, name, mapset, cellhd.ns_res, cellhd.ew_res);

    drawcell(VIEW_MAP1);
    display_conz_points(1);

    Curses_clear_window (PROMPT_WINDOW);

    /* determine initial input method. */
    setup_digitizer();
    if (use_digitizer)
    {
	from_digitizer = 1;
	from_keyboard  = 0;
	from_flag = 1;
    }

    /* go do the work */
    driver();

    quit(0);
}

int quit (int n)
{
    char command[1024];

    End_curses();
    R_close_driver();
    if (use_digitizer)
    {
	sprintf (command, "%s/etc/geo.unlock %s",
	    G_gisbase(), digit_points);
	system (command);
    }
    unlink (tempfile1);
    unlink (tempfile2);
    unlink (cell_list);
    unlink (group_list);
    unlink (vect_list);
    unlink (digit_points);
    unlink (tempfile_elev);
    unlink (tempfile_dot);
    exit(n);
}

int error (char *msg, int fatal)
{
    char buf[200];
    int x,y,button;

Curses_clear_window (PROMPT_WINDOW);
Curses_write_window (PROMPT_WINDOW,1,1, "LOCATION:\n");
Curses_write_window (PROMPT_WINDOW,1,12,G_location());
Curses_write_window (PROMPT_WINDOW,2,1, "MAPSET:\n");
Curses_write_window (PROMPT_WINDOW,2,12,G_mapset());
    Beep();
    if (fatal)
	sprintf (buf, "ERROR: %s", msg);
    else
	sprintf (buf, "WARNING: %s (click mouse to continue)", msg);
    Menu_msg (buf);

    if (fatal)
	quit(1);
    Mouse_pointer (&x, &y, &button);
Curses_clear_window (PROMPT_WINDOW);

    return 0;
}
