#include	<stdio.h>
#include	"gis.h"
#include	"dig_structs.h"

#define		DIG_DIR		"dig"
#define		PLUS_DIR	"dig_plus"
#define		ATT_DIR		"dig_att"

open_dig_files (name, fp_plus, Map, Plus)
	char	*name ;
	FILE   **fp_plus ;
	struct Map_info *Map ;
	struct Plus_head *Plus ;
{

    static char  dig_file[128] ;
    static char  dig_plus_file[128] ;
    static char  att_file[128] ;
    static char  *mapset;

    FILE  *fopen() ;

    mapset =  G_store (G_mapset());

/*  check for existance of support directories  */
    G__make_mapset_element(PLUS_DIR) ;
    G__make_mapset_element(ATT_DIR) ;


    G__file_name( dig_file, DIG_DIR, name, mapset) ;
    G__file_name( att_file, ATT_DIR, name, mapset) ;
    strncpy( Plus->Dig_name, name, HEADSTR) ;


    G__file_name( dig_plus_file, PLUS_DIR, name, mapset) ;

/* If you do that, Vlib routines think that the dig file is a Grass V.3
   file and lauch v.from.3, which crash because the file is V.4  Why ??? */

    *fp_plus = fopen(dig_plus_file, "w");
    fclose( *fp_plus);

    /* Unsupported library routine */
    /* Kids, don't try this at home. */
    if (0 > V2__init_for_create_plus (Map, name))
	{
		G_fatal_error ("Cannot open input file");
	}

/******************************************************************************/

    if ( (*fp_plus = fopen(dig_plus_file, "w"))  ==  NULL)
    {
	    fprintf(stderr, "Can't open file for write: %s\n", dig_plus_file) ;
	    fprintf(stderr, "Contact your GRASS system administrator\n") ;
	    exit(-1) ;
    }

    if ( (Map->att_fp = fopen(att_file, "r+"))  ==  NULL)
    {
	if ( (Map->att_fp = fopen(att_file, "w+"))  ==  NULL)
	{
	    fprintf(stderr, "Can't open attribute file for write: %s\n", att_file) ;
	    fprintf(stderr, "Contact your GRASS system administrator\n") ;
	    exit(-1) ;
	}
    }

/*    Map->digit_file = dig_file; */
    Map->plus_file  = dig_plus_file;
    Map->att_file   = att_file;

    return(0) ;
}
