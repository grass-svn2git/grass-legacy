/* basic/gsort.c --- Quicksort routine: interface to standard qsort(). */

/*--------------------------------------------------------------------------*/

#include "geom/basic.h"

/*---------------------------------------------------------------------------*/

void 
basic_qsort (
    int table[],  /* input/output */
    int i,
    int j,
    int (*compare)(void)
)
     /* This routine sorts table[i..j] in place: int (*compare)() is the
        comparison function, which is called with two arguments that point
        to the elements of table[i..j] that are compared; it is assumed to
        return -1, 0, or +1 with the usual meaning.  Cf, man qsort. */
{
  qsort (&(table[i]), j - i + 1, sizeof (int), compare);
}
