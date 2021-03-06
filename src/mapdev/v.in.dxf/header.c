#include "dxf2vect.h"

/* returns a zero if header not found, returns a 1 if found */
dxf_header(dxf_file) 
FILE	*dxf_file;
{
	dxf_fgets (dxf_line, 80, dxf_file); 
	/* Some dxf files will not have header information*/
	while (strcmp (dxf_line, header) != 0 && strcmp(dxf_line,entitie) != 0) 
	{ 
		dxf_fgets (dxf_line, 80, dxf_file); 
		if (feof (dxf_file)) 
		{ 
			fprintf (stderr, "end of file while looking"); 
			fprintf (stderr, " for HEADER\n"); 
			exit (-1); 
		} 
	} 
	if (strcmp (dxf_line, header) == 0)
	    return (1);
	return (0);
}
