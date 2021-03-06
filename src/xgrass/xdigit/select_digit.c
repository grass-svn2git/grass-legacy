#include        "gis.h"
#include        "ginput.h"
#include	<stdio.h>
#include	"bin_digit.h"
#include	"digit.h"



#define         NULL_STRING     ""
/*
*	show available digitizers and ask if they want to keep their current
*	digitizer or select a different one.  checks the answer against the
*	digitcap file.
*/
extern Widget dignamelabel;
Widget digbutton;

char  *getenv() ;

void
get_new_dgtzer(w, data, cbs)
    Widget w, data;
    XmSelectionBoxCallbackStruct *cbs;
{
    FILE *fp;
    char  *env_digitizer ;
    char  *pid_string ;
    int pid, lock;
    char  lock_name[128] , buf[128];
    struct  driver_desc  Driver ;
    char *s;
    extern Widget reg;

    XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET,&s);

 /*   if (strcmp (getenv("DIGITIZER"), s)) make sure new dig has been chosen */
    {
    /*  open digitizer cap file  */
    sprintf( buf, "%s/etc/digcap", G_gisbase()) ; 
    G_setenv( "DIGITIZER", "none") ;
    N_digitizer = G_store ("nodig");
    Dig_Enabled = 0;             /* Disable Digitizer */
    dig_on_off ();
    Window_Device = MOUSE;       /* Setup Mouse as windowing device */
    Digtiz_Device = MOUSE;

    if ( (fp = fopen(buf, "r"))  ==  NULL)
         make_monolog(1,"Can't open digcap file for read") ;
    else if (get_driver_name (fp, s, &Driver) > 0)
    /* new digitizer has been chosen and matches a dig in digcap file 
       go ahead and try to use it */
    {
	/* unlock old dig, if any */
	if (N_lockname != NULL)
	    unlock_file (N_lockname);
	fclose(fp) ;



/*  get the process id and create lock file   pid = getpid()  */
	if ( (pid_string = getenv("GIS_LOCK")) == NULL)
            make_monolog (1, "ERROR - Can't find gis key to lock digitizer");
	else
	{
	    N_PPID = G_store (pid_string);
	    pid = atoi (pid_string);

	    sprintf( lock_name, "%s/locks/%s", G_gisbase(), Driver.name) ;
	    N_lockname = G_store(lock_name);

            if (strcmp (Driver.name, "none"))
            {
                lock = lock_file( lock_name, pid) ;
                if ( ! lock)
		{
                    make_monolog(1, "Digitizer is already being used.") ;
		}

                else if ( lock < 0)
		{
                    make_monolog (1, "ERROR - Could not lock digitizer.") ;
		}
	        else if( ginput_setup 
			 (Driver.dig_filename, Driver.device, QUERY_MODE))
		{
		    make_monolog (1, "Could not setup  digitizer.") ;
		}

	        else /* everything worked ok */
		{
		    Dig_Enabled = 1;
	            Window_Device = DIGITIZER;
	    	    Digtiz_Device = DIGITIZER;
	    	    G_setenv( "DIGITIZER", Driver.name) ;
		    N_digitizer = Driver.device;
		    set_dig_name (w, dignamelabel, Driver.name);
		    if (CM != NULL)
		    {
		        if (reset_map (data, CM, N_coor_file) >= 0)
			{
			    set_window();
			    /*
			    clear_window();
			    replot(CM);
			    */
			    redraw();
			    dig_on_off ();
		    	    
			}
			else
			{
	            	    Window_Device = MOUSE;
	    	    	    Digtiz_Device = MOUSE;
	    	    	    Point_Device = MOUSE;
			    /*
			    Dig_Enabled = 0;
			    dig_on_off();
			    Dig_Enabled = 1;
			    */
			    XtSetSensitive (reg, True);
			    make_monolog 
			    (1,"Map must be registered before using digitizer\n");
			}
		    }
		    else
		        dig_on_off();
		}
	    }
        }

    }
    }

}

set_dig_name (w, namelabel, name)
    Widget w, namelabel;
    char *name;
{
    char buf[100];
    XmString str;
    
    strcpy(buf, "Digitizer:   ");
    strcat(buf, name);
    str = XmStringCreateSimple (buf);
    XtVaSetValues (namelabel, XmNlabelString, str, NULL);
    XtFree (str);
    XtSetSensitive (digbutton, 0);
}

void 
make_dig_select(w, dignamelabel)
    Widget w, dignamelabel;
{
    static Widget dialog = 0;
    XmString t, *str;
    int count, i;
    struct  driver_desc  Driver ;
    FILE    *fp;
    char buf[128], message[128];
    int status;

    digbutton = w; 
  /*  open digitizer cap file  */
    sprintf( buf, "%s/etc/digcap", G_gisbase()) ;
  /*  if (XtIsRealized (dialog))
  an instance of dialog already exists, don't make a new one 
    ;*/
    if ( (fp = fopen(buf, "r"))  ==  NULL)
    {
        sprintf(message, "Can't open file for read: %s\n", buf) ;
    	make_monolog (1, message);
    }
    else
    {
	count = get_driver_name (fp, "", &Driver);
        str = (XmString *)XtMalloc (count*sizeof(XmString));
        t = XmStringCreateSimple ("Digitizers");

        fseek( fp, 0L, 0) ;
        i = 0 ;

        while ( (status = read_cap_line( fp, &Driver)) > 0)
        {
           str[i++] = XmStringCreateSimple (Driver.name);
        }
        fclose(fp) ;
        if (status < 0)
        {
            sprintf( message,"\n Error in reading digitcap file.\n") ;
    	    make_monolog (1, message);

        }
	else
	{
    	    dialog = XmCreateSelectionDialog (toplevel, "selection", NULL, 0);
    	    XtVaSetValues (dialog, 
		XtNx, Winx,
		XtNy, Winy,
		XmNlistLabelString, t,
		XmNdialogTitle,     t,
		XmNlistItems,       str,
		XmNlistItemCount,   i,
		XmNmustMatch,       True,
		NULL);


    	    XtUnmanageChild 
	        (XmSelectionBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
    	    XtUnmanageChild 
	        (XmSelectionBoxGetChild (dialog, XmDIALOG_APPLY_BUTTON));

	    XtAddCallback (dialog, XmNokCallback, get_new_dgtzer, w);
	    XtAddCallback (dialog, XmNcancelCallback, XtDestroyWidget, NULL);
	    XtAddCallback (dialog, XmNokCallback, XtDestroyWidget, NULL);
    	    XtManageChild (dialog);
	}
        XmStringFree (t);
    	while (--i >= 0)
	    XmStringFree (str[i]);
    	XtFree(str);
    }
}

/******************************************************************
* lock_file (file,pid)
*   char *file
*
*   this routine "locks" the file for process pid as follows:
*
*   1. if file exists, the old pid is read out of the file.
*
*      if the old pid and the new pid are the same, then
*      nothing more is done and the request is successful.
*
*      if this the old pid process is still running, the file is
*      considered locked.
*
*   2. if file does not exist, or if file exists but process is not
*      running (ie, lock was not removed), the file is locked for
*      process pid by writing pid into the file.
*
*
* note: since file is created by this routine, it shouldn't be
*       a file that is used for any other purpose.
*
*       also, this lock mechanism is advisory. programs must call this
*       routine and check the return status.
*
* returns:
*       1 ok lock is in place.
*       0 could not lock because another process has the file locked already
*      -1 error. could not create the lock file
*      -2 error. could not read the lock file.
*      -3 error. could not write the lock file.
******************************************************************/

#include	<errno.h>

#define OK 1
#define ALREADY_LOCKED 0
#define CANT_CREATE -1
#define CANT_READ -2
#define CANT_WRITE -3

extern	int	errno ;

static lock_file (file, lock_pid)
    char *file;
{
    int fd;
    int locked;
    int mask;
    int n;
    int old_pid;


    locked = 0;
    if (access (file, 0) == 0) /* file exists */
    {
	for (n = 0; n < 2; n++)
	{
	    if (get_pid (file, &old_pid))
		break;
	    if (n == 0)
		sleep(1); /* allow time for file creator to write its pid */
	}
	if (n == 2)
	    return CANT_READ;
	if (lock_pid == old_pid)
	    return OK;
	locked = find_process (old_pid);
    }
    if (locked)
	return ALREADY_LOCKED;
    mask = umask (0);
    fd = creat (file, 0666) ;
    umask (mask);
    if (fd < 0)
	return CANT_CREATE;
    if (write(fd, &lock_pid, sizeof lock_pid) != sizeof lock_pid)
    {
	close (fd);
	return CANT_WRITE;
    }
    close (fd);
    return OK;
}

static
get_pid (file, old_pid)
    char *file;
    int *old_pid;
{
    int fd;
    int n;

    if ((fd = open (file, 0)) < 0)
	return 0;
    n = read (fd, old_pid, sizeof (*old_pid));
    close (fd);
    return n == sizeof (*old_pid);
}

static
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

unlock_file (file)
    char *file;
{
    if (access (file,0) != 0)
	return 0;
    unlink (file);
    if (access (file,0) != 0)
	return 1;
    return -1;
}

