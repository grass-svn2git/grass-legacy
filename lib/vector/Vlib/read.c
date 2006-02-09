/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Original author CERL, probably Dave Gerdes or Mike Higgins.
*               Update to GRASS 5.7 Radim Blazek and David D. Gray.
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <grass/Vect.h>


static int read_next_dummy () { return -1; }

#ifndef HAVE_OGR
static int format () { G_fatal_error ("Requested format is not compiled in this version"); return 0; }
#endif

static int (*Read_next_line_array[][3]) () =
{
    { read_next_dummy, V1_read_next_line_nat, V2_read_next_line_nat }
#ifdef HAVE_OGR
   ,{ read_next_dummy, V1_read_next_line_ogr, V2_read_next_line_ogr }
#else
   ,{ read_next_dummy, format, format }
#endif
};

static int (*V2_read_line_array[]) () =
{
   V2_read_line_nat 
#ifdef HAVE_OGR
   , V2_read_line_ogr
#else
   , format
#endif
};

/*!
 \fn int Vect_read_next_line ( struct Map_info *Map,
    struct line_pnts *line_p,
    struct line_cats *line_c)
 \brief get next vector line ?
 \return line type,
           -1 on Out of memory,
           -2 on EOF   
 \param Map_info structure, line_pnts structure, line_cats structure
*/
int
Vect_read_next_line (
    struct Map_info *Map,
    struct line_pnts *line_p,
    struct line_cats *line_c)
{
#ifdef GDEBUG
    G_debug (3, "Vect_read_next_line()");
#endif    
  
    if (!VECT_OPEN (Map))
        return -1;

    return (*Read_next_line_array[Map->format][Map->level]) (Map, line_p, line_c);
}

/*!
 \fn int Vect_read_line ( struct Map_info *Map,
    struct line_pnts *line_p,
    struct line_cats *line_c,
    int    line)
 \brief get vector line ?
 \return line type,
           -1 on Out of memory,
           -2 on EOF   
 \param Map_info structure, line_pnts structure, line_cats structure, line number
*/
int
Vect_read_line (
                struct Map_info *Map,
                struct line_pnts *line_p,
                struct line_cats *line_c,
                int    line)
{

    G_debug (3, "Vect_read_line()");
  
    if (!VECT_OPEN (Map))
        G_fatal_error ( "Vect_read_line(): vector is not opened" );

    if (line < 1 || line > Map->plus.n_lines)
        G_fatal_error ( "Vect_read_line(): line '%d' is not reasonable (max line in map: %d)",
	                 line, Map->plus.n_lines );
    
    return (*V2_read_line_array[Map->format]) (Map, line_p, line_c, line);
}

/*!
 \fn int Vect_line_alive ( struct Map_info *Map, int line )
 \brief check if line is alive or dead
 \return 1 if line alive, 0 if line is dead
 \param Map_info structure, line number
*/
int
Vect_line_alive ( struct Map_info *Map, int line )
{
    if ( Map->plus.Line[line] != NULL ) return 1;
    
    return 0;
}

/*!
 \fn int Vect_node_alive ( struct Map_info *Map, int node )
 \brief check if node is alive or dead
 \return 1 if node alive, 0 if node is dead
 \param Map_info structure, node number
*/
int
Vect_node_alive ( struct Map_info *Map, int node )
{
    if ( Map->plus.Node[node] != NULL ) return 1;
    
    return 0;
}

/*!
 \fn int Vect_area_alive ( struct Map_info *Map, int area )
 \brief check if area is alive or dead
 \return 1 if area alive, 0 if area is dead
 \param Map_info structure, area number
*/
int
Vect_area_alive ( struct Map_info *Map, int area )
{
    if ( Map->plus.Area[area] != NULL ) return 1;
    
    return 0;
}

/*!
 \fn int Vect_isle_alive ( struct Map_info *Map, int isle )
 \brief check if isle is alive or dead
 \return 1 if isle alive, 0 if isle is dead
 \param Map_info structure, isle number
*/
int
Vect_isle_alive ( struct Map_info *Map, int isle )
{
    if ( Map->plus.Isle[isle] != NULL ) return 1;
    
    return 0;
}

