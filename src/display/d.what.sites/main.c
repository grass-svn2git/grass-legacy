#define GLOBAL
#include <stdlib.h>
#include "local_proto.h"

int main(int argc, char **argv)
{
	struct Cell_head window ;
	char temp[128] ;
	char *str;
	int i, j, t, b, l, r ;
	int ml;
	int width, mwidth;
	struct Option *opt1;
	struct Flag *shh, *once, *terse, *dontflash;
	struct GModule *module;

	/* Initialize **site */
	site = NULL;
	
	/* Initialize the GIS calls */
	G_gisinit (argv[0]) ;

	/* Don't fail initially if driver open fails, and don't let call kill
	 * us -- set quiet mode
	 */
	R__open_quiet();
	if (R_open_driver() == 0)
	{
		if(D_get_site_list (&site, &nsites) < 0)
			site = NULL;
		else
		{
			site = (char **)G_realloc(site, (nsites+1)*sizeof(char *));
			site[nsites] = NULL;
		}
		R_close_driver();
	}

	opt1 = G_define_option() ;
	opt1->key        = "map" ;
	opt1->type       = TYPE_STRING ;
	opt1->multiple   = YES ;
	if (site)
	{
		opt1->answers = site;
		opt1->required   = NO ;
	}
	else
	{
		opt1->required = YES;
	}

	opt1->gisprompt  = "old,site_lists,sites" ;
	opt1->description= "Name of existing sites file"; 

	once = G_define_flag();
	once->key = '1';
	once->description ="Identify just one site";
	
	terse = G_define_flag();
	terse->key = 't';
	terse->description = "For parsing by programs: Terse output";
	
	shh = G_define_flag ();
	shh->key = 'q';
	shh->description = "Load quietly";
	
	dontflash = G_define_flag ();
	dontflash->key = 'd';
	dontflash->description = "Disable flashing and extra graphics. "
                                 "Useful over a slow network.";
	
	module = G_define_module();
	module->description = 
	  "Allows the user to interactively query site list descriptions. ";

	if ((argc > 1 || !site) && G_parser(argc, argv))
	    exit(-1);

	if(opt1->answers && opt1->answers[0])
	    site = opt1->answers;

	if(site)
	{
	    for(i=0; site[i]; i++);
	    nsites = i;

	    ml = strlen(G_mapset());
	    width = mwidth = 0;
	    for(i=0; i<nsites; i++)
	    {
		str = strchr(site[i], '@');
		if(str) j = str - site[i];
		else	j = strlen(site[i]);
		if(j > width)
			width = j;

		if(str) j = strlen(str+1);
		else	j = ml;
		if(j > mwidth)
			mwidth = j;

	        if(G_find_sites2(site[i], (str ? str+1 : "")) == NULL)
		{
		    char msg[256];
		    sprintf(msg, "Site file [%s] not available", site[i]);
		    G_fatal_error(msg);
		}
	    }
	    Snum = (int *) G_malloc(sizeof(int)*nsites);
	    CurSites = (Site ***) G_malloc(sizeof(Site **)*nsites);
	}

	if (R_open_driver() != 0)
		G_fatal_error ("No graphics device selected");

	if (D_get_cur_wind(temp))
		G_fatal_error("No current graphics window") ;

	if (D_set_cur_wind(temp))
		G_fatal_error("Current graphics window not available") ;

	/* Read in the map window associated with window */
	G_get_window(&window) ;

	if (D_check_map_window(&window))
		G_fatal_error("Setting graphics window") ;

	if (G_set_window(&window) == -1)
		G_fatal_error("Can't set current graphics window") ;

	/* Determine conversion factors */
	if (D_get_screen_window(&t, &b, &l, &r))
		G_fatal_error("Getting graphics window coordinates") ;
	if (D_do_conversions(&window, t, b, l, r))
		G_fatal_error("Error in calculating conversions") ;
	
	for(i=0; i<nsites; i++){
	    load_sites(i, &window, !(shh->answer));
	}

	what (once->answer, terse->answer, !(dontflash->answer),
	      width, mwidth);

	free_cached_sites();

	R_close_driver();
	exit(0);
}
