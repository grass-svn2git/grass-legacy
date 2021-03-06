#include <stdio.h>
#define STDOUT "/tmp/mlhprev"
static int nlevels = 3;
static int ncolors = 256;

static FILE *std;

dostd(strot,a,b,c,d,e,f)
	char *strot;
	int a,b,c;
	float d,e,f;
{
/*fprintf(stderr,"Opening /tmp/mlhpreview");*/
if ((std = fopen(STDOUT,"a")) == NULL)
	{
	fprintf(stderr,"ERRRRRRRRRR\n");
	exit(-1);
	}

if ( d != 0.0 )
fprintf(std,"%s _%f _%f _%f\n",strot,d,e,f);
else
fprintf(std,"%s _%d _%d _%d\n",strot,a,b,c);
fclose(std);
}


Pset_color_levels(n)
{
dostd("Pset",n,0,0,0.0,0.0,0.0);
    nlevels = n;
    ncolors = n*n*n;
}

Pcolorlevels (red, grn, blu)
    int *red, *grn, *blu;
{
    *red = nlevels;
    *grn = nlevels;
    *blu = nlevels;
dostd("Plevels",*red,*grn,*blu,0.0,0.0,0.0);
}

Pcolormultipliers (red, grn, blu)
    int *red, *grn, *blu;
{
    *red = nlevels*nlevels;
    *grn = nlevels;
    *blu = 1;
dostd("Pmult",*red,*grn,*blu,0.0,0.0,0.0);
}

Pcolornum (red, green, blue)
    float red, green, blue ;
{
    register int r;
    register int g;
    register int b;
    int max;

    max = nlevels - 1;

    r = red * nlevels;
    g = green * nlevels;
    b = blue * nlevels;

    if (r < 0) r = 0;
    if (r > max) r = max;

    if (g < 0) g = 0;
    if (g > max) g = max;

    if (b < 0) b = 0;
    if (b > max) b = max;

dostd("Pcolornum",0,0,0,red,green,blue);
    return (r*nlevels*nlevels + g*nlevels+ b);
}

Pcolorvalue (n, red, grn, blu)
    float *red, *grn, *blu;
{
    int r,g,b;
    int max, n2;
    double n1;

    n2 = nlevels * nlevels;
    max = ncolors - 1;
    n1 = nlevels - 1;

    if (n < 0) n = 0;
    if (n > max) n = max;
    r = n / n2 ;
    g = (n % n2) / nlevels;
    b = n % nlevels ;

/*
    *red = r / n1 ;
    *grn = g / n1 ;
    *blu = b / n1 ;
*/

	if ( n < 100) {
		*red = n/125;
		*grn = n/125;
		*blu = n/125;
		}
	else {
		*red = r / n1 ;
		*grn = g / n1 ;
		*blu = b / n1 ;
		}
dostd("Pcolorval",n,0,0,0.0,0.0,0.0);
}

Pncolors()
{
dostd("Pncolors",0,0,0,0.0,0.0,0.0);
    return ncolors;
}
