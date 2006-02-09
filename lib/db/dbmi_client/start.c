#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __MINGW32__
#include <process.h>
#include <fcntl.h>
unsigned int _CRT_fmode = _O_BINARY;
#endif

#include <grass/dbmi.h>

#define READ  0
#define WRITE 1

extern char *getenv();

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
dbDriver *
db_start_driver (char *name)

{
    dbDriver *driver;
    dbDbmscap *list, *cur;
    char *startup;
    int p1[2], p2[2];
    int pid;
    int stat;
    dbConnection connection;
    char ebuf[5];
    int stdin_orig, stdout_orig;

    /* Set some enviroment variables which are later read by driver.
     * This is necessary when application is running without GISRC file and all
     * gis variables are set by application. 
     * Even if GISRC is set, application may change some variables during runtime,
     * if for example reads data from different gdatabase, location or mapset*/
    
    /* setenv() is not portable, putenv() is POSIX, putenv() in glibc 2.0-2.1.1 doesn't conform to SUSv2,
     * G_putenv() as well, but that is what we want, makes a copy of string */
    if (  G_get_gisrc_mode() == G_GISRC_MODE_MEMORY ) 
    {
        G_debug (3, "G_GISRC_MODE_MEMORY\n" );
	sprintf ( ebuf, "%d", G_GISRC_MODE_MEMORY );
	G_putenv("GRASS_DB_DRIVER_GISRC_MODE", ebuf); /* to tell driver that it must read variables */
	
	if ( G__getenv ( "DEBUG" ) ) {
	    G_putenv( "DEBUG", G__getenv ( "DEBUG" ) );
	} else {
	    G_putenv("DEBUG", "0");
	}

	G_putenv( "GISDBASE", G__getenv("GISDBASE") );
	G_putenv( "LOCATION_NAME", G__getenv("LOCATION_NAME") );
	G_putenv( "MAPSET", G__getenv("MAPSET") );
    } 
    else 
    {
	/* Warning: GISRC_MODE_MEMORY _must_ be set to G_GISRC_MODE_FILE, because the module can be 
	 *          run from an application which previously set enviroment variable to G_GISRC_MODE_MEMORY */
	sprintf ( ebuf, "%d", G_GISRC_MODE_FILE );
	G_putenv("GRASS_DB_DRIVER_GISRC_MODE", ebuf);
    }
    
/* read the dbmscap file */
    if(NULL == (list = db_read_dbmscap()))
	return (dbDriver *) NULL;

/* if name is empty use connection.driverName, added by RB 4/2000 */
    if( name == '\0' )
    {
	db_get_connection( &connection );
	if(NULL == (name = connection.driverName) )
	   return (dbDriver *) NULL;
    }

/* find this system name */
    for (cur = list; cur; cur = cur->next)
	if (strcmp (cur->driverName, name) == 0)
	    break;
    if (cur == NULL)
    {
	char msg[256];

	db_free_dbmscap (list);
	sprintf (msg, "%s: no such driver available", name );
	db_error (msg);
	return (dbDriver *) NULL;
    }

/* allocate a driver structure */
    driver = (dbDriver *) db_malloc (sizeof(dbDriver));
    if (driver == NULL)
    {
	db_free_dbmscap (list);
	return (dbDriver *) NULL;
    }
    
/* copy the relevant info from the dbmscap entry into the driver structure */
    db_copy_dbmscap_entry (&driver->dbmscap, cur);
    startup = driver->dbmscap.startup;

/* free the dbmscap list */
    db_free_dbmscap (list);

/* run the driver as a child process and create pipes to its stdin, stdout */

#ifdef __MINGW32__
    /* create pipes (0 in array for reading, 1 for writing) */
    /* p1 : module -> driver, p2 driver -> module */
    if( _pipe(p1, 512, _O_BINARY) == -1 ||
        _pipe(p2, 512, _O_BINARY) == -1 ) 
    {
        db_syserror ("can't open any pipes");
	return (dbDriver *) NULL;
    }

    /* Set pipes for stdin/stdout driver */
    if ( (stdin_orig  = _dup(_fileno(stdin ))) == -1  ||
         (stdout_orig = _dup(_fileno(stdout))) == -1 ) 
    {
        db_syserror ("can't duplicate stdin/stdout");
	return (dbDriver *) NULL;
    }

    fflush (stdout);
    fflush (stderr);
    if ( _dup2(p1[0], _fileno(stdin)) != 0 ||
         _dup2(p2[1], _fileno(stdout)) != 0 ) 
    {
        db_syserror ("can't duplicate pipes");
	return (dbDriver *) NULL;
    }

    /* Warning: the driver on Windows must have extension .exe
     *          otherwise _spawnl fails. The name used as _spawnl 
     *          parameter can be without .exe 
     */ 
    pid = _spawnl ( _P_NOWAIT, startup, "", NULL );

    /* Reset stdin/stdout for module */
    if ( _dup2(stdin_orig, _fileno(stdin)) != 0 ||
         _dup2(stdout_orig, _fileno(stdout)) != 0 ) 
    {
        db_syserror ("can't reset stdin/stdout");
	return (dbDriver *) NULL;
    }

    /* close duplicates */
    close ( stdin_orig );
    close ( stdout_orig );

    if ( pid == -1 ) {
        db_syserror ("can't _spawnl");
	return (dbDriver *) NULL;
    }

    /* record driver process id in driver struct */
    driver->pid = pid;

    /* convert pipes to FILE* */
    driver->send = fdopen (p1[WRITE], "wb");
    driver->recv = fdopen (p2[READ],  "rb");

    /* most systems will have to use unbuffered io to get the 
     *  send/recv to work */
#ifndef USE_BUFFERED_IO
	setbuf (driver->send, NULL);
	setbuf (driver->recv, NULL);
#endif

    db__set_protocol_fds (driver->send, driver->recv);
    if(db__recv_return_code(&stat) !=DB_OK || stat != DB_OK)
        driver =  NULL;

    return driver;

#else /* __MINGW32__ */

/* open the pipes */
    if ((pipe(p1) < 0 ) || (pipe(p2) < 0 ))
    {
        db_syserror ("can't open any pipes");
	return (dbDriver *) NULL;
    }

/* create a child */
    if ((pid = fork()) < 0)
    {
        db_syserror ("can't create fork");
	return (dbDriver *) NULL;
    }

    if (pid > 0)        /* parent */
    {
        close(p1[READ]);
        close(p2[WRITE]);

/* record driver process id in driver struct */
        driver->pid = pid;

/* convert pipes to FILE* */
	driver->send = fdopen (p1[WRITE], "wb");
	driver->recv = fdopen (p2[READ],  "rb");

/* most systems will have to use unbuffered io to get the send/recv to work */
#ifndef USE_BUFFERED_IO
	setbuf (driver->send, NULL);
	setbuf (driver->recv, NULL);
#endif

	db__set_protocol_fds (driver->send, driver->recv);
	if(db__recv_return_code(&stat) !=DB_OK || stat != DB_OK)
	    driver =  NULL;
	return driver;
    }
    else        /* child process */
    {
        close(p1[WRITE]);
        close(p2[READ]);

        close (0);
        close (1);

        if (dup(p1[READ]) != 0)
        {
            db_syserror("dup r");
            _exit(127) ;
        }

        if (dup(p2[WRITE]) != 1)
        {
            db_syserror("dup w");
            _exit(127) ;
        }

	execl ("/bin/sh", "sh", "-c", startup, 0);

        db_syserror ("execl");
	return NULL; /* to keep lint, et. al. happy */
    }

#endif /* __MINGW32__ */
}
