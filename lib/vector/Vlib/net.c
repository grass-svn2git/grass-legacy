/*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Radim Blazek
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
#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"
#include "Vect.h"

static int clipper ( dglGraph_s    *pgraph ,
                     dglSPClipInput_s  * pargIn ,
                     dglSPClipOutput_s * pargOut ,
                     void *          pvarg )         /* caller's pointer */
{
    double cost;
    
    G_debug ( 5, "Net: clipper()" );

    if ( dglGet_NodeAttrSize(pgraph) > 0 ) {
	memcpy( &cost, dglNodeGet_Attr(pgraph, pargIn->pnNodeFrom), sizeof(cost) );
	G_debug ( 5, "  node = %d pcost = %d + %f (arc + node)", 
		           dglNodeGet_Id(pgraph, pargIn->pnNodeFrom), pargOut->nEdgeCost, cost );
	pargOut->nEdgeCost += (dglInt32_t) cost;
    }

    return 0;   
}

/*!
 \fn Vect_net_build_graph (  struct Map_info *Map,
			int ltype, 
       			int afield,
			int nfield,
			char *afcol,
			char *abcol,
			char *ncol, 
			int geo, 
		        int algorithm ) 
 \brief build network graph
 \return 0 on success, 1 on error
 \param Map_info structure, line type for arcs, arc costs field (if 0,
   use length), node costs field (if 0, do not use node costs), column
   with forward costs for arc, column with backward costs for arc (if
   NULL, back = forward), column with costs for nodes, use geodesic
   calculation for length (LL), not used (in future code for algorithm)
*/
int
Vect_net_build_graph (  struct Map_info *Map,
			int ltype,   /* line type for arcs */
       			int afield,  /* arc costs field (if 0, use length) */
			int nfield,  /* node costs field (if 0, do not use node costs) */
			char *afcol, /* column with forward costs for arc */
			char *abcol, /* column with backward costs for arc (if NULL, back = forward) */
			char *ncol,  /* column with costs for nodes */
			int geo,     /* use geodesic calculation for length (LL) */
		        int algorithm ) /* not used, in future code for algorithm */
{
    int    i, j, from, to, line, nlines, ret, type, cat, skipped, cfound;
    int    *vals, nval, dofw, dobw;
    struct line_pnts *Points;
    struct line_cats *Cats;
    double dcost, ll;
    dglInt32_t  cost, bcost;
    dglGraph_s *gr;
    dglInt32_t opaqueset[ 16 ] = { 360000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct    field_info *Fi;
    dbDriver  *driver;
    dbHandle  handle;
    dbString  stmt;
    char      buf[1000];

    /* TODO int costs -> double */
    G_debug (1, "Vect_build_graph(): ltype = %d, afield = %d, nfield = %d", ltype, afield, nfield); 
    G_debug (1, "    afcol = %s, abcol = %s, ncol = %s", afcol, abcol, ncol); 

    fprintf ( stderr, "Building graph:\n");

    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();

    ll = 0;
    if( G_projection() == 3) /* LL */
        ll = 1;
	
    gr = &(Map->graph);
    if ( ncol != NULL )
        dglInitialize ( gr, 1, sizeof(double), 0, opaqueset ); 
    else
        dglInitialize ( gr, 1, 0, 0, opaqueset ); 

    if ( gr == NULL )
        G_fatal_error ("Cannot build network graph"); 

    db_init_handle (&handle);
    db_init_string ( &stmt);
    
    /* --- Add arcs --- */
    /* Open db connection */
    if ( afcol != NULL ) {
	if ( afield < 1 ) G_fatal_error ("Arc field < 1");
        Fi = Vect_get_field( Map, afield);
	if ( Fi == NULL ) G_fatal_error ("Cannot get field info");
	
	driver = db_start_driver(Fi->driver);
	db_set_handle (&handle, Fi->database, NULL);
        
	if (db_open_database(driver, &handle) != DB_OK)
            G_fatal_error("Cannot open database %s", Fi->database) ;
    }
	
    skipped = 0;
    nlines = Vect_get_num_lines ( Map );
    fprintf ( stderr, "Registering arcs ... ");
    for ( i = 1; i <= nlines; i++ ) {
	G_percent ( i, nlines, 1 ); /* must be befor any continue */
	dofw = dobw = 1;
	Vect_get_line_nodes ( Map, i, &from, &to );
	type = Vect_read_line ( Map, Points, Cats, i );
        if ( !(type & (GV_LINE | GV_BOUNDARY) ) ) continue;
	
	if ( afcol != NULL ) {
	    if ( !(Vect_cat_get(Cats, afield, &cat) ) ) {
		G_debug ( 2, "Category of field %d not attached to the line %d -> line skipped", afield, i);
		skipped += 2; /* Both directions */ 
		continue;
	    } else {
                sprintf( buf, "%s = %d", Fi->key, cat);
		G_debug ( 2, "WHERE: %s", buf );
		nval = db_select_int ( driver, Fi->table, afcol, buf, &vals);
		if ( nval < 1 ) {
		    G_warning ( "Database record for line %d (cat = %d, forward direction) not found " 
			        "(direction of line skipped)", i, cat);
		    dofw = 0;
		} else { 
		    cost = vals[0];
		    G_debug ( 2, "line = %d cat = %d cost = %d", i, cat, cost );
		}
		if ( abcol != NULL ) {
		    nval = db_select_int ( driver, Fi->table, abcol, buf, &vals);
		    if ( nval < 1 ) {
			dobw = 0;
		    } else { 
			bcost = vals[0];
		    }
		} else {
		    if (dofw)
		        bcost = cost;
		    else
			dobw = 0;
		}
		if ( dobw )
	            G_debug ( 2, "line = %d cat = %d bcost = %d", i, cat, bcost );
		else
		    G_warning ( "Database record for line %d (cat = %d, backword direction) not found", 
			        "(direction of line skipped)", i, cat);
	    }
	} else {
	    if ( ll ) { 
		if ( geo )
	            cost = (dglInt32_t) Vect_line_geodesic_length ( Points );
		else
	            cost = (dglInt32_t) 1000000 * Vect_line_length ( Points );
	    } else
	        cost = (dglInt32_t) Vect_line_length ( Points );
	    
	    bcost = cost;
	}
	if ( dofw ) {
            G_debug (3, "Add arc %d from %d to %d cost = %d", i, from, to, cost); 
	    ret = dglAddEdge ( gr, from, to, cost, i );
            if ( ret < 0 )
                G_fatal_error ("Cannot add network arc");
	}
	
	if ( dobw ) {
            G_debug (3, "Add arc %d from %d to %d bcost = %d", -i, to, from, bcost); 
	    ret = dglAddEdge ( gr, to, from, bcost, -i );
            if ( ret < 0 )
                G_fatal_error ("Cannot add network arc");
        }
    }
    
    if ( afcol != NULL && skipped > 0 ) 
        G_debug ( 2, "%d lines missing category of field %d skipped", skipped, afield);
    
    if ( afcol != NULL  ) {
        db_close_database(driver);
	db_shutdown_driver(driver);
    }

    /* Set node attributes */
    G_debug ( 2, "Register nodes");
    if ( ncol != NULL ) {
        G_debug ( 2, "Set nodes' costs");
	if ( nfield < 1 ) G_fatal_error ("Node field < 1");

        fprintf ( stderr, "Setting node costs ... ");

        Fi = Vect_get_field( Map, nfield);
	if ( Fi == NULL ) G_fatal_error ("Cannot get field info");
	
	driver = db_start_driver(Fi->driver);
	db_set_handle (&handle, Fi->database, NULL);
        
	if (db_open_database(driver, &handle) != DB_OK)
            G_fatal_error("Cannot open database %s", Fi->database) ;
	
	for ( i = 1; i <= Vect_get_num_nodes ( Map ); i++ ) {
	    /* TODO: what happens if we set attributes of not existing node (skipped lines,
	     *       nodes without lines) */

	    nlines = Vect_get_node_n_lines ( Map, i );
	    G_debug ( 2, "  node = %d nlines = %d", i, nlines );
	    cfound = 0;
	    cost = 0;
	    for ( j = 0; j < nlines; j++ ) {
	        line = Vect_get_node_line ( Map, i, j );
	        G_debug ( 2, "  line (%d) = %d", j, line );
		type = Vect_read_line ( Map, NULL, Cats, line);
		if ( !(type & GV_POINT) ) continue;
	        if ( Vect_cat_get(Cats, nfield, &cat) ) { /* point with category of field found */
		    /* Read costs from database */
		    sprintf( buf, "%s = %d", Fi->key, cat);
		    G_debug ( 2, "node: WHERE: %s", buf );
		    nval = db_select_int ( driver, Fi->table, ncol, buf, &vals);
		    if ( nval < 1 ) {
			G_warning ( "Database record for node %d (cat = %d) not found " 
				    "(cost set to 0)", i, cat);
		    } else { 
			cost = vals[0];
			G_debug ( 2, "node = %d cat = %d cost = %d", i, cat, cost );
		    }
		    
		    cfound = 1;
		    break;
		}
	    }
	    if ( !cfound ) {
		G_debug ( 2, "Category of field %d not attached to any points in node %d"
			     "(costs set to 0)", nfield, i);
	    }
	    G_debug ( 2, "Set node's cost to %d", cost);
	    dcost = cost;
	    dglNodeSet_Attr(gr, dglGetNode(gr,i), (dglInt32_t*)&dcost); 
	}
        db_close_database(driver);
	db_shutdown_driver(driver);
        fprintf ( stderr, "done.\n");
    }
    
    fprintf ( stderr, "Flattening the graph ... "); 
    ret = dglFlatten ( gr );
    if ( ret < 0 ) G_fatal_error ("GngFlatten error");
    fprintf ( stderr, "done.\n");
    
    /* init SP cache */
    dglInitializeSPCache( gr, &(Map->spCache) );

    fprintf ( stderr, "Graph was built.\n");

    return 0;
}


/*!
 \fn int Vect_net_shortest_path ( struct Map_info *Map, int from, int to, struct ilist *List, double *cost ) 
 \brief find shortest path
 \return number of segments : ( 0 is correct for from = to, or List == NULL ),
              ? sum of costs is better return value,
           -1 : destination unreachable
 \param Map_info structure, from?, to?, ilist? cost
*/
#include<fcntl.h>
int
Vect_net_shortest_path ( struct Map_info *Map, int from, int to, struct ilist *List, double *cost ) 
{
    int i, line, *pclip, cArc, nRet;
    dglSPReport_s * pSPReport;
    dglInt32_t nDistance;

    G_debug (3, "Vect_net_shortest_path(): from = %d, to = %d", from, to ); 
    
    /* Note/TODO : it seems that if from == to dgl goes to nearest node and returns back => 
    *              check here for from == to */
    
    if ( List != NULL )
        Vect_reset_list ( List);

    /* Check if from and to are identical, otherwise dglib returns path to neares node and back! */
    if ( from == to ) {
	if ( cost != NULL ) *cost = 0;
        return 0;
    }

    pclip = NULL;
    if ( List != NULL ) 
        nRet = dglShortestPath ( &(Map->graph), &pSPReport, from, to, clipper, pclip, &(Map->spCache));
    else 
        nRet = dglShortestDistance ( &(Map->graph), &nDistance, from, to, clipper, pclip, &(Map->spCache));

    if ( nRet == 0 ) {
        /* G_warning( "Destination node %d is unreachable from node %d\n" , to , from ); */	    
	if ( cost != NULL )
	   *cost = PORT_DOUBLE_MAX;
	return -1;
    }
    else if ( nRet < 0 ) {
        fprintf( stderr , "dglShortestPath error: %s\n", dglStrerror( &(Map->graph) ) );
	return -1;
    }
    
    if ( List != NULL ) {
	for( i = 0 ; i < pSPReport->cArc ; i ++ ) {
	    line = dglEdgeGet_Id(&(Map->graph), pSPReport->pArc[i].pnEdge);
	    G_debug( 2, "From %ld to %ld - cost %ld user %d distance %ld" ,
			  pSPReport->pArc[i].nFrom,
			  pSPReport->pArc[i].nTo,
			  dglEdgeGet_Cost(&(Map->graph), 
			  pSPReport->pArc[i].pnEdge), /* this is the cost from clip() */
			  line,
			  pSPReport->pArc[i].nDistance );
	    Vect_list_append ( List, line );
	}
    }

    if ( cost != NULL ) {
        if ( List != NULL ) 
	    *cost = (double) pSPReport->nDistance;
	else    
	    *cost = (double) nDistance;
    }
	
    if ( List != NULL ) {
        cArc = pSPReport->cArc;
        dglFreeSPReport( &(Map->graph), pSPReport );
    } else
	cArc = 0;

    return (cArc);
}

