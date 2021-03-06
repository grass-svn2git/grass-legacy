/*
** Written by: Dave Gerdes 5 1988
** US Army Construction Engineering Research Lab
** Modified by: Dave Gerdes 9 1988   portable
** Modified by: Dave Gerdes 1 1990   more portable
*/

#include "digit.h"

#define SUPPORT_PROG "v.support"

/* routines for reading and writing Dig+ structures. */
/* return 0 on success, -1 on failure of whatever kind */
/* if you dont want it written out, then dont call these routines */
/* ie  check for deleted status before calling a write routine */
/*  in as much as it would be nice to hide that code in here,  */
/*  this is a library routine and we chose to make it dependant on */
/*  as few external files as possible */

/*  these routines assume ptr->alloc_lines  is valid */
/*  Make sure it is initialized before calling */

/*  Internally, my default variables for lines/areas/nodes/isles  are type
**  plus_t  which is typedefed as short.  This limits the current version
**  to no more than 32K lines, nodes etc. (excluding points)
**  All in the name of future expansion, I have converted these values to 
**  longs in the dig_plus data file.

**  NOTE: 3.10 changes plus_t to  ints.
**    This assumes that any reasonable machine will use 4 bytes to
**    store an int.  The mapdev code is not guaranteed to work if
**    plus_t is changed to a type that is larger than an int.
*/

dig_x_Rd_P_node (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_node *ptr;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

    if (0 >= dig__fread_port_D (&(ptr->x), 1, fp)) return (-1);
    if (0 >= dig__fread_port_D (&(ptr->y), 1, fp)) return (-1);
    if (0 >= dig__fread_port_P (&(ptr->n_lines  ), 1, fp)) return (-1);

    dig_node_alloc_line (ptr, (int) ptr->n_lines);

    if (ptr->n_lines)	/* Not guaranteed what fread does w/ 0 */
    {
	if (0>=dig__fread_port_P (ptr->lines,   ptr->n_lines, fp)) return(-1);
	if (0>=dig__fread_port_F (ptr->angles,  ptr->n_lines, fp)) return(-1);
    }
    ptr->alive = 1;
    return (0);
}

dig_x_Wr_P_node (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_node *ptr;
{
    if (0 >= dig__fwrite_port_D (&(ptr->x      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->y      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_lines), 1, fp)) return (-1);

    if (ptr->n_lines)
    {
	if (0>= dig__fwrite_port_P (ptr->lines,  ptr->n_lines, fp)) return (-1);
	if (0>= dig__fwrite_port_F (ptr->angles, ptr->n_lines, fp)) return (-1);
    }
    return (0);
}

dig_x_Rd_P_line (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_line *ptr;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

    if (0 >= dig__fread_port_P (&(ptr->N1    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->N2    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->left  ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->right ), 1, fp)) return -1;

    if (0 >= dig__fread_port_D (&(ptr->N     ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->S     ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->E     ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->W     ), 1, fp)) return -1;

    if (0 >= dig__fread_port_L (&(ptr->offset), 1, fp)) return (-1);
    if (0 >= dig__fread_port_P (&(ptr->att   ), 1, fp)) return (-1);

    if (0 >= dig__fread_port_C (&(ptr->type   ), 1, fp)) return (-1);

    return (0);
}

dig_x_Wr_P_line (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_line *ptr;
{
    if (0 >= dig__fwrite_port_P (&(ptr->N1    ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->N2    ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->left  ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->right ), 1, fp)) return (-1);

    if (0 >= dig__fwrite_port_D (&(ptr->N     ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->S     ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->E     ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->W     ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_L (&(ptr->offset), 1, fp)) return (-1);

    if (0 >= dig__fwrite_port_P (&(ptr->att   ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_C (&(ptr->type  ), 1, fp)) return (-1);

    return (0);
}

dig_x_Rd_P_area (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_area *ptr;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

    if (0 >= dig__fread_port_D (&(ptr->N    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->S    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->E    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->W    ), 1, fp)) return -1;

    if (0 >= dig__fread_port_P (&(ptr->att    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->n_lines    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->n_isles    ), 1, fp)) return -1;


    ptr->lines = (plus_t *) dig_falloc (sizeof (plus_t), (int) ptr->n_lines);

    if (ptr->n_lines)
	if (0 >= dig__fread_port_P (ptr->lines, ptr->n_lines, fp)) return -1;
    ptr->alloc_lines = ptr->n_lines;

    /* island stuff */
    ptr->isles = (plus_t *) dig_falloc (sizeof (plus_t), (int) ptr->n_isles);

    if (ptr->n_isles)
	if (0 >= dig__fread_port_P (ptr->isles, ptr->n_isles, fp)) return -1;
    ptr->alloc_isles = ptr->n_isles;

    ptr->alive = 1;
    return (0);
}

dig_x_Wr_P_area (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_area *ptr;
{
    if (0 >= dig__fwrite_port_D (&(ptr->N  ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->S  ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->E  ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->W  ), 1, fp)) return (-1);

    if (0 >= dig__fwrite_port_P (&(ptr->att), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_lines), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_isles), 1, fp)) return (-1);

    if (ptr->n_lines)
	if (0 >= dig__fwrite_port_P (ptr->lines, ptr->n_lines, fp)) return -1;

    if (ptr->n_isles) 		 /* island stuff */
	if (0 >= dig__fwrite_port_P (ptr->isles, ptr->n_isles, fp)) return -1;

    return (0);
}

/* island stuff */
dig_x_Rd_P_isle (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_isle *ptr;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

    if (0 >= dig__fread_port_D (&(ptr->N    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->S    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->E    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->W    ), 1, fp)) return -1;

    if (0 >= dig__fread_port_P (&(ptr->area    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->n_lines ), 1, fp)) return -1;

    ptr->lines = (plus_t *) dig_falloc (sizeof (plus_t), (int) ptr->n_lines);
    if (ptr->n_lines)
	if (0 >= dig__fread_port_P (ptr->lines, ptr->n_lines, fp)) return -1;
    ptr->alloc_lines = ptr->n_lines;

    ptr->alive = 1;
    return (0);
}

dig_x_Wr_P_isle (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_isle *ptr;
{
    if (0 >= dig__fwrite_port_D (&(ptr->N      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->S      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->E      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->W      ), 1, fp)) return (-1);

    if (0 >= dig__fwrite_port_P (&(ptr->area   ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_lines), 1, fp)) return (-1);

    if (ptr->n_lines)
	if (0 >= dig__fwrite_port_P (ptr->lines, ptr->n_lines, fp)) return -1;

    return (0);
}

dig_x_Rd_P_att (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_att *ptr;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

    if (0 >= dig__fread_port_D (&(ptr->x      ), 1, fp)) return -1;
    if (0 >= dig__fread_port_D (&(ptr->y      ), 1, fp)) return -1;
    if (0 >= dig__fread_port_L (&(ptr->offset ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->cat    ), 1, fp)) return -1;
    if (0 >= dig__fread_port_P (&(ptr->index  ), 1, fp)) return -1;
    if (0 >= dig__fread_port_C (&(ptr->type   ), 1, fp)) return -1;

    return (0);
}

dig_x_Wr_P_att (map, ptr, fp)
    struct Map_info *map;
    FILE *fp;
    struct P_att *ptr;
{
    if (0 >= dig__fwrite_port_D (&(ptr->x      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_D (&(ptr->y      ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_L (&(ptr->offset ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_I (&(ptr->cat    ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_P (&(ptr->index  ), 1, fp)) return (-1);
    if (0 >= dig__fwrite_port_C (&(ptr->type   ), 1, fp)) return (-1);

    return (0);
}

dig_x_Rd_Plus_head (map, ptr, fp)
    struct Map_info *map;
    struct Plus_head *ptr;
    FILE *fp;
{
    dig__set_cur_in_head (dig__get_head (map->digit));

rewind (fp);
    if (0 >= dig__fread_port_I (&(ptr->Major       ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_I (&(ptr->Minor       ), 1, fp)) return(-1);


    if (0 >= dig__fread_port_P (&(ptr->n_nodes     ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_lines     ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_areas     ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_atts      ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_isles     ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_llines    ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_alines    ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_P (&(ptr->n_plines    ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_I (&(ptr->n_points    ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_L (&(ptr->Node_offset ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Line_offset ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Area_offset ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Att_offset  ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Isle_offset ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_L (&(ptr->Dig_size    ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Att_size    ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Dig_code    ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Att_code    ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_I (&(ptr->all_areas   ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_I (&(ptr->all_isles   ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_D (&(ptr->snap_thresh ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_D (&(ptr->prune_thresh ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_L (&(ptr->Back_Major ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->Back_Minor ), 1, fp)) return(-1);


    if (0 >= dig__fread_port_L (&(ptr->future3 ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_L (&(ptr->future4 ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_D (&(ptr->F1 ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_D (&(ptr->F2 ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_D (&(ptr->F3 ), 1, fp)) return(-1);
    if (0 >= dig__fread_port_D (&(ptr->F4 ), 1, fp)) return(-1);

    if (0 >= dig__fread_port_C (ptr->Dig_name , HEADSTR, fp)) return(-1);
    if (0 >= dig__fread_port_C (ptr->filler   , HEADSTR, fp)) return(-1);

/* check version numbers */
    if ( ptr->Major != VERSION_MAJOR || 
         (ptr->Major==VERSION_MAJOR && ptr->Minor > VERSION_MAJOR + 5) )
    {
	if(VERSION_MAJOR<ptr->Back_Major || 
	  (VERSION_MAJOR == ptr->Back_Major && VERSION_MINOR < ptr->Back_Minor))
	{
	    fprintf (stderr, "Vector format version (%d.%d) is not known by this release.  EXITING\n",
		ptr->Major, ptr->Minor);
	    fprintf(stderr, "Try running %s to reformat the dig_plus file\n", SUPPORT_PROG);
	    exit (-1);
	}
    }

    return (0);
}

dig_x_Wr_Plus_head (map, ptr, fp)
    struct Map_info *map;
    struct Plus_head *ptr;
    FILE *fp;
{
    /* is there a better place for this? */
    ptr->Major = VERSION_MAJOR;
    ptr->Minor = VERSION_MINOR;

    rewind (fp);

    if (0 >= dig__fwrite_port_I (&(ptr->Major       ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_I (&(ptr->Minor       ), 1, fp)) return(-1);


    /* force to longs for future */
    if (0 >= dig__fwrite_port_P (&(ptr->n_nodes     ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_lines     ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_areas     ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_atts      ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_isles     ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_llines    ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_alines    ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_P (&(ptr->n_plines    ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_I (&(ptr->n_points    ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_L (&(ptr->Node_offset ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Line_offset ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Area_offset ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Att_offset  ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Isle_offset ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_L (&(ptr->Dig_size    ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Att_size    ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Dig_code    ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->Att_code    ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_I (&(ptr->all_areas   ), 1, fp)) return(-1);

  /*if (0 >= dig__fwrite_port_I (&(ptr->all_areas   ), 1, fp)) return(-1);3.1?*/
    if (0 >= dig__fwrite_port_I (&(ptr->all_isles   ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_D (&(ptr->snap_thresh ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_D (&(ptr->prune_thresh ), 1, fp)) return(-1);

	ptr->Back_Major = EARLIEST_MAJOR;
    if (0 >= dig__fwrite_port_L (&(ptr->Back_Major ), 1, fp)) return(-1);
	ptr->Back_Minor = EARLIEST_MINOR;
    if (0 >= dig__fwrite_port_L (&(ptr->Back_Minor ), 1, fp)) return(-1);


    if (0 >= dig__fwrite_port_L (&(ptr->future3 ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_L (&(ptr->future4 ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_D (&(ptr->F1 ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_D (&(ptr->F2 ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_D (&(ptr->F3 ), 1, fp)) return(-1);
    if (0 >= dig__fwrite_port_D (&(ptr->F4 ), 1, fp)) return(-1);

    if (0 >= dig__fwrite_port_C (ptr->Dig_name  , HEADSTR, fp)) return(-1);
    if (0 >= dig__fwrite_port_C (ptr->filler    , HEADSTR, fp)) return(-1);

    return (0);
}
