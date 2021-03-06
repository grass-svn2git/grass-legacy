
/*
*  create_transform_conversion () - main driver routine to prepare
*    the transformation equation.
*
*  yes_no_quest(s)  -  ask the user a yes, no question.
*     returns:   1 for yes, 0 for no
*  Written by the GRASS Team, 02/16/90, -mh.
*/

#include	<stdio.h>
#include	"trans.h"

create_transform_conversion ( Coord, quiet)
	struct  file_info  *Coord ;
	int quiet;
{
	if ( Coord->name[0] != NULL)
		create_transform_from_file( Coord, quiet) ;
	else
		create_transform_from_user () ;
}

create_transform_from_user ()
{
	int  status ;
	int  n_points;
	int  ok ;

init_transform_arrays() ;

n_points = 0 ;
ok = 0 ;
while (! ok)
 {
	/*  go to Vask page to enter the coordinates  */
	if ((n_points =  ask_transform_coor (n_points)) < 0)
		exit(-1) ;

	system("clear") ;

	status =  setup_transform( n_points) ;


	if (status != ALL_OK )
	{
		printf(" Number of points that have been entered: %d\n", n_points );
		print_transform_error(status) ;
		continue ;
	}

	print_transform_resids( n_points) ;
	ok = yes_no_quest ("\n\n\nIf satisfied with the residuals, enter 'y', else 'n' and <Return>:  ");

 }		/*  while (!ok)   */


 return(0) ;

}			/*  create_transform_conversion()  */

yes_no_quest(s)
    char *s;
{
    char buff[200];
    while (1)
    {
	printf("%s",s);
	if (NULL == gets(buff))
		exit(-1) ;
	switch (*buff)
	{
	    case 'Y': case 'y':
		return(1);
	    case 'N': case 'n':
		return(0);
	    default:
		printf("Please answer yes or no");
	}
    }
}


create_transform_from_file ( Coord, quiet)
	struct  file_info  *Coord ;
	int  quiet ;
{
	int  status ;
	int  n_points;
	FILE *fp ;

	init_transform_arrays() ;

	n_points = 0 ;
/*  Get the coordinates from the file.  */
	if ((n_points =  get_coor_from_file (Coord->fp) ) < 0)
		exit(-1) ;

	status =  setup_transform( n_points) ;

	if (status != ALL_OK )
	{
		printf(" Number of points that have been entered: %d\n", n_points );
		print_transform_error(status) ;
		exit(-1) ;
	}

	if (!quiet)
		print_transform_resids( n_points) ;

 return(0) ;

}			/*  create_transform_from_file()  */

