#include <stdio.h>

/******************************************************************
*lock file pid
*
*   this programs "locks" the file for process pid:
*
*   1. if file exists, the pid is read out of the file. if this
*      process is still running, the file is considered locked.
*      exit(1)
*   2. if file does not exist, or if file exists but process is not
*      running (ie, lock was not removed), the file is locked for
*      process pid by writing pid into the file.
*      exit(0).
******************************************************************/

#include <errno.h>
extern int errno;
main (argc, argv) char *argv[];
{
    int pid;
    int lockpid;
    int lock;
    int locked;

    if (argc != 3 || sscanf (argv[2],"%d",&lockpid) != 1)
    {
	fprintf (stderr, "usage: %s file pid\n", argv[0]);
	exit(-1);
    }

#define file argv[1]

    locked = 0;
    if ((lock = open (file, 0)) >= 0) /* file exists */
    {
	sleep(1); /* allow time for file creator to write its pid */
	if (read (lock, &pid, sizeof pid) == sizeof pid)
		locked = find_process (pid);
	close (lock);
    }
    if (locked)
	exit(1);
    umask (0);
    if ((lock = creat (file, 0666)) < 0)
    {
	fprintf (stderr, "%s: ", argv[0]);
	perror (file);
	exit(-1);
    }
    if (write(lock, &lockpid, sizeof lockpid) != sizeof lockpid)
    {
	fprintf (stderr, "%s: can't write lockfile %s\n", argv[0], file);
	exit(-1);
    }
    close (lock);
    exit(0);
}

find_process (pid)
{
/* attempt to kill pid with NULL signal. if success, then
   process pid is still running. otherwise, must check if
   kill failed because no such process, or because user is
   not owner of process
*/
    if (kill (pid, 0) == 0)
	return 1;
    return errno != ESRCH;
}
