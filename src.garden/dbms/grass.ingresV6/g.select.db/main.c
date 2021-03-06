/*
 *   g.select.db
 *
 *
 *   Select an SQL database to use in conjunction with
 *   subsequent GRASS applications.
 *   jaf 12/28/91
 * 
 *   Modified for INGRES RDBMS
 *   Noel Krahn, Katarina Johnsson, Pacific Forestry Centre, Oct 1992
 */

#include <stdio.h>
#include "gis.h"
#define MAIN
#define RS ":"

main(argc, argv)
int argc ;
char **argv ;
{
  char  *getenv(), *strtok(), *strcat() ;
	int i, rsCnt, hit, dblen ;
        char *dbpath, *p;  
        char dbstring[1024];
	struct Option *opt1;
	struct Flag *list ;



        list = G_define_flag();
        list->key               = 'l';
        list->description       = "Use -l for a list of databases in current path.";


	opt1 = G_define_option() ;
	opt1->key        = "database" ;
	opt1->type       = TYPE_STRING ;
	opt1->required   = NO  ;
	opt1->multiple   = NO ;
	opt1->description= "Name of existing database" ;

        hit = 1;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;


	/* Check command line */
	if (G_parser(argc, argv))
		exit(-1);

/**************  INGRES driver code begins ***************/

  if(list->answer)
  {
    char 	*sqlFile, *tmpfile_out, *system_database,
    		*system_database_table;
    char	buf[BUFSIZ];
    FILE	*fp;

    /* Open files for SQL input and output*/
    sqlFile = G_tempfile();
    tmpfile_out = G_tempfile();

        if((fp = fopen(sqlFile,"w")) == NULL) 
    {
      fprintf(stderr, "File write error on temporary file (sql).\n");
      exit(-1);
    }

    /* get the system database name */
    if( !(system_database = getenv("II_DATABASE")))
       system_database = "iidbdb";


    /* get the system database name */
    system_database_table = "iidatabase";

    /* send the SQL command to list all available databases */
    fprintf(fp, "SELECT name FROM %s\n", system_database_table);
    fprintf(fp, "WHERE name NOT LIKE 'ii%%'\n");
    fprintf(fp, "\\g\n");
    fclose(fp);
    sprintf(buf, "sql -s %s < %s > %s ", system_database, sqlFile, tmpfile_out);
    system(buf);
    unlink(sqlFile);

    /* read the SQL output file and display options */
    printf("The following databases are available:\n");
    if((fp = fopen(tmpfile_out,"r")) == NULL) 
    {
      fprintf(stderr, "File write error on temporary file (sql).\n");
      exit(-1);
    }
    
    printf("Available databases:\n");
    while( fgets(buf, BUFSIZ, fp) )
    {
      printf("\t%s ", buf);
    }
    fclose(fp);
    unlink(fp);

    /* Now get the database name */
    printf("Which database would you like to use?\n(Database) ");
    scanf("%s", buf);
   
    opt1->answer = strdup(buf);
  } 
  
     
  /* set up the environment */  
  G_setenv("DATABASE", opt1->answer);
}

