/**************************************************************
 * db.execute driver=name database=name [input=filename]
 *
 *
 *   process one non-select sql statement.
 *   errors cause an error message to be printed to stderr and exit(1)
 *   successful execution results in exit(0)
 ****************************************************************/

#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "dbmi.h"
#include "codes.h"

struct {
	char *driver, *database, *input;
} parms;

void parse_command_line();
int get_stmt( FILE *fd, dbString *stmt);
int stmt_is_empty( dbString *stmt );
    
int
main( int argc, char *argv[] )
{
    dbString stmt;
    dbDriver *driver;
    dbHandle handle;
    int ret;
    FILE *fd;

    parse_command_line (argc, argv);

    if (parms.input)
    {
	fd = fopen (parms.input, "r");
	if (fd == NULL)
	{
	    perror (parms.input);
	    exit(ERROR);
	}
    }
    else
	fd = stdin;

    driver = db_start_driver(parms.driver);
    if (driver == NULL)
	G_fatal_error("No db connection for driver <%s> defined. Run db.connect", parms.driver);

    db_init_handle (&handle);
    db_set_handle (&handle, parms.database, NULL);
    if (db_open_database(driver, &handle) != DB_OK)
	exit(ERROR);

    while( get_stmt(fd, &stmt) )
    {
	if(!stmt_is_empty(&stmt)) {
	    G_debug (3, "sql: %s", db_get_string(&stmt) );
            ret = db_execute_immediate (driver, &stmt);
	    if ( ret != DB_OK )
	       G_warning ( "Error while executing: \"%s\"\n", db_get_string( &stmt ) );
	}
    }

    db_close_database(driver);
    db_shutdown_driver(driver);

    exit(0);
}

void
parse_command_line(argc, argv) char *argv[];
{
    struct Option *driver, *database, *input;
    struct GModule *module;

    /* Initialize the GIS calls */
    G_gisinit(argv[0]) ;

    driver 		= G_define_option();
    driver->key 	= "driver";
    driver->type 	= TYPE_STRING;
    driver->options     = db_list_drivers();
    driver->required 	= NO;
    driver->description = "driver name";

    database 		= G_define_option();
    database->key 	= "database";
    database->type 	= TYPE_STRING;
    database->required 	= NO;
    database->description = "database name";

    input 		= G_define_option();
    input->key 		= "input";
    input->key_desc 	= "filename";
    input->type 	= TYPE_STRING;
    input->required 	= NO;
    input->description 	= "filename with sql statement";

    /* Set description */
    module              = G_define_module();
    module->description = ""\
    "Execute any SQL statement.";

    
    if(G_parser(argc, argv))
	exit(ERROR);

    parms.driver	= driver->answer;
    parms.database	= database->answer;
    parms.input		= input->answer;
}

int
get_stmt(fd, stmt)
    FILE *fd;
    dbString *stmt;
{
    char buf[4000], *str;
    int len, row = 0;

    db_init_string (stmt);

    while ( fgets (buf, 4000, fd) != NULL )
    {
	G_chop (buf);
	db_append_string (stmt, buf);
	len = strlen (buf);
	if ( buf[ len - 1 ] == ';' ) {  /* end of statement */
	    str = db_get_string(stmt);
	    len = strlen ( str );
	    str [len - 1] = 0;
	    return 1;
	}
	row++;
    }

    if ( row > 0 ) return 1;

    return 0;
}

int
stmt_is_empty(stmt)
    dbString *stmt;
{
    char dummy[2];

    return (sscanf (db_get_string(stmt), "%1s", dummy) != 1);
}
