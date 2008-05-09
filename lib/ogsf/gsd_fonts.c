/*!
  \file gsd_fonts.c
 
  \brief OGSF library - loading and manipulating surfaces
 
  GRASS OpenGL gsurf OGSF Library 
 
  \todo This file needs to be re-written in OpenGL

  (C) 1999-2008 by the GRASS Development Team
 
  This program is free software under the 
  GNU General Public License (>=v2). 
  Read the file COPYING that comes with GRASS
  for details.
  
  \author Bill Brown USACERL, GMSL/University of Illinois
*/

#include <string.h>
#include <assert.h>
#include <grass/gstypes.h>
#include <grass/ogsf_proto.h>
#include "rgbpack.h"



/****************************************/
int gsd_get_txtwidth(char *s, int size)
{
    int width, len;

    len = strlen(s);
    width = (size * len) / 2;

    return (width);
}


/*****************************************/
int gsd_get_txtheight(int size)
{
    unsigned long height;

    height = size / 2;

    return (height);

}


/*****************************************/
int get_txtdescender()
{

/* yorig ?? 
 * Is this defined somewhere ?
 */

    return (2);
}

/*****************************************/
int get_txtxoffset()
{

/* xorig ??
 * Is this defined somewhere ?
 */

    return (0);
}

/*****************************************/
void do_label_display(GLuint fontbase, float *lab_pos, char *txt)
{
    glRasterPos2f(lab_pos[X], lab_pos[Y]);
    glListBase(fontbase);
    glCallLists(strlen(txt), GL_BYTE, (GLubyte *) txt);

    return;
}
