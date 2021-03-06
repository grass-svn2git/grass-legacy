#include "gis.h"

#define MAX_COLORS 1024
#define DEVIATION 128

G_make_random_colors (colors,min,max)
    struct Colors *colors ;
    CELL min,max;
{
    unsigned char red, grn, blu;
    int count;
    CELL n;

    G_init_colors (colors);
    if (min > max) return -1;

    srand(time ((long *)0));

    count = MAX_COLORS-DEVIATION + rand() % DEVIATION;
    if (count > max-min+1)
	count = max-min+1;

    for (n = 1; n <= count; n++)
    {
	red = rand() & 0377;
	grn = rand() & 0377;
	blu = rand() & 0377;
	G_add_modular_color_rule (n, red, grn, blu, n, red, grn, blu, colors);
    }
    G_set_color_range (min, max, colors);

    return 1;
}
