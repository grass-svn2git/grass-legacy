
/*
*  This program is a top-level menu that runs a multitude of MAPDEV programs
*  trying to create the files necessary to for the 'digit' program.
*
*  Programs used by this program:
*      (/usr/gis/etc)  a.b.dlg, dlg.to.dig, a.b.vect, build.vect
*      
*      dlg.to.dig also forces all unlabeled Area lines to Line lines.
*
*  Data files used by this program:
*      ascii dlg,  binary dlg,  ascii vector,  binary vector,  dig_att
*  
*  Note: all the programs that this calls must exit with a zero value on 
*  successful completion and a true value on error.
*/


#include    <stdio.h>
#include    <signal.h>
#include    "gis.h"

/**  data directories   **/
#define	B_DIG	"dig"
#define	A_DIG	"dig_ascii"
#define	B_DLG	"bdlg"
#define	A_DLG	"dlg"
#define	ATT	"dig_att"
#define	PLUS	"dig_plus"

static char  *current_mapset ;
static char  *gbase ;

static char  *PROG ;

main (argc, argv)
    int  argc ;
    char  *argv[] ;
{

    int   phase ;  /*  current level  */
    int   level ;  /* how many levels to convert to digit vector format  */
    int   force_areas ;

    int   snap ;
    int   ram ;
    char *p;
    char *getenv();

    char  name[128] ;
    char buf[BUFSIZ];

    /*  store filename and path  */
    char  file2[128] ;

    char  command[128] ;

    G_gisinit("support.vect") ;

    phase = 0 ;
    PROG = argv[0] ;
    gbase = G_gisbase() ;
    current_mapset = G_mapset() ;

    ask_for_name(file2, " VECTOR (DIGIT) FILENAME ",name,B_DIG,"binary vector");

    while (1)
    {
	system("clear") ;
	level = ask_which_level(name) ;


	/* decide if we override the default for using the RAM file */
	ram = 0 ;	 /* default */
#ifdef CERL
	ram = 1;	/* if cerl turn it on */
#endif
	p = getenv ("GTUNE_VSUP_RAM");	/* if TUNE var is set */
	if (p != NULL)
	    if (!strcmp ("ON", p))	/* on */
		ram = 1;
	    else
		ram = 0;		/* else off */


    /*  setup the args on the command line for build.vect  */
	switch (level) {
	    case 1:
		run_support_vect( command, name, ram) ;
		break;
	    case 2:
		sprintf (command, "%s/etc/modcats -v %s", gbase, name);
		system (command);
		break;
	    default:
		return;
		break;
	}
    /**********************  build.vect  ***********************/
	sleep (2);
	printf ("hit RETURN to continue -->");
	gets (buf);
    }

}	/*  main()  */


ask_which_level(map)
    char *map;
{
    int	num ;
    char    buf[80] ;

    printf("Selected file is [%s]\n\n", map);
    printf("     1  -   Build topology information (Needed for digit)\n") ;
    printf("     \n") ;
    printf("     2  -   Edit the category file\n") ;
    printf("     \n") ;
    printf("\n\n Enter a number <1-2>\n anything else to quit: ") ;

    if (gets (buf) == NULL)
	clearerr (stdin), exit (1) ;

    num = atoi(buf) ;
    if( num < 1  ||  num > 2)
	fprintf(stderr, "\n\n ...Leaving %s\n\n", PROG),   exit(1) ;

    return(num) ;
}


ask_for_name( file_name, header, name, dir, file_desc)
    char    *file_name, *header,  *name,  *dir, *file_desc ;
{
    char    *mapset ;

    mapset = G_ask_in_mapset( header, name, dir, file_desc, 0) ;
    if ( ! mapset)
	exit(0) ;

    G__file_name( file_name, dir, name, mapset) ;

    return (0);
}    /**  ask_for_name()  **/

run_support_vect( command, name, ram)
    char    *command, *name ;
    int    ram ;
{
    int snap;

	snap=  G_yes(" Do you want to snap nodes to other nodes within a threshold ", 0)  ;

    G__make_mapset_element(ATT) ;
    G__make_mapset_element(PLUS) ;

/*
*  Usage:  build.vect  mapset  file_name snap=["yes", "no"] ram=["yes", "no"]
*    snap:    "no" to leave nodes as they are,
*	     "yes" to snap nodes
*    ram:     "no" to read/write strictly from file
*	     "yes" read everything into memory
*    thresh:  "no" use the default thresh value
*	     "yes" user wants to set own thresh value for snapping
*/


    sprintf( command, "%s/etc/build.vect  %s  %s  %s  %s %s", gbase,
	current_mapset, name, snap ? "snap=yes": "snap=no",
	ram ? "ram=yes" : "ram=no", snap ? "thresh=yes" : "thresh=no") ;


    if (system( command) )
    {
	fprintf(stderr, "ERROR(%s):  Could not build vector file: '%s'\n"
	    , PROG, name) ;
	exit(-1) ;
    }


return(0) ;


} 

