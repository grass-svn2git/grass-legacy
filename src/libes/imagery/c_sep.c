#include "imagery.h"

#define FAR ((double) -1.0)

double
I_cluster_separation (C,class1,class2)
    struct Cluster *C;
{
    int band;
    double q;
    double d;
    double var;
    double a1,a2;
    double n1,n2;
    double m1,m2;
    double s1,s2;
    double sqrt();

    if (C->count[class1] < 2) return FAR;
    if (C->count[class2] < 2) return FAR;
    n1 = (double) C->count[class1];
    n2 = (double) C->count[class2];

    d = 0.0;
    a1 = a2 = 0.0;
    for (band = 0; band < C->nbands; band++)
    {
	s1 = (double) C->sum[band][class1] ;
	s2 = (double) C->sum[band][class2] ;
	m1 = s1 / n1;
	m2 = s2 / n2;
	q = m1 - m2;
	q = q*q;
	d += q;


	var = (double) C->sum2[band][class1] - (s1 * m1) ;
	var /= n1 - 1;
	if (var)
	    a1 += q/var;

	var = (double) C->sum2[band][class2] - (s2 * m2) ;
	var /= n2 - 1;
	if (var)
	    a2 += q/var;
    }
    if (d == 0.0) return d;

    if (a1 < 0 || a2 < 0) return FAR;
    if (a1)
	a1 = sqrt (6*d/a1);
    if (a2)
	a2 = sqrt (6*d/a2);
    q = a1+a2;
    if (q == 0.0) return FAR;

    return (sqrt(d)/q) ;
}
