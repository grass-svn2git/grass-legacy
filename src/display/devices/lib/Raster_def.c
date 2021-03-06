
#include "driver.h"

Raster_int_def(num, nrows, array, withzeros, color_type)
	int num ;
	int nrows ;
	unsigned int *array ;
	int withzeros ;
	int color_type ;
{
	register unsigned int cur_color ;
	register unsigned int *arr ;
	register int npixles ;
	int our_x, our_y ;
	int (*assign_color)() ;
	int Color() ;  /* GRASS driver color call */
	int color() ;  /* device color call (generally useful for fixed only) */
	
	if(color_type)
		assign_color = Color ;
	else
		assign_color = color ;

	arr = array ;
	(*assign_color)(cur_color = *array) ;

	npixles = 0 ;

	our_x = cur_x ;
	our_y = cur_y ;

	while (--num)
	{
		if (*(++arr) != cur_color)
		{
			if (nrows == 1)
			{
				if(withzeros || cur_color)
					Cont_rel(npixles,0) ;
				else
					Move_rel(npixles,0) ;
				cur_x++ ;
			}
			else
			{
				if(withzeros || cur_color)
					Box_abs(our_x,
					   our_y + nrows,
					   our_x + npixles,
					   our_y) ;
				our_x += npixles ;
			}
			our_x++ ;

			(*assign_color)(cur_color = *arr) ;
			npixles = 0 ;
		}
		else
		{
			npixles++ ;
		}
	}

	if (nrows == 1)
	{
		if(withzeros || cur_color)
			Cont_rel(npixles,0) ;
		else
			Move_rel(npixles,0) ;
		cur_x++ ;
	}
	else
	{
		if(withzeros || cur_color)
			Box_abs(our_x,
			   our_y + nrows,
			   our_x + npixles,
			   our_y) ;
		cur_y = our_y + nrows ;
		cur_x = our_y + npixles ;
	}
}

static
do_nothing(n)
{
	return n ;
}
