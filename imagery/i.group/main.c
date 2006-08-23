/* parser support by Bob Covill, Tekmap 2001
 * <bcovill@tekmap.ns.ca>
 */

#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/imagery.h>
#include <grass/glocale.h>


/* function prototypes */
static int add_or_update_group(char group[INAME_LEN], char **rasters, int k);
static int add_or_update_subgroup(char group[INAME_LEN],
           char subgroup[INAME_LEN], char **rasters, int k);
static int remove_group_files(char group[INAME_LEN], char **rasters, int k);
static int remove_subgroup_files(char group[INAME_LEN],
           char subgroup[INAME_LEN], char **rasters, int k);


int main(int argc, char *argv[])
{
    char title[80];
    int m, k=0;

    struct Option *grp, *rast, *sgrp;
    struct Flag *r, *l;
    struct GModule *module;

    G_gisinit(argv[0]);

    module = G_define_module();
    module->keywords = _("imagery");
    module->description =
	_("Creates and edits groups and subgroups of imagery files.");

    /* Get Args */
    grp = G_define_option();
    grp->key = "group";
    grp->type = TYPE_STRING;
    grp->required = YES;
    grp->gisprompt = "old,group,group";
    grp->description = _("Name of imagery group");

    sgrp = G_define_option();
    sgrp->key = "subgroup";
    sgrp->type = TYPE_STRING;
    sgrp->required = NO;
    sgrp->description = _("Name of imagery sub-group");

    rast = G_define_option();
    rast->key = "input";
    rast->type = TYPE_STRING;
    rast->required = NO;   /* why is it NO ?? */
    rast->multiple = YES;
    rast->gisprompt = "old,cell,raster";
    rast->description = _("Name of raster(s) to include in group");

    r = G_define_flag();
    r->key = 'r';
    r->description = _("Remove selected files from specified group");

    l = G_define_flag();
    l->key = 'l';
    l->description = _("List files from specified (sub)group");

    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);

    /* Determine number of files to include */
    if (rast->answers) {
	for (m = 0; rast->answers[m]; m++) {
	    k = m;
	}
	k++;
    }
    
    if (k < 1 && !l->answer)  /* remove if input is requirement */
        G_fatal_error( _("No input map(s) specified.") );

    I_location_info(title, argv[0]);

    if (r->answer) {
	/* Remove files from Group */
	if (I_find_group(grp->answer) == 0)
	    G_fatal_error(_("Specified group does not exist... Exiting."));

	if (sgrp->answer) {
	    G_message(_("Removing files from subgroup"));
	    remove_subgroup_files(grp->answer, sgrp->answer, rast->answers, k);
	}
	else {
	    G_message(_("Removing files from group"));
	    remove_group_files(grp->answer, rast->answers, k);
	}
    }
    else {
        if (l->answer) {
            struct Ref ref;

	    if (sgrp->answer) {
                /* list subgroup files */
                I_get_group_ref(grp->answer, &ref);
                I_list_subgroup(grp->answer, sgrp->answer, &ref, stdout);
	    } else {
                /* list group files */
                I_get_group_ref(grp->answer, &ref);
                I_list_group(grp->answer, &ref, stdout);
            }
	}
	else {
		/* Create or update Group REF */
		if (I_find_group(grp->answer) == 0)
		    G_message(_("group [%s] - does not yet exist. Creating..."), grp->answer);

		if (sgrp->answer) {
		    G_message(_("Adding files to group [%s]"), grp->answer);
		    add_or_update_group(grp->answer, rast->answers, k);

		    G_message(_("Adding files to subgroup [%s]"), sgrp->answer);
		    add_or_update_subgroup(grp->answer, sgrp->answer, rast->answers, k);
		}
		else {
		    G_message(_("Adding files to group [%s]"), grp->answer);
		    add_or_update_group(grp->answer, rast->answers, k);
		}
	}
    }

    return 0;
}


static int add_or_update_group(char group[INAME_LEN], char **rasters, int k)
{
    int m, n, skip;
    struct Ref ref;
    char *mapset;

    I_get_group_ref(group, &ref);

    for (m = 0; m < k; m++) {
	skip = 0;
	if ((mapset = G_find_cell(rasters[m], "")) == NULL)
	    G_fatal_error(_("Unable to find raster [%s]."), rasters[m]);

	G_message(_("Adding raster map [%s]"), rasters[m]);

	/* Go through existing files to check for duplicates */
	for (n = 0; n < ref.nfiles; n++) {
	    if (strcmp(rasters[m], ref.file[n].name) == 0) {
		G_message(_("Raster map [%s] exists in group."), rasters[m]);
		G_message(_("Skipping [%s]..."), rasters[m]);
		skip = 1;
		continue;
	    }
	}

	if (skip == 0)
	    I_add_file_to_group_ref(rasters[m], mapset, &ref);
    }

    G_message(_("Writing group REF"));
    I_put_group_ref(group, &ref);
    G_message(_("Done."));

    return 0;
}


static int add_or_update_subgroup(char group[INAME_LEN],
			   char subgroup[INAME_LEN], char **rasters, int k)
{
    int m, n, skip;
    struct Ref ref;
    char *mapset;

    I_get_subgroup_ref(group, subgroup, &ref);

    for (m = 0; m < k; m++) {
	skip = 0;
	if ((mapset = G_find_cell(rasters[m], "")) == NULL)
	    G_fatal_error(_("Unable to find raster [%s]."), rasters[m]);

	G_message(_("Adding raster map [%s]"), rasters[m]);

	/* Go through existing files to check for duplicates */
	for (n = 0; n < ref.nfiles; n++) {
	    if (strcmp(rasters[m], ref.file[n].name) == 0) {
		G_message(_("Raster map [%s] exists in group."), rasters[m]);
		G_message(_("Skipping [%s]..."), rasters[m]);
		skip = 1;
		continue;
	    }
	}
	if (skip == 0)
	    I_add_file_to_group_ref(rasters[m], mapset, &ref);
    }

    G_message(_("Writing subgroup REF"));
    I_put_subgroup_ref(group, subgroup, &ref);
    G_message(_("Done."));

    return 0;
}


static int remove_group_files(char group[INAME_LEN], char **rasters, int k)
{
    int m, n, skip;
    struct Ref ref;
    struct Ref ref_tmp;
    char *mapset;
    char tmp_name[INAME_LEN];
    char xname[512], xmapset[512];

    I_get_group_ref(group, &ref_tmp);
    I_init_group_ref(&ref);

    /* Parse through supplied rasters */
    for (m = 0; m < k; m++) {
	skip = 0;
	strcpy(tmp_name, rasters[m]);
	mapset = G_mapset();

	/* Parse out mapset */
	if (G__name_is_fully_qualified(rasters[m], xname, xmapset)) {
	    strcpy(tmp_name, xname);
	    strcpy(mapset, xmapset);
	}

	/* Go through existing files to check for duplicates */
	for (n = 0; n < ref_tmp.nfiles; n++) {
	    if ((strcmp(tmp_name, ref_tmp.file[n].name) == 0) &&
		(strcmp(mapset, ref_tmp.file[n].mapset) == 0)) {
		G_message(_("Found file <%s@%s> in group."), tmp_name, mapset);
	    }
	    else {
		I_add_file_to_group_ref(ref_tmp.file[n].name,
					ref_tmp.file[n].mapset, &ref);
	    }
	}
    }

    G_message(_("Writing group REF"));
    I_put_group_ref(group, &ref);
    G_message(_("Done."));

    return 0;
}


static int remove_subgroup_files(char group[INAME_LEN],
			  char subgroup[INAME_LEN], char **rasters, int k)
{
    int m, n, skip;
    struct Ref ref;
    struct Ref ref_tmp;
    char *mapset;
    char tmp_name[INAME_LEN];
    char xname[512], xmapset[512];

    I_get_subgroup_ref(group, subgroup, &ref_tmp);
    I_init_group_ref(&ref);

    /* Parse through supplied rasters */
    for (m = 0; m < k; m++) {
	skip = 0;
	strcpy(tmp_name, rasters[m]);
	mapset = G_mapset();

	/* Parse out mapset */
	if (G__name_is_fully_qualified(rasters[m], xname, xmapset)) {
	    strcpy(tmp_name, xname);
	    strcpy(mapset, xmapset);
	}

	/* Go through existing files to check for duplicates */
	for (n = 0; n < ref_tmp.nfiles; n++) {
	    if ((strcmp(tmp_name, ref_tmp.file[n].name) == 0) &&
		(strcmp(mapset, ref_tmp.file[n].mapset) == 0)) {
		G_message(_("Found file <%s@%s> in subgroup."), tmp_name, mapset);
	    }
	    else {
		I_add_file_to_group_ref(ref_tmp.file[n].name,
					ref_tmp.file[n].mapset, &ref);
	    }
	}
    }

    G_message(_("Writing subgroup REF"));
    I_put_subgroup_ref(group, subgroup, &ref);
    G_message(_("Done."));

    return 0;
}
