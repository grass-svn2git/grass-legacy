#include <stdlib.h>
#include <math.h>
#include "gis.h"
#include "site.h"

int search_points = 12;

long npoints = 0;
long **npoints_currcell;
int nsearch;
static int i;

struct Point
{
    double north, east;
    double z;
};
struct list_Point
{
    double north, east;
    double z;
    double dist;
};
struct Point ***points;
struct Point *noidxpoints = NULL;
struct list_Point *list;
static struct Cell_head window;
static struct Flag *noindex;
void calculate_distances(int, int, double, double, int*);
void calculate_distances_noindex(double, double);

int main(int argc, char *argv[])
{
    int fd, maskfd;
    CELL  *mask;
    DCELL *dcell;
    struct GModule *module;
    int row, col;
    int searchrow, searchcolumn, pointsfound;
    int *shortlistrows=NULL, *shortlistcolumns=NULL;
    long ncells;
    double north, east;
    double dist;
    double sum1, sum2, interp_value;
    int n, field;
    void read_sites();
    struct
    {
        struct Option *input, *npoints, *output, *dfield;
    } parm;
    struct cell_list
    {
        int row, column;
        struct cell_list *next;
    };
    struct cell_list **search_list, **search_list_start;
    int max_radius, radius;
    int searchallpoints = 0;
   

    parm.input = G_define_option() ;
    parm.input->key        = "input" ;
    parm.input->type       = TYPE_STRING ;
    parm.input->required   = YES ;
    parm.input->description= "Name of input sites map" ;
    parm.input->gisprompt  = "old,site_lists,sites" ;

    parm.output = G_define_option() ;
    parm.output->key        = "output" ;
    parm.output->type       = TYPE_STRING ;
    parm.output->required   = YES;
    parm.output->description= "Name of output raster map" ;
    parm.output->gisprompt  = "any,cell,raster" ;

    parm.npoints = G_define_option() ;
    parm.npoints->key        = "npoints" ;
    parm.npoints->key_desc   = "count" ;
    parm.npoints->type       = TYPE_INTEGER ;
    parm.npoints->required   = NO ;
    parm.npoints->description="Number of interpolation points";
    parm.npoints->answer = "12";

    parm.dfield = G_define_option ();
    parm.dfield->key = "field";
    parm.dfield->type = TYPE_INTEGER;
    parm.dfield->answer = "1";
    parm.dfield->multiple = NO;
    parm.dfield->required = NO;
    parm.dfield->description = "which decimal attribute (if multiple)";
   
    noindex = G_define_flag();
    noindex->key = 'n';
    noindex->description = "Don't index sites by cell (for very large regions;"
                           " uses less memory)";

    G_gisinit(argv[0]);

    module = G_define_module();
    module->description =        
                    "Surface interpolation from sites data by Inverse "
                    "Distance Squared Weighting.";
                    
    if (G_parser(argc, argv))
        exit(1);

    fprintf(stderr, "%s:\n", G_program_name());
   
    if (G_legal_filename(parm.output->answer) < 0)
    {
        fprintf (stderr, "%s=%s - illegal name\n", parm.output->key, parm.output->answer);
        exit(1);
    }


    if(sscanf(parm.npoints->answer,"%d", &search_points) != 1 || search_points<1)
    {
        fprintf (stderr, "%s=%s - illegal number of interpolation points\n", 
                parm.npoints->key, parm.npoints->answer);
        G_usage();
        exit(1);
    }

    sscanf(parm.dfield->answer,"%d", &field);
    if (field < 1)
      G_fatal_error ("Decimal attribute field 0 doesn't exist.");    

    list = (struct list_Point *) G_calloc (search_points, sizeof (struct list_Point));


/* get the window, dimension arrays */
    G_get_window (&window);
   
  if(!noindex->answer)
  { 
    npoints_currcell = (long **)G_malloc(window.rows * sizeof(long));
    points = (struct Point ***)G_malloc(window.rows * sizeof(struct Point));
   
     
    for(row = 0; row < window.rows; row++)
    {
        npoints_currcell[row] = (long *)G_malloc(window.cols * sizeof(long));
        points[row] = (struct Point **)G_malloc(window.cols * sizeof(struct Point));
       
        for(col = 0; col < window.cols; col++)
        {
            npoints_currcell[row][col] = 0;
            points[row][col] = NULL;           
        }
    }
  }
   
/* read the elevation points from the input sites file */
    read_sites (parm.input->answer, field);

    if (npoints == 0)
    {
        fprintf (stderr, "%s: no data points found\n", G_program_name());
        exit(1);
    }
    nsearch = npoints < search_points ? npoints : search_points;

  if(!noindex->answer)
  {
    /* Arbitrary point to switch between searching algorithms. Could do
     * with refinement PK */
    if( (window.rows*window.cols)/npoints > 400 )
    {
        /* Using old algorithm.... */
        searchallpoints = 1;
        ncells = 0;
       
        /* Make an array to contain the row and column indices that have
         * sites in them; later will just search through all these. */
        for( searchrow=0; searchrow<window.rows; searchrow++)
            for(searchcolumn=0; searchcolumn<window.cols; searchcolumn++)
                if( npoints_currcell[searchrow][searchcolumn] > 0 )
                {
                    shortlistrows = (int *)G_realloc(shortlistrows, 
                                                 (1 + ncells)*sizeof(int));
                    shortlistcolumns = (int *)G_realloc(shortlistcolumns, 
                                                 (1 + ncells)*sizeof(int));
                    shortlistrows[ncells] = searchrow;
                    shortlistcolumns[ncells] = searchcolumn;
                    ncells++;
                }
    }        
    else
    {
        /* Fill look-up table of row and column offsets for
         * doing a circular region growing search looking for sites */
        /* Use units of column width */
        max_radius = (int)(0.5 + sqrt(window.cols*window.cols +
          (window.rows*window.ns_res/window.ew_res)*(window.rows*window.ns_res/window.ew_res)));

        search_list = (struct cell_list **)G_malloc(max_radius * sizeof(struct cell_list));
        search_list_start = (struct cell_list **)G_malloc(max_radius * sizeof(struct cell_list));
   
        for(radius = 0; radius < max_radius; radius++)
            search_list[radius] = NULL;
       
        for(row = 0; row < window.rows; row++)
            for(col = 0; col < window.cols; col++)
            {
                radius = (int)sqrt(row*row + col*col);
                if (search_list[radius] == NULL)
                    search_list[radius] = 
                      search_list_start[radius] = G_malloc(sizeof(struct cell_list));
                else
                    search_list[radius] =
                      search_list[radius]->next = G_malloc(sizeof(struct cell_list));
            
                search_list[radius]->row = row;
                search_list[radius]->column = col;
                search_list[radius]->next = NULL;                                        
            }
    }
  }
   
/* allocate buffers, etc. */
   
    dcell=G_allocate_d_raster_buf();
    
    if ((maskfd = G_maskfd()) >= 0)
        mask = G_allocate_cell_buf();
    else
        mask = NULL;


    fd=G_open_raster_new(parm.output->answer, DCELL_TYPE);
    if (fd < 0)
    {
        fprintf (stderr, "%s: can't create %s\n", G_program_name(), parm.output->answer);
        exit(1);
    }

                                  
    fprintf (stderr, "Interpolating raster map <%s> ... %d rows ... ",
        parm.output->answer, window.rows);

    north = window.north + window.ns_res/2.0;
    for (row = 0; row < window.rows; row++)
    {
        fprintf (stderr, "%-10d\b\b\b\b\b\b\b\b\b\b", window.rows-row);

        if (mask)
        {
            if(G_get_map_row(maskfd, mask, row) < 0)
                exit(1);
        }
        north -= window.ns_res;
        east = window.west - window.ew_res/2.0;
        for (col = 0; col < window.cols; col++)
        {
            east += window.ew_res;
                /* don't interpolate outside of the mask */
            if (mask && mask[col] == 0)
            {
                G_set_d_null_value(&dcell[col], 1);
                continue;
            }

	    /* If current cell contains more than nsearch points just average
             * all the points in this cell and don't look in any others */
           
            if (!(noindex->answer) && npoints_currcell[row][col] >= nsearch)
            {
                sum1 = 0.0;
                for (i = 0; i < npoints_currcell[row][col]; i++)                
                    sum1 += points[row][col][i].z;
               
                interp_value = sum1/npoints_currcell[row][col];
            }
            else
            {
	      if(noindex->answer)
	        calculate_distances_noindex(north, east);
	      else
	      {
                pointsfound = 0;
                i = 0;

                if( searchallpoints == 1 )
                {
                    /* If there aren't many sites just check them all to find
                     * the nearest */
                    for( n=0; n<ncells; n++)
                        calculate_distances(shortlistrows[n], shortlistcolumns[n], 
                                            north, east, &pointsfound);
                }
                else
                {
                    radius = 0;
                    while(pointsfound < nsearch)
                    {
                        /* Keep widening the search window until we find
                         * enough points */
                        search_list[radius] = search_list_start[radius];
                        while(search_list[radius] != NULL)
                        {                    
                            /* Always */
                            if (row < (window.rows - search_list[radius]->row) &&
                                col < (window.cols - search_list[radius]->column))
                            {
                                searchrow = row + search_list[radius]->row;
                                searchcolumn = col + search_list[radius]->column;
                                calculate_distances(searchrow, searchcolumn, north, east, &pointsfound);
                            }
     
                            /* Only if at least one offset is not 0 */
                            if ((search_list[radius]->row > 0  ||
                                 search_list[radius]->column > 0 ) &&
                                row >= search_list[radius]->row &&
                                col >= search_list[radius]->column )                           
                            {            
                                searchrow = row - search_list[radius]->row;
                                searchcolumn = col - search_list[radius]->column;                          
                                calculate_distances(searchrow, searchcolumn, north, east, &pointsfound);
                            }
 
                            /* Only if both offsets are not 0 */
                            if (search_list[radius]->row > 0  && 
                                search_list[radius]->column > 0 )
                            {
                                if (row < (window.rows - search_list[radius]->row) &&
                                    col >= search_list[radius]->column )                           
                                {
                                    searchrow = row + search_list[radius]->row;
                                    searchcolumn = col - search_list[radius]->column;
                                    calculate_distances(searchrow, searchcolumn, north, east, &pointsfound);
                                }
                                if (row >= search_list[radius]->row &&
                                    col < (window.cols - search_list[radius]->column))
                                {
                                    searchrow = row - search_list[radius]->row;
                                    searchcolumn = col + search_list[radius]->column;
                                    calculate_distances(searchrow, searchcolumn, north, east, &pointsfound);
                                }
                            }
                    
                            search_list[radius] = search_list[radius]->next;
                        }
                        radius++;
                    }               
                }
              }
	       
                /* interpolate */
                sum1 = 0.0;
                sum2 = 0.0;
                for (n = 0; n < nsearch; n++)
                {
                    if(dist = list[n].dist)
                    {
                        sum1 += list[n].z / dist;
                        sum2 += 1.0/dist;
                    }
                    else
                    {
                        /* If one site is dead on the centre of the cell, ignore
                         * all the other sites and just use this value. 
                         * (Unlikely when using floating point numbers?) */
                        sum1 = list[n].z;
                        sum2 = 1.0;
                        break;
                    }
                }
                interp_value = sum1/sum2;
            }
            dcell[col] = (DCELL) interp_value;
        }
        G_put_d_raster_row(fd,dcell);
    }
    G_close_cell(fd);
    fprintf (stderr, "done          \n");
    exit(0);
}

void newpoint ( double z,double east,double north)
{
    int row, column;

    row   = (int)((window.north - north) / window.ns_res);
    column = (int)((east - window.west) / window.ew_res);
 
    if (row<0 || row>=window.rows || column<0 || column>=window.cols)
        ;
    else /* For now ignore sites outside current region */
    {
  if(!noindex->answer)
  {
        points[row][column] = (struct Point *) G_realloc (points[row][column],
                  (1 + npoints_currcell[row][column]) * sizeof (struct Point));
        points[row][column][npoints_currcell[row][column]].north = north;
        points[row][column][npoints_currcell[row][column]].east  = east;
        points[row][column][npoints_currcell[row][column]].z     = z;
        npoints_currcell[row][column]++;
  }
  else
  {
        noidxpoints = (struct Point *) G_realloc(noidxpoints, 
                              (1 + npoints) * sizeof (struct Point));
        noidxpoints[npoints].north = north;
        noidxpoints[npoints].east  = east;
        noidxpoints[npoints].z     = z;
  }
        npoints++;
        if(!(npoints%1000)) 
            fprintf(stderr,"%10ld sites\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", npoints);
    }
}

void calculate_distances(int row, int column, double north, 
                         double east, int *pointsfound)
{
    int j, n, max;
    double dx, dy, dist;
    static double maxdist;
   
               /* Check distances and find the points to use in interpolation */
    for (j = 0; j < npoints_currcell[row][column]; j ++)
    {
               /* fill list with first nsearch points */
        if (i < nsearch)
        {
            dy = points[row][column][j].north - north;
            dx = points[row][column][j].east  - east;
            list[i].dist = dy*dy + dx*dx;
            list[i].z = points[row][column][j].z;
            i++;

            /* find the maximum distance */
            if (i == nsearch)
            {
                maxdist = list[max=0].dist;
                for (n = 1; n < nsearch; n++)
                {
                    if (maxdist < list[n].dist)
                        maxdist = list[max=n].dist;
                }
            }
        }
        else
        {
       
            /* go thru rest of the points now */
            dy = points[row][column][j].north - north;
            dx = points[row][column][j].east  - east;
            dist = dy*dy + dx*dx;

            if (dist < maxdist)
            {
                /* replace the largest dist */
                list[max].z = points[row][column][j].z;
                list[max].dist = dist;
                maxdist = list[max=0].dist;
                for (n = 1; n < nsearch; n++)
                {
                    if (maxdist < list[n].dist)
                        maxdist = list[max=n].dist;
                }
            }
        
        }
    }
    *pointsfound += npoints_currcell[row][column];
}

void calculate_distances_noindex(double north, double east)
{
    int n, max;
    double dx, dy, dist;
    double maxdist;

                /* fill list with first nsearch points */
	    for (i = 0; i < nsearch ; i++)
	    {
		dy = noidxpoints[i].north - north;
		dx = noidxpoints[i].east  - east;
		list[i].dist = dy*dy + dx*dx;
		list[i].z = noidxpoints[i].z;
	    }
		/* find the maximum distance */
	    maxdist = list[max=0].dist;
	    for (n = 1; n < nsearch; n++)
	    {
		if (maxdist < list[n].dist)
		    maxdist = list[max=n].dist;
	    }
		/* go thru rest of the points now */
	    for ( ; i < npoints; i++)
	    {
		dy = noidxpoints[i].north - north;
		dx = noidxpoints[i].east  - east;
		dist = dy*dy + dx*dx;

		if (dist < maxdist)
		{
			/* replace the largest dist */
		    list[max].z = noidxpoints[i].z;
		    list[max].dist = dist;
		    maxdist = list[max=0].dist;
		    for (n = 1; n < nsearch; n++)
		    {
			if (maxdist < list[n].dist)
			    maxdist = list[max=n].dist;
		    }
		}
	    }

}
