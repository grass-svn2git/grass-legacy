/*----------------------------------------------------------------
 * I_get_ref_points (group_name, cp)
 * I_put_ref_points (group_name, cp)
 * I_new_ref_point
 * 
 * Read or Write the Fiducial Points file  for an
 * imagery group.  Only for ORTHO_PHOTO transfomrations.
 *
 * The fiducial points file has the format:
 *
 *     source_image              photo_coords           status
 *  col (x)     row (y)          (x)      (y)           (1=ok)
 *
 *  
 * 
 *  I_get_ref_points
 *       RETURNS:    1  everything read fine..
 *                   0  file doesn't exist.
 *                  -1  bad format in file.  
 ----------------------------------------------------------------*/

#include "ortho_image.h"


#define REF_POINT_FILE "REF_POINTS"


/*----------------------------------------------------------------*/
I_get_ref_points (groupname, cp)
    char *groupname;
    Control_Points_2D *cp;
{
    FILE *fd;
    char msg[100];
    int stat;

    /* Open the REF_POINTS file */
    fd = I_fopen_group_file_old (groupname, REF_POINT_FILE);


    if (fd == NULL)   /* if doesn't exist, initialize ref points */
    {
	sprintf (msg, "unable to open reference point file for group [%s in %s]",
		groupname, G_mapset());

	cp->e1     = NULL;
	cp->n1     = NULL;
	cp->e2     = NULL;
	cp->n2     = NULL;
	cp->status = NULL;
	cp->count  = -1;

	G_warning (msg);
	return 0;
    }

    /* Try to read the file */
    stat = I_read_ref_points (fd, cp);
    fclose (fd);


    if (stat < 0)  /* Bad format in file */
    {
	sprintf (msg, "bad format in reference point file for group [%s in %s]",groupname, G_mapset());
	G_warning (msg);

	return -1;
    }

    /* points were read ok. */
    return 1;
}

/*----------------------------------------------------------------*/
I_put_ref_points (groupname, cp)
    char *groupname;
    Control_Points_2D *cp;
{
    FILE *fd;
    char msg[100];

    fd = I_fopen_group_file_new (groupname, REF_POINT_FILE);
    if (fd == NULL)
    {
	sprintf (msg, "unable to create reference point file for group [%s in %s]", groupname, G_mapset());
	G_warning (msg);
	return 0;
    }

    I_write_ref_points (fd, cp);
    fclose (fd);
    return 1;
}


/*----------------------------------------------------------------*/
I_new_ref_point (cp, e1, n1, e2, n2, status)
    Control_Points_2D *cp;
    double e1,n1,e2,n2;
    int status;
{
    int i;
    unsigned int size;

/*fprintf (stderr, "Try to new_ref_point \n");*/
    if (status < 0) return;
    i = (cp->count)++ ;
    size =  cp->count * sizeof(double) ;
    cp->e1 = (double *) G_realloc (cp->e1, size);
    cp->e2 = (double *) G_realloc (cp->e2, size);
    cp->n1 = (double *) G_realloc (cp->n1, size);
    cp->n2 = (double *) G_realloc (cp->n2, size);
    size =  cp->count * sizeof(int) ;
    cp->status = (int *) G_realloc (cp->status, size);

    cp->e1[i] = e1;
    cp->e2[i] = e2;
    cp->n1[i] = n1;
    cp->n2[i] = n2;
    cp->status[i] = status;
}


/*----------------------------------------------------------------*/
I_read_ref_points (fd, cp)
    FILE *fd;
    Control_Points_2D *cp;
{
    char buf[100];
    double e1,e2,n1,n2;
    int status;

    cp->count = 0;

/* read the reference point lines. format is:
   image_east image_north  photo_x photo_y  status(1=ok)
*/
    cp->e1 = NULL;
    cp->e2 = NULL;
    cp->n1 = NULL;
    cp->n2 = NULL;
    cp->status = NULL;

/*fprintf (stderr, "Try to read one point \n");*/
    while (G_getl (buf, sizeof buf, fd))
    {
	G_strip(buf);
	if (*buf == '#' || *buf == 0) continue;
	if (sscanf (buf, "%lf%lf%lf%lf%d", &e1, &n1, &e2, &n2, &status) == 5)
	    I_new_ref_point (cp, e1, n1, e2, n2, status);
	else
	    return -4;
    }

    return 1;
}

/*----------------------------------------------------------------*/
I_write_ref_points (fd, cp)
    FILE *fd;
    Control_Points_2D *cp;
{
    int i;

    fprintf (fd,"# %7s %15s %15s %15s %9s status\n","","image","","photo","");
    fprintf (fd,"# %15s %15s %15s %15s   (1=ok)\n","east","north","x","y");
    fprintf (fd,"#\n");
    for (i = 0; i < cp->count; i++)
	if (cp->status[i] >= 0)
	    fprintf (fd, "  %15lf %15lf %15lf %15lf %d\n",
		cp->e1[i], cp->n1[i], cp->e2[i], cp->n2[i], cp->status[i]);
}




