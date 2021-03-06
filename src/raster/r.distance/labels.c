#include "defs.h"

void
read_labels (map)
    struct Map *map;
{
    if (G_read_cats (map->name, map->mapset, &map->labels) < 0)
	exit(1);
}

char *
get_label (map, cat)
    struct Map *map;
    CELL cat;
{
    char *G_get_cat();

    return G_get_cat (cat, &map->labels);
}
