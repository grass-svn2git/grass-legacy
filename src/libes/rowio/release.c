#include <stdio.h>
#include "rowio.h"

void
rowio_release (R)
    ROWIO *R;
{
    int i;

    if (R->rcb)
    {
	for (i = 0; i < R->nrows && R->rcb[i].buf; i++)
	    free (R->rcb[i].buf);
	free (R->rcb);
	R->rcb = NULL;
    }
}
