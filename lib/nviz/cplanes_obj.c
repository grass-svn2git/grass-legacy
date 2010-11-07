/*!
   \file lib/nviz/cplanes_obj.c
   
   \brief Nviz library -- Clip planes manipulation
   
   Based on visualization/nviz/src/cutplanes_obj.c

   (C) 2008, 2010 by the GRASS Development Team
   This program is free software under the GNU General Public License
   (>=v2). Read the file COPYING that comes with GRASS for details.

   \author Updated/modified by Martin Landa <landa.martin gmail.com> (Google SoC 2008/2010)
 */

#include <grass/nviz.h>

static void cp_draw(nv_data *, int, int, int);

/*!
   \brief Creates a clip plane object

   The number of clip planes is fixed (MAX_CPLANES) and
   we'll create them all ahead of time anyway we just let
   the user decide on the id for each.
 */
int Nviz_new_cplane(nv_data * data, int id)
{
    data->num_cplanes++;
    /* Initialize internal attributes for this cutplane */
    data->cp_rot[id][X] = data->cp_rot[id][Y] = data->cp_rot[id][Z] = 0.0;
    data->cp_trans[id][X] = data->cp_trans[id][Y] = data->cp_trans[id][Z] =
	0.0;
    data->cp_on[id] = 0;

    return 1;
}

/*!
   \brief Turn off (make inactive) the given clip plane

   \param data nviz data
   \param cplane id
 */
int Nviz_off_cplane(nv_data * data, int id)
{
    data->cp_on[id] = 0;
    GS_unset_cplane(id);

    return 1;
}

/*!
   \brief Draw the clip plane

   \param bound1
   \param bound2
 */
int Nviz_draw_cplane(nv_data * data, int bound1, int bound2)
{
    cp_draw(data, data->cur_cplane, bound1, bound2);

    return 1;
}

/*!
   \brief Draw current clip plane

   \param data nviz data
   \param current id of current clip plane
   \param surf1 first surface id
   \param surf2 second surface id
 */
void cp_draw(nv_data * data, int current, int surf1, int surf2)
{
    int i, nsurfs;
    int surf_min = 0, surf_max = 0, temp;
    int *surf_list;

    GS_set_draw(GSD_BACK);
    GS_clear(data->bgcolor);
    GS_ready_draw();

    /* If surf boundaries present then find them */
    surf_list = GS_get_surf_list(&nsurfs);
    if ((surf1 != -1) && (surf2 != -1)) {
	for (i = 0; i < nsurfs; i++) {
	    if (surf_list[i] == surf1)
		surf_min = i;
	    if (surf_list[i] == surf2)
		surf_max = i;
	}

	if (surf_max < surf_min) {
	    temp = surf_min;
	    surf_min = surf_max;
	    surf_max = temp;
	}

	surf_max++;
    }
    else {
	surf_min = 0;
	surf_max = nsurfs;
    }

    if (nsurfs > 1) {
	for (i = 0; i < MAX_CPLANES; i++) {
	    if (data->cp_on[i])
		GS_draw_cplane_fence(surf_list[0], surf_list[1], i);
	}
    }

    for (i = surf_min; i < surf_max; i++) {
	GS_draw_wire(surf_list[i]);
    }

    GS_done_draw();

    return;
}
