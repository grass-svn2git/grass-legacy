/*
**  Written by Dave Gerdes  11/90
**  US Army Construction Engineering Research Lab
**
**  Modified by Dave Gerdes  12/90 to add dig_plus support
**  Last modified by Jacques Bouchard 19. Jan. 1999: bugfix
**
*/
#include    <stdio.h>

# if	defined(__STDC__) || defined(__cplusplus)
#	include    <stdlib.h>
# else
	extern	double	atof();
# endif

#include    "gis.h"
#include    "digit.h"
#include    "dig_head.h"


#define MAIN

/*
#define DEBUG

*/
/*  command line args */

double dig_unit_conversion ();

main (argc, argv)
    int argc;
    char **argv;
{
    int   ret ;
    char *mapset;
    char *dig_name;


/*  check args and set flags  */
	
    parse_command_line (argc, argv, &dig_name);
     
/* Show advertising */
    G_gisinit(argv[0]);

    printf ("\n\n   v.clean:\n\n");

    if ((mapset = G_find_file2 ("dig", dig_name, "")) == NULL)
    {
	char buf[200];
	sprintf  (buf, "Could not find DIG file %s\n", dig_name);
	G_fatal_error (buf);
    }
    
    if (strcmp (mapset, G_mapset()))
	G_fatal_error ("Can only clean files in your mapset");
	
    exit (export (dig_name, mapset));
}

#ifdef DEBUG
debugf (format, a, b, c, d, e, f, g, h, i, j, k, l)
    char *format;
    int a, b, c, d, e, f, g, h, i, j, k, l;
{
    fprintf (stderr, format, a, b, c, d, e, f, g, h, i, j, k, l);
}
#endif


struct Map_info Map;
struct Map_info Outmap;
struct dig_head Head;


/*  Can't copy in place, cuz I'm not sure if I can depend on the
**   ftruncate() call being available.  So will copy dig file
**   to a temp file, then clean from it into original, truncating
**   it as I do so.
**`
*/

/* coming in, mapset is guaranteed to be the users own mapset */
export(dig_name, mapset)
    char *dig_name, *mapset;
{
	FILE *Out;
	FILE *In;
	char buf[1024];
	char *tmpfile;
	int Plus;		/* flag if have Plus file or not */
	struct Map_info Map;
	struct Plus_head Plus_head;
	FILE *plus_fp;
	char *err;
	int deleted;

	if ( ! mapset)
	{
	    G_fatal_error ("No mapset specified.");
	}

/***************************************************************************/

	/* Copy orig  dig file  to .tmp */

	Vect_set_open_level (1);
	if (0 > Vect_open_old (&Map, dig_name, mapset))
	    G_fatal_error ("Can't open vector file");

	tmpfile = G_tempfile ();
	Out = fopen (tmpfile, "w");

	if (0 > cp_filep (Map.dig_fp, Out))
	    G_fatal_error ("File copy failed.  Cannot Proceed.");

	(void)fclose(Out);

	/* Vect_close (&Map);*/ /*Jacques Bouchard */

/***************************************************************************/


	{
	    int level;

	    if (0 > (level = Vect__open_update_1 (&Map, dig_name)))
		G_fatal_error (err);
	    else
	    {
		if (level > 1)
		    Plus = 1;
		else
		    Plus = 0;

		/* close pointer to old digit file */
		fclose (Map.dig_fp);
	    }
	}



	if (NULL == (In  = fopen (tmpfile, "r")))
	{
	    cleanup (tmpfile);
	    G_fatal_error ("Failed opening temp file");
	}
	if (0 >= Vect_open_new (&Outmap, dig_name))
	{
	    G_fatal_error ("Failed openning new file");
	    exit (0);
	}

	Map.dig_fp = In;

	Vect_copy_head_data (&(Map.head), &(Outmap.head));

	deleted = doit (Plus, &Map, &Outmap);

	/**********************************/

	if (Plus)
	{
	    if (NULL == (plus_fp = fopen (Map.plus_file, "w")))
	    {
		fprintf (stderr, "Can't open Plus file for final write!\n");
		exit (-1);
	    }
	    dig_map_to_head (&Map, &Plus_head);

	    if (0 > dig_write_plus_file (plus_fp, &Map, &Plus_head))
	    {
		fprintf (stderr, "Error writing final plus file\n");
		exit (-1);
	    }
	}

	/**********************************/

	Vect_close (&Outmap);
	Vect_close (&Map);

	cleanup (tmpfile);

	fprintf (stderr, "      %d Dead line%s removed\n", deleted, deleted == 1 ? "" : "s");

	return(0) ;
}

cleanup (file)
    char *file;
{
    unlink (file);
}

doit (Plus, Map, Outmap)
    int Plus;
    struct Map_info *Map;
    struct Map_info *Outmap;
{
    struct line_pnts *Points;
    register int line, type;
    int binary;
    long old_offset;
    long new_offset;
    int old, new; 
    int deleted;
    int i;
    
    Points = Vect_new_line_struct ();

    /* read_next_line() by default only gives us ALIVE lines! */

    /* Make sure reads start at beginning */
    Vect_rewind (Map);
    line = 0;
    deleted = 0;
    while (1)
    {
	line++;
	old_offset = ftell (Map->dig_fp);
	if (0 > (type = V1_read_line (Map, Points, old_offset)))
	{
	    if (type == -1)
	    {
		fprintf (stderr, "Out of memory on line %d\n", line);
		return (-1);
	    }
	    else 	/* EOF */
		return (deleted);
	}

	if (type < 16)	/* write ONLY if line ALIVE */
	{
	    if (Plus)		/* update Plus structure */
	    {
		new_offset = ftell (Outmap->dig_fp);
		for (i = 1 ; i <= Map->n_lines ; i++)
		    if (Map->Line[i].offset == old_offset)
		    {
		        Map->Line[i].offset = new_offset;
			break;
		    }
	    }
	    Vect_write_line (Outmap, type, Points);
	}
	else
	    deleted++;
    }
    /*NOTREACHED*/
}

cp_filep  (in, out)
    FILE *in, *out;
{
    char buf[BUFSIZ];
    int red, ret;
    int err=0;

    fseek (in, 0L, 0);
    {
        while (red = fread (buf, 1, BUFSIZ, in))
	{
            if (red != fwrite (buf, 1, red, out))
	    {
		err++;
		break;
	    }
	}
        fclose (in);
    }
    return (err);
}

#define KEY1 "map"
parse_command_line (argc, argv, filename)
    int argc;
    char **argv;
    char **filename;
{

    struct Option *map;
    struct Flag *noplus;
    static char strbuf[100];

    map = G_define_option ();
    map->key           = KEY1;
    map->type          = TYPE_STRING;
    map->required      = YES;
    map->multiple      = NO;
    map->description   = "Vector file to be cleaned";

    if (G_parser (argc, argv))
        exit (-1);

    *filename = map->answer;

    return (0);
}
