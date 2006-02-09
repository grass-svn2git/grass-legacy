/*
 ****************************************************************
 * char *
 * G_home ()
 *
 *   returns char pointer to home directory for user
 *   dies if can't determine
 *
 * char *
 * G__home()
 *
 *   returns char pointer to home directory for user
 *   NULL if can't determine
 *
 ***************************************************************/
#include <stdlib.h>
#include <grass/gis.h>
#include <grass/glocale.h>


/*!
 * \brief user's home directory
 *
 * Returns a pointer to a string
 * which is the full path name of the user's home directory.
 *
 *  \param ~
 *  \return char * 
 */

char *
G_home ()
{
    char *home;
    
/*!
 * \brief user's home directory
 *
 * Returns a pointer to a string
 * which is the full path name of the user's home directory.
 *
 *  \param ~
 *  \return char * 
 */

char *G_home();

    if ((home = G__home()))
	return home;
    
    G_fatal_error (_("unable to determine user's home directory"));
    exit(-1);
}

char *
G__home ()
{
    static char *home = 0;
    char buf[1024];
    FILE *fd,*G_popen();

/* first call must get home
* execute the command "cd; pwd" and read the
* output to get the home directory
*/
    if (!home)
    {
	if((fd = G_popen ("cd; pwd","r")))
	{
	    if (fscanf (fd,"%s", buf) == 1)
		home = G_store (buf);
	    G_pclose (fd);
	}
    }

    return home;
}
