
/*
 * \brief calculates patch number index
 *
 *   Author: Claudio Porta and Lucio Davide Spano
 *
 *   This program is free software under the GPL (>=v2)
 *   Read the COPYING file that comes with GRASS for details.
 *
 */

#include <stdlib.h>
#include <fcntl.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "../r.li.daemon/daemon.h"

int main(int argc, char *argv[]){
	struct Option *raster, *conf, *output;
	struct GModule *module;
	
	G_gisinit(argv[0]);
	module = G_define_module();
	module->description =_("Calculates patch number index on a raster file, using a 4 neighbour algorithm");
	
	/* define options */
	
	raster = G_define_standard_option(G_OPT_R_MAP);
	
	conf = G_define_option();
	conf->key = "conf";
	conf->description = "configuration file in ~/.r.li/history/ folder \
	(i.e conf=my_configuration";
	conf->gisprompt = "file,file,file";
	conf->type = TYPE_STRING;
	conf->required = YES;
	
	output = G_define_standard_option(G_OPT_R_OUTPUT);
	
	
	if (G_parser(argc, argv))
	   exit(EXIT_FAILURE);

	return calculateIndex(conf->answer, patch_number, NULL, raster->answer, output->answer);
	
}

int patch_number(int fd, char ** par, area_des ad, double *result){
	CELL *buf, *sup;
	int count=0, i,j, connected=0, complete_line=1, other_above=0;
//	double area;
	char *mapset/*, c[150]*/;
	struct Cell_head hd;
	CELL complete_value;
//	double EW_DIST1,EW_DIST2,NS_DIST1,NS_DIST2;
	int mask_fd=-1, *mask_buf, *mask_sup, null_count=0;
	
	G_set_c_null_value(&complete_value, 1);
	mapset = G_find_cell(ad->raster, "");
	if (G_get_cellhd(ad->raster, mapset, &hd) == - 1)
		return 0;
	sup = G_allocate_cell_buf();
	
	/* open mask if needed */
	if (ad->mask == 1){
		if ((mask_fd = open(ad->mask_name, O_RDONLY, 0755)) < 0)
				return 0;
		mask_buf = malloc(ad->cl * sizeof(int));
		mask_sup = malloc(ad->cl * sizeof(int));		
	}	
	
		/*calculate number of patch*/
		
	for(i = 0; i<ad->rl; i++){
		buf = RLI_get_cell_raster_row(fd, i+ad->y, ad);
		if(i > 0){
			sup = RLI_get_cell_raster_row(fd, i-1+ad->y, ad);
		}
		/* mask values */
		if (ad->mask == 1){
			int k;
			if (i>0){
				int *tmp;
				tmp = mask_sup;
				mask_buf = mask_sup;
			}
			if (read(mask_fd, mask_buf, (ad->cl * sizeof(int))) < 0)
				return 0;
			for (k=0; k<ad->cl; k++){
					if (mask_buf[k] == 0){
						G_set_c_null_value(mask_buf+k, 1);
						null_count++;
					}
			}
					
		}
			
		
		if (complete_line){
			if (! G_is_null_value( &(buf[ad->x]), CELL_TYPE) &&\
				buf[ad->x] != complete_value)
				count++;
			
			for(j=0; j<ad->cl-1; j++){
				
				if(buf[j+ad->x] != buf[j+1+ad->x]){
					complete_line=0;
					if(!G_is_null_value( &(buf[j+1+ad->x]), CELL_TYPE) &&\
						buf[j+1+ad->x] != complete_value)
						count++;
				}
						
			}						
			if (complete_line){
				complete_value = buf[ad->x];
			}
		}
		else{
			complete_line = 1;
			connected=0;
			other_above=0;
			for(j=0; j<ad->cl; j++){
				if (sup[j+ad->x] == buf[j+ad->x]){
					connected = 1;
					if(other_above){
						other_above=0;
						count--;
					}
				}
				else{
					if(connected && \
						!G_is_null_value( &(buf[j+ad->x]), CELL_TYPE))
						other_above=1;
				}
				if (j<ad->cl-1 && buf[j+ad->x] != buf[j+1+ad->x]){
					complete_line =0;
					if (!connected &&\
						! G_is_null_value( &(buf[j+ad->x]), CELL_TYPE)){
						
						count++;
						connected =0;
						other_above =0;								
						}
					else{
						connected=0;
						other_above=0;
					}
				}				
			}
			if (!connected &&\
				sup[ad->cl-1+ad->x] != buf[ad->cl-1+ad->x]){
				if(!G_is_null_value( &(buf[ad->cl-1+ad->x]), CELL_TYPE)){
					count++;
					complete_line=0;
				}
			}
			
			if (complete_line)
				complete_value = buf[ad->x];
			
		}
		
	}
	
	
	
	*result= count;
	return 1;
}
