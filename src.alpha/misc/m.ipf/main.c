/*-
 * m.ipf - iterative marginal fitting and smoothing of zero counts
 * Copyright (C) 1994. James Darrell McCauley.
 *
 * Author: James Darrell McCauley (mccauley@ecn.purdue.edu)
 *         USDA Fellow
 *         Department of Agricultural Engineering
 *         Purdue University
 *         West Lafayette, Indiana 47907-1146 USA
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for non-commercial purposes is hereby granted. This
 * software is provided "as is" without express or implied warranty.
 *
 * JAMES DARRELL MCCAULEY (JDM) MAKES NO EXPRESS OR IMPLIED WARRANTIES
 * (INCLUDING BY WAY OF EXAMPLE, MERCHANTABILITY) WITH RESPECT TO ANY
 * ITEM, AND SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL
 * OR CONSEQUENTAL DAMAGES ARISING OUT OF THE POSSESSION OR USE OF
 * ANY SUCH ITEM. LICENSEE AND/OR USER AGREES TO INDEMNIFY AND HOLD
 * JDM HARMLESS FROM ANY CLAIMS ARISING OUT OF THE USE OR POSSESSION
 * OF SUCH ITEMS.
 *
 * Modification History:
 */

#include "version.h"
#include <math.h>
#include "gis.h"

int main (argc, argv)
  char **argv;
  int argc;
{
  int i, j, n, pn, **matrix;
  char format[32], *input;
  double stop, **smoothed, **normalized, **ipf(), **elimzero();
  FILE *fd;

  struct
  {
    struct Option *input, *format, *stop;
  } parm;
  struct
  {
    struct Flag *dozero, *marginal, *showstop;
  } flag;

  G_gisinit (argv[0]);

  parm.input = G_define_option ();
  parm.input->key = "input";
  parm.input->type = TYPE_STRING;
  parm.input->required = NO;
  parm.input->description = "unix file containing sites";

  parm.format = G_define_option ();
  parm.format->key = "format";
  parm.format->key_desc = "conversion_string";
  parm.format->type = TYPE_STRING;
  parm.format->required = NO;
  parm.format->description = "format to print results [%7.3f]";

  parm.stop = G_define_option ();
  parm.stop->key = "stop";
  parm.stop->type = TYPE_DOUBLE;
  parm.stop->required = NO;
  parm.stop->description = "stopping criteria [100.01]";

  flag.dozero = G_define_flag ();
  flag.dozero->key = 'z';
  flag.dozero->description = "print table smoothed with zero counts";
  flag.dozero->answer = NULL;

  flag.marginal = G_define_flag ();
  flag.marginal->key = 'm';
  flag.marginal->description = "print marginals with all tables";
  flag.marginal->answer = NULL;

  flag.showstop = G_define_flag ();
  flag.showstop->key = 'e';
  flag.showstop->description = "indicate when ipf stopped";
  flag.showstop->answer = NULL;

  if (G_parser (argc, argv))
    exit (1);
  if (input = parm.input->answer)
  {
    if ((fd = fopen (input, "r")) == NULL)
    {
      fprintf (stderr, "%s - ", G_program_name ());
      perror (input);
      exit (1);
    }
  }
  else
    fd = stdin;

  if (parm.format->answer)
    sprintf (format, "%s ", parm.format->answer);
  else
    G_strcpy (format, "%7.3f ");

  if (parm.stop->answer)
    sscanf (parm.stop->answer, "%lf", &stop);
  else
    stop = 100.01;


  /* get busy */

  if (fscanf (fd, "%d\n", &n) != 1)
    G_fatal_error ("Error reading contingency table size");
  if ((matrix = (int **) G_malloc (n * sizeof (int *))) == NULL)
    G_fatal_error ("main: problems allocating memory 1");
  else
    for (i = 0; i < n; ++i)
      if ((matrix[i] = (int *) G_malloc (n * sizeof (int))) == NULL)
	G_fatal_error ("main: problems allocating memory 2");

  if (flag.marginal->answer!=NULL)
    pn = n + 1;
  else
    pn = n;

  for (i = 0; i < n; ++i)
    for (j = 0; j < n; ++j)
      if (fscanf (fd, "%d\n", &matrix[i][j]) != 1)
	G_fatal_error ("Error reading contingency table");

  smoothed = elimzero (n, matrix);

  if (flag.dozero->answer!=NULL) 
  {
    for (i = 0; i < pn; ++i)
    {
      for (j = 0; j < pn; ++j)
	printf (format, smoothed[i][j]);
      printf ("\n");
    }
    printf ("\n");
  }

  normalized = ipf (n, smoothed, stop, (flag.showstop->answer==NULL) ? 0 : 1);
  for (i = 0; i < pn; ++i)
  {
    for (j = 0; j < pn; ++j)
      printf (format, normalized[i][j]);
    printf ("\n");
  }
  return 0;
}
