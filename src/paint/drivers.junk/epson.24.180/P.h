
#include <stdio.h>

struct PRINTER
{
    int fd;		/* file descriptor */
    int b;		/* buf counter */
    int bufsize;	/* buf size */
    unsigned char *buf;	/* output buffer */
    int tty;		/* is a tty? */
} ;
typedef unsigned char COLOR[2048][3];


#ifndef GLOBAL
#define GLOBAL extern
#endif

GLOBAL struct PRINTER printer ;
GLOBAL int nrows, ncols ;
GLOBAL int ras_row;
GLOBAL int ras_nrows;
GLOBAL int ras_row_parity;
GLOBAL int ras_row_idx;
GLOBAL int ras_row_grp;
GLOBAL COLOR YELLOW;
GLOBAL COLOR CYAN;
GLOBAL COLOR MAGENTA;

#define COLOR_BLACK 0
#define COLOR_MAGENTA 1
#define COLOR_CYAN 2
#define COLOR_YELLOW 4
