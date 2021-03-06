#include "kappa.h"

long
count_sum (ns,nl)
  int *ns;
{
  long count;
  int k,n;

  k = n = *ns;
  count = 0;

  if (nl >= 0) {
    while(n < nstats && same_cats(k,n,nl))
      count += Gstats[n++].count;
  }
  else {
    while(n < nstats)
      count += Gstats[n++].count;
  }

  *ns = n;
  return count;
}

same_cats (a,b,nl)
{
  long *cat_a,*cat_b;

  cat_a = Gstats[a].cats;
  cat_b = Gstats[b].cats;

  while (nl-- >= 0)
    if (*cat_a++ != *cat_b++) return 0;
  return 1;
}
