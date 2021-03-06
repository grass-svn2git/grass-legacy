/*
*  Written by the GRASS Team,  02/16/90, -mh .
*/

/******************
*  INCLUDES:       *
*******************/

#include  <stdio.h>

/******************
*  DEFINES:       *
*******************/

#define		MIN_COOR	4
#define		MAX_COOR	10


/*  return status for setup_transform() and the transform library  */
#define  ALL_OK  1
#define  POINTS_NOT_SPREAD  -1
#define  NEED_MORE_POINTS  -2


/******************
*  GLOBALS:       *
*******************/

/**
* The coordinates of the points from the map that is to be converted
* are placed in ax[] and ay[].
* Those cooresponding points in the other coordinate system
* are placed in bx[], by[].
*
* The use[] contains a true if that point is to be used by the transform
* library or a false (0) if it is not to be used.
* The residuals each set of points contributes is placed in residuals[].
*
* Yah, I made them global.  So shoot me.
**/

double	ax[MAX_COOR] ;			/*  current map   */
double	ay[MAX_COOR] ;

double	bx[MAX_COOR] ;			/*  map we are going to   */
double	by[MAX_COOR] ;

int	use[MAX_COOR] ;		/*  where the coordinate came from */
double	residuals[MAX_COOR] ;
double	rms ;

/*  this may be used in the future  */
int	reg_cnt ;		/*  count of registered points */


/******************
*  STRUCTS:       *
*******************/

/*  For GRASS data files  */
struct  file_info
{
	FILE  *fp ;
	char  *mapset ;
	char  name[80] ;
	char  full_name[256] ;
} ;


/* general flags that get set from the command line  */
struct command_flags
{
    int    verbose ;	/*  do we print residual info  */
    int    usage ;
};


