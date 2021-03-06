#include "gis.h"

#define STATS "r.stats"
#define RECLASS "r.reclass"

#define MDEBUG(a) fprintf(stderr,"%s\n",a);



o_distrib(basemap,covermap,outputmap,usecats)
char *basemap,*covermap,*outputmap;
int usecats;
{
  char *me="o_distrib";
  char command[1024];
  long csum, area, catb, basecat, covercat;
  double sum,tot;
  long  stat, cat, total_count;
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
    o_out(fd2, 0L, 0);	/* force at least one reclass rule */

    catb = 0;
    csum = 0;    
/*    fprintf(stderr,"***** Stage 1 - Calculating sums ****\n");/**/
    while (fscanf (fd1, "%ld %ld %ld", &basecat, &covercat, &area) == 3)
    {
	if (catb != basecat)
	{
           o_out(fd2, catb, csum);
           csum = 0;
           catb = basecat;            
	}
	csum += area;
    }
    o_out(fd2, catb, csum);

    rewind(fd1);
    freopen(tempfile2,"r",fd2);
       
/*    fprintf(stderr,"***** Stage 2 - Calculating percents of values in cover  ****\n");/**/
        
    catb = 0;
    tot = 0;
    total_count = 0;

    while (fscanf (fd1, "%ld %ld %ld", &basecat, &covercat, &area) == 3)
    {
	if (catb != basecat && basecat > 0)
	{
           if(fscanf (fd2, "%ld %ld", &cat, &total_count) != 2) return(1);
           catb = basecat;
           /* fprintf(stderr,"Total (must be 100): %lf\n",tot);/**/
           tot=0;
        }   
        
        if(basecat)
        {
          sum = (double)(100.0 * area)/total_count;
          fprintf(stdout,"%8ld %8ld %lf\n", basecat, covercat, sum);
          /*tot+=sum;
          fprintf(stderr,"Area: %ld   Tot: %ld  totsum: %lf\n",area,total_count,tot);/**/
        }
    }
    
    fclose (fd1);
    fclose (fd2);
    unlink (tempfile1);
    unlink (tempfile2);
    return(stat);
}

o_out(fd, cat, sum)
    FILE *fd;
    long cat;
    long sum;
{
    if (sum == 0 || cat == 0) return;
    fprintf (fd, "%ld %ld\n", cat, sum);
}
   
  
