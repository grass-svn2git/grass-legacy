#include <string.h>
#include "gis.h"
#include "site.h"
#include "raster.h"
#include "display.h"

#ifndef GLOBAL
#define GLOBAL extern
#endif

#define MAXSITES 25000


GLOBAL char **site;
GLOBAL int nsites;
GLOBAL struct Cell_head *Wind;
GLOBAL int *Snum;
GLOBAL Site ***CurSites;


/* loadsites.c */
int load_sites(int, struct Cell_head *, int);
int site_mem(Site *);
int compress_cached_site(Site *);
int free_cached_sites(void);
Site *closest_site(int, double, double);
/* show.c */
int show_buttons(int);
/* what.c */
int what(int, int, int, int, int);
void draw_sector(double, double, double, double, double, double);
void draw_point_x(int, int, int);
void draw_point_plus(int, int, int);
