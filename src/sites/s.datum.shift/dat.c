/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:       s.datum.shift
 * AUTHOR(S):    Andreas Lange - andreas.lange@rhein-main.de
 * PURPOSE:      create a list of all available map datums.
 * COPYRIGHT:    (C) 2000 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *   	    	 License (>=v2). Read the file COPYING that comes with GRASS
 *   	    	 for details.
 *
 *****************************************************************************/

#include <string.h>
#include "gis.h"
#include "CC.h"

char *
datum_list(void)
{
  int len, n;
  char *list;
  char *name;
  
  len=0;
  for (n=0; (name = CC_datum_name(n)) != NULL; n++)
    len += strlen(name)+1;
  list = G_malloc(len);
  for (n=0; (name = CC_datum_name(n)) != NULL; n++)
    {
      if (n) 
	G_strcat (list, ",");
      else 
	*list = '\0';
      G_strcat (list, name);
    }
  return list;
}
