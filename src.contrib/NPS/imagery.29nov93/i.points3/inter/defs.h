/* #include "imagery.h" */
#include "ortho_image.h" 
#include <curses.h> 


typedef enum { CurrentEnv=0, TargetEnv=1 } GrassEnv;

/* this is a list for displayed raster and vectors w/ color */
typedef struct { 
   char vect_name  [40];
   char vect_mapset[40];
   char vect_color [40];
} VectDisplay;

typedef struct
{
   int  num_raster;
   char rast_name[40];
   char rast_mapset[40];
   int  num_vectors;
   VectDisplay vects[10]; 
}  DisplayList;

/* this is a curses structure */
typedef struct
{
    int top, left, bottom, right;
} Window;

/* this is a graphics structure */
typedef struct
{
    int top, bottom ,left, right;
    int nrows, ncols;
    struct
    {
	int configured;
	struct Cell_head head;
	struct Colors colors;
	char name[100];
	char mapset[100];
	int top, bottom ,left, right;
	double ew_res, ns_res;	/* original map resolution */
    } cell;
    struct
    {
	int configured;
	struct Cell_head head;
	struct Colors    colors;
	char name[30];
	char mapset[30];
	int top, bottom ,left, right;
	double ew_res, ns_res;	/* original map resolution */
    } vect;
} View;

/*   Replaced with following structure for CRS.c
typedef struct
{
    char name[100];
    struct Ref ref;
    struct Control_Points points;
    double E12[3], N12[3], E21[3], N21[3];
    int equation_stat;
} Group;
*/

/* now replace with more generic struct in ortho_imagery.h 
typedef struct
{
    char name[100];
    struct Ref ref;
    struct Control_Points points;
    double E12[10], N12[10], E21[10], N21[10];
    int equation_stat;
} Group;
*/


typedef struct
{
    int   type;         /* object type */
    int (*handler)();	/* routine to handle the event */
    char *label;	/* label to display if MENU or OPTION */
    int   binding;      /* OPTION bindings */
    int  *status;	/* MENU,OPTION status */
    int top,bottom,left,right;
} Objects;

#define MENU_OBJECT    1
#define OPTION_OBJECT  2
#define INFO_OBJECT    3
#define TITLE_OBJECT   4
#define OPTITLE_OBJECT 5
#define OTHER_OBJECT  6

#define MENU(label,handler,status) \
	{MENU_OBJECT,handler,label,0,status,0,0,0,0}
#define OPTION(label,binding,status) \
	{OPTION_OBJECT,NULL,label,binding,status,0,0,0,0}
#define INFO(label,status) \
	{INFO_OBJECT,NULL,label,0,status,0,0,0,0}
#define TITLE(label,status) \
	{TITLE_OBJECT,NULL,label,0,status,0,0,0,0}
#define OPTITLE(label,status) \
	{TITLE_OBJECT,NULL,label,0,status,0,0,0,0}
#define OTHER(handler,status) \
	{OTHER_OBJECT,handler,NULL,0,status,0,0,0,0}
