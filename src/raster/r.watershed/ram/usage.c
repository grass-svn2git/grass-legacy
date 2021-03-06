#include <stdio.h>

usage (me)
char	*me;
{
    char buf[1200];

    sprintf (buf, "USAGE for basin delineation:\n%s -4 el=elevation_map t=swale_threshold [ov=overland_flow_map] [dr=drain_direction_map] [de=depression_map] [ac=accumulation_map] [di=display_map] ba=watershed_basin_map [se=stream_segment_map]\n\nUSAGE for ARMSED FILE creation:\n%s [-4] el=elevation_map t=swale_threshold [ov=overland_flow_map] [dr=drain_direction_map] [de=depression_map] [ac=accumulation_map] [di=display_map] [ba=watershed_basin_map] [se=stream_segment_map] ha=half_basin_map ar=ARMSED_file_name\n\nUSAGE for slope length determination:\n%s [-4] el=elevation_map t=swale_threshold [dr=drain_direction_map] [de=depression_map] [ac=accumulation_map] [di=display_map] [ms=max_slope_length] [ob=overland_blocking_map] [S=slope_steepness_map] LS=length_slope_map [r=rill_erosion_map] [sd=slope_deposition value or map]", me, me, me);
    /*
    sprintf (buf, "USAGE for basin delineation:\n%s -4 el=elevation_map t=swale_threshold [ov=overland_flow_map] [dr=drain_direction_map] [de=depression_map] [ac=accumulation_map] [di=display_map] ba=watershed_basin_map [se=stream_segment_map]\n\nUSAGE for ARMSED FILE creation:\n%s [-4] el=elevation_map t=swale_threshold [ov=overland_flow_map] [dr=drain_direction_map] [de=depression_map] [ac=accumulation_map] [di=display_map] [ba=watershed_basin_map] [se=stream_segment_map] ha=half_basin_map ar=ARMSED_file_name\n", me, me);
    */
    G_fatal_error (buf);
    exit (1);
}
