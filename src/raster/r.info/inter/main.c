#include <unistd.h>
#include <stdlib.h>
#include "gis.h"
#include "local_proto.h"

int main (int argc, char *argv[])
{
    char *mapset;
    char name[100];
    char *tempfile;
    char command[512];

    G_gisinit(argv[0]);

    mapset = G_ask_cell_old ("", name);
    if (mapset == NULL)
	    exit(0);

    tempfile = G_tempfile () ;
    unlink (tempfile);

    sprintf (command, "r.info '%s' > '%s'",
	G_fully_qualified_name(name, mapset), tempfile);
    system (command);

    more_print(tempfile);
    unlink (tempfile);
    return (0);
}
