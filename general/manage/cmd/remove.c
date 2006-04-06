#include <stdlib.h>
#include <string.h>
#include <grass/glocale.h>
#define MAIN
#include "list.h"
#include "local_proto.h"

int 
main (int argc, char *argv[])
{
    int i,n;
    struct GModule *module;
    struct Option **parm, *p;
    char *name, *mapset;
    char rname[256], rmapset[256];
    int nrmaps;
    char **rmaps, *location_path, buf1[256], buf2[256];
    FILE *fp;
    int result = 0;

    init (argv[0]);

    module = G_define_module();
    module->description =
		_("Removes data base element files from "
		"the user's current mapset.");

    parm = (struct Option **) G_calloc (nlist, sizeof(struct Option *));

    for (n = 0; n < nlist; n++)
    {
	p = parm[n] = G_define_option();
	p->key = list[n].alias;
	p->type = TYPE_STRING;
	p->required = NO;
	p->multiple = YES;
	p->gisprompt = G_malloc (64);
	sprintf (p->gisprompt, "old,%s,%s", list[n].mainelem, list[n].maindesc);
	p->description = G_malloc (64);
	sprintf (p->description, _("%s file(s) to be removed"), list[n].alias);
    }

    if (G_parser(argc, argv))
	exit(1);

    location_path = G__location_path();
    mapset = G_mapset();

    for (n = 0; n < nlist; n++)
    {
	if (parm[n]->answers)
	    for (i = 0; (name = parm[n]->answers[i]); i++)
	    {
		if(G_is_reclassed_to(name, mapset, &nrmaps, &rmaps) > 0)
		{
		    fprintf(stderr,
		       "[%s@%s] is a base map. Remove reclassed map%s first:\n",
					name, mapset, (nrmaps > 1 ? "s" : ""));

		    fprintf(stderr, " %s", *rmaps);
		    for(rmaps++; *rmaps; rmaps++)
		        fprintf(stderr, ",%s", *rmaps);
		    fprintf(stderr, "\n");
		    continue;
		}
		if(G_is_reclass(name, mapset, rname, rmapset) > 0 &&
		   G_is_reclassed_to(rname, rmapset, &nrmaps, &rmaps) > 0)
		{
		    char *p = strchr(rname, '@');
		    if (p)
			*p = '\0';
		    sprintf (buf1, "%s/%s/cell_misc/%s/reclassed_to",
				    location_path, rmapset, rname);
		    sprintf(buf2, "%s@%s", name, mapset);

		    if(nrmaps == 1 && !strcmp(rmaps[0], buf2))
		    {
			remove(buf1);
		    }
		    else
		    {
		        fp = fopen(buf1, "w");
		        for(; *rmaps; rmaps++)
		        {
			    if(strcmp(*rmaps, buf2))
			        fprintf(fp, "%s\n", *rmaps);
		        }
		        fclose(fp);
		    }
		}
		if ( do_remove (n, name) == 1 )
                {
		    result = 1;
                }
	    }
    }
    exit(result);
}
