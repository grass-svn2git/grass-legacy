#include "coin.h"

row_total (row, with_no_data, count, area)
    long *count;
    double *area;
{
    int col;
    struct stats_table *x;

    x = table + row*ncat1;
    *count = 0;
    *area  = 0.0;
    for (col = 0; col < ncat1; col++)
    {	
	if (with_no_data || (col != no_data1))
	{
	    *count += x->count;
	    *area  += x->area ;
	}
	x += 1;
    }
}

col_total (col, with_no_data, count, area)
    long *count;
    double *area;
{
    int row;
    struct stats_table *x;

    x = table + col;
    *count = 0;
    *area  = 0.0;
    for (row = 0; row < ncat2; row++)
    {	
	if (with_no_data || (row != no_data2))
	{
	    *count += x->count;
	    *area  += x->area ;
	}
	x += ncat1;
    }
}
