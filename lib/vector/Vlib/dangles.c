/**************************************************************
 *
 * MODULE:       vector library
 *  
 * AUTHOR(S):    Radim Blazek
 *               
 * PURPOSE:      Clean lines
 *               
 * COPYRIGHT:    (C) 2001 by the GRASS Development Team
 *
 *               This program is free software under the 
 *               GNU General Public License (>=v2). 
 *               Read the file COPYING that comes with GRASS
 *               for details.
 *
 **************************************************************/
#include <stdlib.h> 
#include "gis.h"
#include "Vect.h"

void remove_dangles ( struct Map_info *Map, int type, int chtype, double maxlength, 
	              struct Map_info *Err, FILE *msgout );

/*!
 \fn void Vect_remove_dangles ( struct Map_info *Map, int type, double maxlength, 
                              struct Map_info *Err, FILE *msgout)
 \brief Remove dangles from vector map.

  Remove dangles of given type shorter than maxlength from vector map. 
  Line is considered to be a dangle if on at least one end node is no other line of given type(s).
  If a dangle is formed by more lines, such string of lines is taken as one dangle and 
  either deleted are all parts or nothing.
  Optionaly deleted dangles are written to error map. 
  Input map must be opened on level 2 for update.

 \param Map input map where have to be deleted
 \param type type of dangles 
 \param maxlength maxlength of dangles or -1 for all dangles
 \param Err vector map where deleted dangles are written or NULL
 \param msgout file pointer where messages will be written or NULL
*/
void 
Vect_remove_dangles ( struct Map_info *Map, int type, double maxlength, struct Map_info *Err, FILE *msgout )
{
    remove_dangles ( Map, type, 0, maxlength, Err, msgout );
}
    
/*!
 \fn void Vect_chtype_dangles ( struct Map_info *Map, double maxlength, 
                              struct Map_info *Err, FILE *msgout)
 \brief Change boundary dangles to lines.

  Change boundary dangles to lines. 
  Boundary is considered to be a dangle if on at least one end node is no other boundary.
  If a dangle is formed by more boundaries, such string of boundaries is taken as one dangle and 
  either deleted are all parts or nothing.
  Optionaly deleted dangles are written to error map. 
  Input map must be opened on level 2 for update.

 \param Map input map where have to be deleted
 \param type type of dangles 
 \param maxlength maxlength of dangles or -1 for all dangles
 \param Err vector map where deleted dangles are written or NULL
 \param msgout file pointer where messages will be written or NULL
*/
void 
Vect_chtype_dangles ( struct Map_info *Map, double maxlength, struct Map_info *Err, FILE *msgout )
{
    remove_dangles ( Map, 0, 1, maxlength, Err, msgout );
}
/*
  Remove dangles of given type shorter than maxlength from vector map. 
  Line is considered to be a dangle if on at least one end node is no other line of given type(s).
  If a dangle is formed by more lines, such string of lines is taken as one dangle and 
  either deleted are all parts or nothing.
  Optionaly, if chtype is set to 1, only GV_BOUNDARY are checked for dangles, and if dangle is found
  lines are not deleted but rewritten with type GVLINE.
  Optionaly deleted dangles are written to error map. 
  Input map must be opened on level 2 for update.

  Parameters:
  Map input map where have to be deleted
  type type of dangles 
  chtype change boundaries to lines
  maxlength maxlength of dangles or -1 for all dangles
  Err vector map where deleted dangles are written or NULL
  msgout file pointer where messages will be written or NULL
*/
void 
remove_dangles ( struct Map_info *Map, int type, int chtype, double maxlength, struct Map_info *Err, FILE *msgout )
{
    struct line_pnts *Points;
    struct line_cats *Cats;
    int    i, line, ltype, next_line, nnodelines;
    int    nnodes, node, node1, node2, next_node;
    int    lcount, tmp_next_line;
    double length;
    int    dangles_removed = 0; /* number of removed dangles */
    int    lines_removed = 0; /* number of lines removed */
    struct ilist *List; /* List of lines in chain */
    char   *lmsg;

    if ( chtype ) {
	type = GV_BOUNDARY; /* process boundaries only */
	lmsg = "changed lines";
    } else {
	lmsg = "removed lines";
    }
    
    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();
    List = Vect_new_list ();
    
    if ( msgout ) fprintf (msgout, "Removed dangles: %5d  %s: %5d", 
	                            dangles_removed, lmsg, lines_removed ); 

    nnodes = Vect_get_num_nodes (Map);
    G_debug (1, "nnodes =  %d", nnodes );
    
    for ( node = 1; node <= nnodes; node++ ){ 
	G_debug (2, "node =  %d", node);
	if ( !Vect_node_alive (Map, node) ) continue; 
	
	nnodelines = Vect_get_node_n_lines ( Map, node );
    
	lcount = 0; /* number of lines of given type */
	for ( i = 0; i < nnodelines; i++) {
	    line = Vect_get_node_line (Map, node, i);
	    G_debug (3, "    node line %d = %d", i, line);
	    
	    ltype = Vect_read_line (Map, NULL, NULL, abs(line));
	    
	    if ( ltype & type ) {
		lcount++;
		next_line = line;
	    }
	}

	Vect_reset_list ( List );
	if ( lcount == 1 ) {
	    G_debug (3, "    node %d is dangle -> follow the line %d", node, next_line);

	    while ( next_line != 0 ) {
	        Vect_list_append ( List, abs(next_line) );

		/* Look at the next end of the line if just one another line of the type is connected */
	        Vect_get_line_nodes ( Map, abs(next_line), &node1, &node2 );
		next_node = next_line > 0 ? node2 : node1;
		
		G_debug (3, "    next_node = %d", next_node);

		nnodelines = Vect_get_node_n_lines ( Map, next_node );
	    
		lcount = 0; /* number of lines of given type (except current next_line) */
		for ( i = 0; i < nnodelines; i++) {
		    line = Vect_get_node_line (Map, next_node, i);
		    G_debug (3, "      node line %d = %d", i, line);
		    
	            ltype = Vect_read_line (Map, NULL, NULL, abs(line));

		    if ( ltype & type && abs(line) != abs(next_line) ) {
			lcount++;
			tmp_next_line = line;
		    }
		}
		if ( lcount == 1 )
		    next_line = tmp_next_line;
		else
		    next_line = 0;
		    
	    }

	    /* Length of the chain */
	    length = 0;
	    for ( i = 0; i < List->n_values; i++) {
		G_debug (3, "  chain line %d = %d", i, List->value[i]);
		ltype = Vect_read_line (Map, Points, NULL, List->value[i]);
		length += Vect_line_length ( Points );
	    }

	    if ( maxlength < 0 || length < maxlength ) { // delete the chain
		G_debug (3, "  delete the chain" );
		
		for ( i = 0; i < List->n_values; i++) {
		    ltype = Vect_read_line (Map, Points, Cats, List->value[i]);
		    
		    // Write to Err deleted dangle
		    if ( Err ) {
			Vect_write_line ( Err, ltype, Points, Cats );
		    }
			
		    if ( !chtype ) {
		        Vect_delete_line (Map, List->value[i]);
		    } else { 
		        G_debug (3, "  rewrite line %d", List->value[i] );
			Vect_rewrite_line ( Map, List->value[i], GV_LINE, Points, Cats);
		    }
			
		    lines_removed++;
		}
	    }
	    if ( msgout ) {
		if ( msgout ) fprintf (msgout, "\rRemoved dangles: %5d  %s: %5d", 
						dangles_removed, lmsg, lines_removed ); 
		fflush ( msgout );
	    }
	    dangles_removed++;
	}
    }
    if ( msgout ) {
	if ( msgout ) fprintf (msgout, "\rRemoved dangles: %5d  %s: %5d", 
					dangles_removed, lmsg, lines_removed ); 
        fprintf (msgout, "\n" ); 
    }
}

