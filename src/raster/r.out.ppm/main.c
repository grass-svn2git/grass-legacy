/* Written by Bill Brown, USA-CERL
 * March 21, 1994
*/

/* Use to convert grass raster file to PPM
 * uses currently selected region
*/

#include <string.h>

#include "gis.h"

#define DEF_RED 255
#define DEF_GRN 255
#define DEF_BLU 255

typedef int FILEDESC;
double atof();

main(argc, argv)
    int argc;
    char *argv[];
{

    struct Option 	*rast, *ppm_file;
    struct Flag 	*bequiet, *gscale;
    char 		*cellmap, *map, *p, errbuf[100], ofile[1000];
    unsigned char 	*set, *ored, *ogrn, *oblu;
    CELL		*cell_buf;
    int 		row, col, do_stdout = 0;
    struct Cell_head	w;
    FILEDESC    	cellfile = NULL;
    FILE 		*fp;


    G_gisinit (argv[0]);

    rast = G_define_option();
    rast->key                    = "input";
    rast->type                   = TYPE_STRING;
    rast->required               = YES;
    rast->multiple               = NO;
    rast->gisprompt              = "old,cell,Raster";
    rast->description            = "Raster file to be converted.";

    ppm_file = G_define_option();
    ppm_file->key                    = "output";
    ppm_file->type                   = TYPE_STRING;
    ppm_file->required               = NO;
    ppm_file->multiple               = NO;
    ppm_file->answer	       	     = "<rasterfilename>.ppm";
    ppm_file->description            
		    = "Name for new PPM file. (use out=- for stdout)";

    bequiet = G_define_flag ();
    bequiet->key = 'q';
    bequiet->description = "Run quietly";

    gscale = G_define_flag ();
    gscale->key = 'G';
    gscale->description = "Output greyscale instead of color";

    if (G_parser (argc, argv))
	exit (-1);

    /* kludge to work with r.out.mpeg */
    if(rast->answer[0] == '/')
	rast->answer++;
    
    if(strcmp(ppm_file->answer,"<rasterfilename>.ppm")){
	if(strcmp(ppm_file->answer,"-"))
	    strcpy (ofile, ppm_file->answer);
	else
	    do_stdout = 1;
    }
    else{
	map = p = rast->answer;
	/* knock off any GRASS location suffix */
	if ((char*)NULL != (p = strrchr (map, '@'))) {
		if (p != map)
		*p = '\0';
	}
	strcpy (ofile, map);
	strcat(ofile,".ppm");
    }

    G_get_set_window (&w); 

    if(!bequiet->answer)
	fprintf(stderr,"rows = %d, cols = %d\n", w.rows, w.cols);

    /* open cell file for reading */
    {  
	cellmap = G_find_file2 ("cell", rast->answer, "");
	if(!cellmap){
	    sprintf(errbuf,"Couldn't find raster file %s", rast->answer);
	    G_fatal_error(errbuf);
	}

	if ((cellfile = G_open_cell_old(rast->answer, cellmap)) == -1) 
	{
	    sprintf(errbuf,"Not able to open cellfile for [%s]", rast->answer);
	    G_fatal_error(errbuf);
	}
    }

    cell_buf = (CELL *)G_malloc (w.cols * sizeof (CELL));
    ored = (unsigned char *)G_malloc (w.cols * sizeof (unsigned char));
    ogrn = (unsigned char *)G_malloc (w.cols * sizeof (unsigned char));
    oblu = (unsigned char *)G_malloc (w.cols * sizeof (unsigned char));
    set = (unsigned char *)G_malloc (w.cols * sizeof (unsigned char));

    /* open ppm file for writing */
    {
	if(do_stdout) fp = stdout;
	else if(NULL == (fp = fopen(ofile, "w"))) {
	    sprintf(errbuf,"Not able to open file for [%s]", ofile);
	    G_fatal_error(errbuf);
	}
    }
    /* write header info */

    if(!gscale->answer)
	fprintf(fp,"P6\n");
	/* Magic number meaning rawbits, 24bit color to ppm format */
    else 
	fprintf(fp,"P5\n");
	/* Magic number meaning rawbits, 8bit greyscale to ppm format */

    if(!do_stdout){
	fprintf(fp,"# CREATOR: %s from GRASS raster file \"%s\"\n", 
		argv[0], rast->answer);
	fprintf(fp,"# east-west resolution: %f\n", w.ew_res);
	fprintf(fp,"# north-south resolution: %f\n", w.ns_res);
    /* comments */
    }

    fprintf(fp,"%d %d\n",w.cols,w.rows); 
    /* width & height */

    fprintf(fp,"255\n");
    /* max intensity val */


    if(!bequiet->answer)
	fprintf(stderr,"Converting %s...",rast->answer);

    {	
    struct Colors colors;

    G_read_colors (rast->answer, cellmap, &colors);

    if(!gscale->answer){ 	/* 24BIT COLOR IMAGE */
	for (row = 0; row < w.rows; row++) {
	    if(!bequiet->answer)
		G_percent (row, w.rows, 5);
	    G_get_map_row(cellfile, cell_buf, row); 
	    G_lookup_colors (cell_buf, ored, ogrn, oblu, set, w.cols, &colors);

	    for(col=0; col < w.cols; col++){
		if(set[col]){
		    putc(ored[col],fp);
		    putc(ogrn[col],fp);
		    putc(oblu[col],fp);
		}
		else{
		    putc(DEF_RED,fp);
		    putc(DEF_GRN,fp);
		    putc(DEF_BLU,fp);
		}
	    }
	}
    }
    else{ 			/* GREYSCALE IMAGE */
	for (row = 0; row < w.rows; row++) {

	    if(!bequiet->answer)
		G_percent (row, w.rows, 5);
	    G_get_map_row(cellfile, cell_buf, row); 
	    G_lookup_colors (cell_buf, ored, ogrn, oblu, set, w.cols, &colors);

	    for(col=0; col < w.cols; col++){

#ifdef XV_STYLE
		    /*.33R+ .5G+ .17B*/
		    putc((((ored[col])*11+(ogrn[col])*16
			+(oblu[col])*5) >> 5), fp);
#else
		    /*NTSC Y equation: .30R+ .59G+ .11B*/
		    putc((((ored[col])*19+(ogrn[col])*38
			+(oblu[col])*7) >> 6), fp);
#endif
	    }
	}
    }

    G_free_colors(&colors);

    }
    free(cell_buf);
    free(ored);
    free(ogrn);
    free(oblu);
    free(set);
    G_close_cell(cellfile);
/*
    if(!do_stdout)
*/
	fclose(fp);

    if(!bequiet->answer)
	fprintf(stderr,"\nDone.\n"); 
    
    return(1);
}



