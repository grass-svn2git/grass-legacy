#ifndef MAIN
# define EXT extern
#else 
# define EXT
#endif

#include "gis.h"
#include "Vect.h"

typedef struct {
    int     cat;   /* category */
    int     count1, count2; /* Count of found values; i1: count, coor, sides; i2: sides */
                            /* for sides set to 2, if more than 1 area category was found, */
                            /* including no category (cat = -1)! */
    int     i1,i2; /* values; i1: query (result int), sides; i2: sides */
    double  d1,d2; /* values (length, area, x/y, query) */
    char    *str1;  /* string value (query) */
    int     *qcat; /* array query categories */
    int     nqcats; /* number of query cats */
    int     aqcats; /* number of allocated query cats */
    char    null;   /* no records selected by query */
}VALUE;

EXT VALUE   *Values;

#define OPTIONS struct _options_
EXT OPTIONS
{
    char *name;
    char *mapset;
    int  field;    
    char *col1;
    char *col2;
    char *qcol;
    int  type;
    int  option;
    int  print;     /* print only */
    int  sql;       /* print only sql statements */
    int  units;
    int  qfield;    /* query field */
} options;

#define VSTAT struct _vstat_
EXT VSTAT
{
    int  rcat;      /* number of categories read from map */
    int  select;    /* number of categories selected from DB */
    int  exist;     /* number of cats existing in selection from DB */
    int  notexist;  /* number of cats not existing in selection from DB */
    int  dupl;      /* number of cats with duplicate elements (currently O_COOR only) */
    int  update;    /* number of updated rows */
    int  error;     /* number of errors */
    int  qtype;     /* C type of query column */
} vstat;

#define O_CAT		1
#define O_AREA		2
#define O_LENGTH	3
#define O_COUNT		4
#define O_COOR		5  /* Point coordinates */
#define O_QUERY		6  /* Query database records linked by another field (qfield) */
#define O_SIDES         7  /* Left and right area of boundary */

#define U_ACRES		1
#define U_HECTARES	2
#define U_KILOMETERS	3
#define U_METERS	4
#define U_MILES		5
#define U_FEET		6

/* areas.c */
int read_areas(struct Map_info *);

/* calc.c */
double length(register int, register double *, register double *);

/* find.c */
int find_cat(int);

/* line.c */
int read_lines(struct Map_info *);

/* parse.c */
int parse_command_line(int, char *[]);

/* query.c */
int query(struct Map_info *);

/* report.c */
int report(void);
int print_stat(void);

/* units.c */
int conv_units(void);

/* update.c */
int update (struct Map_info *);
