/****************************************************************
 * G_system (command)
 *     char *command;
 *
 * This is essentially the UNIX system() call, except for the signal
 * handling. During the call, user generated signals (intr, quit)
 * for the parent are ignored, but allowed for the child. Parent
 * signals are reset upon completion.
 *
 * This routine is useful for menu type programs that need to run
 * external commands and allow these commands to be interrupted by
 * the user without killing the menu itself.
 *
 * Note: if you want the signal settings to be the same for the
 * parent and the command being run, set them yourself and use
 * the UNIX system() call instead.
 ****************************************************************/

#include <signal.h>
#include <stdio.h>

G_system (command)
    char *command;
{
    int status, pid, w;
    void (*sigint)(), (*sigquit)() ;

    sigint  = signal (SIGINT,  SIG_IGN);
    sigquit = signal (SIGQUIT, SIG_IGN);

    fflush (stdout);
    fflush (stderr);

    if ( (pid = fork()) == 0)
    {
	signal (SIGINT,  SIG_DFL);
	signal (SIGQUIT, SIG_DFL);

	execl ("/bin/sh", "sh", "-c", command, 0);
	_exit(127);
    }

    if (pid < 0)
    {
	fprintf (stderr, "WARNING: can not create a new process\n");
	status = -1;
    }
    else
    {
	while ( (w = wait (&status)) != pid && w != -1)
	    ;

	if (w == -1)
	    status = -1;
    }

    signal (SIGINT,  sigint);
    signal (SIGQUIT, sigquit);

    return (status);
}
