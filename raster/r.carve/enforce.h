#ifndef __ENFORCE_H__
#define __ENFORCE_H__

#include <stdio.h>
#include "gis.h"
#include "bitmap.h"
#include "Vect.h"


#define APP_VERSION 1.0
#define MAX_PTS     10000

/* 2x2 determinat */
#define DET2_2(a,b,c,d) ((a*d) - (b*c))

#define LINTERP(a,b,r)  ((a)+(r) * ((b)-(a)))
#define SQR(x) (x * x)


typedef double Point2[2];

typedef struct{
    Point2 pnts[MAX_PTS];
    int npts;
    double sum_x, sum_y, sum_xy, sum_x_sq, slope, yinter;
} PointGrp;


/* enforce_ds.c */
extern int enforce_downstream(int infd, int outfd, char *outvect,
                    struct Map_info *Map, struct Map_info *outMap,
                    RASTER_MAP_TYPE rtype, double width, double depth, 
                    int noflat, int quiet);

/* line_dist.c */
#if 0
extern double distance2(double fromx, double fromy, double tox, double toy);
extern double distance2_point_to_seg(double x, double y, double x1, 
                    double y1, double x2, double y2);
#endif
extern double xy_distance2_point_to_seg(double *px, double *py,
                    double x1, double y1, double x2, double y2);
extern double xy_distance3_point_to_seg(double *px, double *py,
                    double x1, double y1, double x2, double y2,
                    int *status);

/* lobf.c */
extern Point2 *pg_getpoints(PointGrp *pg);
extern Point2 *pg_getpoints_reversed(PointGrp *pg);
extern double pg_y_from_x(PointGrp *pg, double x);
extern void pg_init(PointGrp *pg);
extern void pg_addpt(PointGrp *pg, Point2 pt);

/* support.c */
extern int update_history(char *raster_name);

/* vect.c */
extern int open_new_vect(struct Map_info *map, char *vect);
extern int close_vect(struct Map_info *map, int build_support);
extern int write_xyz_points(struct Map_info *map, Point2 *pgxypts, 
                    Point2 *pgpts, const int npts, const double depth);

#endif /* __ENFORCE_H__ */

