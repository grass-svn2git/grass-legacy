/*
 *   g.select.inf
 *   Select an SQL database to use in conjunction with
 *   subsequent GRASS applications.
 *   jaf 12/28/91
 *   
 * g.select.pg  
 *
 * Selects a Postgres database to use
 * J.Soimasuo 7th March 1994
 */

#include <stdio.h>
#include <stdlib.h>
#include "gis.h"
#include "glocale.h"

#define MAIN

void listdb(char *);
int getdbname(char *);

int main(argc, argv)
     int argc;
     char **argv;
{
    int hit;
    struct Option *opt1;
    struct Option *pghost;
    struct Flag *list;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]);

    list = G_define_flag();
    list->key = 'l';
    list->description = _("Use -l flag for list of databases.");

    pghost = G_define_option();
    pghost->key = "host";
    pghost->type = TYPE_STRING;
    pghost->required = NO;
    pghost->multiple = NO;
    pghost->description = _("Postgres server name:");

    opt1 = G_define_option();
    opt1->key = "database";
    opt1->type = TYPE_STRING;
    opt1->required = NO;
    opt1->multiple = NO;
    opt1->description = _("Postgres database name:");

    hit = 1;


    /* Check command line */
    if (G_parser(argc, argv))
	exit(-1);

    /* Make sure database is available */

    if (pghost->answer)
	G_setenv("PG_HOST", pghost->answer);
    else
	G_unsetenv("PG_HOST");

    if (list->answer) {
	fprintf(stderr, _("The following databases are in the Unix catalogue:\n"));

	listdb(pghost->answer);
	exit(0);
    }

    if (!opt1->answer) {
	opt1->answer = getenv("USER");
    }

    hit = getdbname(opt1->answer);

    if (hit == 0) {
	G_setenv("PG_DBASE", opt1->answer);

    }
    else {
	G_warning(_("This database does not exist."));
	fprintf(stderr, _("The following databases are in the Unix catalogue:\n"));
	listdb(pghost->answer);
	exit(-1);
    }

    exit(0);
}
