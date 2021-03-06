/* Cell-file line extraction */
/*   Input/output and line tracing routines */

/* Mike Baba */
/* DBA Systems*/
/* Farfax, VA */
/* Jan 1990 */

/* Jean Ezell */
/* US Army Corps of Engineers */
/* Construction Engineering Research Lab */
/* Modelling and Simulation Team */
/* Champaign, IL  61820 */
/* March 1988 */

/* input is a GRASS cell file */
/* output is a binary or ascii digit file */

/* Global variables: */
/*    direction     indicates whether we should use fptr or bptr to */
/*                  move to the "next" point on the line */
/*    first_read    flag to indicate that we haven't read from input */
/*                  file yet */
/*    last_read     flag to indicate we have reached EOF on input */
/*    input_fd      input cell file descriptor */
/*    bin_digit     output digit file */
/*    ascii_digit   output ascii digit file */
/*    row_length    length of each row of the cell file (i.e., number of */
/*                  columns) */
/*    n_rows        number of rows in the cell file */
/*    row_count     number of the row just read in--used to prevent reading */
/*                  beyond end of the cell file */
/*    which_outputs which output files are to be generated */
/*    edge_type     LINE edge or AREA edge */
/*    error_prefix  our name as found from the argument list */

/* Entry points: */
/*    write_line    write a line out to the digit files */
/*    read_row      read another row of data--handles putting a "no data" */
/*                  boundary around the edges of the file */
/*    syntax        check syntax of command line and compile which_outputs */
/*    open_file     open input and output files */
/*    close_file    close input and output files */
/*    show          debugging routine to print out everything imaginable */
/*                  about a COOR structure */

#include <stdio.h>
#include <sys/wait.h>
#include "gis.h"
#include "Vect.h"
#include "extr_lines.h"

#define BACKWARD 1
#define FORWARD 2
#define OPEN 1
#define END 2
#define LOOP 3
#define BINARY 1
#define ASCII 2
#define LINE_EDGE 1
#define AREA_EDGE 2

static struct Map_info Map;
static struct line_pnts *Points;

static struct Cell_head cell_head;
static int which_outputs;
static int edge_type;
static char *error_prefix;
static int direction;
static int first_read, last_read;
static char cell_name[256], lab_name[256], bin_name[256];
static FILE *bin_digit, *ascii_digit;
static int input_fd;
static int row_length, row_count, n_rows;


/* write_line - attempt to write a line to output */
/* just returns if line is not completed yet */

write_line(seed)
    struct COOR *seed;
{
	struct COOR *point, *begin, *end, *find_end(), *move();
	int dir, line_type, n, n1;

	point = seed;
	if (dir = at_end(point))	/* already have one end of line */
	{
		begin = point;
		end = find_end(point,dir,&line_type,&n);
		if (line_type == OPEN)
		{
			return(-1);	/* unfinished line */
		}
		direction = dir;
	}
	else	/* in middle of a line */
	{
		end = find_end(point,FORWARD,&line_type,&n);
		if (line_type == OPEN)		/* line not finished */
		{
			return(-1);
		}
		if (line_type == END)		/* found one end at least */
		{					/* look for other one */
			begin = find_end(point,BACKWARD,&line_type,&n1);
			if (line_type == OPEN)		/* line not finished */
			{
				return(-1);
			}
			if (line_type == LOOP)		/* this should NEVER be the case */
			{
				fprintf(stderr,"%s:  write_line:  found half a loop!\n",error_prefix);
				return(-1);
			}
			direction = at_end(begin);	/* found both ends now; total length */
			n += n1;				/*   is sum of distances to each end */
		}
		else	/* line_type = LOOP by default */
		{					/* already have correct length */
			begin = end;			/* end and beginning are the same */
			direction = FORWARD;		/* direction is arbitrary */
		}
	}
	/* if (n > 2) */
	write_ln(begin,end,n);
	return(0);
}

/* write_ln - actual writing part of write_line */
/* writes binary and supplemental file */

static int 
write_ln (begin,end,n)
    struct COOR *begin, *end;		/* start and end point of line */
    int n;				/* number of points to write */
{
	double x, y;
	struct COOR *p, *last;
	int i, type;
	char *xmalloc();

	if (edge_type == LINE_EDGE)
		type = LINE;
	if (edge_type == AREA_EDGE)
		type = AREA;
	++n;

	p = begin;
	y = cell_head.north - ((double) p->row + 0.5) * cell_head.ns_res;
	x = cell_head.west + ((double) p->col + 0.5) * cell_head.ew_res;


/****************************************************************
 * shapiro 27 feb 1992.
 * bug fixed by:  by Jinn-Guey Lay: jinn@uhunix.uhcc.Hawaii.edu
 ***************************************************************/
	Vect_reset_line (Points);
/***************************************************************/

	if (which_outputs & BINARY)
	    if(Vect_append_point (Points, x, y)==-1)
		 G_fatal_error("Out of Memory");

	for (i = 1; i < n; i++)
	{
		last = p;
		if ((p = move(p)) == NULL)		/* this should NEVER happen */
		{
			fprintf(stderr,"%s:  write_line:  line terminated unexpectedly\n",error_prefix);
			fprintf(stderr,"  previous (%d) point %x (%d,%d,%d) %x %x\n",direction,last,last->row,last->col,last->node,last->fptr,last->bptr);
			exit(-1);
		}
		y = cell_head.north - ((double) p->row + 0.5) *cell_head.ns_res;
		x = cell_head.west + ((double) p->col + 0.5) * cell_head.ew_res;

		if (which_outputs & BINARY)
		    Vect_append_point (Points, x, y);
	/*	xfree(last,"write_ln, last");*/
	}


    /* now free all the pointers */
    p = begin;

    for (i = 1; i < n; i++) 
    {
    /*
	if( i<10)
         printf(" row: %d col: %d\n", p->row, p->col);
	 */
	   last = p;
	if ((p = move(p)) == NULL)
	  break;
	if(last==p) break;
	if(last->fptr!=NULL)
	   if(last->fptr->fptr==last) last->fptr->fptr=NULL;
	/* now it can already ne NULL */
	if(last->fptr!=NULL)
	   if(last->fptr->bptr==last) last->fptr->bptr=NULL;
	if(last->bptr!=NULL)
	   if(last->bptr->fptr==last) last->bptr->fptr=NULL;
	if(last->bptr!=NULL)
	   if(last->bptr->bptr==last) last->bptr->bptr=NULL;
	free(last);
    } /* end of for i */
    if(p!=NULL)
	free(p);

	Vect_write_line (&Map, type, Points);
}

/* move - move to next point in line */

static struct COOR *
move(point)
    struct COOR *point;
{
	if (direction == FORWARD)
	{
		if (point->fptr == NULL)		/* at open end of line */
			return(NULL);
		if (point->fptr->fptr == point)	/* direction change coming up */
			direction = BACKWARD;
		return(point->fptr);
	}
	else
	{
		if (point->bptr == NULL)
			return(NULL);
		if (point->bptr->bptr == point)
			direction = FORWARD;
		return(point->bptr);
	}
}

/* find_end - search for end of line, starting at a given point and */
/* moving in a given direction */

static struct COOR *find_end(seed,dir,result,n)
struct COOR *seed;
int dir, *result, *n;
{
	struct COOR *start;

	start = seed;
	direction = dir;
	*result = *n = 0;
	while (!*result)
	{
		seed = move(seed);
		(*n)++;
		if (seed == start)
			*result = LOOP;
		else
		{
			if (seed == NULL)
				*result = OPEN;
			else
			{
				if (at_end(seed))
					*result = END;
			}
		}
	}
	return(seed);
}

/* at_end - test whether a point is at the end of a line;  if so, give */
/* the direction in which to move to go away from that end */

static int at_end(ptr)
struct COOR *ptr;
{
	if (ptr->fptr == ptr)
		return(BACKWARD);
	if (ptr->bptr == ptr)
		return(FORWARD);
	return(0);
}

/* syntax - check syntax of command line and compile which_outputs to tell */
/* which output files the user wants generated; returns -1 on error, zero */
/* otherwise; default output files are binary digit and dlg label; */
/* default line type is lines (not area edges), anything */
/* other than default produces only the specific files requested */

syntax(argc,argv,input,output)
int argc;
char *argv[];
char *input, *output;
{
	char *type, *format;
	struct Option *opt1, *opt2, *opt4 ;

	opt1 = G_define_option() ;
	opt1->key        = "input" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = YES ;
	opt1->gisprompt  = "old,cell,raster" ;
	opt1->description= "Name of raster file" ;

	opt2 = G_define_option() ;
	opt2->key        = "output" ;
	opt2->type       = TYPE_STRING ;
	opt2->required   = YES ;
	opt2->description= "Name of new vector file" ;
	opt2->gisprompt  = "new,dig,vector" ;

	opt4 = G_define_option() ;
	opt4->key        = "type" ;
	opt4->type       = TYPE_STRING ;
	opt4->required    = NO;
	opt4->answer      = "line";
	opt4->description= "Line type of the extracted vectors";
	opt4->options    = "line,area" ;

	which_outputs = BINARY;
	edge_type = LINE_EDGE;

	if (G_parser(argc, argv)) 
	    exit(-1);

	strcpy(input, opt1->answer);
	strcpy(output, opt2->answer);

	which_outputs = BINARY;

	if (strncmp(opt4->answer,"line",4) == 0)
		edge_type = LINE_EDGE;
	else if (strncmp(opt4->answer,"area",4) == 0)
		edge_type = AREA_EDGE;

	error_prefix = argv[0];
	return(0);
}

read_row (buf)
    CELL *buf;
{
	if (last_read)
		return(0);
	if (first_read)
	{
		blank_line(buf);
		first_read = 0;
	}
	else
	{
		if (row_count >= n_rows)
		{
			last_read = 1;
			blank_line(buf);
		}
		else
		{
			G_get_map_row(input_fd,buf + 1,row_count++);
			*buf = *(buf + row_length + 1) = 0;
		}
	}
	return(row_length + 2);
}

static int 
blank_line(buf)
    CELL *buf;
{
	int i;

	for (i = 0; i < row_length + 2; i++)
		*(buf + i) = 0;
}

open_file (cell,digit)
    char *cell, *digit;
{
	FILE *open_it();
	char *mapset, *p;

	/* open cell file */
	if ((mapset = G_find_cell(cell,"")) == NULL)
	{
		fprintf(stderr,"%s:  open_file:  cell file %s not found\n",error_prefix,cell);
		exit(-1);
	}
	sscanf(cell,"%s",cell_name);
	if ((input_fd = G_open_cell_old(cell_name,mapset)) < 0)
	{
		fprintf(stderr,"%s:  open_file:  could not open cell file %s in %s\n",error_prefix,cell_name,mapset);
		exit(-1);
	}
	if (G_get_cellhd(cell_name,mapset,&cell_head) == -1)
	{
		fprintf(stderr,"%s:  open_file:  could not read header for cell file %s in %s\n",error_prefix,cell_name,mapset);
		exit(-1);
	}
	G_set_window(&cell_head);
	/* open digit file */

	G__make_mapset_element("dig");
	G__make_mapset_element("dig_att");

	if (which_outputs & BINARY)
	    Vect_open_new (&Map, digit);
	
	Points = Vect_new_line_struct ();

	G__file_name (lab_name, "dig_att", digit, G_mapset());

	first_read = 1;
	last_read = 0;
	direction = FORWARD;
	row_length = cell_head.cols;
	n_rows = cell_head.rows;
	row_count = 0;
	alloc_bufs(row_length + 2);
	fill_head();
}

static FILE *
open_it (name)
    char *name;
{
	FILE *file;

	if ((file = fopen(name,"w")) == NULL)
	{
		fprintf(stderr,"%s:  open_it:  could not open output file %s\n",error_prefix,name);
		exit(-1);
	}
	return(file);
}

close_file()
{
    G_close_cell(input_fd);

    if (which_outputs & BINARY)
	Vect_close (&Map);
	
}


fill_head()
{
    struct dig_head head;

    Vect__init_head (&head);

    /* put some junk into the digit file header */
    strcpy(head.organization,"organization");
    strcpy(head.date,"");
    strcpy(head.your_name,"name");
    strcpy(head.map_name,"mapname");
    strcpy(head.source_date,"");
    strcpy(head.line_3,"");
    head.orig_scale = 24000;
    head.plani_zone = cell_head.zone;
    head.W = cell_head.west;
    head.N = cell_head.north;
    head.E = cell_head.east;
    head.S = cell_head.south;
    head.digit_thresh = 0.04;
    head.map_thresh = 0.04;

    /* 4.0  copy head data to Map.head*/
    if (which_outputs & BINARY)
	Vect_copy_head_data (&head, &(Map.head));

}

char *xmalloc(size,label)
int size;
char *label;
{
	char *addr, *G_malloc();

	addr = G_malloc(size);
	return(addr);
}

xfree(addr,label)
char *addr, *label;
{
	/* free(addr);*/
	free(addr);
}

char *xrealloc(addr,size,label)
char *addr, *label;
int size;
{
	char *addr2, *G_realloc();

	addr2 = G_realloc(addr,size);
	return(addr2);
}






