/*
 * \brief calculates mean patch size index
 *
 *   Author: Serena Pallecchi
 *
 *   This program is free software under the GPL (>=v2)
 *   Read the COPYING file that comes with GRASS for details.
 */


#include <grass/gis.h>
#include <grass/glocale.h>

#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

#include "../r.li.daemon/defs.h"
#include "../r.li.daemon/avlDefs.h"
#include "../r.li.daemon/avlID.h"
#include "../r.li.daemon/daemon.h"


int calculate (int fd,area_des ad, struct Cell_head hd,double *result);
int calculateD (int fd,area_des ad,struct Cell_head hd, double *result);
int calculateF (int fd,area_des ad,struct Cell_head hd, double *result);


int main(int argc, char *argv[])
{
  struct Option *raster, *conf, *output;
  struct GModule *module;
  
  G_gisinit(argv[0]);
  module = G_define_module();
  module->description =_("Calculates mean patch size index on a raster file, using a 4 neighbour algorithm");
  
  /* define options */
  
  raster = G_define_standard_option(G_OPT_R_MAP);

  conf = G_define_option();
  conf->key = "conf";
  conf->description = "configuration file in ~/.r.li/history/ folder (i.e conf=my_configuration)";
  conf->type = TYPE_STRING;
  conf->required = YES;
  conf->gisprompt = "file,file,file";
  
  output = G_define_standard_option(G_OPT_R_OUTPUT);
  
  if (G_parser(argc, argv))
    exit(EXIT_FAILURE);

  return calculateIndex(conf->answer, meanPatchSize, NULL, raster->answer, output->answer);
  
}



int meanPatchSize(int fd, char ** par, area_des ad, double *result)
{

  char * mapset;
  
  int ris=0;
  
  double indice=0;

  struct Cell_head hd;
  
  

  mapset = G_find_cell(ad->raster, "");
  if (G_get_cellhd(ad->raster, mapset, &hd) == - 1)
    return ERRORE;
  
  

  switch(ad->data_type)
    {
    case CELL_TYPE:
      {
	ris=calculate(fd,ad,hd,&indice);
	break;
      }
    case DCELL_TYPE:
      {
	ris=calculateD(fd,ad,hd,&indice);
	break;
      }
    case FCELL_TYPE:
      {
	ris=calculateF(fd,ad,hd,&indice);
	break;
      }
       default:
	{
		G_fatal_error("data type unknown");
		return ERRORE;
	}
    }
  if(ris!=OK)
    {
      return ERRORE;
    }
  
  *result=indice;
  return OK;
  
}


int calculate (int fd,area_des ad, struct Cell_head hd,double *result)
{
  CELL * buf;
  CELL * buf_sup;
  
  CELL corrCell;
  CELL precCell;
  CELL supCell;
  
  int i,j;
  int mask_fd=-1, *mask_buf;
  int ris=0;
  int masked=FALSE;
  int a=0; /*if all cells are null a=0*/
   
  long npatch=0;
  long tot=0;
  long zero=0;
  long uno=1;
  long idCorr=0;
  long lastId=0;
  long doppi=0;
  long * mask_patch_sup;
  long * mask_patch_corr;
  
  double indice=0;
  double area=0;
  double areaCorrect=0;
  double EW_DIST1,EW_DIST2,NS_DIST1,NS_DIST2;
  
  avlID_tree albero=NULL;
  
  avlID_table * array;
  
  
  
  /* open mask if needed */
  if (ad->mask == 1)
    {
      if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	return ERRORE;
      mask_buf = G_malloc(ad->cl * sizeof(int));
      if (mask_buf==NULL)
      {
	      G_fatal_error("malloc mask_buf failed");
		return ERRORE;
      }
      masked=TRUE;
    }
  
  mask_patch_sup=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_sup==NULL)
    {
      G_fatal_error("malloc mask_patch_sup failed");
      return ERRORE;
    }
  
  mask_patch_corr=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_corr==NULL)
    {
      G_fatal_error("malloc mask_patch_corr failed");
      return ERRORE;
    }
  
  buf_sup=G_allocate_cell_buf();
  if (buf_sup==NULL)
    {
      G_fatal_error("malloc buf_sup failed");
      return ERRORE;
    }
  
  buf=G_allocate_cell_buf();
  if (buf==NULL)
    {
      G_fatal_error("malloc buf failed");
      return ERRORE;
    }
  
  G_set_c_null_value(buf_sup+ad->x,ad->cl); /*the first time buf_sup is all null*/
  
  for(i=0;i<ad->cl;i++)
    {
      mask_patch_sup[i]=0;
      mask_patch_corr[i]=0;
    }
  
 /*for each raster row*/

  for(j = 0; j< ad->rl; j++)
    {
      if(j>0)
	{
	  buf_sup= RLI_get_cell_raster_row(fd, j-1+ad->y, ad);
	}
      buf = RLI_get_cell_raster_row(fd, j+ad->y, ad);
      if(masked)
	{
	  if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	  {
	    	G_fatal_error("mask read failed");
	    return ERRORE;
	  }
	}
      G_set_c_null_value(&precCell,1);
      for(i=0; i < ad->cl; i++) 
	{ /*for each cell in the row*/
	  area++;
	  corrCell=buf[i+ad->x];
     
	  if ((masked) && (mask_buf[i+ad->x]==0)) 
	    {
	      G_set_c_null_value(&corrCell,1);
	      area--;
	    }
	  
	  /*valid cell*/
	  if(!(G_is_null_value(&corrCell,CELL_TYPE)))
	    {
	      
	      if(i>0)
		precCell=buf[i-1+ad->x];
	      
	      if(j==0)
		G_set_c_null_value(&supCell,1);
	      else
		supCell=buf_sup[i+ad->x];
	      
	      
	      if(corrCell!=precCell)
		{
		  if(corrCell!=supCell)
		    {
		      /*new patch*/
		      if (idCorr==0) /*first patch*/
			{
			  lastId=1;
			  idCorr=1;
			  mask_patch_corr[i]=idCorr;
			}
		      else /*not first patch*/
			/* put in the tree previous values */
			{	
			  if(albero==NULL)
			    {
			      albero=avlID_make(idCorr,uno);
			      if (albero==NULL){
				G_fatal_error("avlID_make error");
				return ERRORE;
			      }
			      npatch++;
			      
			    }
			  else /* tree not null*/
			    {
			      ris=avlID_add(&albero,idCorr,uno);  
			      switch(ris)
				{
				case ERR:
				  {
				    G_fatal_error("avlID_add error");
				    return ERRORE;
				  }
				case ADD:
				  {
				    npatch++;
				    break;
				  }
				case PRES:
				  {
				    break;
				  }
				default:
				  {
				    G_fatal_error("avlID_add unknown error");
				    return ERRORE;
				  }
				}
			    }
			  
			  lastId++;
			  idCorr=lastId;
			  mask_patch_corr[i]=idCorr;
			}
		    }
		  else /*current cell and upper cell are equal*/
		    {
		      if((corrCell==precCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
			{
			  long r=0;
			  long del=mask_patch_sup[i];
			  
			  r=avlID_sub(&albero,del);
			  if(r==0)
			    {
			      G_fatal_error("avlID_sub error");
			      return ERRORE;
			    }
			  /*Remove one patch because it makes part of a patch already found*/
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			  r=i;
			  while (i<ad->cl)
			    {
			      if(mask_patch_sup[r]==del)
				{
				  mask_patch_sup[r]=idCorr;	
				}
			      else
				{
				  r=ad->cl+1;
				}
			    }
			}
		      
		      if(albero==NULL)
			{
			  albero=avlID_make(idCorr,uno);
			  if (albero==NULL){
			    G_fatal_error("avlID_make error");
			    return ERRORE;
			  }
			  npatch++;
			}
		      else /* tree not null*/
			{
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			}
		      
		      idCorr=mask_patch_sup[i];
		      mask_patch_corr[i]=idCorr;			    
		    }
		}
	      else /*current cell and previous cell are equal*/
		{
		  
		  if((corrCell==supCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
		    {
		      int l;
		      
		      mask_patch_corr[i]=mask_patch_sup[i];
		      l=i-1;
		      while(l>=0)
			{
			  if(mask_patch_corr[l]==idCorr)
			    {
			      mask_patch_corr[l]=mask_patch_sup[i];
			      l--;
			    }
			  else
			    {
			      l=(-1);
			    }
			}
		      lastId--;
		      idCorr=mask_patch_sup[i];
		    }
		  else
		    {
		      mask_patch_corr[i]=idCorr;
		    }
		  
		}
	    }
	  else /*cell not to consider or cell is null*/
	    {
	      mask_patch_corr[i]=0;
	    }
	}
      mask_patch_sup=mask_patch_corr;
    }
  
  
  
  if (a!=0)
    {
      if(albero==NULL)
	{
	  albero=avlID_make(idCorr,uno);
	  if (albero==NULL)
	    {
	      G_fatal_error("avlID_make error");
	      return ERRORE;
	    }
	  npatch++;
	}
      else
	{
	  ris=avlID_add(&albero,idCorr,uno);
	  switch(ris)
	    {
	    case ERR:
	      {
		G_fatal_error("avlID_add error");
		return ERRORE;
	      }
	    case ADD:
	      {
		npatch++;
		break;
	      }
	    case PRES:
	      {
		break;
	      }
	    default:
	      {
		G_fatal_error("avlID_add unknown error");
		return ERRORE;
	      }
	    }
	}
    
      
      array=G_malloc(npatch*sizeof(avlID_tableRow));
      if(array==NULL)
      {
	      G_fatal_error("malloc array failed");
		return ERRORE;
      }
      tot=avlID_to_array(albero,zero,array);
      
      if (tot!=npatch)
	{
	  G_warning("avlID_to_array unaspected value. the result could be wrong");
	  return ERRORE;
	}
      
      for(i=0;i<npatch;i++)
	{
	  if(array[i]->tot==0)
	    doppi++;  
	}
      npatch=npatch-doppi;
      
      /*calculate distance*/
      G_begin_distance_calculations();
      /* EW Dist at North edge */
      EW_DIST1 =G_distance(hd.east, hd.north, hd.west, hd.north);
      /* EW Dist at South Edge */
      EW_DIST2 =G_distance(hd.east, hd.south, hd.west, hd.south);
      /* NS Dist at East edge */
      NS_DIST1 =G_distance(hd.east, hd.north, hd.east, hd.south);
      /* NS Dist at West edge */
      NS_DIST2 =G_distance(hd.west, hd.north, hd.west, hd.south);
      
      areaCorrect = (((EW_DIST1 + EW_DIST2) / 2) / hd.cols)*			\
	(((NS_DIST1 + NS_DIST2) / 2) / hd.rows)*			\
	(area);	
      indice=areaCorrect/npatch;
      G_free(array);
    }
  else
    indice=(double)(0);
  
  
  *result=indice;
  
  
  if(masked)
    G_free(mask_buf);
  G_free(mask_patch_sup);
  G_free(mask_patch_corr);
  
  return OK;
}


int calculateD (int fd,area_des ad, struct Cell_head hd,double *result)
{
  DCELL * buf;
  DCELL * buf_sup;
  
  DCELL corrCell;
  DCELL precCell;
  DCELL supCell;
  
  int i,j;
  int mask_fd=-1, *mask_buf;
  int ris=0;
  int masked=FALSE;
  int a=0; /*if all cells are null a=0*/
   
  long npatch=0; 
  long tot=0;
  long zero=0;
  long uno=1;
  long idCorr=0;    
  long lastId=0;
  long doppi=0;
  long * mask_patch_sup;
  long * mask_patch_corr;
  
  double indice=0;
  double area=0;
  double areaCorrect=0;    
  double EW_DIST1,EW_DIST2,NS_DIST1,NS_DIST2;
  
  avlID_tree albero=NULL;
  
  avlID_table * array;
  
  
  
  /* open mask if needed */
  if (ad->mask == 1)
    {
      if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	return ERRORE;
      mask_buf = G_malloc(ad->cl * sizeof(int));
      if (mask_buf==NULL)
      {
	      G_fatal_error("malloc mask_buf failed");
		return ERRORE;
      }
      masked=TRUE;
    }
  
  mask_patch_sup=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_sup==NULL)
    {
      G_fatal_error("malloc mask_patch_sup failed");
      return ERRORE;
    }
  
  mask_patch_corr=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_corr==NULL)
    {
      G_fatal_error("malloc mask_patch_corr failed");
      return ERRORE;
    }
  
  buf_sup=G_allocate_d_raster_buf ();
   if (buf_sup==NULL)
    {
      G_fatal_error("malloc buf_sup failed");
      return ERRORE;
    }
  
  buf=G_allocate_d_raster_buf ();
  if (buf==NULL)
    {
      G_fatal_error("malloc buf failed");
      return ERRORE;
    }
  
  G_set_d_null_value(buf_sup+ad->x,ad->cl); /*the first time buf_sup is all null*/
  
  for(i=0;i<ad->cl;i++)
    {
      mask_patch_sup[i]=0;
      mask_patch_corr[i]=0;
    }
  
  /*read each row and put in an avlId tree cell value with the number of cells which have that value */
  for(j = 0; j< ad->rl; j++)
    {
      if(j>0)
	{
	  buf_sup= RLI_get_dcell_raster_row(fd, j-1+ad->y, ad);
	}
      buf = RLI_get_dcell_raster_row(fd, j+ad->y, ad);
      if(masked)
	{
	  if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	  {
	    	G_fatal_error("mask read failed");
	    return ERRORE;
	  }
	}
      G_set_d_null_value(&precCell,1);
      for(i=0; i < ad->cl; i++) 
	{ /*for each cell in the row*/
	  area++; 
	  corrCell=buf[i+ad->x];
	  
	  if ((masked) && (mask_buf[i+ad->x]==0))
	    {
	      G_set_d_null_value(&corrCell,1);
	      area--;
	    }
	  
	  if(!(G_is_null_value(&corrCell,DCELL_TYPE)))
	    {
	      if(i>0)
		precCell=buf[i-1+ad->x];
	      
	      if(j==0)
		G_set_d_null_value(&supCell,1);
	      else
		supCell=buf_sup[i+ad->x];
	      
	      if(corrCell!=precCell)
		{
		  if(corrCell!=supCell)
		    {
		      /*new patch*/
		      if (idCorr==0) /*first patch*/
			{
			  lastId=1;
			  idCorr=1;
			  mask_patch_corr[i]=idCorr;
			}
		      else /*not first patch*/
			/* put in the tree previous value */
			{	
			  if(albero==NULL)
			    {
			      albero=avlID_make(idCorr,uno);
			      if (albero==NULL){
				G_fatal_error("avlID_make error");
				return ERRORE;
			      }
			      npatch++;
			      
			    }
			  else /* tree not null */
			    {
			      ris=avlID_add(&albero,idCorr,uno);  
			      switch(ris)
				{
				case ERR:
				  {
				    G_fatal_error("avlID_add error");
				    return ERRORE;
				  }
				case ADD:
				  {
				    npatch++;
				    break;
				  }
				case PRES:
				  {
				    break;
				  }
				default:
				  {
				    G_fatal_error("avlID_add unknown error");
				    return ERRORE;
				  }
				}
			    }
			  
			  lastId++;
			  idCorr=lastId;
			  mask_patch_corr[i]=idCorr;
			}
		    }
		  else /*current cell and upper cell are equal*/
		    {
		      if((corrCell==precCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
			{
			  long r=0;
			  long del=mask_patch_sup[i];
			  /*Remove one patch because it makes part of a patch already found*/
			  r=avlID_sub(&albero,del);
			  if(r==0)
			    {
			      G_fatal_error("avlID_sub error");
			      return ERRORE;
			    }
			  
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			  r=i;
			  while (i<ad->cl)
			    {
			      if(mask_patch_sup[r]==del)
				{
				  mask_patch_sup[r]=idCorr;	
				}
			      else
				{
				  r=ad->cl+1;
				}
			    }
			}
		      
		      if(albero==NULL)
			{
			  albero=avlID_make(idCorr,uno);
			  if (albero==NULL){
			    G_fatal_error("avlID_make error");
			    return ERRORE;
			  }
			  npatch++;
			}
		      else /*tree not null*/
			{
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			}
		      
		      idCorr=mask_patch_sup[i];
		      mask_patch_corr[i]=idCorr;			    
		    }
		}
	      else /*current cell and previous cell are equals*/
		{
		  
		  if((corrCell==supCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
		    {
		      int l;
		      
		      mask_patch_corr[i]=mask_patch_sup[i];
		      l=i-1;
		      while(l>=0)
			{
			  if(mask_patch_corr[l]==idCorr)
			    {
			      mask_patch_corr[l]=mask_patch_sup[i];
			      l--;
			    }
			  else
			    {
			      l=(-1);
			    }
			}
		      lastId--;
		      idCorr=mask_patch_sup[i];
		    }
		  else
		    {
		      mask_patch_corr[i]=idCorr;
		    }
		  
		}
	    }
	  else /*cell null or not to consider*/
	    {
	      mask_patch_corr[i]=0;
	    }
	}  
      mask_patch_sup=mask_patch_corr;
    }
  
  
  
  if (a!=0)
    {
      if(albero==NULL)
	{
	  albero=avlID_make(idCorr,uno);
	  if (albero==NULL)
	    {
	      G_fatal_error("avlID_make error");
	      return ERRORE;
	    }
	  npatch++;
	}
      else
	{
	  ris=avlID_add(&albero,idCorr,uno);
	  switch(ris)
	    {
	    case ERR:
	      {
		G_fatal_error("avlID_add error");
		return ERRORE;
	      }
	    case ADD:
	      {
		npatch++;
		break;
	      }
	    case PRES:
	      {
		break;
	      }
	    default:
	      {
		G_fatal_error("avlID_add unknown error");
		return ERRORE;
	      }
	    }
	}
      
      
      array=G_malloc(npatch*sizeof(avlID_tableRow));
      if(array==NULL)
      {
	      G_fatal_error("malloc array failed");
		return ERRORE;
      }
      tot=avlID_to_array(albero,zero,array);
      
      if (tot!=npatch)
	{
	  G_warning("avlID_to_array unaspected value. the result could be wrong");
	  return ERRORE;
	}
      
      for(i=0;i<npatch;i++)
	{
	  if(array[i]->tot==0)
	    doppi++;
	  
	}
      npatch=npatch-doppi;
      
      /*calculate distance*/
      G_begin_distance_calculations();
      /* EW Dist at North edge */
      EW_DIST1 =G_distance(hd.east, hd.north, hd.west, hd.north);
      /* EW Dist at South Edge */
      EW_DIST2 =G_distance(hd.east, hd.south, hd.west, hd.south);
      /* NS Dist at East edge */
      NS_DIST1 =G_distance(hd.east, hd.north, hd.east, hd.south);
      /* NS Dist at West edge */
      NS_DIST2 =G_distance(hd.west, hd.north, hd.west, hd.south);
      
      areaCorrect = (((EW_DIST1 + EW_DIST2) / 2) / hd.cols)*			\
	(((NS_DIST1 + NS_DIST2) / 2) / hd.rows)*			\
	(area);	
      indice=areaCorrect/npatch;
      G_free(array);
    }
  else
    indice=(double)(0);
  
  
  *result=indice;
  
  
  if(masked)
    G_free(mask_buf);
  G_free(mask_patch_sup);
  G_free(mask_patch_corr);
  
  return OK;
}


int calculateF (int fd,area_des ad, struct Cell_head hd,double *result)
{
  FCELL * buf;
  FCELL * buf_sup;
  
  FCELL corrCell;
  FCELL precCell;
  FCELL supCell;
  
  int i,j;
  int mask_fd=-1, *mask_buf;
  int ris=0;
  int masked=FALSE;
  int a=0; /*if all cells are null a=0*/
   
   
  long npatch=0; 
  long tot=0;
  long zero=0;
  long uno=1;
  long idCorr=0;    
  long lastId=0;
  long doppi=0;
  long * mask_patch_sup;
  long * mask_patch_corr;
  
  double indice=0;
  double area=0;
  double areaCorrect=0;    
  double EW_DIST1,EW_DIST2,NS_DIST1,NS_DIST2;
  
  avlID_tree albero=NULL;
  
  avlID_table * array;
  
  
  
  /* open mask if needed */
  if (ad->mask == 1)
    {
      if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	return ERRORE;
      mask_buf = G_malloc(ad->cl * sizeof(int));
      if (mask_buf==NULL)
      {
	      G_fatal_error("malloc mask_buf failed");
		return ERRORE;
      }
      masked=TRUE;
    }
  
  mask_patch_sup=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_sup==NULL)
    {
      G_fatal_error("malloc mask_patch_sup failed");
      return ERRORE;
    }
  
  mask_patch_corr=G_malloc(ad->cl *sizeof( long));
  if (mask_patch_corr==NULL)
    {
      G_fatal_error("malloc mask_patch_corr failed");
      return ERRORE;
    }
  
  buf_sup=G_allocate_f_raster_buf ();
  if (buf_sup==NULL)
    {
      G_fatal_error("malloc buf_sup failed");
      return ERRORE;
    }
  
    
  buf=G_allocate_f_raster_buf ();
  if (buf==NULL)
    {
      G_fatal_error("malloc buf failed");
      return ERRORE;
    }
  
  G_set_f_null_value(buf_sup+ad->x,ad->cl); /*the first time buf_sup is all null*/
  
  for(i=0;i<ad->cl;i++)
    {
      mask_patch_sup[i]=0;
      mask_patch_corr[i]=0;
    }
 /*for each raster row*/

  for(j = 0; j< ad->rl; j++)
    {
      if(j>0)
	{
	  buf_sup= RLI_get_fcell_raster_row(fd, j-1+ad->y, ad);
	}
      buf = RLI_get_fcell_raster_row(fd, j+ad->y, ad);
      if(masked)
	{
	  if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	  {
	    	G_fatal_error("mask read failed");
	    return ERRORE;
	  }
	}
      G_set_f_null_value(&precCell,1);
      for(i=0; i < ad->cl; i++) 
	{ /*for each cell in the row*/
	  
	  area++;
	  corrCell=buf[i+ad->x];
	  if (((masked) && (mask_buf[i+ad->x]==0)) )
	    {
	      G_set_f_null_value(&corrCell,1);
	      area--;
	    }
	  if(!(G_is_null_value(&corrCell,FCELL_TYPE)))
	    {
	      if(i>0)
		precCell=buf[i-1+ad->x];
	      if(j==0)
		G_set_f_null_value(&supCell,1);
	      else
		supCell=buf_sup[i+ad->x];
	      
	      
	      if(corrCell!=precCell)
		{
		  if(corrCell!=supCell)
		    {
		      /*new patch*/
		      if (idCorr==0) /*first patch*/
			{
			  lastId=1;
			  idCorr=1;
			  mask_patch_corr[i]=idCorr;
			}
		      else /*not first patch*/
			/* put in the tree previous value */
			{	
			  if(albero==NULL)
			    {
			      albero=avlID_make(idCorr,uno);
			      if (albero==NULL){
				G_fatal_error("avlID_make error");
				return ERRORE;
			      }
			      npatch++;
			      
			    }
			  else /*tree not empty*/
			    {
			      ris=avlID_add(&albero,idCorr,uno);  
			      switch(ris)
				{
				case ERR:
				  {
				    G_fatal_error("avlID_add error");
				    return ERRORE;
				  }
				case ADD:
				  {
				    npatch++;
				    break;
				  }
				case PRES:
				  {
				    break;
				  }
				default:
				  {
				    G_fatal_error("avlID_add unknown error");
				    return ERRORE;
				  }
				}
			    }
			  
			  lastId++;
			  idCorr=lastId;
			  mask_patch_corr[i]=idCorr;
			}
		    }
		  else /*current cell and upper cell are equal*/
		    {
		      if((corrCell==precCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
			{
			  long r=0;
			  long del=mask_patch_sup[i];

			  r=avlID_sub(&albero,del);
			  if(r==0)
			    {
			      G_fatal_error("avlID_sub error");
			      return ERRORE;
			    }
			  /*Remove one patch because it makes part of a patch already found*/
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			  r=i;
			  while (i<ad->cl)
			    {
			      if(mask_patch_sup[r]==del)
				{
				  mask_patch_sup[r]=idCorr;	
				}
			      else
				{
				  r=ad->cl+1;
				}
			    }
			}

		      if(albero==NULL)
			{
			  albero=avlID_make(idCorr,uno);
			  if (albero==NULL){
			    G_fatal_error("avlID_make error");
			    return ERRORE;
			  }
			  npatch++;
			}
		      else /*l'albero non è null*/
			{
			  ris=avlID_add(&albero,idCorr,uno);
			  switch(ris)
			    {
			    case ERR:
			      {
				G_fatal_error("avlID_add error");
				return ERRORE;
			      }
			    case ADD:
			      {
				npatch++;
				break;
			      }
			    case PRES:
			      {
				break;
			      }
			    default:
			      {
				G_fatal_error("avlID_add unknown error");
				return ERRORE;
			      }
			    }
			}
		      
		      idCorr=mask_patch_sup[i];
		      mask_patch_corr[i]=idCorr;			    
		    }
		}
	      else /*current cell and previous cell are equal*/
		{		  
		  if((corrCell==supCell)&&(mask_patch_sup[i]!=mask_patch_corr[i-1]))
		    {
		      int l;

		      mask_patch_corr[i]=mask_patch_sup[i];
		      l=i-1;
		      while(l>=0)
			{
			  if(mask_patch_corr[l]==idCorr)
			    {
			      mask_patch_corr[l]=mask_patch_sup[i];
			      l--;
			    }
			  else
			    {
			      l=(-1);
			    }
			}
		      lastId--;
		      idCorr=mask_patch_sup[i];
		    }
		  else
		    {
		      mask_patch_corr[i]=idCorr;
		    }
		  
		}
	    }
	  else /*null cell or cell not to consider*/
	    {
	      mask_patch_corr[i]=0;
	    }
	}
      mask_patch_sup=mask_patch_corr;
    }
  
  
  
  if (a!=0)
    {
      if(albero==NULL)
	{
	  albero=avlID_make(idCorr,uno);
	  if (albero==NULL)
	    {
	      G_fatal_error("avlID_make error");
	      return ERRORE;
	    }
	  npatch++;
	}
      else
	{
	  ris=avlID_add(&albero,idCorr,uno);
	  switch(ris)
	    {
	    case ERR:
	      {
		G_fatal_error("avlID_add error");
		return ERRORE;
	      }
	    case ADD:
	      {
		npatch++;
		break;
	      }
	    case PRES:
	      {
		break;
	      }
	    default:
	      {
		G_fatal_error("avlID_add unknown error");
		return ERRORE;
	      }
	    }
	}
      
      
      array=G_malloc(npatch*sizeof(avlID_tableRow));
      if(array==NULL)
      {
	      G_fatal_error("malloc array failed");
		return ERRORE;
      }
      tot=avlID_to_array(albero,zero,array);
      
      if (tot!=npatch)
	{
	  G_warning("avlID_to_array unaspected value. the result could be wrong");
	  return ERRORE;
	}
      
      for(i=0;i<npatch;i++)
	{
	  if(array[i]->tot==0)
	    doppi++;
	  
	}
      npatch=npatch-doppi;
      
      /*calculate distance*/
      G_begin_distance_calculations();
      /* EW Dist at North edge */
      EW_DIST1 =G_distance(hd.east, hd.north, hd.west, hd.north);
      /* EW Dist at South Edge */
      EW_DIST2 =G_distance(hd.east, hd.south, hd.west, hd.south);
      /* NS Dist at East edge */
      NS_DIST1 =G_distance(hd.east, hd.north, hd.east, hd.south);
      /* NS Dist at West edge */
      NS_DIST2 =G_distance(hd.west, hd.north, hd.west, hd.south);
      
      areaCorrect = (((EW_DIST1 + EW_DIST2) / 2) / hd.cols)*			\
	(((NS_DIST1 + NS_DIST2) / 2) / hd.rows)*			\
	(area);	
      indice=areaCorrect/npatch;
       G_free(array);
    }
  else
    indice=(double)(0);
  
  
  *result=indice;
  
 
  if(masked)
    G_free(mask_buf);
  G_free(mask_patch_sup);
  G_free(mask_patch_corr);
  
  return OK;
}



