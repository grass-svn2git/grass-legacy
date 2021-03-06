#include "imagery.h"
I_cluster_assign (C,interrupted)
    struct Cluster *C;
    int *interrupted;
{
    int p, c;
    int class, band;
    double d,q;
    double dmin;

/*
fprintf (stderr,"I_cluster_assign(npoints=%d,nclasses=%d,nbands=%d)\n",
    C->npoints, C->nclasses, C->nbands);
*/

    for (p = 0; p < C->npoints; p++)
    {
	if (*interrupted) return;

	for (c = 0; c < C->nclasses; c++)
	{
	    d = 0.0;
	    for (band = 0; band < C->nbands; band++)
	    {
		q = (double) C->points[band][p];
		q -= C->mean[band][c];
		d += q*q;
	    }
	    if (c == 0 || d < dmin)
	    {
		class = c;
		dmin = d;
	    }
	}
	C->class[p] = class;
	C->count[class]++;
	for (band = 0; band < C->nbands; band++)
	    C->sum[band][class] += (double) C->points[band][p] ;
    }
}
