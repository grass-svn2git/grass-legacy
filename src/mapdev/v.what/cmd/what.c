#include "gis.h"
#include "digit.h"

static int nlines = 50;

what (interactive, once, Map, Cats)
  struct Map_info *Map;
  struct Categories *Cats;
{
  int lcat, acat;
  int row, col;
  int nrows, ncols;
  struct Cell_head window;
  int screen_x, screen_y;
  double east, north;
  float tmp_east, tmp_north;
  int button;
  double D_get_d_north (), D_get_d_south ();
  double D_get_d_east (), D_get_d_west ();
  double D_d_to_u_row (), D_d_to_u_col ();
  double sq_meters;

  P_LINE *Line;
  P_AREA *Area;
  plus_t line, area;

  G_get_set_window (&window);
  nrows = window.rows;
  ncols = window.cols;

  screen_x = ((int) D_get_d_west () + (int) D_get_d_east ()) / 2;
  screen_y = ((int) D_get_d_north () + (int) D_get_d_south ()) / 2;

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
      if (scanf("%f %f", &tmp_east, &tmp_north) == EOF)
        break;
      east = (double) tmp_east;
      north = (double) tmp_north;
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

      /* Area stats - just for grins */
      dig_find_area2 (Map, Area, &sq_meters);

      printf ("Size - Sq Meters: %.3lf\t\tHectares: %.3lf\n",
	      sq_meters, (sq_meters / 10000.));

      printf ("           Acres: %.3lf\t\tSq Miles: %.4lf\n",
	      ((sq_meters * 10.763649) / 43560.), ((sq_meters * 10.763649) / 43560.) / 640.);
      nlines += 3;

    }

  } while (!once);
}

/* TODO */
show_buttons (once)
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
}
