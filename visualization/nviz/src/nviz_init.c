/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include "tk.h"
#include "interface.h"
#include "gis.h"
#include "G3d.h"
#include "coldefs.h"
#include "bitmap.h"
/* get from gislib: */
#include "version.h"

char startup_script[] = "toplevel .wait_ok\n\
label .wait_ok.wait -text \"Please wait...\" -fg red -bg black\n\
pack .wait_ok.wait -ipadx 20 -ipady 20 -expand 1 -fill both\n\
wm geometry .wait_ok \"+800+50\"\n\
wm geometry . \"+100+100\"\n\
wm title . \"NVIZ\"\n\
update\n\
grab .wait_ok.wait\n";

int script_mode=0;

int parse_command(Nv_data * data, Tcl_Interp * interp,	/* Current interpreter. */
		  int argc, char **argv)
{
    struct Option *elev, *colr, *vct;
    struct Option *panel_path, *script, *state;
    struct Flag *no_args, *script_kill, *demo, *verbose;
    struct GModule *module;
    char *arglist[3], *autoload;
    int i, aload = 1;
 
    /*
     * Flags and Options:
     * -q : quickstart, starts nvwish without querying for the usual maps
     *
     * path : panel path, adds the given directory to the path to
     *        search for panels
     *
     * script : script file, after starting nviz immediately plays the
     *          named script
     *
     * -k : script kill, if this flag is set, then Nviz will exit after completing
     *      a script started from the command line
     *
     * -x : demo mode, the usual "please wait" messages are nuked.
     */

    module = G_define_module();
    module->description =
	"nviz - Visualization and animation tool for GRASS data";

    elev = G_define_option();
    elev->key = "elevation";
    elev->type = TYPE_STRING;
    elev->required = NO;
    elev->multiple = YES;
    elev->gisprompt = "old,cell,Raster";
    elev->description = "Raster file(s) for Elevation";

    colr = G_define_option();
    colr->key = "color";
    colr->type = TYPE_STRING;
    colr->required = NO;
    colr->multiple = YES;
    colr->gisprompt = "old,cell,Raster";
    colr->description = "Raster file(s) for Color";

    vct = G_define_option();
    vct->key = "vector";
    vct->type = TYPE_STRING;
    vct->required = NO;
    vct->multiple = YES;
    vct->gisprompt = "old,vector,Vector";
    vct->description = "Vector overlay file(s)";

    no_args = G_define_flag();
    no_args->key = 'q';
    no_args->description = "Quickstart - Do not load any data";

    script_kill = G_define_flag();
    script_kill->key = 'k';
    script_kill->description = "Script kill option";

    demo = G_define_flag();
    demo->key = 'x';
    demo->description = "Start in Demo mode";

    verbose = G_define_flag();
    verbose->key = 'v';
    verbose->description = "Output more comments (default=quiet)";

    panel_path = G_define_option();
    panel_path->key = "path";
    panel_path->type = TYPE_STRING;
    panel_path->required = NO;
    panel_path->description = "Set alternative panel path";

    script = G_define_option();
    script->key = "script";
    script->type = TYPE_STRING;
    script->required = NO;
    script->description = "Execute script file at startup";

    state = G_define_option();
    state->key = "state";
    state->type = TYPE_STRING;
    state->required = NO;
    state->description = "Load previosly saved state file";

    if (G_parser(argc, argv))
	exit(0);
    /* Exit status is zero to avoid TCL complaints */

    {
	float defs[MAX_ATTS];

	defs[ATT_TOPO] = 0;
	defs[ATT_COLOR] = DEFAULT_SURF_COLOR;
	defs[ATT_MASK] = 0;
	defs[ATT_TRANSP] = 0;
	defs[ATT_SHINE] = 60;
	defs[ATT_EMIT] = 0;
	GS_set_att_defaults(defs, defs);
    }

    /* Put in the "please wait..." message unless we are in demo mode */
    if ((strstr(argv[0], "nviz") != NULL) && (!demo->answer)) {
	if (Tcl_Eval(interp, startup_script) != TCL_OK)
	    G_fatal_error("%s", interp->result);

    }

    if (verbose->answer)
    {
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Version: %s\n", GRASS_VERSION_STRING);
    fprintf(stderr, "\n");
    fprintf(stderr,
	    "Authors: Bill Brown, Terry Baker, Mark Astley, David Gerdes\n");
    fprintf(stderr, "\tmodifications: Jaro Hofierka, Bob Covill\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
	    "Please cite one or more of the following references in publications\n");
    fprintf(stderr, "where the results of this program were used:\n");
    fprintf(stderr,
	    "Brown, W.M., Astley, M., Baker, T., Mitasova, H. (1995).\n");
    fprintf(stderr,
	    "GRASS as an Integrated GIS and Visualization System for\n");
    fprintf(stderr,
	    "Spatio-Temporal Modeling, Proceedings of Auto Carto 12, Charlotte, N.C.\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
	    "Mitasova, H., W.M. Brown, J. Hofierka, 1994, Multidimensional\n");
    fprintf(stderr, "dynamic cartography. Kartograficke listy, 2, p. 37-50.\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
	    "Mitas L., Brown W. M., Mitasova H., 1997, Role of dynamic\n");
    fprintf(stderr,
	    "cartography in simulations of landscape processes based on multi-variate\n");
    fprintf(stderr,
	    "fields. Computers and Geosciences, Vol. 23, No. 4, pp. 437-446\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "http://www2.gis.uiuc.edu:2280/modviz/viz/nviz.html\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "The papers are available at\n");
    fprintf(stderr, "http://www2.gis.uiuc.edu:2280/modviz/\n");
    } /* done verbose */


    /* Look for quickstart flag */
    if (no_args->answer) {
	elev->answers = colr->answers = vct->answers = NULL;
    }


    /* Look for scriptkill flag */
    if (script_kill->answer) {
	if (Tcl_VarEval(interp, "set NvizScriptKill 1 ", NULL) != TCL_OK)
	    G_fatal_error("%s", interp->result);
    }

    /* See if an alternative panel path is specified */
    if (panel_path->answer) {
	/* If so then set the variable NvizAltPath to the alternative path
	 */
	if (Tcl_VarEval(interp, "set NvizAltPath ", panel_path->answer,
			NULL) != TCL_OK)
	    G_fatal_error("%s", interp->result);
    }

    /* Get State file from command line */
    if (state->answer) {
	if (Tcl_VarEval(interp, "set NvizLoadState ", state->answer,
			NULL) != TCL_OK)
	    G_fatal_error("%s", interp->result);
    }

    /* See if a script file was specified */
    if (script->answer) {
	/* If so then set the variable NvizPlayScript to the file */
	if (Tcl_VarEval(interp, "set NvizPlayScript ", script->answer,
			NULL) != TCL_OK)
	    G_fatal_error("%s", interp->result);
    }

#ifdef XSCRIPT
    AUTO_FILE = aut->answer;
    /* either file name or NULL */

    Write_script = swrit->answer;
#endif

    /* Consult the user's .grassrc file to see if we should
     * automatically set the colormap of loaded surfaces to be
     * the same as the raster used for topography.  The appropriate
     * resource is:
     *     Nviz_AutoSurfColors
     * If this resource isn't specified, it defaults to true.
     */
    autoload = G__getenv("Nviz_AutoSurfColors");
    if ((autoload != NULL) && (!strcmp(autoload, "false")))
	aload = 0;

    /* Parse answeres from user */
    /* Run check to make sure elev == colr */
    if (elev->answers && colr->answers) {
	int ee, cc;
	for (i = 0; elev->answers[i]; i++) {
	    ee = i;
	}
	for (i = 0; colr->answers[i]; i++) {
	    cc = i;
	}
	if (ee != cc)
	    G_fatal_error
		("Number of elevation files does not match number of colors files");
    }

    if (elev->answers) {
	char tmp[30];

	for (i = 0; elev->answers[i]; i++) {
	    arglist[1] = "surf";
	    arglist[2] = elev->answers[i];
	    Nnew_map_obj_cmd(data, interp, 3, arglist);

	    /* See if we should autoload the color file */
	    if (aload) {
		strncpy(tmp, interp->result, 29);
		if (colr->answers) {
		    if (Tcl_VarEval(interp, tmp, " set_att color ",
				    colr->answers[i], NULL) != TCL_OK)
			G_fatal_error("%s", interp->result);
		}
		else {
		    if (Tcl_VarEval(interp, tmp, " set_att color ",
				    elev->answers[i], NULL) != TCL_OK)
			G_fatal_error("%s", interp->result);
		}
	    }
	}

	if (i > 1)
	    set_default_wirecolors(data, i);
    }

    if (vct->answers) {
	for (i = 0; vct->answers[i]; i++) {
	    arglist[1] = "vect";
	    arglist[2] = vct->answers[i];
	    Nnew_map_obj_cmd(data, interp, 3, arglist);
	}
    }

    return(TCL_OK);
}


/*
 * Ngetargs: gets command line args from tcl. Tcl stores argv[0] by
 * itself and the rest of the args as a single string so Ngetargs goes
 * through some string manipulation to put all the args back into a single array
 * so that G_parser can deal with them without getting sick.
 */

int Ngetargs(Tcl_Interp * interp,	/* Current interpreter. */
	     char ***args)
{
    char *tmp, *tmp2, *argv0;
    const char *cmd;
    int argc;

    argv0 = Tcl_GetVar(interp, "argv0", TCL_LEAVE_ERR_MSG);
    tmp = Tcl_GetVar(interp, "argv", TCL_LEAVE_ERR_MSG);
    cmd = Tcl_GetNameOfExecutable();

    if (strstr(argv0, "script_tools") != NULL ||
		    strstr(argv0, "script_play") != NULL ||
		    strstr(argv0, "script_get_line") != NULL ||
		    strstr(argv0, "script_file_tools") != NULL) {
	    fprintf(stderr, "Entering script mode ...\n");
	    tmp2 = (char *)malloc((strlen(cmd) + 2) * (sizeof(char)));
	    sprintf(tmp2, "%s", cmd);
	    script_mode = 1;
    } else if ( strstr(cmd, argv0) == NULL) {
	    tmp2 = (char *)malloc((strlen(cmd) + strlen(argv0) + strlen(tmp) + 4 ) * (sizeof(char)));
	    sprintf(tmp2, "%s %s %s", cmd, argv0, tmp);
    } else {
	    tmp2 = (char *)malloc((strlen(argv0) + strlen(tmp) + 2) * (sizeof(char)));
	    sprintf(tmp2, "%s %s", argv0, tmp);
    }

    if (TCL_ERROR == Tcl_SplitList(interp, tmp2, &argc, args))
	    exit(-1);

    return (argc);
}
    
int make_red_yellow_ramp(int *ramp, int num, int minval, int maxval)
{
    int g, i, incr;

    incr = (maxval - minval) / (num - 1);
    for (i = 0; i < num; i++) {
	g = minval + incr * i;
	RGB_TO_INT(maxval, g, 0, ramp[i]);
    }

    return 0;
}


/* Sorts surfaces by mid elevation, lowest to highest.
 * Puts ordered id numbers in id_sort, leaving surfs unchanged.
 * Puts ordered indices of surfaces from id_orig in indices.
 */
int sort_surfs_mid(int *id_sort, int *indices, int num)
{
    int i, j;
    float midvals[MAX_SURFS];
    float tmp, max, tmin, tmax, tmid;
    int *surf_list;

    surf_list = GS_get_surf_list(&i);
    for (i = 0; i < num; i++) {
	GS_get_zextents(surf_list[i], &tmin, &tmax, &tmid);
	if (i == 0)
	    max = tmax;
	else
	    max = max < tmax ? tmax : max;
	midvals[i] = tmid;
    }

    for (i = 0; i < num; i++) {
	tmp = midvals[0];
	indices[i] = 0;
	for (j = 0; j < num; j++) {
	    if (midvals[j] < tmp) {
		tmp = midvals[j];
		indices[i] = j;
	    }
	}
	midvals[indices[i]] = max + 1;
	id_sort[i] = surf_list[indices[i]];
    }
    return(TCL_OK);
}

int set_default_wirecolors(Nv_data * dc, int surfs)
{

#ifdef DO_GREYSCALE
    int *surf_list;
    int i, color, greyincr, greyval;

    greyincr = 200 / (surfs + 1);	/* just use upper values */

    surf_list = GS_get_surf_list(&i);
    for (i = 0; i < surfs; i++) {
	greyval = 55 + greyincr * (i + 1);
	RGB_TO_INT(greyval, greyval, greyval, color);
	GS_set_wire_color(surf_list[i], color);
    }
    free(surf_list);

#else

    int i, ramp[MAX_SURFS];
    int sortSurfs[MAX_SURFS], sorti[MAX_SURFS];
    make_red_yellow_ramp(ramp, surfs, 30, 255);
    sort_surfs_mid(sortSurfs, sorti, surfs);

    for (i = 0; i < surfs; i++) {
	GS_set_wire_color(sortSurfs[i], ramp[i]);
    }

#endif
    return 0;
}

int Ninit(Tcl_Interp * interp, Tk_Window w)
{
    static Nv_data data;
    char nviz_script[] = "source [exec g.gisenv GISBASE]/etc/nviz2.2/scripts/nviz2.2_script\n";

    /* compile in the home directory */
    Tcl_SetVar(interp, "src_boot", getenv("GISBASE"), TCL_GLOBAL_ONLY);

    init_commands(interp, &data);

    Ninitdata(interp, &data);

    if (!script_mode)
	    Tcl_Eval(interp, nviz_script); /* source nviz_script to start main window */

    return(TCL_OK);
}

void swap_togl();

int Ninitdata(Tcl_Interp * interp,	/* Current interpreter. */
	      Nv_data * data)
{
    char **argv;
    int argc;
    G3D_Region win3;
    struct Cell_head win2;
    char *xdbase, *xlocation, *xmapset;
    char xname[512], xname_perm[512];
    

    argc = Ngetargs(interp, &argv);

    G_gisinit(argv[0]);
    
    /*G3d_initDefaults ();*/
        xdbase = G_gisdbase();
        xlocation = G_location();
	xmapset = G_mapset();
	sprintf(xname, "%s/%s/%s/%s", xdbase, xlocation, xmapset, G3D_WINDOW_ELEMENT);
	sprintf(xname_perm, "%s/%s/PERMANENT/%s", xdbase, xlocation, G3D_DEFAULT_WINDOW_ELEMENT);
	if (! G3d_readWindow (&win3, xname))
	{
		if (! G3d_readWindow (&win3, xname_perm))
		{
			 fprintf(stderr, "Creating DEFAULT_WIND3\n");
			 G_get_default_window (&win2);
			 G3d_incorporate2dRegion(&win2, &win3);
			 win3.top = 100.;
			 win3.bottom = 0.;
			 win3.tb_res = 2.;
			 G3d_adjustRegionRes (&win3);
			 G3d_writeWindow(&win3, xname_perm);
		}
		fprintf(stderr, "Creating WIND3 File\n");
		G3d_writeWindow(&win3, xname);
	}

    GS_libinit();

	GVL_libinit(); /* TODO */

	GS_set_swap_func(swap_togl);
    data->NumCplanes = 0;
    data->CurCplane = 0;
    if (!script_mode)
	    parse_command(data, interp, argc, argv);

    return(TCL_OK);
}
