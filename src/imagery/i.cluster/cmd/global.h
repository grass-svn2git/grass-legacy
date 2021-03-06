#include "imagery.h"

#ifndef GLOBAL
# define GLOBAL extern
#endif



GLOBAL struct Cluster C;
GLOBAL struct Signature in_sig;

GLOBAL int maxclass ;
GLOBAL double conv ;
GLOBAL double sep ;
GLOBAL int iters ;
GLOBAL int mcs;
GLOBAL char *group;
GLOBAL char *subgroup;
GLOBAL struct Ref ref;
GLOBAL char *outsigfile;
GLOBAL char *insigfile;
GLOBAL char *reportfile;
GLOBAL CELL **cell;
GLOBAL int *cellfd;
GLOBAL FILE *report;
GLOBAL int sample_rows, sample_cols;
GLOBAL int verbose;
GLOBAL long start_time;
