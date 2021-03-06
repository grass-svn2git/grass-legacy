#include "imagedefs.h"

struct Ref
{
    int nfiles;
    struct Ref_Files
    {
	char name[30];
	char mapset[30];
    } *file;
    struct Ref_Color
    {
	unsigned char *table      ;  /* color table for min-max values */
	unsigned char *index      ;  /* data translation index */
	unsigned char *buf        ;  /* data buffer for reading color file */
	int fd                    ;  /* for image i/o */
	CELL min, max             ;  /* min,max CELL values */
	int n                     ;  /* index into Ref_Files */
    } red, grn, blu;
} ;

struct Tape_Info
{
    char title[75];
    char id[2][75];
    char desc[5][75];
} ;

struct Control_Points
{
    int  count;
    double *e1;
    double *n1;
    double *e2;
    double *n2;
    int *status;
} ;

struct Signature
{
    int nbands;
    int nsigs;
    char title[100];
    struct One_Sig
    {
	char desc[100];
	int npoints;
	double *mean;	/* one mean for each band */
	double **var;   /* covariance band-band   */
	int status;     /* may be used to 'delete' a signature */
	float r,g,b;	/* color */
	int have_color;
    } *sig;
} ;

struct Cluster
{
    int nbands;
    int npoints;
    CELL **points ;
    int np;

    double *band_sum     ; /* sum over each band */
    double *band_sum2    ; /* sum of squares over each band */

    int    *class        ; /* class of each point */
    int    *reclass      ; /* for removing empty classes  */
    int    *count        ; /* number of points in each class */
    int    *countdiff    ; /* change in count */
    double **sum         ; /* sum over band per class */
    double **sumdiff     ; /* change in sum */
    double **sum2        ; /* sum of squares per band per class */
    double **mean        ; /* initial class means */
    struct Signature S   ; /* final signature(s) */

    int nclasses;
    int merge1, merge2;
    int iteration;
    double percent_stable;
} ;
