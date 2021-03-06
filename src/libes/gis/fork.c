/*************************************************************
 * G_fork() 
 *
 * Issue a system fork() call and protect the child from all
 * signals (which it does by changing the process group for the child)
 *
 * returns:
 *     -1 fork failed.
 *      0 child
 *     >0 parent
 ************************************************************/

G_fork()
{
    int pid;

    pid = fork();

/*
 * change the process group for the child (pid == 0)
 * note: we use the BSD calling sequence, since
 * it will work ok for ATT call which has no arguments
 */
    if (pid==0)
	setpgrp (0, getpid());
    
    return pid;
}
