/* basic/math2.c --- Substitutes for missing math funcs PLUS some int math. */

/*--------------------------------------------------------------------------*/

#include "basic.h"

/*--------------------------------------------------------------------------*/

int basic_ipower (x, y)
     int x, y;
     /* Returns (int) x "to the power of" y, assuming y >= 0.
        Time complexity: O (ln(y)). */
{
  int aux;
  if (y == 0)
    return (1);
  else if (Odd (y))
    return (x * basic_ipower (x, y - 1));
  else
    {
      aux = basic_ipower (x, y / 2);
      return (aux * aux);
    }
}

/*--------------------------------------------------------------------------*/

#if defined (sgi) || defined (__convex__) || defined (_IBMR2)

double log2 (x)
     double x;
{
  return (log (x) / log (2.0));
}

double exp2 (x)
     double x;
{
  return (exp (x * log (2.0)));
}

double exp10 (x)
     double x;
{
  return (exp (x * log (10.0)));
}

#endif  /* #ifdef sgi */
