#include <grass/geo.h>

/* $GISBASE-relative locations of parameter files */
#define STP1927PARAMS "/etc/state27"
#define STP1983PARAMS "/etc/state83"

/* get_deg.c */
int get_deg(char *, int);
/* get_num.c */
extern double LLSTUFF[NLLSTUFF];
int get_KFACT(int);
int get_MFACT(int);
int get_MSFACT(int);
int get_NFACT(int);
int get_QFACT(int);
int get_WFACT(int);
int get_AZIM(int);
int get_TILT(int);
int get_HEIGH(int);
int get_SNUM(int);
int get_SPATH(int);
int get_x0(int);
int get_y0(int);
int get_zone(void);
int get_LL_stuff(int, int);
double prompt_num_double(char *, double, int);
int prompt_num_int(char *, int, int);
/* get_stp.c */
void get_stp_proj(char[]);
int get_stp_code(int, char *, char *);
int get_stp_num(void);
int ask_fips(FILE *, int *, int *, int *);
/* main.c */
/* some global variables */
extern int ier, proj_index, zone, snum, spath;

extern double radius, kfact, mfact, msfact, nfact, qfact,
              wfact, unit_fact, x_false, y_false, heigh, azim, tilt;
int min1(int, int);
#ifdef __GNUC_MINOR__
int leave(int) __attribute__ ((__noreturn__));
#else
int leave(int);
#endif

/* this is from gislib! */
/* table.c */
int init_table(void);
int get_proj_index(char *);
int init_unit_table(void);

/* get_datum.c */
int ask_datum(char *, char *, char *);
