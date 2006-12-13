/* Function: groupfile
**
** Author: Paul W. Carlson	July 1992
*/

#include <string.h>
#include "ps_info.h"
#include "group.h"
#include "local_proto.h"

int read_group (void)
{
    int i;
    char fullname[100];

    PS.do_raster = 0;
    PS.do_colortable = 0;
    if (PS.cell_fd >= 0)
    {
	G_close_cell(PS.cell_fd);
	G_free(PS.cell_name);
	G_free(PS.cell_mapset);
	G_free_colors(&PS.colors);
	PS.cell_fd = -1;
    }

    /* get group info */
    I_init_group_ref(&grp.ref);
    if (I_get_group_ref(grp.group_name, &grp.ref) == 0)
	G_fatal_error("Can't get group information");

    /* get file names for R, G, & B */
    I_init_ref_color_nums(&grp.ref);
    grp.name[0] = G_store(grp.ref.file[grp.ref.red.n].name);
    grp.name[1] = G_store(grp.ref.file[grp.ref.grn.n].name);
    grp.name[2] = G_store(grp.ref.file[grp.ref.blu.n].name);
    grp.mapset[0] = G_store(grp.ref.file[grp.ref.red.n].mapset);
    grp.mapset[1] = G_store(grp.ref.file[grp.ref.grn.n].mapset);
    grp.mapset[2] = G_store(grp.ref.file[grp.ref.blu.n].mapset);


    /* read in colors */
    for (i = 0; i < 3; i++)
    {
    	if (G_read_colors(grp.name[i], grp.mapset[i], &(grp.colors[i])) == -1)
    	{
    	    sprintf(fullname, "%s in %s", grp.name[i], grp.mapset[i]);
            error(fullname, "", "can't read color table");
            return 0;
    	}
    }

    /* open cell files for reading */
    for (i = 0; i < 3; i++)
    {
        if ((grp.fd[i] = G_open_cell_old(grp.name[i], grp.mapset[i])) < 0)
        {
    	    sprintf(fullname, "%s in %s", grp.name[i], grp.mapset[i]);
            error(fullname, "", "can't open raster map");
	    G_free_colors(&(grp.colors[i]));
            return 0;
        }
    }

    strcpy(PS.celltitle, grp.group_name);
    G_strip(PS.celltitle);
    return 1;
}
