#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gis.h"

int main(int argc, char *argv[])
{
    int n, store;
    int tty;
    char *name, *value, *ptr;
    struct Option *get, *set, *store_opt;
    struct GModule *module;
    
    G_gisinit (argv[0]);

    module = G_define_module();
    module->description =
		"Outputs the user's current GRASS variable settings.";

    get               = G_define_option() ;
    get->key          = "get" ;
    get->type         = TYPE_STRING ;
    get->description  = "GRASS variable to get" ;
    get->required     = NO;

    set               = G_define_option() ;
    set->key          = "set" ;
    set->type         = TYPE_STRING ;
    set->description  = "GRASS variable to set" ;
    set->required     = NO;

    store_opt               = G_define_option() ;
    store_opt->key          = "store" ;
    store_opt->type         = TYPE_STRING ;
    store_opt->options      = "gisrc,mapset";
    store_opt->answer       = "gisrc";
    store_opt->description  = "Where GRASS variable is stored" ;
    store_opt->required     = NO;
    
    /* Print or optionally set environment variables */
    if (argc == 1)
    {
        tty = isatty(1);
        for (n=0; (name = G__env_name (n)); n++)
        {
            if ((value = G__getenv(name)))
            {
                if (tty)
                    fprintf (stdout,"%s=%s\n", name, value);
                else
                    fprintf (stdout,"%s='%s';\n", name, value);
            }
        }
        return 0;
    }

    if (G_parser(argc, argv) < 0)
       exit(-1);

    store = G_VAR_GISRC;
    if ( store_opt->answer[0] == 'm' ) store = G_VAR_MAPSET;
    
    if (get->answer != NULL)
    {
	value = G__getenv2 (get->answer, store) ;
	if (value != NULL)
	    fprintf (stdout, "%s\n", value);
	return 0;
    }

    if (set->answer != NULL)
    {
	value = NULL ;
	name = set->answer ;
	ptr = strchr (name, '=') ;
	if (ptr != NULL)
	{ 
	    *ptr = '\0';
	    value = ptr + 1;
	}
	/* Allow unset without '=' sign */
	if (value != NULL && *value == '\0')
	  value = NULL ;
	  
        G_setenv2 (name, value, store) ;

	return 0;
    }
	
    /* Something's wrong if we got this far */
    G_usage();
    return -1;
}
