#define MAIN
#include "xgrass.h"
#include "sgi.h"

static XrmOptionDescRec initTable[] = {
{"-title",	"*title",	XrmoptionSepArg, (caddr_t) NULL},
{"-font",	"*fontList",	XrmoptionSepArg, (caddr_t)"fixed"},
{"-fn",		"*fontList",	XrmoptionSepArg, (caddr_t)"fixed"},
};

char *version = "XGRASS4.2 ";
char *rel_date = "30 September 1997";

extern XtEventHandler _XgAddHistoryCommand();

#define xgrass_width 64
#define xgrass_height 64
static char xgrass_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x0f, 0xe4, 0xcf, 0x3f, 0xf8, 0xf1, 0x9f, 0xff,
   0x0f, 0x72, 0x8e, 0x71, 0xdc, 0x19, 0xde, 0xf0, 0x1e, 0x71, 0x9c, 0x71,
   0xc6, 0x19, 0xd8, 0xc0, 0x3c, 0x31, 0x80, 0x71, 0xc6, 0x19, 0xc0, 0x00,
   0xb8, 0x30, 0x80, 0x3f, 0xfe, 0xf9, 0xcf, 0x7f, 0x58, 0xb0, 0x87, 0x03,
   0xce, 0xf1, 0x9f, 0xff, 0xe8, 0x30, 0x8f, 0x0d, 0xc6, 0x01, 0x1c, 0xe0,
   0xe4, 0x31, 0x9e, 0x1d, 0xc6, 0x01, 0x1c, 0xe0, 0xc4, 0x73, 0x9c, 0x39,
   0xc6, 0x19, 0xdc, 0xe0, 0xc2, 0x73, 0x9c, 0x39, 0xc6, 0x39, 0xdc, 0xe1,
   0x81, 0xe7, 0xcf, 0x73, 0xef, 0xfb, 0xcf, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3e, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x81, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x07, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x38, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x18, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x10, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x16, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x07, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01,
   0x04, 0x18, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x03, 0x0c, 0x00, 0x00,
   0x00, 0x00, 0x38, 0x80, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x40,
   0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x20, 0x80, 0x03, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x18, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x1c,
   0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0xfe, 0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0f, 0xf9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x60,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x03, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x7e,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xe0, 0xc1, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0xff, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

main(argc,argv)
    unsigned argc;
    char **argv;
{
    char *cppflags = NULL;
    char cmd[80];
    extern FILE *yyin;
    Arg al[15];
    int ac = 0;
    Atom protocol;
    Widget shell;
    Pixmap xgrass_icon;
    FILE *fp, *popen();
    char command[256];
    char buf[256];
    char s[256];
    char *gisrc;
    char *libdir;
    char *home;
    char **sessions;
    int i;
    int count = 0;
    Boolean noSessions = True;

    /*
     * get XGRASSLIBDIR
     */
    if ((libdir = (char *)getenv("XGRASSLIBDIR")) == NULL ) {
	fprintf(stderr,"Oops, no XGRASSLIBDIR set.\n");
	fprintf(stderr,"Try starting with \"xgrass\" ?\n");
	exit(-1);
    }

    /* 
     * set GIS_LOCK to current process id, so that kids will know their 
     * parent...
     */
#if defined(mips) || defined(BSD)
    sprintf(buf,"%d",getpid());
    if ( setenv("GIS_LOCK",_XgStrDup(buf),1) != 0 ) {
#else
    sprintf(buf,"GIS_LOCK=%d",getpid());
    if ( putenv(_XgStrDup(buf)) != 0 ) {
#endif
	fprintf(stderr,"Setting GIS_LOCK failed, not enough memory\n");
	exit(-1);
    }

    /*
     * Check for existance of $HOME/.xgrass, create it if not there...
     */
    home = (char *)getenv("HOME");
    sprintf(buf,"%s/.xgrass",home);
    if ( access(buf,X_OK) == -1 ) {
	fprintf(stderr,"Welcome to xgrass\t%s\t%s\n\n",version,rel_date);
        fprintf(stderr,"We will have to do a bit of setup before we begin.\n");
        fprintf(stderr,"We need a place to store session information.\n");
        fprintf(stderr,"Creating \"%s\"\n",buf);
#ifdef mips
	mkdir(buf,0777);
#else
	mkdir(buf,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
#endif
    } 
    /*
     * Check for existance of $HOME/.xgrass/session, create it if not there...
     */
    sprintf(buf,"%s/.xgrass/session",home);
    if ( access(buf, X_OK) == -1 ) {
        fprintf(stderr,"Creating \"%s\"\n",buf);
#ifdef mips
	mkdir(buf,0777);
#else
	mkdir(buf,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
#endif
    }
    /*
     * Check for existance of $HOME/.xgrass/histories, create it if not there...
     */
    sprintf(buf,"%s/.xgrass/histories",home);
    if ( access(buf, X_OK) == -1 ) {
        fprintf(stderr,"Creating \"%s\"\n",buf);
#ifdef mips
	mkdir(buf,0777);
#else
	mkdir(buf,S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
#endif
    }
    /*
     * check for GISRC environment variable.
     * If one exists, unset it...and set to a temporary...
     */
    if ((gisrc = (char *)getenv("GISRC")) != NULL ) {
#if  defined(mips) || defined(BSD)
	setenv("GISRC","",1);
#else
	putenv(_XgStrDup("GISRC="));
#endif
    }
#if defined(mips) || defined(BSD)
    sprintf(buf,"%s/.xgrass/session/.grassrc%d",home,getpid());
    setenv("GISRC",_XgStrDup(buf),1);
#else
    sprintf(buf,"GISRC=%s/.xgrass/session/.grassrc%d",home,getpid());
    putenv(_XgStrDup(buf));
#endif
    sprintf(buf,"%s/.xgrass/session/.grassrc%d",home,getpid());
    G__setenv("GISRC",buf);

    /*
     * Check for saved session files...
     * Should use opendir, readdir....
     */
    sprintf(buf,"%s/.xgrass/session",home);
    sessions = _XgDirectoryListing(buf, &count, False, 1, False);

    /* start signal handling */
    signal(SIGKILL,Interrupt);
    signal(SIGHUP,Interrupt);
    signal(SIGINT,Interrupt);
    signal(SIGQUIT,Interrupt);

    /* set some global values */
    _XG_Global.menuRunning = False;
    _XG_Global.progName = 
	(NULL != (_XG_Global.progName = G_rindex(argv[0],'/'))) ?
                  ++_XG_Global.progName:*argv;
    _XG_Global.menuData.specFile = NULL;
    _XG_Global.history_enabled = XG_HISTORY_ENABLE;

    /* initialize the toolkit  */
    /* and open the display (and a few other things...)  */
    XtSetArg(al[ac], XmNallowShellResize, True); ac++;
    _XG_Global.applShell = XtAppInitialize(&_XG_Global.appContext, "XGrass", initTable, XtNumber(initTable), &argc, argv, NULL, al, ac);

    _XG_Global.display = XtDisplay(_XG_Global.applShell);

    /* now the parse the rest of the command line */
    cppflags = ParseCommand(argc,argv);

    /* if the specification file was not set by command line, go find it */
    if ( _XG_Global.menuData.specFile == NULL ) {
	char temp[1024];

	sprintf(temp,"%s/.xgrass-menu",getenv("HOME"));
	if ( (access(temp,R_OK)) == 0 ) {
	    _XG_Global.menuData.specFile = XtNewString(temp);
	} else {
	   sprintf(temp,"%s/xgrass-menu",libdir);
	    if ( (access(temp,R_OK)) < 0 ) {
	       fprintf(stderr, "Can't find specification file...\n");
	       exit(0);
	    }
	    _XG_Global.menuData.specFile = XtNewString(temp);
	}
    }
    
    /* set lex input _XG_Global.menuData.specFile to return from cpp or 
       open the file */
    if ( cppflags == NULL ) 
        sprintf(cmd,"/lib/cpp -P %s",_XG_Global.menuData.specFile);
    else
        sprintf(cmd,"/lib/cpp -P %s %s",cppflags,_XG_Global.menuData.specFile);

    if ((yyin = popen(cmd,"r")) == NULL ) {
        sprintf(errorbuf,"Can't open %s",_XG_Global.menuData.specFile);
        FatalError("running C preprocessor",errorbuf);
    }


    xgrass_icon = XCreateBitmapFromData(_XG_Global.display,
	DefaultRootWindow(_XG_Global.display), 
	xgrass_bits, xgrass_width, xgrass_height);

    shell = _XG_Global.applShell;

    __XgHistorySetup();

    protocol = XmInternAtom(XtDisplay(shell), "WM_DELETE_WINDOW", False);
    XmAddWMProtocols(shell, &protocol, 1);
    XtAddEventHandler(shell, NoEventMask, True, _XgWMClientMessage, shell);
    XtAddEventHandler(shell, PropertyChangeMask, False, _XgAddHistoryCommand, shell);

    if ( XmIsMotifWMRunning(shell) ) {
        unsigned int decor_flags, func_flags;

        decor_flags = MWM_DECOR_BORDER | MWM_DECOR_RESIZEH;
        decor_flags |= MWM_DECOR_TITLE | MWM_DECOR_MENU;
        decor_flags |= MWM_DECOR_MINIMIZE;

        func_flags = MWM_FUNC_CLOSE | MWM_FUNC_RESIZE;
        func_flags |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;

        XtVaSetValues(shell,
            XmNmwmDecorations, decor_flags,
            XmNmwmFunctions, func_flags,
            NULL);
    }

    /* parse the script, and get ptr to the internal representation */
    if ( yyparse() ) exit(1);

    if (_XG_Global.session) {
        StartSession(_XG_Global.session);
    } else {
	if (_XG_Global.database && _XG_Global.location && _XG_Global.mapset) {
	  StartTrio(_XG_Global.database,_XG_Global.location,_XG_Global.mapset);
	}
	else {
	    if ( count == 0 ) {
		/*
		 * If no session files, put up xgdbset to create a new session
		 */
		XgDbSet(NULL,0,True);
	    } else {
		/*
		 * If session files, ask if user wants to start from saved session
		 * If yes, get the session to start with, else create a new session.
		 */
		XgDbSet(sessions,count,True);
	    }
	}
    }

    XtRealizeWidget(_XG_Global.applShell);
    XgMainLoop (_XG_Global.appContext);
}
