/*
 *************************************************************
 * char * G_mask_info ()
 *
 *   returns a printable text of mask information
 *
 ************************************************************
 * G__mask_info (name, mapset)
 *
 *      char name[50], mapset[50];
 *
 * function:
 *   determine the status off the automatic masking
 *   and the name of the cell file which forms the mask
 *
 *   (the mask file is actually MASK in the current mapset,
 *   but is usually a reclassed cell file, and the reclass
 *   name and mapset are returned)
 *
 * returns:
 *   -1   no masking (name, mapset undefined)
 *        name, mapset are undefined
 *
 *    1   mask file present, masking on
 *        name, mapset hold mask file name, mapset
 *
 ***************************************************************/ 

#include "gis.h"
char *
G_mask_info ()
{
    static char text[200];
    char name[50];
    char mapset[50];

    switch (G__mask_info (name, mapset))
    {
    case 1:
	    sprintf (text, "<%s> in mapset <%s>", name, mapset);
	    break;
    case -1:
	    strcpy (text, "none");
	    break;
    default:
	    strcpy (text, "not known");
	    break;
    }

    return text;
}

G__mask_info (name, mapset)
    char *name;
    char *mapset;
{
    char rname[50], rmapset[50];

    strcpy (name, "MASK");
    strcpy (mapset, G_mapset());

    if(!G_find_cell (name, mapset))
	return -1;

    if(G_is_reclass (name, mapset, rname, rmapset) > 0)
    {
	strcpy (name, rname);
	strcpy (mapset, rmapset);
    }

    return 1;
}
