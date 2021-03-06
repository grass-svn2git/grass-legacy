
	/*---------------------------------------------------------*
	 *               AGNPS/GRASS Interface Project             *
	 *  Developed in the Agriculture Engineering Department    *
	 *                at Purdue University                     *
	 *                        by                               *
	 *         Raghavan Srinivasan and Bernard Engel           *
	 *                                                         *
	 *   (c)Copyright, 1992 Purdue Research Foundation, West   *
	 *   Lafayette, Indiana 47907. All Rights Reserved. Unless *
	 *   permission is granted, this material shall not be     *
	 *   copied, reproduced or coded for reproduction by any   *
	 *   electrical, mechanical or chemical processes,  or     *
	 *   combinations thereof, now known or later developed.   *
	 *---------------------------------------------------------*/

/*	August, 1991  Agricultural Engineering, Purdue University
	Raghavan Srinivasan (srin@ecn.purdue.edu)
	
	CN_hy_cond()

	To create CN, and hydrologic condition map that is required
	by the r.cn program to generate distributed CN. 
	The hy_Cond map will be generated by reclassing the land use map
	to "good" condition. This is assumed and generally true for 
	mid-west conditions. Then run the r.cn program to generate
	CN map.
*/

#include "agnps_input.h"


int CN_hy_cond()
{
	char buf[512], *tempfile;
	int     i, j;
	int     nrow, ncol;
	FILE	*fd, *fopen();
        int G_window_rows(), G_window_cols(), G_get_map_row(), G_system();

	nrow = G_window_rows();
	ncol = G_window_cols();

	strcpy(hy_cond->p,"temp_hy_cond");

/* To create a temp file for reclassing the landuse map into hy_cond layer */

	tempfile = G_tempfile();
	fd = fopen(tempfile,"w");

	for(i = 0; i < nrow; i++)
	{
	G_get_map_row(landuse->fd,landuse->rbuf,i);
	
	for(j=0;j < ncol;j++){
		if(landuse->rbuf[j] > 0){
		fprintf(fd,"%d=%d good\n",landuse->rbuf[j],landuse->rbuf[j]);
			}
		}
	}

	fclose(fd);
	printf("creating hy_cond map\n");
	sprintf(buf,"r.reclass input=%s output=%s < %s",landuse->p, hy_cond->p, tempfile);
	G_system(buf);
	sprintf(buf,"/bin/rm -f %s",tempfile);
	G_system(buf);

	if(amc <=0 || amc>3)
	/* assume default AMC as II */
	amc = 2;
	printf("creating CN map\n");
        sprintf(buf,"r.cn sg=%s hc=%s lu=%s pr=%s amc=%d cn=temp_cn",hyg->p,hy_cond->p,landuse->p,mgt_practice->p,amc);
	printf("%s\n",buf);
	G_system(buf);

	strcpy(temp_cn_map->p, "temp_cn");
        return 0;
}
