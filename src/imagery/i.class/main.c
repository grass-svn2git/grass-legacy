#define MAIN
#define GLOBAL

#include "globals.h"

int error();

main(argc, argv)
     int argc;
     char *argv[];
{
  char *tempfile;
  char name[40], mapset[40];
  int row,nrows;
  int col,ncols;
  struct Cell_head window, cellhd;

  /* Initialize the gis library */
  G_gisinit(argv[0]);

  /* must have a graphics terminal selected */
  R_open_driver();

  /* check to see if a MASK is set */
  if (G_maskfd() >= 0)
    {
      printf ("\nWARNING: You have a mask set.");
      printf ("\nIf you continue the mask will be removed.\n");
      if (!G_yes("Do you want to continue? ", -1)) exit(0);
    }
  /* remove any old mask */
  remove_mask();

  /* get group/subgroup, and signature files */
  ask_files(argv[0]);

  /* initialize the Region structure */
  init_region();

  tempfile = G_tempfile();
  interrupt_char = G_intr_char();

  /* initialize the graphics */
  g_init();

  /* set up signal handling */
  set_signals();

  /* put out a title */
  display_title(VIEW_MAP1);

  /* ask the user for the cell map to be displayed */
  if (G_ask_cell_old("Enter the name of the cell map to be displayed",
		     name) == NULL)
    exit(0);
  strcpy(mapset, G_find_cell(name, ""));
  if(G_get_cellhd(name, mapset, &cellhd)!=0)
    G_fatal_error("Did not find input cell map"); 
  G_adjust_window_to_box (&cellhd, &VIEW_MAP1->cell.head, VIEW_MAP1->nrows, VIEW_MAP1->ncols);
  Configure_view (VIEW_MAP1, name, mapset, cellhd.ns_res, cellhd.ew_res);
  /* configure the MASK view right over the top of the map1 view */
  G_adjust_window_to_box(&cellhd, &VIEW_MASK1->cell.head, VIEW_MASK1->nrows,
			 VIEW_MASK1->ncols);
  Configure_view(VIEW_MASK1, "MASK", G_mapset(), cellhd.ns_res, cellhd.ew_res);

  draw_cell(VIEW_MAP1,OVER_WRITE);

  /* Initialize the text terminal */
  Begin_curses();
  Curses_clear_window (PROMPT_WINDOW);

  Region.saved_npoints = 0;
  
  G_set_error_routine (error);
	
  driver();
  
  quit();

}

quit()
{
  write_signatures();
  End_curses();
  R_close_driver();
}

error(msg, fatal) 
     char *msg; 
{
  char buf[200]; 
  int x,y,button;

  Curses_clear_window (PROMPT_WINDOW);
  Curses_write_window (PROMPT_WINDOW,1,1, "LOCATION:\n");
  Curses_write_window (PROMPT_WINDOW,1,12,G_location());
  Curses_write_window (PROMPT_WINDOW,2,1, "MAPSET:\n");
  Curses_write_window (PROMPT_WINDOW,2,12,G_location());
  Beep();
  if (fatal)
    sprintf (buf, "ERROR: %s", msg);
  else
    sprintf (buf, "WARNING: %s (click mouse to continue)", msg);
  Menu_msg (buf);
  
  if (fatal)
    quit();
  Mouse_pointer (&x, &y, &button);
  Curses_clear_window (PROMPT_WINDOW);
}
