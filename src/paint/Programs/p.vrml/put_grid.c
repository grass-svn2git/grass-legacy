#include "pv.h"


typedef int FILEDESC;

/*
 * Use centers of GRASS CELLS as vertexes for grid. 
 * Currently, grid space is "unitized" so that the 
 * largest dimension of the current region in GRASS == 1.0 
*/

double G_row_to_northing();
double G_col_to_easting();


vrml_put_grid(vout, w, elevfd, colorfd, colr, color_ok, rows, cols, shh)
FILE *vout;
struct Cell_head *w;
FILEDESC elevfd, colorfd;
struct Colors *colr;
int rows, cols, color_ok;
{
char str[512];

#ifdef FP_GRASS
FCELL *tf;
FCELL *dbuf;
    dbuf = (FCELL *)G_malloc (cols * sizeof (FCELL));  
#else
CELL *tf;
CELL *dbuf;
    dbuf = (CELL *)G_malloc (cols * sizeof (CELL));  
#endif

#ifdef V2.0
    fprintf(vout,"grid\n");
    vrml_putline(0,vout,"grid");
#else
    vrml_putline(0,vout,"Separator");
    vrml_putline(1,vout,OCB);


    /* write grid vertices */
    {
	double east, north;
	double coordx, coordy, coordz;
	int row, col;

	if(!shh)
	    fprintf(stderr, "Writing vertices...");

	vrml_putline(0,vout,"Coordinate3");
	vrml_putline(1,vout,OCB);
	vrml_putline(0,vout,"point");
	vrml_putline(1,vout,OSB);

	for (row = 0; row < rows; row++) {
	    tf = dbuf;

	    if(!shh)
		G_percent(row, rows - 1, 10);

#ifdef FP_GRASS
	    G_get_f_raster_row(elevfd, tf, row);
#else
	    G_get_map_row (elevfd, tf, row);
#endif
	    coordz = G_row_to_northing((double)row, w);
	    do_coordcnv(&coordz, 'z');

	    /* write a row */
	    for(col=0; col < cols; col++){
		coordx = G_col_to_easting((double)col, w);
		do_coordcnv(&coordx, 'x');
#ifdef FP_GRASS
		/* HACK: no nulls in vrml grid */
		if (G_is_f_null_value (tf)) *tf = 0.0;
#endif
		coordy = *tf;
		do_coordcnv(&coordy, 'y');
		sprintf(str,"%f %f %f,", coordx, coordy, coordz);
		vrml_putline(0,vout,str);
		tf++;
	    }
	    /* end a row */

	}
	vrml_putline(-1,vout,CSB);  /* end point */
	vrml_putline(-1,vout,CCB); /* end Coordinate3 */
    }
    
    if(color_ok)
    /* write material color */
    {
	int row, col;
        unsigned char *red, *green, *blue, *set;

	if(!shh)
	    fprintf(stderr,"Writing color file...");

	vrml_putline(0,vout,"Material");
	vrml_putline(1,vout,OCB);
	vrml_putline(0,vout,"diffuseColor");
	vrml_putline(1,vout,OSB);

	/* allocate buffers */
	red = G_malloc (cols);
	green = G_malloc (cols);
	blue = G_malloc (cols);
	set = G_malloc (cols);

	tf = dbuf;
	for (row = 0; row < rows; row++) {

	    if(!shh)
		G_percent(row, rows - 1, 5);

#ifdef FP_GRASS
	    G_get_f_raster_row(colorfd, tf, row);
	    G_lookup_f_raster_colors (tf, red, green, blue, set, cols, colr);
#else
	    G_get_map_row (colorfd, tf, row);
	    G_lookup_colors (tf, red, green, blue, set, cols, colr);
#endif

	    for(col=0; col < cols; col++){
		sprintf(str,"%.3f %.3f %.3f,", 
			red[col]/255., green[col]/255., blue[col]/255.);
		vrml_putline(0,vout,str);
	    }
	}

	vrml_putline(-1, vout,CSB);    /* end diffuseColor */
	vrml_putline(-1, vout,CCB);    /* end Material */

	vrml_putline(0,vout,"MaterialBinding");
	vrml_putline(1,vout,OCB);
	vrml_putline(0,vout,"value PER_VERTEX_INDEXED");
	vrml_putline(-1, vout,CCB);    /* end MaterialBinding */

	free(red);
	free(green);
	free(blue);
	free(set);
    }

    /* write face set indices */
    {
    int row, col, c1, c2;

	vrml_putline(0,vout,"IndexedFaceSet");
	vrml_putline(1,vout,OCB);
	vrml_putline(0,vout,"coordIndex");
	vrml_putline(1,vout,OSB);

	    /* write indexes */
	for (row = 0; row < rows-1; row++) {
	    for(col=0; col < cols-1; col++){
		c1 = row*cols+col;
		c2 = c1 + cols +1;
	        sprintf(str,"%d, %d, %d, -1, %d, %d, %d, -1,",
		c1, c1+cols, c1+1, 
		c2, c2-cols, c2-1 );
		vrml_putline(0,vout,str);
	    }
	}

	vrml_putline(-1, vout,CSB);    /* end coordIndex */
	vrml_putline(-1, vout,CCB);    /* end IndexedFaceSet */
    }


    vrml_putline(-1, vout,CCB);    /* end Separator */

#endif


/*
    free(ibuf);
*/
    free(dbuf);

}
