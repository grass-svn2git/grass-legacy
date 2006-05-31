#ifndef __ENFORCE_H__
#define __ENFORCE_H__

#include <stdio.h>
#include <grass/gis.h>
#include <grass/bitmap.h>
#include <grass/Vect.h>


#define APP_VERSION 1.0
#define MAX_PTS     10000

/* 2x2 determinat */
#define DET2_2(a,b,c,d) ((a*d) - (b*c))

#define LINTERP(a,b,r)  ((a)+(r) * ((b)-(a)))
#define SQR(x) (x * x)


typedef double Point2[2];

typedef struct {
    Point2 pnts[MAX_PTS];
    int npts;
    double sum_x, sum_y, sum_xy, sum_x_sq, slope, yinter;
} PointGrp;


struct parms {
    struct Option *inrast, *invect, *outrast, *outvect;
    RASTER_MAP_TYPE raster_type;
    double swidth, sdepth;
    int wrap, quiet, noflat;
};


/* enforce_ds.c */
extern int enforce_downstream(int /*infd*/, int /*outfd*/, 
                    struct Map_info * /*Map*/, struct Map_info * /*outMap*/,
                    struct parms * /* parm */);

/* lobf.c */
extern Point2 *pg_getpoints(PointGrp *);
extern Point2 *pg_getpoints_reversed(PointGrp *);
extern double pg_y_from_x(PointGrp *, const double);
extern void pg_init(PointGrp *);
extern void pg_addpt(PointGrp *, Point2);

/* raster.c */
void *read_raster(void *, const int, const RASTER_MAP_TYPE, const int);
void *write_raster(void *, const int, const RASTER_MAP_TYPE, const int);

/* support.c */
extern int update_rast_history(struct parms *);

/* vect.c */
extern int open_new_vect(struct Map_info *, char *);
extern int close_vect(struct Map_info *, const int);


#endif /* __ENFORCE_H__ */
