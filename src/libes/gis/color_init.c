/**********************************************************************
 *
 * G_init_colors (colors)
 *      struct Colors *colors         structure to hold color info
 *
 * Initializes the color structure for subsequent calls to G_add_color_rule()
 *********************************************************************/

#include "gis.h"

G_init_colors (colors)
    struct Colors *colors;
{
    colors->version = 0;
    colors->shift = 0;
    colors->invert = 0;
    colors->cmin = 0;
    colors->cmax = -1;
    colors->fixed.min = 0;
    colors->fixed.max = -1;
    colors->fixed.rules = NULL;
    colors->fixed.lookup.active = 0;
    colors->modular.min = 0;
    colors->modular.max = -1;
    colors->modular.rules = NULL;
    colors->modular.lookup.active = 0;
}
