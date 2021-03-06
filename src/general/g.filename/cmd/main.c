#include <stdio.h>
#include "gis.h"
main(argc, argv)
int   argc;
char *argv[];
{
	char path[1024];
	char *element;
	char *mapset;
	char *name;
	struct Option *opt1 ;
	struct Option *opt2 ;
	struct Option *opt3 ;

	/* Define the different options */

	opt1 = G_define_option() ;
	opt1->key        = "element";
	opt1->type       = TYPE_STRING;
	opt1->required   = YES;
	opt1->description= "Name of an element" ;


	opt2 = G_define_option() ;
	opt2->key        = "mapset";
	opt2->type       = TYPE_STRING;
	opt2->required   = YES;
	opt2->description= "Name of a mapset" ;

	opt3 = G_define_option() ;
	opt3->key        = "file";
	opt3->type       = TYPE_STRING;
	opt3->required   = YES;
	opt3->description= "Name of an database file" ;

	G_gisinit (argv[0]);

	if (G_parser(argc, argv))
		exit(-1);

	element = opt1->answer;
	mapset = opt2->answer;
	name = opt3->answer;

	if (strcmp (mapset, ".") == 0 || strcmp (mapset, "") == 0)
		mapset = G_mapset();

	G__make_mapset_element(element);
	G__file_name (path, element, name, mapset);

	printf ("file='%s'\n", path);
	exit(0);
}
