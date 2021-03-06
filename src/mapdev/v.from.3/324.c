/*
**  Written by Dave Gerdes  11/90
**  US Army Construction Engineering Research Lab
**
**  Last modified by Dave Gerdes  12/90 to add dig_plus support
*/
#include    <stdio.h>
#include    "gis.h"
#include    "digit.h"
#include    "dig_head.h"


#define MAIN

long ftell ();
double atof ();

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
    int noplus;


/*  check args and set flags  */
	
    parse_command_line (argc, argv, &dig_name, &noplus);
     
/* Show advertising */
    G_gisinit(argv[0]);
    /* printf ("\n\n   v.3_to_4:\n\n"); */

    if ((mapset = G_find_file2 ("dig", dig_name, "")) == NULL)
    {
	char buf[200];
	sprintf  (buf, "Could not find DIG file %s\n", dig_name);
	G_fatal_error (buf);
    }
    
    if (strcmp (mapset, G_mapset()))
	G_fatal_error ("Can only update files in your mapset");
	

    /* if dig_plus exists and noplus flag, then remove file */
    if (noplus)
    {
	FILE *fp;
	char buf[300];

	G__file_name (buf, "dig_plus", dig_name, mapset);
	if (NULL != (fp = fopen (buf, "r")))
	{
	    fclose (fp);
	    unlink (buf);
	}
    }
    
    exit (export (dig_name, mapset, noplus));
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
export(dig_name, mapset, noplus)
    char *dig_name, *mapset;
    int noplus;
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

	if ( ! mapset)
	{
	    G_fatal_error ("No mapset specified.");
	}

/***************************************************************************/

	/* extra code added to check for 4.0 format before
	** we go too far
	*/
	Vect_set_open_level (1);
	if (0 > Vect_open_old (&Map, dig_name, mapset))
	    G_fatal_error ("Can't open vector file");

	if (Map.head.Version_Major >= 4)
	{
	    printf ( "File is already in 4.0 format\n");
	    sleep (2);
	    exit (1);
	}

	tmpfile = G_tempfile ();
	Out = fopen (tmpfile, "w");

	if (0 > cp_filep (Map.dig_fp, Out))
	    G_fatal_error ("File copy failed.  Cannot Proceed.");

	fclose (Out);
	Vect_close (&Map);

/***************************************************************************/


	if (noplus)
	{
	    Plus = 0;
	    Vect_set_open_level (1);		/* no plus */
	    Vect_open_old (&Map, dig_name, mapset);
	    fclose (Map.dig_fp);
	}
	else
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

	doit (Plus, &Map, &Outmap);

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

        /*fclose (Map.att);*/
	/*  Dont bother calling dig_P_fini() cuz I dont have a digit
	**  file open for it 
	*/
        /*dig_P_fini (&Map);*/

	/**********************************/

	Vect_close (&Outmap);
	Vect_close (&Map);

	cleanup (tmpfile);

	fprintf (stderr, "Done.\n");

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
    int diff;
    int left;
    int old, new; 
    int i;
    
    Points = Vect_new_line_struct ();
    Vect_set_constraint_type (Map, -1);   /* all lines */
    Map->level = 1;	/*  ALARM!!!   never do this.  I had to force it 
			**  to use level 1 reads so I could get dead lines too
			*/

    left = diff = 0;
    line = 0;
    while (1)
    {
	if (0 > (type = Vect_read_next_line (Map, Points)))
	{
	    if (type == -1)
	    {
		fprintf (stderr, "Out of memory");
		exit (1);
	    }
	    /* else eof */
	    break;
	}
	Vect_write_line (Outmap, type, Points);
    }
}

cp_filep  (in, out)
    FILE *in, *out;
{
    char buf[BUFSIZ];
    int red, ret;
    int no_file = 0;
    int err=0;

    {
        while (red = fread (buf, 1, BUFSIZ, in))
	{
            if (!(ret = fwrite (buf, 1, red, out)))
	    {
		err++;
		break;
	    }
	}
        fclose (in);
    }
    if (0 != fclose (out))
	err++;

    return (err);
}

#define KEY1 "map"
parse_command_line (argc, argv, filename, flag)
    int argc;
    char **argv;
    char **filename;
    int *flag;
{

    struct Option *map;
    struct Flag *noplus;
    static char strbuf[100];

    map = G_define_option ();
    map->key           = KEY1;
    map->type          = TYPE_STRING;
    map->required      = YES;
    map->multiple      = NO;
    map->description   = "3.0 Vector file to be updated to 4.0";

    noplus = G_define_flag ();
    noplus->key = 'p';
    noplus->description = "Remove the dig_plus file";

    if (G_parser (argc, argv))
        exit (-1);

    *filename = map->answer;
    *flag = noplus->answer;


    return (0);
}
