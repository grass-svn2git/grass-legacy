/*
 *   d.what.s.db
 *
 *
 *   Generate a list of database attributes
 *   associated with site coordinates selected
 *   using the GRASS mouse.
 *
 *   jaf 12/30/91
 */

#include <stdio.h>
#include "gis.h"
#include "infx.h"
#define MAIN

main(argc, argv)
int argc ;
char **argv ;
{
    FILE *fp;
    char *dbname;
    char buf[1024];
    int i, button, selPassed;
    int stat = 0 ;

	selPassed = 0;


	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	/* Check DATABASE env variable */
        if ((dbname=G__getenv("DATABASE")) == NULL) {
            fprintf(stderr,
                  "Please run g.select.ora to identify a current database.\n");
	    exit(-1);
           }


	/* Initialze SQL query structure
	sql = (struct Sql *)G_malloc(sizeof(struct Sql)) ;
	G_zero (sql, sizeof(struct Sql)) ;                         */
   

        /* Check for -s flag indicating selectfile input */
        for (i=0; i<argc; i++)
                if(strcmp(argv[i],"-s")==0)
                        selPassed = 1;


        if (selPassed)          /* user provides SQL command file       */
                 stat = getSelectOpts(argc,argv);
          else                  /*  Pgm builds SQL command file         */
                 stat = getAllOpts(argc, argv);

	exit(stat) ;

}
