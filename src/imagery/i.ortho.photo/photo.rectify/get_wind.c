#include "global.h"
/* FILE *Bugsr; */

get_target_window()
{
    char name[30], mapset[30];
    struct Cell_head cellhd;

    printf ("\n\n");
    while(1)
    {
	char buf[100];
	printf ("Please select one of the following options\n");
	printf (" 1. Use the current window in the target location\n");
	printf (" 2. Determine the smallest window which covers the image\n");
	printf ("> ");
	if (!G_gets(buf)) continue;
	G_strip (buf);
	if (strcmp (buf,"1") == 0) 
	  { /**ask_window (&target_window);**/ return;}
	if (strcmp (buf,"2") == 0) break;
    }
    ask_file_from_list (name, mapset);

#ifdef DEBUG3
    fprintf (Bugsr,"ask_file: %s in %s \n",name,mapset);
#endif

    if (G_get_cellhd (name, mapset, &cellhd) < 0)
	exit(-1);

#ifdef DEBUG3
    fprintf (Bugsr,"current window: n s = %f %f, \n",cellhd.north, cellhd.south);
    fprintf (Bugsr,"current window: w e = %f %f, \n",cellhd.west,  cellhd.east);
    fflush  (Bugsr);
#endif

    georef_window (&cellhd, &target_window);
    ask_window (&target_window);

/**
    if(!G_yes("Would you like this window saved as the window in the target location?\n", -1))
	return;
**/
    select_target_env();
    if(G_put_window (&target_window)>=0)
	printf ("Window Saved!\n");
    select_current_env();
}

georef_window (w1, w2)
    struct Cell_head *w1, *w2;
{
    double n,e,z1;
    double n0,e0; 
    double aver_z;


    /* get an average elevation from the active control points */    
    get_aver_elev (&group.control_points,&aver_z);
#ifdef DEBUG3
    fprintf (Bugsr, "Aver elev = %f \n",aver_z);
#endif

    /* compute ortho ref of all corners */

    I_georef (w1->west, w1->north, &e0, &n0, group.E12, group.N12);
    I_inverse_ortho_ref (e0,n0,aver_z, &e, &n, &z1, &group.camera_ref, group.XC, group.YC, group.ZC, group.omega, group.phi, group.kappa);

#ifdef DEBUG3
    fprintf (Bugsr,"NORTH WEST CORNER\n");
    fprintf (Bugsr,"group.E12 = %f %f %f, \n",group.E12[0],group.E12[1],group.E12[2]);
    fprintf (Bugsr,"group.N12 = %f %f %f, \n",group.N12[0],group.N12[1],group.N12[2]);
    fprintf (Bugsr,"image  x = %f y = %f, photo x = %f y = %f \n",w1->west,w1->north,e0,n0);
    fprintf (Bugsr,"target x = %f y = %f \n", e,n);
    fflush  (Bugsr);
#endif

    w2->north = w2->south = n;
    w2->west  = w2->east  = e;

    I_georef (w1->east, w1->north, &e0, &n0, group.E12, group.N12);
    I_inverse_ortho_ref (e0,n0,aver_z, &e, &n, &z1, &group.camera_ref, group.XC, group.YC, group.ZC, group.omega, group.phi, group.kappa);

#ifdef DEBUG3
    fprintf (Bugsr,"NORTH EAST CORNER\n");
    fprintf (Bugsr,"image  x = %f y = %f, photo x = %f y = %f \n",w1->east,w1->north,e0,n0);
    fprintf (Bugsr,"target x = %f y = %f \n", e,n);
    fflush  (Bugsr);
#endif

    if (n > w2->north) w2->north = n;
    if (n < w2->south) w2->south = n;
    if (e > w2->east ) w2->east  = e;
    if (e < w2->west ) w2->west  = e;

    I_georef (w1->west, w1->south, &e0, &n0, group.E12, group.N12);
    I_inverse_ortho_ref (e0,n0,aver_z, &e, &n, &z1, &group.camera_ref, group.XC, group.YC, group.ZC, group.omega, group.phi, group.kappa);

#ifdef DEBUG3
    fprintf (Bugsr,"SOUTH WEST CORNER\n");
    fprintf (Bugsr,"image  x = %f y = %f, photo x = %f y = %f \n",w1->west,w1->south,e0,n0);
    fprintf (Bugsr,"target x = %f y = %f \n", e,n);
    fflush  (Bugsr);
#endif

    if (n > w2->north) w2->north = n;
    if (n < w2->south) w2->south = n;
    if (e > w2->east ) w2->east  = e;
    if (e < w2->west ) w2->west  = e;

    I_georef (w1->east, w1->south, &e0, &n0, group.E12, N12);
    I_inverse_ortho_ref (e0,n0,aver_z, &e, &n, &z1, &group.camera_ref, group.XC, group.YC, group.ZC, group.omega, group.phi, group.kappa);

#ifdef DEBUG3
    fprintf (Bugsr,"SOUTH EAST CORNER\n");
    fprintf (Bugsr,"image  x = %f y = %f, photo x = %f y = %f \n",w1->east,w1->south,e0,n0);
    fprintf (Bugsr,"target x = %f y = %f \n", e,n);
    fflush  (Bugsr);
#endif

    if (n > w2->north) w2->north = n;
    if (n < w2->south) w2->south = n;
    if (e > w2->east ) w2->east  = e;
    if (e < w2->west ) w2->west  = e;

    w2->ns_res = (w2->north - w2->south) / w1->rows;
    w2->ew_res = (w2->east  - w2->west ) / w1->cols;

#ifdef DEBUG3
    fprintf (Bugsr,"FINAL\n");
    fprintf (Bugsr,"east = %f \n west = %f \n north = %f \n south = %f \n",w2->east,w2->west,w2->north,w2->south);

    fprintf (Bugsr,"RESOLUTION\n");
    fprintf (Bugsr,"EW = %f\n",w2->ew_res);
    fprintf (Bugsr,"NS = %f\n",w2->ns_res);
    fflush  (Bugsr);
#endif

}

