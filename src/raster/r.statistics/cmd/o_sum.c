#include "gis.h"

#define STATS "r.stats"
#define RECLASS "r.reclass"

#define MDEBUG(a) fprintf(stderr,"%s\n",a);


o_sum(basemap,covermap,outputmap,usecats,cats) 
char *basemap,*covermap,*outputmap;
int usecats;
struct Categories *cats;
{
    char *me="o_sum";
    char command[1024];
    
    long catb, basecat, covercat;
    double x, area, sum1;
    int stat;
    char *tempfile1, *tempfile2;
    FILE *fd1, *fd2;



    tempfile1 = G_tempfile();
    tempfile2 = G_tempfile();


    sprintf (command, "%s -cz input='%s,%s' fs=space > %s",
	STATS, basemap, covermap, tempfile1);
    if (stat = system(command))
    {
	unlink(tempfile1);
	fprintf (stderr, "%s: ERROR running %s command\n", me, STATS);
	exit(stat);
    }

    fd1 = fopen (tempfile1, "r");
    fd2 = fopen (tempfile2, "w");
    if (fd1 == NULL || fd2 == NULL)
    {
	unlink(tempfile1);
	unlink(tempfile2);
	fprintf (stderr, "%s: ERROR can't open tempfile\n", me);
	exit(1);
    }
    sum_out(fd2, 0L, 0.0);	/* force at least one reclass rule */

    catb = 0;
    sum1 = 0.0;
    

    while (fscanf (fd1, "%ld %ld %lf", &basecat, &covercat, &area) == 3)
    {
	if (catb != basecat)
	{
	    sum_out(fd2, catb, sum1);
	    sum1 = 0.0;
	    catb = basecat;
	}
	if (usecats)
	    sscanf (G_get_cat((CELL)covercat, &cats), "%lf", &x);
	else
	    x = covercat;
	sum1 += x * area;
/*        fprintf(stderr,"sum: %d\n",(int)sum1);*/
        
    }
    sum_out(fd2, basecat, sum1);
    fclose (fd1);
    fclose (fd2);
    sprintf (command, "%s input='%s' output='%s' < %s",
	RECLASS, basemap, outputmap, tempfile2);
    stat = system(command);
    unlink (tempfile1);
    unlink (tempfile2);
    return(stat);
}

sum_out(fd, cat, sum1)
    FILE *fd;
    long cat;
    double sum1;
{
    char buf[80];

    if (cat == 0)
	*buf = 0;
    else
    {
	sprintf (buf, "%.10lf", sum1);
	G_trim_decimal (buf);
    }
    fprintf (fd, "%ld = %ld %s\n", cat, cat, buf);
}
