#include <assert.h>

#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/dbmi.h>
#include <grass/glocale.h>

#include <grass/PolimiFunct.h>

int P_Create_Aux_Table (dbDriver *, char *);
		   
void 
P_Sparse_Correction (struct Map_info *,		/**/
			struct Map_info *,		/**/
			struct Map_info *,		/**/
			struct Cell_head *,		/**/
			BOUND_BOX, 			/**/
			BOUND_BOX, 			/**/
			double **, 			/**/
			double *,			/**/
			int *, 				/**/
			double, 			/**/
			int, 				/**/
			dbDriver *,			/**/
			double				/**/);
	
double **
P_Regular_Correction (struct Cell_head *, 		/**/
			BOUND_BOX, 		 	/**/
			BOUND_BOX,		 	/**/
			double **, 			/**/
			double *, 			/**/
			double, 			/**/
			double, 			/**/
			int, 				/**/
			int				/**/);

int Insert_Correction (double,			/**/
	    		int,				/**/
	    		dbDriver *			/**/);

int UpDate_Correction (double,			/**/ 
	    		int,				/**/ 
	    		dbDriver *			/**/);

int Select_Correction (double *,			/**/ 
	    		int, 				/**/
	    		dbDriver *			/**/);

int Create_AuxEdge_Table (dbDriver *);

struct Point* 
P_Read_Vector_Correction (struct Map_info *,	/**/
			  struct Cell_head *,	/**/
			  int *,		/**/
			  int *,		/**/
			  int			/**/);

void P_Aux_to_Raster (double **, int);
int correction (int, double, double);
