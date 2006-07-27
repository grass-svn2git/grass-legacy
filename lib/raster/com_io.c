
#include <grass/config.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <grass/gis.h>
#include <grass/glocale.h>
#include <grass/raster.h>
#include <grass/graphics.h>

#include "driverlib.h"
#include "transport.h"

struct transport loc_trans =
{
	LOC_open_driver,
	LOC__open_quiet,
	LOC_stabilize,
	LOC_kill_driver,
	LOC_close_driver,
	LOC_release_driver,
	LOC_screen_left,
	LOC_screen_rite,
	LOC_screen_bot,
	LOC_screen_top,
	LOC_get_num_colors,
	LOC_color_table_float,
	LOC_color_table_fixed,
	LOC_color_offset,
	LOC_color,
	LOC_standard_color,
	LOC_RGB_color,
	LOC_reset_color,
	LOC_reset_colors,
	LOC_line_width,
	LOC_erase,
	LOC_move_abs,
	LOC_move_rel,
	LOC_cont_abs,
	LOC_cont_rel,
	LOC_polydots_abs,
	LOC_polydots_rel,
	LOC_polyline_abs,
	LOC_polyline_rel,
	LOC_polygon_abs,
	LOC_polygon_rel,
	LOC_box_abs,
	LOC_box_rel,
	LOC_text_size,
	LOC_text_rotation,
	LOC_set_window,
	LOC_text,
	LOC_get_text_box,
	LOC_font,
	LOC_font_freetype,
	LOC_charset,
	LOC_font_freetype_release,
	LOC_panel_save,
	LOC_panel_restore,
	LOC_panel_delete,
	LOC_set_RGB_color,
	LOC_RGB_raster,
	LOC_raster_char,
	LOC_raster_int,
	LOC_get_location_with_box,
	LOC_get_location_with_box_old,
	LOC_get_location_with_line,
	LOC_get_location_with_line_old,
	LOC_get_location_with_pointer,
	LOC_get_location_with_pointer_old,
	LOC_pad_create,
	LOC_pad_current,
	LOC_pad_delete,
	LOC_pad_invent,
	LOC_pad_list,
	LOC_pad_select,
	LOC_pad_append_item,
	LOC_pad_delete_item,
	LOC_pad_get_item,
	LOC_pad_list_items,
	LOC_pad_set_item
};

#ifdef HAVE_SOCKET

struct transport rem_trans =
{
	REM_open_driver,
	REM__open_quiet,
	REM_stabilize,
	REM_kill_driver,
	REM_close_driver,
	REM_release_driver,
	REM_screen_left,
	REM_screen_rite,
	REM_screen_bot,
	REM_screen_top,
	REM_get_num_colors,
	REM_color_table_float,
	REM_color_table_fixed,
	REM_color_offset,
	REM_color,
	REM_standard_color,
	REM_RGB_color,
	REM_reset_color,
	REM_reset_colors,
	REM_line_width,
	REM_erase,
	REM_move_abs,
	REM_move_rel,
	REM_cont_abs,
	REM_cont_rel,
	REM_polydots_abs,
	REM_polydots_rel,
	REM_polyline_abs,
	REM_polyline_rel,
	REM_polygon_abs,
	REM_polygon_rel,
	REM_box_abs,
	REM_box_rel,
	REM_text_size,
	REM_text_rotation,
	REM_set_window,
	REM_text,
	REM_get_text_box,
	REM_font,
	REM_font_freetype,
	REM_charset,
	REM_font_freetype_release,
	REM_panel_save,
	REM_panel_restore,
	REM_panel_delete,
	REM_set_RGB_color,
	REM_RGB_raster,
	REM_raster_char,
	REM_raster_int,
	REM_get_location_with_box,
	REM_get_location_with_box_old,
	REM_get_location_with_line,
	REM_get_location_with_line_old,
	REM_get_location_with_pointer,
	REM_get_location_with_pointer_old,
	REM_pad_create,
	REM_pad_current,
	REM_pad_delete,
	REM_pad_invent,
	REM_pad_list,
	REM_pad_select,
	REM_pad_append_item,
	REM_pad_delete_item,
	REM_pad_get_item,
	REM_pad_list_items,
	REM_pad_set_item
};

#endif

const struct transport *trans;

static void init_transport(void)
{
	const char *p;

	if (trans)
		return;

	p = getenv("GRASS_RENDER_IMMEDIATE");

#ifdef HAVE_SOCKET
	trans = (p && strcmp(p, "TRUE") == 0)
		? &loc_trans
		: &rem_trans;
#else
	trans = &loc_trans;
#endif
}

int R_open_driver(void)
{
	init_transport();
	return trans->open_driver();
}

int R__open_quiet(void)
{
	init_transport();
	return trans->open_quiet();
}

int R_stabilize(void)
{
	return trans->stabilize();
}

int R_kill_driver(void)
{
	return trans->kill_driver();
}

int R_close_driver(void)
{
	return trans->close_driver();
}

int R_release_driver(void)
{
	return trans->release_driver();
}

