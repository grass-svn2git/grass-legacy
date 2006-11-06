
/*
 * \brief calculates mean pixel attribute index
 *
 *   Author: Serena Pallecchi
 *
 *   This program is free software under the GPL (>=v2)
 *   Read the COPYING file that comes with GRASS for details.
 *
 */

#include <grass/gis.h>
#include <grass/glocale.h>

#include <stdlib.h>
#include <fcntl.h>
#include <math.h>


#include "../r.li.daemon/defs.h"
#include "../r.li.daemon/daemon.h"





int calculate (int fd,area_des ad, double *result);
int calculateD (int fd,area_des ad, double *result);
int calculateF (int fd,area_des ad, double *result);

int main(int argc, char *argv[])
{
    struct Option *raster, *conf, *output;
    struct GModule *module;
    
    G_gisinit(argv[0]);
    module = G_define_module();
    module->description =_("Calculates mean pixel attribute index on a raster file.\nReturn -1 if all the cells in the area are null");
    
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

    return calculateIndex(conf->answer, meanPixelAttribute , NULL, raster->answer, output->answer);


}


int meanPixelAttribute(int fd, char ** par, area_des ad, double *result)
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
	    ris=calculate(fd,ad,&indice);
	    break;
	}
	case DCELL_TYPE:
	{
	    ris=calculateD(fd,ad,&indice);
	    break;
	}
	case FCELL_TYPE:
	{
	    ris=calculateF(fd,ad,&indice);
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



int calculate (int fd,area_des ad, double *result)
{
    CELL * buf;
    
    int i,j;
    int mask_fd=-1, *mask_buf;
    int masked=FALSE;

    double area=0;
    double indice=0;
    double somma=0;


    /* open mask if needed */
    if (ad->mask == 1)
    {
	if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	{
	    G_fatal_error("can't open mask");
	    return ERRORE;
	}
	mask_buf = G_malloc(ad->cl * sizeof(int));
	if (mask_buf==NULL)
	{
	    G_fatal_error("malloc mask_buf failed");
	    return ERRORE;
	}
	masked=TRUE;
    }


    for(j = 0; j< ad->rl; j++)
    {         /*for each raster row*/
	buf = RLI_get_cell_raster_row(fd, j+ad->y, ad);/*read raster row*/

	if (masked)
	{/*read mask row if needed*/

	    if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	    {
	    	G_fatal_error("mask read failed");
		return ERRORE;
	    }
	}

	for(i=0; i < ad->cl; i++)
	{ /*for each cell in the row*/
		area++;
		if(masked && mask_buf[i+ad->x]==0)
			{
				G_set_c_null_value(&buf[i+ad->x],1);
				area--;
			}
	    if(!(G_is_null_value(&buf[i+ad->x],CELL_TYPE)))
		{/*if it's a cell to consider*/
			somma=somma+buf[i+ad->x];
	    }
	}
    }


      if(area==0)
	  indice=(double)-1;
      else
	  indice=somma/area;

      *result=indice;
if (masked)
	{
	G_free(mask_buf);
	}
      return OK;
}
int calculateD (int fd,area_des ad, double *result)
{
    DCELL * buf;
    
    int i,j;
    int mask_fd=-1, *mask_buf;
    int masked=FALSE;

    double area=0;
    double indice=0;
    double somma=0;


    /* open mask if needed */
    if (ad->mask == 1)
    {
	if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	{
	    G_fatal_error("can't open mask");
	    return ERRORE;
	}
	mask_buf = G_malloc(ad->cl * sizeof(int));
	if (mask_buf==NULL)
	{
	    G_fatal_error("malloc mask_buf failed");
	    return ERRORE;
	}
	masked=TRUE;
    }



    for(j = 0; j< ad->rl; j++)
    {         /*for each raster row*/
	buf = RLI_get_dcell_raster_row(fd, j+ad->y, ad);/*read raster row*/

	if (masked)
	{/*read mask row if needed*/

	    if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	    {
	    	G_fatal_error("mask read failed");
		return ERRORE;
	    }
	}

	for(i=0; i < ad->cl; i++)
	{ /*for each cell in the row*/
		area++;
	    if ((masked) && (mask_buf[i+ad->x]==0))
	    {
		    area--;
		    G_set_d_null_value(&buf[i+ad->x],1);
	    }
		if(!(G_is_null_value(&buf[i+ad->x],DCELL_TYPE)))
		{
		somma=somma+buf[i+ad->x];
	    }
	}
    }


      if(area==0)
	  indice=(double)-1;
      else
	  indice=somma/area;

      *result=indice;
if (masked)
	{
	G_free(mask_buf);
	}
      return OK;
}

int calculateF (int fd,area_des ad, double *result)
{
    FCELL * buf;
    
    int i,j;
    int mask_fd=-1, *mask_buf;
    int masked=FALSE;

    double area=0;
    double indice=0;
    double somma=0;


    /* open mask if needed */
    if (ad->mask == 1)
    {
	if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
	{
	    G_fatal_error("can't open mask");
	    return ERRORE;
	}
	mask_buf = G_malloc(ad->cl * sizeof(int));
	if (mask_buf==NULL)
	{
	    G_fatal_error("malloc mask_buf failed");
	    return ERRORE;
	}
	masked=TRUE;
    }



    for(j = 0; j< ad->rl; j++)
    {         /*for each raster row*/
	buf = RLI_get_fcell_raster_row(fd, j+ad->y, ad);/*read raster row*/

	if (masked)
	{/*read mask row if needed*/

	    if(read(mask_fd,mask_buf,(ad->cl * sizeof (int)))<0)
	    {
	    	G_fatal_error("mask read failed");
		return ERRORE;
	    }
	}

	for(i=0; i < ad->cl; i++)
	{ /*for each cell in the row*/
		area++;
		
	    if ((masked) && (mask_buf[i+ad->x]==0))
	    {
		    area--;
		    G_set_f_null_value(&buf[i+ad->x],1);
	    }
	    if(!(G_is_null_value(&buf[i+ad->x],FCELL_TYPE)))
		{		
		somma=somma+buf[i+ad->x];
	    }
	}
    }


      if(area==0)
	  indice=(double)-1;
      else
	  indice=somma/area;

      *result=indice;
if (masked)
	{
	G_free(mask_buf);
	}
      return OK;
}
