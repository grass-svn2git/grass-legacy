/* getSelectOpts.c - passes select range of program options to G_parser.
                     The [-s] option indicates that an input
                     file with SQL commands is being provided. This
                     ability to include a well formed SQL command
                     file gives the user more control over output
                     columns and complex database joins etc.
                     If the sql file requires input from GRASS
                     (eg category val or coord X,Y use a [?]
                     as a placeholder as per PREPARE). The SQL
                     input file will be parsed and the [?] will
                     replaced prior to executing the query.


                  jaf 2/19/92
*/

#include <string.h>
#include <stdlib.h>
#include "what.h"
#include "glocale.h"

int getSelectOpts(argc, argv)
     int argc;
     char **argv;

{

    int i;
    static char SQL_stmt[QRY_LENGTH];
    /* arbitrary limit of query size */
    /* cfa 11/098  */
    FILE *fp;
    struct Flag *select;
    struct Option *sql, *distance, *hv;
    int stat = 0;
    char *print_out;

    memset(SQL_stmt, '\0', sizeof(SQL_stmt));


    select = G_define_flag();
    select->key = 's';
    select->description = _("Use [s] for query with a command file.");


    sql = G_define_option();
    sql->key = "sql";
    sql->key_desc = "file";
    sql->type = TYPE_STRING;
    sql->required = NO;
    sql->multiple = NO;
    sql->description = _("SQL query. ");


    distance = G_define_option();
    distance->key = "distance";
    distance->type = TYPE_STRING;
    distance->required = YES;
    distance->multiple = NO;
    distance->description = _("Cursor radius.");

/* add interactive distance g.select.dist
   and mouse choosing  into this routine
   cfa 11/98  */

    hv = G_define_option();
    hv->key = "hv";
    hv->type = TYPE_STRING;
    hv->answer = "v";
    hv->description = _("DB output type - [v(ert)/h(oriz)]:");


    /* Check for help flag */
    for (i = 0; i < argc; i++)
	if (strcmp(argv[i], "help") == 0)
	    argv[1] = "help";


    if ((argc == 2) && (strcmp(argv[1], "-s") == 0)) {	/* Run interactive parser */
	/*argv[1] == NULL ; */
	argc = 1;
    }

    /* Invoke parser */
    if (G_parser(argc, argv))
	exit(-1);

    print_out = hv->answer;

    if ((fp = fopen(sql->answer, "r")) == NULL) {
	fprintf(stderr, _("File read error on %s\n"), sql->answer);
	exit(-1);
    }

    fread(SQL_stmt, QRY_LENGTH, 1, fp);
    /* read all lines of sql stmt into a var  */
    fclose(fp);


    stat = runPg(SQL_stmt, distance->answer, print_out);

    return stat;
}
