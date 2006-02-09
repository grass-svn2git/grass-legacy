#include <stdlib.h>
#include <unistd.h>
#include <grass/imagery.h>
#include <grass/glocale.h>
#include "bouman.h"


int main (int argc, char *argv[])
{
    struct parms parms; /* command line parms */
    struct files files; /* file descriptors, io, buffers */
    struct SigSet S;
	struct GModule *module;

    G_gisinit (argv[0]);

	module = G_define_module();
	module->description =
		_("Performs contextual image classification "
		"using sequential maximum a posteriori (SMAP) estimation.");

    parse (argc,argv, &parms);
    openfiles (&parms, &files);
    read_signatures (&parms, &S);
    create_output_labels (&S, &files);

    segment (&S, &parms, &files);

    closefiles(&parms, &files);
    exit(0);
}
