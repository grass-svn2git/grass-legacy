/**
 * \file color_get.c
 *
 * \brief Functions to get colors from a raster map.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * \author GRASS GIS Development Team
 *
 * \date 1999-2006
 */

#include <grass/gis.h>


/**
 * \fn int G_get_color (CELL n, int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief Get a category color.
 *
 * The <b>red, green</b>, and
 * <b>blue</b> intensities for the color associated with category <b>n</b>
 * are extracted from the <b>colors</b> structure. The intensities will be in
 * the range 0 �- 255.  Also works for null cells.
 *
 *  \param[in] n CELL to get color from
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return int
 */

int G_get_color (CELL n, int *red, int *grn, int *blu, struct Colors *colors)
{
    CELL cat;
    unsigned char r, g, b, set;

    cat = n;
    G_lookup_colors (&cat, &r, &g, &b, &set, 1, colors);

    *red = (int) r;
    *grn = (int) g;
    *blu = (int) b;

    return (int)set;
}


/**
 * \fn int G_get_raster_color (void *rast, int *red, int *grn, int *blu, struct Colors *colors, RASTER_MAP_TYPE map_type)
 *
 * \brief Gets color from raster.
 *
 * Looks up the rgb colors for
 * <em>rast</em> in the color table <em>colors</em>
 *
 *  \param[in] rast input raster map
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \param[in] map_type type of map (CELL_TYPE,FCELL_TYPE,DCELL_TYPE)
 *  \return int
 */

int G_get_raster_color (void *rast,
    int *red, int *grn, int *blu,
    struct Colors *colors, RASTER_MAP_TYPE map_type)
{
    unsigned char r, g, b, set;

    G_lookup_raster_colors (rast, &r, &g, &b, &set, 1, colors, map_type);

    *red = (int) r;
    *grn = (int) g;
    *blu = (int) b;

    return (int)set;
}


/**
 * \fn int G_get_c_raster_color (CELL *rast, int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief Gets color for a CELL raster.
 *
 * Looks up the rgb colors for CELL
 * <em>rast</em> in the color table <em>colors</em>
 *
 *  \param[in] rast input CELL raster
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return int
 */

int G_get_c_raster_color (CELL *rast,
    int *red, int *grn, int *blu,
    struct Colors *colors)
{
    return G_get_raster_color(rast, red, grn, blu, colors, CELL_TYPE);
}


/**
 * \fn int G_get_f_raster_color (FCELL *rast, int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief Gets color for a FCELL raster.
 *
 *  Looks up the rgb colors for FCELL <em>rast</em> in the color table
 * <em>colors</em>
 *
 *  \param[in] rast input FCELL raster
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return int
 */

int G_get_f_raster_color (FCELL *rast,
    int *red, int *grn, int *blu,
    struct Colors *colors)
{
    return G_get_raster_color(rast, red, grn, blu, colors, FCELL_TYPE);
}


/**
 * \fn int G_get_d_raster_color (DCELL *rast, int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief Gets color for a DCELL raster.
 *
 *  Looks up the rgb colors for DCELL <em>rast</em> in the color table
 * <em>colors</em>
 *
 *  \param[in] rast input DCELL raster
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return int
 */

int G_get_d_raster_color (DCELL *rast,
    int *red, int *grn, int *blu,
    struct Colors *colors)
{
    return G_get_raster_color(rast, red, grn, blu, colors, DCELL_TYPE);
}


/**
 * \fn int G_get_null_value_color (int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief  Gets color for null value.
 *
 * Puts the red, green, and blue components of <b>colors</b> for the
 * NULL-value into <b>red, grn, and blu</b>.
 *
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return always returns 0
 */

int G_get_null_value_color (int *red, int *grn, int *blu,
    struct Colors *colors)
{
  if(colors->null_set)
  {
      *red = (int) colors->null_red;
      *grn = (int) colors->null_grn;
      *blu = (int) colors->null_blu;
  }
  else if(colors->undef_set)
  {
      *red = (int) colors->undef_red;
      *grn = (int) colors->undef_grn;
      *blu = (int) colors->undef_blu;
  }
  else
      *red = *blu = *grn = 255; /* white */

  return 0;
}


/**
 * \fn int G_get_default_color (int *red, int *grn, int *blu, struct Colors *colors)
 *
 * \brief Gets default color.
 *
 *  Puts the red, green, and blue components of the
 * <tt>"default"</tt> color into <b>red, grn, and blu</b>.
 *
 *  \param[out] red red value
 *  \param[out] grn green value
 *  \param[out] blu blue value
 *  \param[in] colors Colors struct
 *  \return always returns 0
 */

int G_get_default_color (int *red, int *grn, int *blu,
    struct Colors *colors)
{
  if(colors->undef_set)
  {
      *red = (int) colors->undef_red;
      *grn = (int) colors->undef_grn;
      *blu = (int) colors->undef_blu;
  }
  else
      *red = *blu = *grn = 255; /* white */

  return 0;
}
