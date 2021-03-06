#define GLOBAL
#include "global.h"

main(argc, argv) char *argv[];
{
    char *name, *location, *mapset, *camera, msg[100];
    int n, nfiles, subnfiles, ok;
    char tl[100];
    char math_exp[100];
    char units[100];
    char nd[100];

#ifdef DEBUG3
    Bugsr = fopen ("ortho_rectify.rst","w");
    if (Bugsr == NULL)   {
       sprintf (msg, "Cant open debug file ortho_rectify\n");
       G_fatal_error (msg);
    }
#endif

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
    elev_layer  = (char *) G_malloc (40*sizeof (char),1);
    mapset_elev = (char *) G_malloc (40*sizeof (char),1);

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
    if (nfiles <= 0)
    {
	sprintf (msg, "No files in this group!\n");
	G_fatal_error (msg);
    }
    ref_list = (int *) G_malloc (nfiles * sizeof(int));
    new_name = (char **) G_malloc (nfiles * sizeof(char *));
    for (n = 0; n < nfiles; n++)
	ref_list[n] = -1;

    /* get the target */
    get_target(group.name);

    /* ask for files to be rectified */
    ask_files (group.name);

#ifdef DEBUG3
    fprintf (Bugsr,"Looking for elevation file in group: %s\n", group.name);
#endif

    /* get the block elevation layer cell file  in target location */
    I_get_group_elev (group.name, elev_layer, mapset_elev, tl, math_exp, 
			 units, nd);

#ifdef DEBUG3
    fprintf (Bugsr,"Block elevation: %s in %s\n", elev_layer, mapset_elev);
#endif

    /* get the elevation layer header in target location */
    select_target_env(); 
    G_get_cellhd (elev_layer, mapset_elev, &elevhd);
    select_current_env();

    /** look for camera info  for this block **/
    if (!I_get_group_camera (group.name,camera))
    {   sprintf (msg, "No camera reference file selected for group [%s]\n", 
		group.name);
	G_fatal_error(msg);
    }
    if (!I_get_cam_info (camera, &group.camera_ref))
    {   sprintf (msg, "Bad format in camera file for group [%s]\n", group.name);
	G_fatal_error(msg);
    }

    /* get initial camera exposure station, if any*/
    if (I_find_initial(group.name))
    {
       if (!I_get_init_info (group.name, &group.camera_exp))
       {
	  sprintf (msg, "Bad format in initial exposusre station file for group [%s]\n", group.name);
	  G_warning (msg);
       }
    }

    /* read the reference points for the group, compute image-to-photo trans. */
    get_ref_points ();

    /* read the control points for the group, convert to photo coords. */
    get_conz_points ();

    /* ask for window to be used in target location */
    select_current_env();
    get_target_window();

    /* modify elevation values if needed */
    /***
    select_target_env();
    ask_elev_data();
    select_current_env();
    ***/ 

    /*  go do it */
    exec_rectify ();
}




