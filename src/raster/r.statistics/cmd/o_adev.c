#include <stdio.h>
#include <math.h>
#include "gis.h"

#define MEM  1024


o_adev(basemap,covermap,outputmap,usecats,cats)
char *basemap,*covermap,*outputmap;
int usecats;
struct Categories *cats;
{
  char command[1024];
  FILE *popen(), *stats, *reclass;
  int first, mem, i, count;
  long basecat, covercat, catb, catc;
  double value, adev, x;
  double *tab;
      
    
             
    mem = MEM * sizeof(double);
    tab = (double *) G_malloc(mem);
    
    sprintf(command, "r.stats -cz input='%s,%s' fs=space", 
                      basemap, covermap);

    stats = popen(command,"r");  

    sprintf (command, "r.reclass i='%s' o='%s'", basemap, outputmap);
    reclass = popen (command, "w");

                                            
    first = 1;
    while (read_stats(stats, &basecat, &covercat, &value))
    {
	if (first)
	{
	   first = 0;
	   catb  = basecat;
	   catc  = covercat;
	   i = 0;
	   count = 0;
	}

	if (basecat != catb)
	{
           a_dev(tab, count, &adev); 
           fprintf (reclass, "%ld = %ld %lf\n", catb, catb, adev);
	   catb = basecat;
	   catc = covercat;
	   count = 0;
        }
        
        if(usecats)
           sscanf (G_get_cat((CELL)covercat, cats), "%lf", &x);
        else
           x = covercat;
        
        for(i = 0; i < value; i++)
        {
           if(count * sizeof(double) >= mem )
           {
             mem += MEM * sizeof(double);
             tab = (double *)G_realloc(tab,mem);
             /* fprintf(stderr,"MALLOC: %d KB needed\n",(int)(mem/1024)); /**/
           }
           tab[count++] = x;
        }
        
    }
    if (first)
    {
	catb = catc = 0;
    }
    else
    {
    	a_dev(tab, count, &adev); 
    	fprintf (reclass, "%ld = %ld %lf\n", catb, catb, adev);
    }
    
    pclose(stats);
    pclose(reclass);/**/
    
    return(0);
}



/***********************************************************************
*
*  Given an array of data[1...n], this routine returns its average 
*  deviation adev.
*
************************************************************************/

int a_dev(data, n, adev)
double *data;
int n;
double *adev;
{      
 double ave, s;
 int i;

   if(n < 1)
   {
    fprintf(stderr,"o_adev: No data in array\n");
    return (1);
   }

   *adev = 0.0;
   s     = 0.0;
   
   for(i = 0; i < n; i++)              /* First pass to get the mean     */
      s += data[i];
   ave = s / n;

   for (i = 0; i < n; i++)             
   {                     
      *adev += fabs(data[i] - ave);     /* deviation from the mean             */
   }

   *adev /= n;
   return(0);
}