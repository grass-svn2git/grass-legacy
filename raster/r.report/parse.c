#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "gis.h"
#include "glocale.h"

int parse_command_line (int argc, char *argv[])
{
	char pl_desc[256];
	char pw_desc[256];
	int i;
	struct
	    {
		struct Option *cell;
		struct Option *units;
		struct Option *pl;    /* page length */
		struct Option *pw;    /* page width */
		struct Option *outfile;
		struct Option *nv;
		struct Option *nsteps;
	} parms;
	struct
	    {
		struct Flag *f;
		struct Flag *m;
		struct Flag *h;
		struct Flag *q;
		struct Flag *e;
		struct Flag *n;
		struct Flag *N;
	        struct Flag *i ;   /* use quant rules for fp map, 
				      i.e. read it as int */
	        struct Flag *C ;   /*  report for fp ranges in Cats file 
				       (fp maps only) */

	} flags;

	parms.cell = G_define_option();
	parms.cell->key    = "map";
	parms.cell->type   = TYPE_STRING ;
	parms.cell->required = YES ;
	parms.cell->multiple = YES ;
	parms.cell->gisprompt  = "old,cell,raster" ;
	parms.cell->description = _("Raster map(s) to report on");

	parms.units = G_define_option();
	parms.units->key   = "units";
	parms.units->type   = TYPE_STRING ;
	parms.units->required = NO ;
	parms.units->multiple = YES ;
	parms.units->description =
	    _("mi(les),me(ters),k(ilometers),a(cres),h(ectares),c(ell_counts),p(ercent_cover)");

	parms.nv = G_define_option();
	parms.nv->key   = "null";
	parms.nv->type   = TYPE_STRING ;
	parms.nv->required = NO ;
        parms.nv->multiple   = NO;
	parms.nv->answer     = "*";
        parms.nv->description= _("Character representing no data cell value");

	parms.pl = G_define_option();
	parms.pl->key = "pl";
	parms.pl->type = TYPE_INTEGER;
	parms.pl->required = NO ;
	sprintf (pl_desc, _("Page length (default: %d lines)"), DEFAULT_PAGE_LENGTH);
	parms.pl->description = pl_desc;

	parms.pw = G_define_option();
	parms.pw->key = "pw";
	parms.pw->type = TYPE_INTEGER;
	parms.pw->required = NO ;
	sprintf (pw_desc, _("Page width (default: %d characters)"), DEFAULT_PAGE_WIDTH);
	parms.pw->description = pw_desc;

	parms.outfile = G_define_option();
	parms.outfile->key = "output";
	parms.outfile->type = TYPE_STRING;
	parms.outfile->required = NO ;
	parms.outfile->description = _("Name of an output file to hold the report");
        parms.nsteps = G_define_option();
        parms.nsteps->key    = "nsteps";
        parms.nsteps->type   = TYPE_INTEGER;
	parms.nsteps->required   = NO;
        parms.nsteps->multiple   = NO;
	parms.nsteps->answer     = "255";
        parms.nsteps->description= _("Number of fp subranges to collect stats from");

	flags.h = G_define_flag();
	flags.h->key = 'h';
	flags.h->description = _("Suppress page headers");

	flags.f = G_define_flag();
	flags.f->key = 'f';
	flags.f->description = _("Use formfeeds between pages");

	flags.q = G_define_flag();
	flags.q->key = 'q';
	flags.q->description = _("Quiet");

	flags.e = G_define_flag();
	flags.e->key = 'e';
	flags.e->description = _("Scientific format");

	flags.n = G_define_flag();
	flags.n->key = 'n';
	flags.n->description = _("Filter out all no data cells");

	flags.N = G_define_flag();
	flags.N->key = 'N';
	flags.N->description = _("Filter out cells where all maps have no data");

	flags.C = G_define_flag() ;
        flags.C->key         = 'C' ;
	flags.C->description = _("Report for cats fp ranges (fp maps only)");

	flags.i = G_define_flag() ;
        flags.i->key = 'i';
	flags.i->description = _("Read fp map as integer (use map's quant rules");


/* hidden feature.
 * if first arg is >file just run r.stats into this file and quit
 * if first arg is <file, run report from stats in file
 * (this feature is for the interactive version of this program -
 *  to get more than one report without re-running r.stats)
 */
	stats_flag = EVERYTHING;
	if (argc > 1)
	{
		if (argv[1][0] == '<' || argv[1][0] == '>')
		{
			stats_file = argv[1]+1;
			if (argv[1][0] == '<')
				stats_flag = REPORT_ONLY;
			else
			{
				unlink (stats_file);
				stats_flag = STATS_ONLY;
			}
			argc--;
			argv++;
		}
	}

	if (G_parser(argc,argv))
		exit(0);

	use_formfeed = flags.f->answer;
	with_headers = !flags.h->answer;
	verbose      = !flags.q->answer;
	e_format     = flags.e->answer;
	no_nulls     = flags.n->answer;
	no_nulls_all = flags.N->answer;
	cat_ranges   = flags.C->answer;
	as_int       = flags.i->answer;

	for (i = 0; parms.cell->answers[i]; i++)
		parse_layer (parms.cell->answers[i]);
	if (parms.units->answers)
		for (i = 0; parms.units->answers[i]; i++)
			parse_units (parms.units->answers[i]);
        
	sscanf(parms.nsteps->answer, "%d", &nsteps);
	if(nsteps <= 0)
	{
	     G_warning("%s: nsteps has to be > 0; using nsteps=255");
	     nsteps = 255;
        }

	if (parms.pl->answer)
	{
		if (sscanf (parms.pl->answer, "%d", &page_length) != 1 || page_length < 0)
		{
			fprintf (stderr, "Illegal page length\n");
			G_usage();
			exit(1);
		}
	}

	if (parms.pw->answer)
	{
		if (sscanf (parms.pw->answer, "%d", &page_width) != 1 || page_width < 1)
		{
			fprintf (stderr, "Illegal page width\n");
			G_usage();
			exit(1);
		}
	}
	if (parms.outfile->answer)
	{	
		if (freopen (parms.outfile->answer, "w", stdout) == NULL)
		{
			perror (parms.outfile->answer);
			exit(1);
		}
	}
	no_data_str  = parms.nv->answer;

	return 0;
}

int parse_units (char *s)
{
	int x;

	if (match (s, "miles",2))
		x = SQ_MILES;
	else if (match (s, "meters",2))
		x = SQ_METERS;
	else if (match (s, "kilometers",1))
		x = SQ_KILOMETERS;
	else if (match (s, "acres",1))
		x = ACRES;
	else if (match (s, "hectares",1))
		x = HECTARES;
	else if (match (s, "cell_counts",1))
		x = CELL_COUNTS;
	else if (match (s, "counts",1))
		x = CELL_COUNTS;
	else if (match (s, "percent_cover",1))
		x = PERCENT_COVER;
	else
	{
		G_usage();
		exit(1);
	}
	if (nunits >= MAX_UNITS)
	{
		fprintf (stderr, "\nERROR: %s: only %d unit%s allowed\n",
		    G_program_name(), MAX_UNITS, MAX_UNITS==1?"":"s");
		exit(1);
	}
	unit[nunits].type  = x;
	nunits++;

    return 0;
}

int parse_layer (char *s)
{
	char msg[100];
	char name[200];
	char *mapset;
	struct FPRange fp_range;
	int n;

	strcpy (name, s);
	mapset = G_find_cell2 (name, "");

	if (mapset == NULL)
	{
		sprintf (msg, "%s: <%s> raster map not found\n", G_program_name(), s);
		G_fatal_error (msg);
		exit(1);
	}

	n = nlayers++ ;
	layers = (LAYER *)G_realloc(layers, nlayers * sizeof(LAYER));
        is_fp = (int *) G_realloc (is_fp, (nlayers+1) * sizeof(int));
	DMAX = (DCELL *) G_realloc (DMAX, (nlayers+1) * sizeof(DCELL));
	DMIN = (DCELL *) G_realloc (DMIN, (nlayers+1) * sizeof(DCELL));
	if(!as_int)
	      is_fp[n] = G_raster_map_is_fp(name, mapset);
        else
	      is_fp[n] = 0;
        if(is_fp[n])
	{
           if(G_read_fp_range (name, mapset, &fp_range) < 0)
           {
	     sprintf (msg,"%s: can't read fp range for [%s]",G_program_name(),
		     name);
             G_fatal_error (msg);
           }
 	   G_get_fp_range_min_max (&fp_range, &DMIN[n], &DMAX[n]);
        }

	layers[n].name = G_store (name);
	layers[n].mapset = mapset;
	G_read_cats (name, mapset, &layers[n].labels);

    return 0;
}

int match (char *s, char *key, int min)
{
	int len;

	len = strlen (s);
	if (len < min) return 0;
	return strncmp (s, key, len) == 0;
}
