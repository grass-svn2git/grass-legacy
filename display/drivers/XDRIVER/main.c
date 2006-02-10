
#include <stdio.h>
#include "XDRIVER.h"

int main(int argc, char **argv)
{
	struct driver drv;

	drv.Box_abs		= XD_Box_abs;
	drv.Box_rel		= NULL;
	drv.Can_do_float	= XD_Can_do_float;
	drv.Client_Open		= XD_Client_Open;
	drv.Client_Close	= XD_Client_Close;
	drv.Color_table_float	= XD_Color_table_float;
	drv.Color_table_fixed	= XD_Color_table_fixed;
	drv.Erase		= NULL;
	drv.Get_with_box	= XD_Get_location_with_box;
	drv.Get_with_line	= XD_Get_location_with_line;
	drv.Get_with_pointer	= XD_Get_location_with_pointer;
	drv.Graph_set		= XD_Graph_set;
	drv.Graph_close		= XD_Graph_close;
	drv.Line_width		= XD_Line_width;
	drv.Panel_save		= XD_Panel_save;
	drv.Panel_restore	= XD_Panel_restore;
	drv.Panel_delete	= XD_Panel_delete;
	drv.Polydots_abs	= XD_Polydots_abs;
	drv.Polydots_rel	= XD_Polydots_rel;
	drv.Polyline_abs	= XD_Polyline_abs;
	drv.Polyline_rel	= XD_Polyline_rel;
	drv.Polygon_abs		= XD_Polygon_abs;
	drv.Polygon_rel		= XD_Polygon_rel;
	drv.RGB_set_colors	= XD_RGB_set_colors;
	drv.RGB_raster		= XD_RGB_raster;
	drv.Raster_int		= XD_Raster_int;
	drv.Respond		= XD_Respond;
	drv.Work_stream		= XD_Work_stream;
	drv.Do_work		= XD_Do_work;
	drv.reset_color		= XD_reset_color;
	drv.lookup_color	= XD_lookup_color;
	drv.get_table_type	= XD_get_table_type;
	drv.color		= XD_color;
	drv.get_color		= XD_get_color;
	drv.draw_line		= XD_draw_line;
	drv.draw_point		= XD_draw_point;

	return LIB_main(&drv, argc, argv);
}

