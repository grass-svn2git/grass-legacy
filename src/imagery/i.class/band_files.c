#include "globals.h"


/* open and allocate space for the subgroup band files */
open_band_files()
{
  int n, nbands;
  char *name, *mapset;

  /* allocate row buffers and open cell files */
  nbands = Refer.nfiles ;
  Bandbuf = (CELL **) G_malloc (nbands * sizeof (CELL *));
  Bandfd = (int *) G_malloc (nbands * sizeof (int));
  for (n=0; n < nbands; n++)
    {
      Bandbuf[n] = G_allocate_cell_buf();
      name   = Refer.file[n].name;
      mapset = Refer.file[n].mapset;
      if ((Bandfd[n] = G_open_cell_old (name, mapset)) < 0)
	G_fatal_error("Unable to open band files.\n");
    }
}


/* close and free space for the subgroup band files */
close_band_files()
{
  int n, nbands;

  nbands = Refer.nfiles ;
  for (n=0; n < nbands; n++)
    {
      free(Bandbuf[n]);
      G_close_cell(Bandfd[n]);
    }
  free(Bandbuf);
  free(Bandfd);
}
