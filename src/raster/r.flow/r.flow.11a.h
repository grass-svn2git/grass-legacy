/****************************************************************************
**  Written by Maros Zlocha, Jaroslav Hofierka, Helena Mitasova, Winter 1992
**  Comenius University, Bratislava, Slovakia and
**  US Army Construction Engineering Research Laboratories, Champaign, IL,USA.
**  Copyright Maros Zlocha, Jaroslav Hofierka, Helena Mitasova, USA-CERL  1992
**  Drastically re-engineered by Joshua Caplan, Spring, 1994
*****************************************************************************/

/*
 * changes from version 6: lg matrix changed to single cell row buffer
 * 	x, y components of point structure are now exact coordinates
 * 	r, c reflect file structure (i.e. r = distance in ns-res units from N) 
 * changes from version 7: next_point changed to eliminate arctangent
 *	theta and other angles kept in degrees instead of radians
 * changes from version 8: tangents replaced by lookup table
 *	bounding box re-introduced to reduce number of truncations
 * changes from version 9: source code split into separate modules
 *	aspect optionally computed internally (aspin no longer required)
 *	aspect optionally computed on the fly, eliminating o matrix
 *	resolution no longer used to compute distances (allows lat/long proj)
 *	downslope flag now the default, new options "bound", "-z", "-q" added
 *	diagnostic/verbose output --> stderr; debugging output --> stdout
 *	epsilon introduced to defeat quantization errors
 * changes from version 10: segmentation "-M" added, "-z" renamed to "-3"
 *	further modularization and abstraction
 * changes from version 11: replaced function pointers get, put, ... by macros
 */

#include <math.h>
#include "gis.h"
#include "segment.h"
#include "Vect.h"
#include "bitmap.h"

#define D_PI	180.
#define D2_PI	(2. * D_PI)
#define DEG2RAD	(M_PI / D_PI)
#define UNDEF	365.		/* change to undefined when available */
#define UNDEFZ	0.		/* change to undefined when available */
#define HORIZ	1		/* magic	*/
#define VERT	0		/*      numbers	*/

#define ROUND(x)	(int) ((x) + 0.5)

typedef struct
{
    char   *elevin;		/* name of input elevation file		*/
    char   *aspin;		/* name of input aspect file		*/
    char   *barin;		/* name of barrier input file		*/
    char   *flout;		/* name of output flowline file		*/
    char   *lgout;		/* name of output length file		*/
    char   *dsout;		/* name of output density file		*/
    int     skip;		/* cells between flowlines output	*/
    int	    bound;		/* constant bound on path length	*/
    char    up;			/* direction to compute lines		*/
    char    l3d;		/* three-dimensional length		*/
    char    mem;		/* always recompute aspect		*/
    char    seg;		/* use segmented arrays			*/
    char    quiet;		/* no diagnostic output			*/
}
params;

typedef struct
{
    CELL   **buf;		/* internal row storage			*/
    SEGMENT *seg;		/* state for segment library		*/
    int      sfd;		/* file descriptor for segment file	*/
    int	     row_offset,	/* border widths of buf (for		*/
             col_offset;	/*	extrapolating border data)	*/
    char    *name;		/* for error messages			*/
}
layer;

/******************************* GLOBALS ********************************/

/* heap memory */
struct  Cell_head region;	/* resolution and boundaries		*/
struct  Map_info  fl;		/* output vector file header		*/
struct  BM       *bitbar;	/* space-efficient barrier matrix	*/
int     lgfd;			/* output length file descriptor	*/
char    string[1024];		/* space for strings			*/
layer   el, as, ds;		/* elevation, aspect, density		*/
double  tang[361];		/* tangents lookup table		*/
double *ew_dist;		/* east-west distances for rows		*/
double *epsilon[2];		/* quantization errors for rows		*/

/* command-line parameters */
params  parm;
