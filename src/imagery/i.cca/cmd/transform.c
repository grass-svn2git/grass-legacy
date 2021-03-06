
#include "gis.h"
#include "globals.h"


/***************************************************************************/

transform(datafds,outfds,rows,cols,eigmat,bands,mins,maxs)
int datafds[MX], outfds[MX];
int   rows, cols;
double eigmat[MX][MX];
int bands;
CELL mins[MX],maxs[MX];
{
  int i,j,k,l;
  double sum[MX];
  CELL *rowbufs[MX];

  /* allocate row buffers for each band */
  for (i=1; i<=bands; i++)
    if((rowbufs[i] = G_allocate_cell_buf()) == NULL)
      G_fatal_error("unable to allocate cell buffers.");

  for(i=0 ; i<rows ; i++) {
    /* get one row of data */
    for (j=1; j<=bands; j++)
      if (G_get_map_row(datafds[j], rowbufs[j], i) < 0)
        G_fatal_error("Error reading cell map during transform.");

    /* transform each cell in the row */
    for (l=0; l<cols; l++) {
      for(j=1 ; j<=bands ; j++) {
        sum[j]=0.0;
        for(k=1 ; k<=bands ; k++)       {
          sum[j] += eigmat[j][k] * (double) rowbufs[k][l];
        }
      }
      for(j=1 ; j<=bands ; j++) {
        rowbufs[j][l] = (CELL) (sum[j] + 0.5);
        if (rowbufs[j][l] > maxs[j]) maxs[j] = rowbufs[j][l];
        if (rowbufs[j][l] < mins[j]) mins[j] = rowbufs[j][l];
      }
    }

    /* output the row of data */
    for (j=1; j<=bands; j++)
      if (G_put_map_row(outfds[j], rowbufs[j]) < 0)
        G_fatal_error("Error reading cell map during transform.");
  }
  for (i=1; i<=bands; i++) free(rowbufs[i]);

  printf("Transform completed.\n");
}

/***************************************************************************/
