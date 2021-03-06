#define MAIN
#include "list.h"

struct Option *element;

main (argc,argv) char *argv[];
{
	int n, len;
	struct Option *mapset;
	struct Flag *full;

	init (argv[0]);

	element = G_define_option();
	element->key =      "type";
	element->key_desc = "datatype";
	element->type     = TYPE_STRING;
	element->required = YES;
	element->multiple = NO;
	element->description = "data type";
	for (len=0,n=0 ; n < nlist; n++)
	    len += strlen (list[n].alias)+1;
	element->options = G_malloc(len);
	for (n=0; n < nlist; n++)
	{
	    if (n)
	    {
		strcat (element->options, ",");
		strcat (element->options, list[n].alias);
	    }
	    else
		strcpy (element->options, list[n].alias);
	}

	mapset = G_define_option();
	mapset->key = "mapset";
	mapset->type = TYPE_STRING;
	mapset->required = NO;
	mapset->multiple = NO;
	mapset->description = "mapset to list (default: current search path)";
#define MAPSET mapset->answer

	full = G_define_flag();
	full->key = 'f';
	full->description = "verbose listing";
#define FULL full->answer

	n = parse(argc, argv);

	if (MAPSET == NULL)
		MAPSET = "";
	if (strcmp (MAPSET,".") == 0)
		MAPSET = G_mapset();
	if (FULL)
	{
		char lister[300];
		sprintf (lister, "%s/etc/lister/%s", G_gisbase(), list[n].element[0]);
		if (access (lister, 1) == 0) /* execute permission? */
			execl (lister, argv[0], MAPSET, 0);
	}
	do_list (n,MAPSET);
	exit(0);
}

parse(argc, argv)
char *argv[];
{
	int n;

	if (G_parser(argc, argv))
	{
		fprintf (stderr, "\nWhere %s is one of:\n", element->key);
		show_elements();
		exit(1);
	}
	for (n = 0 ; n < nlist; n++)
	{
		if (strcmp (list[n].alias, element->answer) == 0)
			break;
	}

	if (n >= nlist)
	{
		fprintf (stderr, "%s: - no such database %s\n",
		    element->answer, element->key);
		G_usage();
		fprintf (stderr, "\nWhere %s is one of:\n", element->key);
		show_elements();
		exit(1);
	}
	return n;
}
