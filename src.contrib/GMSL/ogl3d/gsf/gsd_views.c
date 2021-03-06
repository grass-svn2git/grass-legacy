
/*  gsd_views.c
    Bill Brown, USACERL  
    January 1993
*/
	
#include "gstypes.h"
#include "math.h"

/*
#define TRACE_DFUNCS
*/
/* DEBUG */
#include <stdio.h>

#ifdef USE_OGL
#include <GL/gl.h>

#endif

extern float GS_global_exag();

/************************************************************************/
gsd_get_los(vect, sx, sy)
float vect[2][3];
short sx, sy;       /* screen coordinates */
{
static int first;
double fx, fy, fz, tx, ty, tz;

#ifdef USE_OGL
GLdouble modelMatrix[16], projMatrix[16];
GLint viewport[4];
fprintf(stderr,"Made it here!\n");
    GS_ready_draw();
    glPushMatrix();

    gsd_do_scale(1);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glPopMatrix();

	/* OGLXXX XXX I think this is backwards gluProject(XXX); */
	/* WAS: mapw(Vobj, sx, sy, &fx, &fy, &fz, &tx, &ty, &tz); */
    gluUnProject((GLdouble)sx, (GLdouble)sy, 0.0, modelMatrix, 
		projMatrix, viewport, &fx, &fy, &fz);
    gluUnProject((GLdouble)sx, (GLdouble)sy, 1.0, modelMatrix, 
		projMatrix, viewport, &tx, &ty, &tz);

    vect[FROM][X] = fx;
    vect[FROM][Y] = fy;
    vect[FROM][Z] = fz;
    vect[TO][X] = tx;
    vect[TO][Y] = ty;
    vect[TO][Z] = tz;

    /* DEBUG - should just be a dot */
	/* OGLXXX frontbuffer: other possibilities include GL_FRONT_AND_BACK */
    glDrawBuffer((1) ? GL_FRONT : GL_BACK);
    glPushMatrix();
    gsd_do_scale(1);
    gsd_linewidth(3);
    gsd_color_func(0x8888FF);
	/* OGLXXX for multiple, independent line segments: use GL_LINES */
    glBegin(GL_LINE_STRIP);
    glVertex3fv(vect[FROM]);
    glVertex3fv(vect[TO]);
    glEnd();
    gsd_linewidth(1);
    glPopMatrix();
	/* OGLXXX frontbuffer: other possibilities include GL_FRONT_AND_BACK */
    glDrawBuffer((0) ? GL_FRONT : GL_BACK);

    return(1);

#endif

}


/************************************************************************/
/* establishes viewing & projection matrices */

gsd_set_view(gv, gd)
geoview *gv;
geodisplay *gd;
{

/* will expand when need to check for in focus, ortho, etc. */
#ifdef USE_OGL
int twist;
float sx, sy, sz, fromz;
double up[3];

/*
print_view_fields(gv);
*/

    gsd_check_focus(gv);
    /*
    twist = gsd_zup_twist(gv);
    */
    gsd_get_zup(gv, up);

    gd->aspect = GS_get_aspect();

    {GLint mm;glGetIntegerv(GL_MATRIX_MODE, &mm);glMatrixMode(GL_PROJECTION);glLoadIdentity();gluPerspective(.1*(gv->fov),  gd->aspect,  gd->nearclip,  gd->farclip);glMatrixMode(mm);};
    glLoadMatrixf(ID_matrix);

	/* OGLXXX lookat: replace UPx with vector */
    gluLookAt(gv->from_to[FROM][X], gv->from_to[FROM][Y], gv->from_to[FROM][Z],
	      gv->from_to[TO][X], gv->from_to[TO][Y], gv->from_to[TO][Z], 
	      up[X], up[Y], up[Z]);
#endif

    /* have to redefine clipping planes when view changes */
    gsd_update_cplanes();

}

/************************************************************************/
gsd_check_focus(gv)
geoview *gv;
{
float zmax, zmin;

    GS_get_zrange(&zmin, &zmax, 0);

    if(gv->infocus){
	GS_v3eq(gv->from_to[TO], gv->real_to);
	gv->from_to[TO][Z] -= zmin;
	GS_v3mult(gv->from_to[TO], gv->scale);
	gv->from_to[TO][Z] *= gv->vert_exag;

	GS_v3normalize(gv->from_to[FROM], gv->from_to[TO]);
    }

}

/************************************************************************/
gsd_get_zup(gv, up)
geoview *gv;
double *up;
{
float alpha;
float zup[3], zupmag, fup[3];


    /* neg alpha OK since sin(-x) = -sin(x) */
    alpha = (2.0 * atan(1.0)) - acos(gv->from_to[FROM][Z] - gv->from_to[TO][Z]);

    zup[X] = gv->from_to[TO][X];
    zup[Y] = gv->from_to[TO][Y];

    if(sin(alpha))
	zup[Z] = gv->from_to[TO][Z] + 1 / sin(alpha);
    else
	zup[Z] = gv->from_to[FROM][Z] + 1.0;

    GS_v3dir(gv->from_to[FROM], zup, fup);

    up[X] = fup[X];
    up[Y] = fup[Y];
    up[Z] = fup[Z];

}

/************************************************************************/
gsd_zup_twist(gv)
geoview *gv;
{

    float fr_to[2][4];
    float look_theta, pi;
    float alpha, beta;
    float zup[3], yup[3], zupmag, yupmag;

    pi = 4.0 * atan(1.0);

    /* *************************************************************** */
    /* This block of code is used to keep pos z in the up direction,
     * correcting for SGI system default which is pos y in the up
     * direction.  Involves finding up vectors for both y up and
     * z up, then determining angle between them.  LatLon mode uses y as
     * up direction instead of z, so no correction necessary.  Next rewrite,
     * we should use y as up for all drawing.
    */
    GS_v3eq(fr_to[FROM], gv->from_to[FROM]);
    GS_v3eq(fr_to[TO], gv->from_to[TO]);

    /* neg alpha OK since sin(-x) = -sin(x) */
    alpha = pi/2.0 - acos(fr_to[FROM][Z] - fr_to[TO][Z]);

    zup[X] = fr_to[TO][X];
    zup[Y] = fr_to[TO][Y];

    if(sin(alpha))
	zup[Z] = fr_to[TO][Z] + 1 / sin(alpha);
    else
	zup[Z] = fr_to[FROM][Z] + 1.0;

    zupmag = GS_distance(fr_to[FROM],zup);

    yup[X] = fr_to[TO][X];
    yup[Z] = fr_to[TO][Z];

    /* neg beta OK since sin(-x) = -sin(x) */
    beta = pi/2.0 - acos(fr_to[TO][Y] - fr_to[FROM][Y]);
    if(sin(beta)) yup[Y] = fr_to[TO][Y] - 1 / sin(beta);
    else yup[Y] = fr_to[FROM][Y] + 1.0;

    yupmag = GS_distance(fr_to[FROM],yup);

    look_theta = (1800.0/pi) * 
		acos(((zup[X]-fr_to[FROM][X])*(yup[X]-fr_to[FROM][X])
		+ (zup[Y]-fr_to[FROM][Y])*(yup[Y]-fr_to[FROM][Y]) 
		+ (zup[Z]-fr_to[FROM][Z])*(yup[Z]-fr_to[FROM][Z]))/
		(zupmag * yupmag));

    if(fr_to[TO][X] - fr_to[FROM][X] < 0.0) look_theta = -look_theta;
    if(fr_to[TO][Z] - fr_to[FROM][Z] < 0.0){ /* looking down */
	if(fr_to[TO][Y] - fr_to[FROM][Y] < 0.0)
	    look_theta = 1800 - look_theta;
    }
    else{  /* looking up */
	if(fr_to[TO][Y] - fr_to[FROM][Y] > 0.0) 
	    look_theta = 1800 - look_theta;
    }


    return((int)(gv->twist + 1800 + look_theta));

}

/************************************************************************/

gsd_do_scale(doexag)
int doexag;
{
float sx, sy, sz;
float min, max;
    
    GS_get_scale(&sx, &sy, &sz, doexag);    
    gsd_scale(sx, sy, sz);
    GS_get_zrange(&min, &max, 0);
    gsd_translate(0.0,0.0,-min);
}

/************************************************************************/

gsd_real2model(point)
Point3 point;
{
float sx, sy, sz;
float min, max, n, s, w, e;
    
    GS_get_region(&n, &s, &w, &e);
    GS_get_scale(&sx, &sy, &sz, 1);
    GS_get_zrange(&min, &max, 0);
    point[X] = (point[X] - w) * sx;
    point[Y] = (point[Y] - s) * sy;
    point[Z] = (point[Z] - min) * sz ;

}
    
/************************************************************************/

gsd_model2real(point)
Point3 point;
{
float sx, sy, sz;
float min, max, n, s, w, e;
    
    GS_get_region(&n, &s, &w, &e);
    GS_get_scale(&sx, &sy, &sz, 1);
    GS_get_zrange(&min, &max, 0);
    point[X] = (sx? point[X]/sx: 0.0) + w;
    point[Y] = (sy? point[Y]/sy: 0.0) + s;
    point[Z] = (sz? point[Z]/sz: 0.0) + min;

}
    
/************************************************************************/

gsd_model2surf(gs, point)
geosurf *gs;
Point3 point;
{
float min, max, sx, sy, sz;

    /* so far, only one geographic "region" allowed, so origin of
    surface is same as origin of model space, but will need to provide 
    translations here to make up the difference, so not using gs yet */
    
    if(gs){
	/* need to undo z scaling & translate */
	GS_get_scale(&sx, &sy, &sz, 1);
	GS_get_zrange(&min, &max, 0);

	point[Z] = (sz? point[Z]/sz: 0.0) + min;

	/* need to unscale x & y */
	point[X] = (sx? point[X]/sx: 0.0);
	point[Y] = (sy? point[Y]/sy: 0.0);
    }


}

/************************************************************************/

gsd_surf2real(gs, point)
geosurf *gs;
Point3 point;
{

    if(gs){
	point[X] += gs->ox;
	point[Y] += gs->oy;
    }

}

/************************************************************************/

gsd_real2surf(gs, point)
geosurf *gs;
Point3 point;
{

    if(gs){
	point[X] -= gs->ox;
	point[Y] -= gs->oy;
    }

}

/************************************************************************/


