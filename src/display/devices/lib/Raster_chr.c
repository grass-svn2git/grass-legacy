#include "driver.h"
#include <stdio.h>

Raster_char(num, nrows, array, withzeros, color_type)
	int num ;
	int nrows ;
	unsigned char *array ;
	int withzeros ;
	int color_type ;
{
	char *malloc() ;
	char *realloc() ;
	static int array_alloc = 0 ;
	static int *int_array ;

/* Check integer array allocation */
	if(! array_alloc)
	{
		array_alloc = num ;
		int_array = (int *)malloc(array_alloc * sizeof(int)) ;
	}
	else
	{
		if (num > array_alloc)
		{
			array_alloc = num ;
			int_array = (int *)realloc((char *)int_array, num * sizeof(int)) ;
		}
	}

/* Copy char array to integer array */
	if (int_array == NULL)
	{
		fprintf(stderr, "ERROR: insufficient memory in Raster_char\n") ;
		exit(-1) ;
	}
		
	{
		register int i ;
		register int *iptr ;
		register unsigned char *cptr ;
		iptr = int_array ;
		cptr = array ;
		i = num ;
		while(i--)
			*(iptr++) = *(cptr++) ;
	}
	    
/* Call Raster_int */
	Raster_int(num, nrows, int_array, withzeros, color_type) ;
}
