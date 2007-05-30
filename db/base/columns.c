/****************************************************************************
 *
 * MODULE:       db.columns
 * AUTHOR(S):    Radim Blazek <radim.blazek gmail.com> (original contributor)
 *               Glynn Clements <glynn gclements.plus.com>, Markus Neteler <neteler itc.it>
 * PURPOSE:      list the column names for a table
 * COPYRIGHT:    (C) 2002-2006 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/codes.h>
#include <stdlib.h>
#include <grass/glocale.h>

struct {
	char *driver, *database, *table;
} parms;

void parse_command_line();

int
main(int argc, char *argv[])
{
    dbDriver *driver;
    dbHandle handle;
    dbTable *table;
    dbString table_name;
    int col, ncols;

    parse_command_line (argc, argv);

    driver = db_start_driver(parms.driver);
    if (driver == NULL) {
        G_fatal_error(_("Cannot start driver <%s>"), parms.driver);
        exit(ERROR);
    }
       
    db_init_handle (&handle);
    db_set_handle (&handle, parms.database, NULL);
    if (db_open_database(driver, &handle) != DB_OK)
    {
	exit(ERROR);
    }
    db_init_string(&table_name);
    db_set_string(&table_name, parms.table);
    if(db_describe_table (driver, &table_name, &table) != DB_OK)
	exit(ERROR);

    db_close_database(driver);
    db_shutdown_driver(driver);

    ncols = db_get_table_number_of_columns(table);
    for (col = 0; col < ncols; col++)
	fprintf(stdout, "%s\n", db_get_column_name(db_get_table_column(table, col)));
    exit(OK);
}

void
parse_command_line(int argc, char *argv[])
{
    struct Option *driver, *database, *table;
    struct GModule *module;
    char *drv, *db;

    /* Initialize the GIS calls */
        G_gisinit(argv[0]) ;

    table 		= G_define_option();
    table->key 		= "table";
    table->type 	= TYPE_STRING;
    table->required 	= YES;
    table->description 	= _("table name");

    driver 		= G_define_option();
    driver->key 	= "driver";
    driver->type 	= TYPE_STRING;
    driver->options     = db_list_drivers();
    driver->required 	= NO;
    driver->description = _("driver name");
    if ( (drv=db_get_default_driver_name()) )
        driver->answer = drv;

    database 		= G_define_option();
    database->key 	= "database";
    database->type 	= TYPE_STRING;
    database->required 	= NO;
    database->description = _("database name");
    if ( (db=db_get_default_database_name()) )
         database->answer = db;

    /* Set description */
    module              = G_define_module();
    module->keywords = _("database, SQL");
    module->description = _("list all columns for a given table.");

        
    if(G_parser(argc, argv))
	exit(ERROR);

    parms.driver	= driver->answer;
    parms.database	= database->answer;
    parms.table		= table->answer;
}

