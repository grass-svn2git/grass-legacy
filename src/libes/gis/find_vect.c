/*
 **********************************************************************
 *  char *
 *  G_find_vector (name, mapset)
 *        char *name       file name to look for
 *        char *mapset     mapset to search. if mapset is ""
 *                         will search in mapset search list
 *
 *	searches for a vector file from the mapset search list
 *      or in a specified mapset.
 *	returns the mapset name where the vector file was found.
 *
 *  returns:
 *      char *  pointer to a string with name of mapset
 *              where vector file was found, or NULL if not found
 *  note:
 *      rejects all names that begin with .
 *
 *      if name is of the form nnn in ppp then 
 *      name = nnn and mapset = ppp
 **********************************************************************/

#include "gis.h"

char *
G_find_vector (name, mapset)
	char *name;
	char *mapset;
{
	return G_find_file ("dig", name, mapset);
}

char *
G_find_vector2 (name, mapset)
	char *name;
	char *mapset;
{
	return G_find_file2 ("dig", name, mapset);
}
