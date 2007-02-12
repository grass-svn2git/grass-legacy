#define MAIN
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include "list.h"
#include "local_proto.h"

int 
main (int argc, char *argv[])
{
    int i,n;
    struct GModule *module;
    struct Option **parm, *p;
    char *old, *new;
    int nrmaps;
    char *mapset, *location_path, **rmaps;
    int result = 0;

    init (argv[0]);

    module = G_define_module();
    module->keywords = _("general, map management");
    module->description =
	    _("Renames data base element files in the user's current mapset.");

    parm = (struct Option **) G_calloc (nlist, sizeof(struct Option *));

    for (n = 0; n < nlist; n++)
    {
	p = parm[n] = G_define_option();
	p->key = list[n].alias;
	p->key_desc="old,new";
	p->type = TYPE_STRING;
	p->required = NO;
	p->multiple = NO;
	p->gisprompt = G_malloc (64);
	sprintf (p->gisprompt, "old,%s,%s", list[n].mainelem, list[n].maindesc);
	p->description = G_malloc (64);
	sprintf (p->description, _("%s file(s) to be renamed"), list[n].alias);
    }

    if (G_parser(argc, argv))
	exit(1);

/* check for even number of names for each element */
    for (n = 0; n < nlist; n++)
    {
	i = 0;
	if (parm[n]->answers)
	    while (parm[n]->answers[i])
		i++;
	if (i%2) /* must be even number of names */
	{
	    G_usage();
	    G_fatal_error(_("must be even number of names"));
	}
    }

    location_path = G__location_path();
    mapset = G_mapset();

    for (n = 0; n < nlist; n++)
    {
	if (parm[n]->answers == NULL)
	    continue;
	i = 0;
	while (parm[n]->answers[i])
	{
	    old = parm[n]->answers[i++];
	    new = parm[n]->answers[i++];
	    if(!find (n, old, mapset))
	    {
		fprintf (stderr, _("ERROR: <%s> not found\n"), old);
		continue;
	    }
	    if (find (n, new, "") && !(module->overwrite))
	    {
		fprintf (stderr, _("ERROR: <%s> already exists in mapset <%s>\n"), new, find (n, new, ""));
		continue;
	    }
	    if (G_legal_filename (new) < 0)
	    {
		fprintf (stderr, _("ERROR: <%s> illegal name\n"), new);
		continue;
	    }
	    if (strcmp (old, new) == 0)
	    {
		fprintf (stderr, _("%s=%s,%s: files are the same, no rename required\n"),
		    parm[n]->key,old,new);
		continue;
	    }

            if(G_is_reclassed_to(old, mapset, &nrmaps, &rmaps) > 0)
            {
		int ptr, l;
    		char buf1[256], buf2[256], buf3[256], *str;
		FILE *fp;

		fprintf(stderr, "Renaming in reclassed map%s...\n",
				(nrmaps > 1 ? "s" : ""));

		for(; *rmaps; rmaps++)
		{
                    fprintf(stderr, " %s\n", *rmaps);
		    sprintf(buf3, "%s", *rmaps);
		    if((str = strchr(buf3, '@')))
		    {
			*str = 0;
			sprintf(buf2, "%s", str+1);
		    }
		    else
		    {
			sprintf(buf2, "%s", mapset);
		    }
		    sprintf(buf1, "%s/%s/cellhd/%s", location_path, buf2, buf3);

		    fp = fopen(buf1, "r");
		    if(fp == NULL)
			continue;

		    fgets(buf2, 255, fp);
		    fgets(buf2, 255, fp);
		    fgets(buf2, 255, fp);

		    ptr = ftell(fp);
		    fseek(fp, 0L, SEEK_END);
		    l = ftell(fp) - ptr;

		    str = (char *) G_malloc(l);
		    fseek(fp, ptr, SEEK_SET);
		    fread(str, l, 1, fp);
		    fclose(fp);

		    fp = fopen(buf1, "w");
		    fprintf(fp, "reclass\n");
		    fprintf(fp, "name: %s\n", new);
		    fprintf(fp, "mapset: %s\n", mapset);
		    fwrite(str, l, 1, fp);
		    G_free (str);
		    fclose(fp);
		}
            }
            if ( do_rename (n, old, new) == 1 )
            {
                result = 1;
            }
	}
    }
    exit(result);
}
