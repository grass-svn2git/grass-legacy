/* Function: vectfile
**
** This PostScript version is just slightly modified p.map code.
**
** Modified by: Paul W. Carlson		March 1992
** Modified by: Janne Soimasuo August 1994 line_cat added
** Modified by: Radim Blazek Jan 2000 acolor, label added
*/
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "Vect.h"
#include "ps_info.h"
#include "local_proto.h"

#define KEY(x) (strcmp(key,x)==0)

static char *help[]=
{
    "color    color",
    "width    #",
    "masked   [y|n]",
    "acolor   r g b",
    "label    label",
    "lpos     0|1-20",
    "pat      EPS pattern file",
    "scale    #",
    "pwidth   #",
    ""
};

int 
read_vareas (char *name, char *mapset)
{
    char fullname[100];
    char buf[1024];
    char *key, *data;
    double  width;
    int itmp, vec;
    int r, g, b;
    int ret;
    struct Map_info Map;

    vector_alloc (); /* allocate space */

    sprintf (fullname, "%s in %s", name, mapset);

    Vect_set_open_level(2);
    Vect_set_fatal_error ( GV_FATAL_PRINT );
    if ( 2 > Vect_open_old (&Map, name, mapset)) {
	error(fullname, "", "can't open vector file");
	gobble_input();
	return 0;
    }
    Vect_close(&Map);

    vec = vector.count;
    
    vector.layer[vec].type = VAREAS;
    vector.layer[vec].name   = G_store(name);
    vector.layer[vec].mapset = G_store(mapset);
    vector.layer[vec].masked = 0 ;
    
    vector.layer[vec].field = 1;
    vector.layer[vec].cats = NULL;
    vector.layer[vec].where = NULL;

    vector.layer[vec].width  = 1. ;
    vector.layer[vec].cwidth  = 0. ;    
    vector.layer[vec].offset  = 0. ;
    vector.layer[vec].coffset  = 0. ;            
    set_color ( &(vector.layer[vec].color), 0, 0, 0 );
    set_color ( &(vector.layer[vec].fcolor), 125, 125, 125 );
    vector.layer[vec].linestyle = NULL;
    vector.layer[vec].ref = LINE_REF_CENTER;    
    vector.layer[vec].label = NULL ;    
    vector.layer[vec].lpos = -1 ;
    vector.layer[vec].pat = NULL ;    
    vector.layer[vec].scale = 1. ;    
    vector.layer[vec].pwidth = 1. ;    

    while (input(2, buf, help))
    {
	if (!key_data(buf, &key, &data)) continue;

	if (KEY("masked"))
	{
	    vector.layer[vec].masked = yesno(key,data) ;
	    if (vector.layer[vec].masked) PS.mask_needed = 1;
	    continue;
	}

	if (KEY("field")) 
	{
	    G_strip(data);
	    vector.layer[vec].field = atoi (data);
	    continue;
	}

	if (KEY("cats")) 
	{
	    G_strip(data);
	    vector.layer[vec].cats = G_store(data);
	    continue;
	}

	if (KEY("where")) 
	{
	    G_strip(data);
	    vector.layer[vec].where = G_store(data);
	    continue;
	}

	if (KEY("width"))
	{
	    width = -1.;
	    *mapset = 0;
	    if (sscanf(data, "%lf%s", &width, mapset) < 1 || width < 0.)
	    {
		width = 1.;
		error(key, data, "illegal width");
		continue;
	    }
	    if(mapset[0] == 'i') width = width/72.;
	    vector.layer[vec].width = width;
	    continue;
	}
	
	if (KEY("color"))
	{
	    ret = G_str_to_color( data, &r, &g, &b);
	    if ( ret == 1 )
	        set_color ( &(vector.layer[vec].color), r, g, b );
	    else if ( ret == 2 )
		unset_color ( &(vector.layer[vec].color));
	    else
		error (key,data,"illegal color request");

	    continue;
	}

        if (KEY("fcolor")) /* area color */
        {
	    ret = G_str_to_color( data, &r, &g, &b);
	    if ( ret == 1 )
	        set_color ( &(vector.layer[vec].fcolor), r, g, b );
	    else if ( ret == 2 )
		unset_color ( &(vector.layer[vec].fcolor));
	    else
		error (key,data,"illegal color request");

	    continue;
        } 

	if (KEY("label")) /* map legend label */
	{
	    G_strip(data);
	    vector.layer[vec].label = G_store(data);
	    continue;
	}

        if (KEY("lpos"))
        {
            if (sscanf(data, "%d", &itmp) < 1 || itmp < 0 )
            {
                itmp = -1;
                error(key, data, "illegal lpos");
                continue;
            }
            vector.layer[vec].lpos = itmp;
	    continue;
        }
	
	if (KEY("pat"))
	{
	    G_chop(data); 
	    vector.layer[vec].pat = G_store(data);
	    continue;
	}
	
	if (KEY("scale"))
	{
	    G_chop(data); 
	    vector.layer[vec].scale = atof(data);
	    continue;
	}

	if (KEY("pwidth"))
	{
	    width = -1.;
	    if (sscanf(data, "%lf%s", &width, mapset) < 1 || width < 0.)
	    {
		width = 0.;
		error(key, data, "illegal pwidth");
		continue;
	    }
	    if(mapset[0] == 'i') width = width/72.;
	    vector.layer[vec].pwidth = width;
	    continue;
	}

	error(key, "", "illegal request");
    }

    vector.count++;
    return 1;
}
