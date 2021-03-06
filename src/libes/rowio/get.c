#include <stdio.h>
#include "rowio.h"

char *
rowio_get (R, row)
    ROWIO *R;
{
    int i;
    int age;
    int cur;
    char *select();

    if (row < 0)
	return (char *) NULL;

    if (row == R->cur)
	return R->buf;

    for (i = 0; i < R->nrows; i++)
	if (row == R->rcb[i].row)
	    return select (R, i);

    age = 0;
    cur = 0;

    for (i = 0; i < R->nrows; i++)
	if (R->rcb[i].row < 0)	/* free slot ! */
	{
	    cur = i;
	    break;
	}
	else if (age < R->rcb[i].age)
	{
	    cur = i;
	    age = R->rcb[i].age;
	}
    
    pageout (R, cur);

    i = (*R->getrow) (R->fd, R->rcb[cur].buf, R->rcb[cur].row = row, R->len);
    R->rcb[cur].dirty = 0;
    if (!i)
    {
	R->rcb[cur].row = -1;
	if (cur == R->cur)
	    R->cur = -1;
	return ((char *) NULL);
    }

    return select (R, cur);
}

rowio_flush(R)
    ROWIO *R;
{
    int i;
    for (i = 0; i < R->nrows; i++)
	pageout (R, i);
}

static
pageout (R, cur)
    ROWIO *R;
{
    if (R->rcb[cur].row < 0) return;
    if (! R->rcb[cur].dirty) return;
    (*R->putrow) (R->fd, R->rcb[cur].buf, R->rcb[cur].row, R->len);
    R->rcb[cur].dirty = 0;
}

static
char *
select (R, n)
    ROWIO *R;
{
    int i;

    R->rcb[n].age = 0;
    for (i = 0; i < R->nrows; i++)
	R->rcb[i].age++;
    R->cur = R->rcb[n].row;
    return R->buf = R->rcb[n].buf;
}
