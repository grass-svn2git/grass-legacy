/*
 * Based on:
 *  s.rand
 *  Copyright (C) 1993-1995. James Darrell McCauley.
 *
 * Author: James Darrell McCauley darrell@mccauley-usa.com
 * 	                          http://mccauley-usa.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Modification History:
 *
 * $Id$
 *
 * s.rand v 0.5B <25 Jun 1995> Copyright (c) 1993-1995. James Darrell McCauley
 * <?? ??? 1993> - began coding and released test version (jdm)
 * <10 Jan 1994> - changed RAND_MAX for rand(), since it is different for
 *                 SunOS 4.1.x and Solaris 2.3. stdlib.h in the latter defines
 *                 RAND_MAX, but it doesn't in the former. v0.2B (jdm)
 * <02 Jan 1995> - clean Gmakefile, man page. added html v0.3B (jdm)
 * <25 Feb 1995> - cleaned 'gcc -Wall' warnings (jdm)
 * <25 Jun 1995> - new site API (jdm)
 * <13 Sep 2000> - released under GPL
 */

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "gis.h"
#include "Vect.h"
#include "glocale.h"

#ifndef RAND_MAX 
#define RAND_MAX (pow(2.0,31.0)-1) 
#endif
double myrand(void);

#if defined(__CYGWIN__) || defined(__APPLE__) || defined(__MINGW32__)
double drand48()
{
	return(rand()/32767.0);
}
#define srand48(sv) (srand((unsigned)(sv)))
#endif

int main (int argc, char *argv[])
{
  char *output;
  double (*rng) (), max;
  int i, n, b;
  struct Map_info Out;
  struct line_pnts *Points;
  struct line_cats *Cats;
  struct Cell_head window;
  struct GModule *module;
  struct
  {
    struct Option *output, *nsites, *zmin, *zmax;
  } parm;
  struct
  {
    struct Flag *rand, *drand48, *z;
  } flag;

  G_gisinit (argv[0]);
  
  module = G_define_module();
  module->description =        
                  _("Randomly generate a 2D/3D GRASS vector points map.");
                  
  parm.output = G_define_option ();
  parm.output->key = "output";
  parm.output->type = TYPE_STRING;
  parm.output->required = YES;
  parm.output->gisprompt = "new,vector,vector";
  parm.output->description = _("vector file to be created");

  parm.nsites = G_define_option ();
  parm.nsites->key = "n";
  parm.nsites->type = TYPE_INTEGER;
  parm.nsites->required = YES;
  parm.nsites->description = _("number of points to be created");

  parm.zmin = G_define_option ();
  parm.zmin->key = "zmin";
  parm.zmin->type = TYPE_DOUBLE;
  parm.zmin->required = NO;
  parm.zmin->description = _("minimum z height (needs -z flag)");
  parm.zmin->answer = "0.0";

  parm.zmax = G_define_option ();
  parm.zmax->key = "zmax";
  parm.zmax->type = TYPE_DOUBLE;
  parm.zmax->required = NO;
  parm.zmax->description = _("maximum z height (needs -z flag)");
  parm.zmax->answer = "0.0";

  flag.z = G_define_flag ();
  flag.z->key             = 'z';
  flag.z->description = _("Create 3D output");

  flag.drand48 = G_define_flag ();
  flag.drand48->key = 'd';
  flag.drand48->description = _("Use drand48() function (default=rand() )");

  if (G_parser (argc, argv))
    exit (1);

  output = parm.output->answer;
  n = atoi(parm.nsites->answer);
  b = (flag.drand48->answer == '\0') ? 0 : 1;

  if (n <= 0) {
    G_fatal_error ( _("%s given an illegal number of sites [%d]"), G_program_name (), n);
  }

  if ( flag.z->answer )
        Vect_open_new (&Out, output, 1 ); 
  else 
        Vect_open_new (&Out, output, 0 ); 

  Vect_hist_command ( &Out );
  
  if (b)
  {
    rng=drand48;
    max=1.0;
    srand48 ((long)getpid ()); 
  }
  else  /* default is rand() */
  {
    rng=myrand;
    max=RAND_MAX;
    srand (getpid ()); 
  }

  G_get_window (&window);

  Points = Vect_new_line_struct ();
  Cats = Vect_new_cats_struct ();
  
  for(i=0; i<n; ++i)
  {
      double x, y, z;
	
      Vect_reset_line ( Points );
      Vect_reset_cats ( Cats );
      
      x = rng()/max*(window.west-window.east)+window.east;
      y = rng()/max*(window.north-window.south)+window.south;
      

      if ( flag.z->answer ) {
         z = rng()/max*(atof(parm.zmax->answer)-atof(parm.zmin->answer))+atof(parm.zmin->answer);
         Vect_append_point ( Points, x, y, z );
      } else 
         Vect_append_point ( Points, x, y, 0.0 );

      Vect_cat_set (Cats, 1, i+1);
      Vect_write_line ( &Out, GV_POINT, Points, Cats );
  }

  Vect_build (&Out, stderr);
  Vect_close (&Out);

  return (0);
}

double myrand()
{
  return (double) rand();
}

