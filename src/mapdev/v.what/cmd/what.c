#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gis.h"
#include "display.h"
#include "raster.h"
#include "Vect.h"
#include "local_proto.h"

static int nlines = 50;

int 
what (char **coords, int interactive, int once, struct Map_info *Map, struct Categories *Cats)
{
  int i, j;
  int lcat, acat;
  int row, col;
  int nrows, ncols;
  struct Cell_head window;
  int screen_x, screen_y;
  double east, north;
  float tmp_east, tmp_north;
  int button;
  double sq_meters;
  char buf[1024];

  P_LINE *Line;
  P_AREA *Area;
  plus_t line, area;
  struct line_pnts * Points; /* used during area calculation */

  G_get_set_window (&window);
  /* 
   inititalizations needed to do area calculations for
   grid cells, based on the current window "projection" field.
  */
  G_begin_polygon_area_calculations();

  nrows = window.rows;
  ncols = window.cols;

  screen_x = ((int) D_get_d_west () + (int) D_get_d_east ()) / 2;
  screen_y = ((int) D_get_d_north () + (int) D_get_d_south ()) / 2;

  if(coords)
  {
      interactive = 0;
      once = 0;
  }

  i = 0;
  do
  {
    if (interactive)
    {
      show_buttons (once);
      R_get_location_with_pointer (&screen_x, &screen_y, &button);
      if (button == 3)
        break;
      east = D_d_to_u_col ((double) screen_x);
      north = D_d_to_u_row ((double) screen_y);
    }
    else
    {
      if(!coords && isatty(0))
      {
        fprintf(stdout,"Enter E N coordinates, Ctrl-D to stop:\n");
        fflush(stdout);
      }
      if(!coords)
      {
          while(buf[0] = 0, fgets(buf, 1024, stdin),
	        sscanf(buf, "%f %f", &tmp_east, &tmp_north) != 2)
	  if(!strncmp(buf, "end\n", 4) || !strncmp(buf, "exit\n", 5) || !buf[0]){
	      buf[0] = 0;
              break;
	  }

          if(!buf[0])
	      break;

          east = (double) tmp_east;
          north = (double) tmp_north;
      }
      else
      {
	  if(!coords[i])
	      break;

	  east = atof(coords[i++]);
	  north = atof(coords[i++]);
      }
    }
    row = (window.north - north) / window.ns_res;
    col = (east - window.west) / window.ew_res;
    if (row < 0 || row >= nrows)
      continue;
    if (col < 0 || col >= ncols)
      continue;

    line = dig_point_to_line (Map, east, north, -1);
    area = dig_point_to_area (Map, east, north);

    printf ("\nUTM  - %9.2f %10.2f\n", east, north);
    nlines++;

    if (line + area == 0)
    {
      printf ("Nothing Found.\n");
      nlines++;
    }

    if (line == 0)
      /* printf ("Line not found.\n") */
      ;
    else
    {
      Line = &(Map->Line[line]);
      if (Line->att)
      {
	lcat = Map->Att[Line->att].cat;
	if (Cats->num > 0)
	  printf ("Line - Category %d %s\n", lcat,
		  G_get_cat (lcat, Cats));
	else
	  printf ("Line - Category %d <not labeled>\n", lcat);
      }
      else
	printf ("Line - Category <not tagged>\n");
      nlines++;
    }

    if (area == 0)
      /* printf ("Area not found.\n")  */
      ;
    else
    {
      Area = &(Map->Area[area]);
      if (Area->att)
      {
	acat = Map->Att[Area->att].cat;
	if (Cats->num > 0)
	  printf ("Area - Category %d %s\n", acat,
		  G_get_cat (acat, Cats));
	else
	  printf ("Area - Category %d <not labeled>\n", acat);
      }
      else
	printf ("Area - Category <not tagged>\n");

	/* Initialization of the structure */
	Points = Vect_new_line_struct();

	/* First, calculating the whole area */
	Vect_get_area_points(Map, area, Points);
	sq_meters = 
	  G_area_of_polygon(Points->x, Points->y, Points->n_points);

	/* Then substracting islands, if any  */
	for(j = 0; j < Area->n_isles; j++) {
		Vect_get_isle_points(Map, Area->isles[j], Points);
		sq_meters -= 
		  G_area_of_polygon(Points->x, Points->y, Points->n_points);
	}

	/* Now tell the world */
      printf ("Size - Sq Meters: %.3f\t\tHectares: %.3f\n",
	      sq_meters, (sq_meters / 10000.));

      printf ("           Acres: %.3f\t\tSq Miles: %.4f\n",
	      ((sq_meters * 10.763649) / 43560.), ((sq_meters * 10.763649) / 43560.) / 640.);
      nlines += 3;

    }

  } while (!once);

  return 0;
}

/* TODO */
int 
show_buttons (int once)
{
  if (once)
  {
    fprintf (stderr, "\nClick mouse button on desired location\n\n");
    nlines = 3;
  }
  else if (nlines >= 18)	/* display prompt every screen full */
  {
    fprintf (stderr, "\n");
    fprintf (stderr, "Buttons\n");
    fprintf (stderr, " Left:  what's here\n");
    fprintf (stderr, " Right: quit\n");
    nlines = 4;
  }

  return 0;
}
