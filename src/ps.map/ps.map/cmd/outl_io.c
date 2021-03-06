
#include "gis.h"
#include "ps_info.h"

static int blank_line ();

static struct Cell_head cell_head;
static char *error_prefix;
static int first_read, last_read;
static char cell_name[256];
static int in_file_d;
static int row_length, row_count, n_rows, total_areas;


o_io_init ()
{
    error_prefix = "ps.map";
    n_rows = PS.w.rows;
    row_length = PS.w.cols;
}

o_read_row (buf)
    CELL *buf;
{
    int i;

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
	    G_get_map_row(in_file_d,buf + 1,row_count++);
	    *buf = *(buf + row_length + 1) = 0;
	}
    }
    return(row_length + 2);
}

static int 
blank_line (buf)
    CELL *buf;
{
    int i;

    for (i = 0; i < row_length + 2; i++)
	*(buf + i) = 0;
}

o_open_file (cell)
char *cell;
{
    char *mapset, *p;

    /* open cell file */
    if ((mapset = G_find_cell(cell,"")) == NULL)
    {
	fprintf(stderr,"%s:  o_open_file:  cell file %s not found\n",error_prefix,cell);
	exit(-1);
    }
    sscanf(cell,"%s",cell_name);
    if ((in_file_d = G_open_cell_old(cell_name,mapset)) < 0)
    {
	fprintf(stderr,"%s:  o_open_file:  could not open cell file %s in %s\n",error_prefix,cell_name,mapset);
	exit(-1);
    }
    first_read = 1;
    last_read = 0;
    row_count = 0;
    o_alloc_bufs(row_length + 2);
}

o_close_file()
{
    G_close_cell(in_file_d);
}

#ifdef DEBUG
char *
xmalloc(size,label)
int size;
char *label;
{
    char *addr, *G_malloc();

    addr = G_malloc(size);
    fprintf(stdout,"MALLOC:   %8d   %7d          %s\n",addr,size,label);
    return(addr);
}

xfree(addr,label)
char *addr, *label;
{
    fprintf(stdout,"FREE:     %8d                %s\n",addr,label);
    free(addr);
}

char *
xrealloc(addr,size,label)
char *addr, *label;
int size;
{
    char *addr2, *G_realloc();

    addr2 = G_realloc(addr,size);
    fprintf(stdout,"REALLOC:  %8d   %7d  (%8d)   %s\n",addr2,size,addr,label);
    return(addr2);
}
#endif
