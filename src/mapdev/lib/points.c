/*
**
**   This file is one of two that can be loaded.  The flag CERL_PORTABLE
**   should *NOT* be defined unless you know what you are doing
**   See the file README_386 in this directory for more information.
*/
#ifdef CERL_PORTABLE
#include "./xpoints.c"
#else


/* %W% %G% */
/*
**  Written by: Dave Gerdes 5 1988
**  US Army Construction Engineering Research Lab
*/


/* for globs.h */
#define CONTROL

#include "digit.h"

/*  Read_line ()
**     read line info from digit file into line_points structure 
**
**  Returns     (int)  type  or
**	 -2  End of file
**	-1 Out of memory
*/

/*  Read_line handles ALL memory management internally 
**  all caller has to do is pass  double **x, **y   
**   memory is reused on each call
*/
long ftell ();

int
dig_Read_line (fp, offset, xarray, yarray, n_points)
    FILE *fp;
    long offset;
    double **xarray, **yarray;
    int *n_points;
{
    int itype;
    int alloced;
    int ret;
    static struct line_pnts Points;
    static int first_time;	/* 0 on startup */

    if (first_time == 0)
    {
	Points.alloc_points = 0;
	first_time = -1;
    }
    if (0 > (ret = dig__Read_line (&Points, fp, offset)))
	goto done;

    *n_points = Points.n_points;
    *xarray = Points.x;
    *yarray = Points.y;

done:
    return (ret);
}

/* write line info to DIGIT file */
/*  returns offset into file */
long
dig_Write_line(digit, type, xarray, yarray, n_points) 
    FILE *digit;
    char type;
    double *xarray, *yarray;
    int n_points;
{
    long offset;

    if (Lines_In_Memory)
    {
	fprintf (stderr, "CATASTROPHIC ERROR! attempt to append to memory file\n");
	return (-1);
    }
    fseek( digit, 0L, 2) ;		/*  end of file */
    offset = ftell (digit);

    dig_Rewrite_line (digit, offset, type, xarray, yarray, n_points);

    return (offset);
}

/* write line info to DIGIT file */
/*  at the given offset */
/*  obviously the number of points must NOT have changed */
/*  from when line was read in */

dig_Rewrite_line (digit, offset, type, xarray, yarray, n_points) 
    FILE *digit;
    long offset;
    char type;
    double *xarray, *yarray;
    int n_points;
{
    int itype;
    struct line_pnts points;

    points.x = xarray;
    points.y = yarray;
    points.n_points = n_points;
    return (dig__Rewrite_line (digit, offset, type, &points));
}
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
dig__Read_line (p, fp, offset)
    struct line_pnts *p;
    FILE *fp;
    long offset;
{
    int n_points;
    int itype;

    if (Lines_In_Memory)
	return (dig__Mem_Read_line (p, offset));

    fseek (fp, offset, 0);
    if (0 >= fread (&itype, sizeof(int), 1, fp) )
    goto done;
    itype = dig_old_to_new_type (itype);
    if (0 >= fread (&n_points, sizeof(int), 1, fp) )
    goto done;

    if (0 > dig_alloc_points (p, n_points+1))
    {
/*DEBUG*/ fprintf (stderr, "ALLOC_POINTS Returned error\n");
/*DEBUG*/ fprintf (stderr, "Requested %d  Previous %d\n", n_points+1, p->alloc_points);
	return (-1);
    }

    p->n_points = n_points;
    if (0 >= fread (p->x, sizeof(double), n_points, fp))
    goto done;
    if (0 >= fread (p->y, sizeof(double), n_points, fp))
    goto done;

    return (itype);
done:
    return (-2);
}

/* this is an experimental routine to use a memory copy of the digit file
** to maybe speed things up 
**  see  memory_io.c for the rest
*/
int
dig__Mem_Read_line (p, offset)
    struct line_pnts *p;
    long offset;
{
    int n_points;
    int itype;

    dig_mseek (0, offset, 0);

    if (0 >= dig_mread (&itype, sizeof(int), 1, 0) )
    goto done;
    itype = dig_old_to_new_type (itype);
    if (0 >= dig_mread (&n_points, sizeof(int), 1, 0) )
    goto done;

    if (0 > dig_alloc_points (p, n_points+1))
	return (-1);

    p->n_points = n_points;
    if (0 >= dig_mread (p->x, sizeof(double), n_points, 0))
    goto done;
    if (0 >= dig_mread (p->y, sizeof(double), n_points, 0))
    goto done;

    return (itype);
done:
    return (-2);
}

/* write line info to DIGIT file */
/*  returns offset into file */
long
dig__Write_line(digit, type, points) 
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
    fseek( digit, 0L, 2) ;		/*  end of file */
    offset = ftell (digit);

    dig__Rewrite_line (digit, offset, type, points);

    return (offset);
}

/* write line info to DIGIT file */
/*  at the given offset */
/*  obviously the number of points must NOT have changed */
/*  from when line was read in */

dig__Rewrite_line (digit, offset, type, points) 
    FILE *digit;
    long offset;
    char type;
    struct line_pnts *points;
{
    int itype;

    fseek (digit, offset, 0);
    itype = (int) dig_new_to_old_type (type);
    fwrite(&itype, sizeof(int), 1, digit);
    fwrite(&(points->n_points), sizeof(int), 1, digit);
    fwrite(points->x, sizeof(double), points->n_points, digit);
    fwrite(points->y, sizeof(double), points->n_points, digit);
    fflush (digit);

    /* 
    ** if we have a memory file loaded, have to update it
    **
    **  NOTE WELL:  this is not currently set up to expand the memory file
    **  this is only designed to be used for true Re_writes, ie 
    **    updating a type or endpoints etc. not for adding new lines
    **
    */
    if (Lines_In_Memory)
    {
	dig_mseek (0, offset, 0);
	dig_mwrite(&itype, sizeof(int), 1, 0);
	dig_mwrite(&(points->n_points), sizeof(int), 1, 0);
	dig_mwrite(points->x, sizeof(double), points->n_points, 0);
	dig_mwrite(points->y, sizeof(double), points->n_points, 0);
    }
}

/*
**   TYPE codes:		 old codes are still supported in dig file
**
**		OLD		NEW
** LINE	 	 0		0x01
** AREA	 	 1		0x02
** DOT	 	 2		0x04
**
** DEAD_LINE	 4		0x10
** DEAD_AREA	 5		0x20
** DEAD_DOT	 6		0x40
*/

char
dig_old_to_new_type (type)
    char  type;
{
    /* see defines.h for new and old codes */
    /* old codes are  FILE_* 		   */
    /*
    type = 1 << type;
    return (type);
    */
    switch (type) {
	case FILE_LINE:
	    type = LINE;
	    break;
	case FILE_AREA:
	    type = AREA;
	    break;
	case FILE_DOT:
	    type = DOT;
	    break;
	case FILE_DEAD_LINE:
	    type = DEAD_LINE;
	    break;
	case FILE_DEAD_AREA:
	    type = DEAD_AREA;
	    break;
	case FILE_DEAD_DOT:
	    type = DEAD_DOT;
	    break;
	default:
	    fprintf (stderr, "SYSTEM_ERROR: OLD_T_NEW Got a bad type code %x\n", type);
	    type = 0;
	    break;
    }
    return (type);
}

char
dig_new_to_old_type (type)
    char  type;
{
    switch (type) {
	case LINE:
	    type = FILE_LINE;
	    break;
	case AREA:
	    type = FILE_AREA;
	    break;
	case DOT:
	    type = FILE_DOT;
	    break;
	case DEAD_LINE:
	    type = FILE_DEAD_LINE;
	    break;
	case DEAD_AREA:
	    type = FILE_DEAD_AREA;
	    break;
	case DEAD_DOT:
	    type = FILE_DEAD_DOT;
	    break;
	default:
	    fprintf (stderr, "SYSTEM_ERROR: NEW_T_OLD Got a bad type code %x\n", type);
	    type = 0;
	    break;
    }
    return (type);
}



#endif   /* CERL_PORTABLE */
