/****************************************************************************
 *
 * MODULE:       g.gui
 *
 * AUTHOR(S):    Martin Landa <landa.martin gmail.com>
 *		 Hamish Bowman <hamish_b yahoo com> (fine tuning)
 *
 * PURPOSE:      Start GRASS GUI from command line.
 *
 * COPYRIGHT:    (C) 2008 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/glocale.h>
#include <grass/spawn.h>

int main(int argc, char *argv[])
{
    struct Option *type, *rc_file;
    struct Flag *oneoff;
    struct GModule *module;
    char *gui_type_env;
    char progname[GPATH_MAX];

    G_gisinit(argv[0]);

    module = G_define_module();
    module->keywords = _("general, gui");
    module->description =
	_("Launches a GRASS graphical user interface (GUI) session.");

    type = G_define_option();
    type->key = "gui";
    type->type = TYPE_STRING;
    type->required = YES;
    type->description = _("GUI type");
    type->descriptions = _("tcltk;Tcl/Tk based GUI - GIS Manager (gis.m);"
			   "oldtcltk;Old Tcl/Tk based GUI - Display Manager (d.m);"
			   "wxpython;wxPython based next generation GUI");
    type->options = "tcltk,oldtcltk,wxpython";
    gui_type_env = G__getenv("GRASS_GUI");
    if (gui_type_env && strcmp(gui_type_env, "text")) {
	type->answer = G_store(gui_type_env);
    }
    else {
	type->answer = "tcltk";
    }

    rc_file = G_define_standard_option(G_OPT_F_INPUT);
    rc_file->key = "workspace";
    rc_file->required = NO;
    rc_file->description = _("Name of workspace file");

    oneoff = G_define_flag();
    oneoff->key         = 'u';
    oneoff->description = _("Update default GUI setting");

    if (argc > 1 && G_parser(argc, argv))
	exit(EXIT_FAILURE);


    if ( ( (gui_type_env && oneoff->answer) &&
	    strcmp(gui_type_env, type->answer) != 0 ) || !gui_type_env ) {
	G_message(_("<%s> is now the default GUI"), type->answer);
	G_setenv("GRASS_GUI", type->answer);

    }

    if (strcmp(type->answer, "oldtcltk") == 0) {
	if (rc_file->answer) {
	    G_spawn("d.m", "dmrc=%s", rc_file->answer, NULL);
	}
	else {
	    G_system("d.m");
	}
    }
    else if (strcmp(type->answer, "tcltk") == 0) {
	if (rc_file->answer) {
	    G_spawn("gis.m", "dmrc=%s", rc_file->answer, NULL);
	}
	else {
	    G_system("gis.m");
	}
    }
    else if (strcmp(type->answer, "wxpython") == 0) {
	sprintf (progname, "%s/etc/wxpython/scripts/wxgui", G_gisbase());
	if (rc_file->answer) {
	    G_spawn(progname, "wxgui", rc_file->answer, NULL);
	}
	else {
	    G_spawn(progname, "wxgui", NULL);
	}
    }

    exit(EXIT_SUCCESS);
}
