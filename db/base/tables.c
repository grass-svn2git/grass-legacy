/**************************************************************
 * db.tables driver=name database=name
 *
 *
 *  list all tables in a database
 ****************************************************************/

#include "gis.h"
#include "dbmi.h"
#include "codes.h"
#include <stdlib.h>

struct {
	char *driver, *database;
	int s;
} parms;

void parse_command_line();

int
main(int argc, char *argv[])
{
    dbDriver *driver;
    dbHandle handle;
    dbString *names;
    int i, count;
    int system_tables;

    parse_command_line (argc, argv);

    driver = db_start_driver(parms.driver);
    if (driver == NULL)
	G_fatal_error("No db connection for driver <%s> defined. Run db.connect", parms.driver);

    db_init_handle (&handle);
    db_set_handle (&handle, parms.database, NULL);
    if (db_open_database(driver, &handle) != DB_OK)
	exit(ERROR);

    system_tables = parms.s;
    if(db_list_tables (driver, &names, &count, system_tables) != DB_OK)
	exit(ERROR);
    for (i = 0; i < count; i++)
	fprintf (stdout,"%s\n", db_get_string (&names[i]));

    db_close_database(driver);
    db_shutdown_driver(driver);

    exit(OK);
}

void
parse_command_line(int argc, char *argv[])
{
    struct Option *driver, *database;
    struct Flag *p, *s;
    struct GModule *module;
    char *drv, *db;
    char *fakestart;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]) ;

    /* fake session for HTML generation with parser */
    fakestart = getenv( "GRASS_FAKE_START" );

    driver 		= G_define_option();
    driver->key 	= "driver";
    driver->type 	= TYPE_STRING;
    driver->options     = db_list_drivers();
    driver->required 	= NO;
    driver->description = "driver name";
    if ( fakestart == NULL && (drv=db_get_default_driver_name()) ) 
	driver->answer = drv;

    database 		= G_define_option();
    database->key 	= "database";
    database->type 	= TYPE_STRING;
    database->required 	= NO;
    database->description = "database name";
    if ( fakestart == NULL && (db=db_get_default_database_name()) ) 
	database->answer = db;

    p = G_define_flag();
    p->key               = 'p';
    p->description       = "print tables and exit";    

    s			= G_define_flag();
    s->key		= 's';
    s->description	= "system tables instead of user tables";

    /* Set description */
    module              = G_define_module();
    module->description = ""\
    "List all tables for a given database.";


    if(G_parser(argc, argv))
	exit(ERROR);

    parms.driver	= driver->answer;
    parms.database	= database->answer;
    parms.s		= s->answer;
}
