/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S): 
*               Written by (in alphabetic order):
*                     Di Simone Alessio                 a.disimone@inwind.it
*                     Di Sorbo  Alessandro              a.disorbo@inwind.it
*                     Ragni Domenico                    domrag@inwind.it
*                     Romano Enrico                     enr.omano@genie.it
*                     Serino Antonio                    antoseri@libero.it
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2002 by the authors
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include "Vect.h"
#include <stdlib.h>

#ifdef HAVE_POSTGRES

/******************************************************************************
* Function name: V1_close_post.
* Arguments    : Map.
* Return       : Status (-1 error 0 all ok), and modify Map structures.
*
* Description  :
*              Commit transaction, close connection and free Map structure
*
*******************************************************************************/

int
V1_close_post (struct Map_info *Map)
{
  G_debug (1, "V1_close_post():");

  if (!VECT_OPEN (Map)) 
      return 1;

  if (Map->mode & (GV_MODE_WRITE | GV_MODE_RW)) {
      Vect__write_head (Map);
      Vect_write_dblinks ( Map );
  }

  PQfinish (Map->fInfo.post.conn);
  return 0;
}

#endif
