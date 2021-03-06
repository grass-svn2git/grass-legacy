#include "imagery.h"
print_separability(fd,C)
    FILE *fd;
    struct Cluster *C;
{
    int c1, c2;
    int first, last;
    double I_cluster_separation();
    double q;

    I_cluster_sum2 (C);
    fprintf (fd, "\nclass separability matrix\n\n");
    for (first = 0; first < C->nclasses; first = last)
    {
	last = first + 10;
	if (last > C->nclasses)
	    last = C->nclasses;
	fprintf (fd, "\n    ");
	for (c2 = first; c2 < last; c2++)
	    fprintf (fd, "   %3d", c2+1);
	fprintf (fd, "\n\n");
	for (c1=first; c1 < C->nclasses; c1++)
	{
	    fprintf (fd, "%3d ",c1+1); 
	    for (c2=first; c2 <= c1 && c2 < last; c2++)
	    {
		q = I_cluster_separation(C,c1,c2);
		if (q == 0.0)
		    fprintf (fd, " %5d", 0);
		else if (q > 0.0)
		    fprintf (fd, " %5.1lf", q);
		else
		    fprintf (fd, "  --- ");
	    }
	    fprintf (fd, "\n");
	}
	fprintf (fd, "\n");
    }
}
