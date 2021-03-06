/*
**  Written by Dave Gerdes  8/1988
**  US Army Construction Engineering Research Lab
*/
#include "digit.h"


/*
** highlight every area line that does not have an area attached to it. 
** this is to aid finding bad areas that were not completed by support.vect 
** this should NOT be used untill after labeling entire map OR running  
** support.vect on the file 
*/
unfinished_areas (map)
    struct Map_info *map;
{
    register int i, error;
    char buf[10];
    P_LINE *Line;
    int ret;

    ret = 0;
    error = 0;
    TimeOutCursor (1);
    for (i = 1 ; i <= map->n_lines ; i++)
    {
	if (Check_for_interrupt())

	    break;

	Line = &(map->Line[i]);

	/* note this code is cool for islands */
	/* Even if line is attached to an island, if not to an */
	/* area, something is wrong */

	if (Line->type != AREA)
	    continue;
	if (LINE_ALIVE (Line) && (Line->right <= 0 && Line->left <= 0))
	{
	    if(0 > V1_read_line (map, &Gpoints, Line->offset))
	    {
		if (error > 10)  /* ... it's late.. */
		{
		    write_info (3, "Multiple Errors reading digit file!");
		    return (-1);
		}
		error++;
		continue;
	    }
		
	    _highlight_line(Line->type, &Gpoints, i, map);
	    ret++;
	}
    }
    TimeOutCursor (0);
    return (ret);
}
