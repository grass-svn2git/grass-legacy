#include "gis.h"
#include "global.h"
#include <math.h>

int read_list_of_sites(list,Lsite)
     /* 
	read a list of of sites file list(usually, the member 
	answers of the Option structure) and load
	within the ListSite structure Lsite
     */
     char **list;
     ListSite *Lsite;
{
  int nfiles;
  char **ptr;  
  int i;
  char *mapset;
  FILE *fps;
  int dims=0,cat=0,strs=0,dbls=0;
  int sites_alloced;
  char tempbuf[500];


  
  nfiles = 0;
  if((ptr = list) != NULL)
    for (; *ptr != NULL; ptr++){
      Lsite[nfiles].name = (char *) G_calloc(100, sizeof(char));
      sprintf(Lsite[nfiles].name,"%s",*ptr);
      nfiles += 1;
    }
  
  i = 0;
  while(i < nfiles){
    mapset = G_find_file ("site_lists", Lsite[i].name, "");
    fps = G_fopen_sites_old (Lsite[i].name, mapset);
    if(fps == NULL){
      sprintf(tempbuf, "Unable to find file <%s>", Lsite[i].name);
      G_fatal_error(tempbuf);
    }
    if (G_site_describe (fps, &dims, &cat, &strs, &dbls)!=0)
      G_fatal_error("failed to guess format");
    
    Lsite[i].nsites = 0;
    sites_alloced = 10;
    Lsite[i].sites=(Site **) G_malloc(sites_alloced*sizeof(Site *));
    Lsite[i].sites[Lsite[i].nsites] = G_site_new_struct (0, dims, strs, dbls);
    while (G_site_get (fps ,Lsite[i].sites[Lsite[i].nsites] ) != EOF){
      Lsite[i].nsites += 1;
      if (Lsite[i].nsites==sites_alloced)
	{
	  sites_alloced+=10;
	  Lsite[i].sites=(Site **) G_realloc(Lsite[i].sites, 
					     sites_alloced*sizeof(Site *));
	  if (Lsite[i].sites==NULL)
	    G_fatal_error("memory reallocation error");
	}

      Lsite[i].sites[Lsite[i].nsites] = G_site_new_struct (0, dims, strs, dbls);
    }
    i += 1;

  }
  return nfiles;
}

double gaussian2dBySigma(d,sigma)
     /*
       probability for gaussian distribution 
     */
     double sigma, d;
{
  double res;
  res=1/(2.*M_PI*sigma*sigma)*exp(-d*d/(2.*sigma*sigma));
  return(res);
}

double gaussian2dByTerms(d,term1,term2)
     /*
       term1=1./(2.*M_PI*sigma*sigma)
       term2=2.*sigma*sigma;
     */
     double d, term1, term2;
{
  double res;
  res=term1*exp(-d*d/term2);
  return(res);
}


double segno(double x){
  double y; 
  y = (x > 0 ? 1. : 0.) + (x < 0 ? -1. : 0.);
  return y;
}


double kernel1(d,rs,lambda)
     /*
     */
     double d, rs, lambda;
{
  double res,a;
  a=lambda-1.;
  if(lambda==1.){
    res=1./(M_PI*(d*d+rs*rs));
  }else{
    res=segno(a)*(a/M_PI)*(pow(rs,2.*a))*(1/pow(d*d+rs*rs,lambda));
  }
    /*  res=1./(M_PI*(d*d+rs*rs));*/
  return(res);
}

double invGaussian2d(sigma,prob)
     double sigma, prob;
{
  double d;
  d = sqrt(-2*sigma*sigma*log(prob*M_PI*2*sigma*sigma));
  return(d);
}     

double euclidean_distance(x,y,n)
     /*
       euclidean distance between vectors x and y of length n
     */
     double *x, *y;
     int n;
{
  int j;
  double out = 0.0;
  double tmp;
  

  for(j=0;j<n;j++){
    tmp = x[j] - y[j];
    out += tmp * tmp;
  }
  

  return sqrt(out);
}

  

