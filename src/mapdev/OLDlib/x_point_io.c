#include "digit.h"

long ftell ();


/*  Read_line ()
**     read line info from digit file into line_points structure 
**
**  Returns     (int)  type  or
**	 -2  End of file
**	-1 Out of memory

**  if Line_In_Memory is TRUE, then offset is taken off of Mem_Line_Ptr
**  in memory.
*/


int
dig_x__Read_line (p, fp, offset)
    struct line_pnts *p;
    FILE *fp;
    long offset;
{
    int n_points;
    long itype;

    if (Lines_In_Memory)
	return (dig_x__Mem_Read_line (p, offset));

    dig__set_cur_in_head (dig__get_head (fp));

    fseek (fp, offset, 0);

    if (0 >= dig__fread_port_L (&itype, 1, fp)) goto done;
    itype = dig_old_to_new_type ((char) itype);
    if (0 >= dig__fread_port_I (&n_points, 1, fp)) goto done;


    if (0 > dig_alloc_points (p, (int) n_points+1))
    {
/*DEBUG*/ fprintf (stderr, "ALLOC_POINTS Returned error\n");
/*DEBUG*/ fprintf (stderr, "Requested %d  Previous %d\n", (int) n_points+1, p->alloc_points);
	return (-1);
    }

    p->n_points = n_points;
    if (0 >= dig__fread_port_D (p->x, n_points, fp)) goto done;
    if (0 >= dig__fread_port_D (p->y, n_points, fp)) goto done;

    return ((int) itype);
done:
    return (-2);
}


/* write line info to DIGIT file */
/*  returns offset into file */
long
dig_x__Write_line(digit, type, points) 
    FILE *digit;
    char type;
    struct line_pnts *points;
{
    long offset;

    if (Lines_In_Memory)
    {
	fprintf (stderr, "CATASTROPHIC ERROR! attempt to append to memory file\n");
	return (-1);
    }
    fseek (digit, 0L, 2) ;		/*  end of file */
    offset = ftell (digit);

    dig_x__Rewrite_line (digit, offset, type, points);

    return (offset);
}

/* write line info to DIGIT file */
/*  at the given offset */
/*  obviously the number of points must NOT have changed */
/*  from when line was read in */

dig_x__Rewrite_line (digit, offset, type, points) 
    FILE *digit;
    long offset;
    char type;
    struct line_pnts *points;
{
    long itype, n_points;
    double *Ptmp;

    fseek (digit, offset, 0);

    itype = (long) dig_new_to_old_type (type);
    if (0 >= dig__fwrite_port_L (&itype, 1, digit)) return -1;

    dig__fwrite_port_I (&points->n_points, 1, digit);

    if (0 >= dig__fwrite_port_D (points->x, points->n_points, digit)) return -1;
    if (0 >= dig__fwrite_port_D (points->y, points->n_points, digit)) return -1;

    fflush (digit);

    /* 
    ** if we have a memory file loaded, have to update it
    **
    **  NOTE WELL:  this is not currently set up to expand the memory file
    **  this is only designed to be used for true Re_writes, ie 
    **    updating a type or endpoints etc. not for adding new lines
    **
    */
#ifdef MEMORY_IO	/* not upgraded to 4.0 yet */
    if (Lines_In_Memory)
    {
	dig_mseek (0, offset, 0);
	dig_mwrite(&itype, sizeof(long), 1, 0);
	dig_mwrite(&n_points, sizeof(long), 1, 0);

	dig_mwrite(Ptmp, sizeof(double), points->n_points, 0);

	Ptmp = dig__double_convert (points->x, NULL, points->n_points);
	dig_mwrite(Ptmp, sizeof(double), points->n_points, 0);
    }
#endif
    return 0;
}



/* this is an experimental routine to use a memory copy of the digit file
** to maybe speed things up 
**  see  memory_io.c for the rest
*/

#ifndef  MEMORY_IO	/* not updated to 4.0 yet */

int
dig_x__Mem_Read_line (p, offset)
    struct line_pnts *p;
    long offset;
{
    G_fatal_error ("dig_x__Mem_Read_line() was called");
}

#else

int
dig_x__Mem_Read_line (p, offset)
    struct line_pnts *p;
    long offset;
{
    int n_points;
    long l_points;
    long itype;

    dig_mseek (0, offset, 0);

    if (0 >= dig_mread (&itype, sizeof(long), 1, 0) )
    goto done;
    dig__long_convert (&itype, &itype, 1);
    itype = dig_old_to_new_type ((char) itype);
    if (0 >= dig_mread (&l_points, sizeof(long), 1, 0) )
    goto done;
    dig__long_convert (&l_points, &l_points, 1);
    n_points = (int) l_points;

    if (0 > dig_alloc_points (p,  n_points+1))
	return (-1);

    p->n_points = n_points;
    if (0 >= dig_mread (p->x, sizeof(double),  n_points, 0))
    goto done;
    dig__double_convert (p->x, p->x,  n_points);
    if (0 >= dig_mread (p->y, sizeof(double),  n_points, 0))
    goto done;
    dig__double_convert (p->y, p->y,  n_points);

    return ((int) itype);
done:
    return (-2);
}
#endif /* MEMORY_IO */

