#include "gis.h"
#include "ncb.h"

null_cats ()
{
    int ncats;

    ncats = G_number_of_cats (ncb.newcell.name, ncb.newcell.mapset);
    G_init_cats (ncats, ncb.title, &ncb.cats);
}
