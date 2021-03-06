
/*  gsd_prim.c
    Bill Brown, USACERL  
    January 1993
    Primitive drawing functions
*/

#include "gstypes.h"

#ifdef USE_OGL	
#include "GL/gl.h"
#include "GL/glu.h"
#endif

/* DEBUG */
#include <stdio.h>

static int Shade;
extern int W_width;
extern int W_height;
/************************************************************************/
gsd_colormode(cm)
int cm;
{

#ifdef USE_OGL
    switch(cm){
	case CM_COLOR:
	/* OGLXXX
	 * lmcolor: if LMC_NULL,  use:
	 * glDisable(GL_COLOR_MATERIAL);
	 * LMC_COLOR: use glDisable(GL_COLOR_MATERIAL);
	 */
	    glDisable(GL_COLOR_MATERIAL);
	    glDisable(GL_LIGHTING);
	    break;
	case CM_EMISSION:
	/* OGLXXX
	 * lmcolor: if LMC_NULL,  use:
	 * glDisable(GL_COLOR_MATERIAL);
	 */
	    glEnable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
	    break;
	case CM_DIFFUSE:
	/* OGLXXX
	 * lmcolor: if LMC_NULL,  use:
	 * glDisable(GL_COLOR_MATERIAL);
	 */
/*
	    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
*/
	    glEnable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    glColorMaterial(GL_FRONT, GL_DIFFUSE);
	    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	    break;
	case CM_AD:
	/* OGLXXX
	 * lmcolor: if LMC_NULL,  use:
	 * glDisable(GL_COLOR_MATERIAL);
	 */
	    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	    glEnable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    break;
	case CM_NULL:
	/* OGLXXX
	 * lmcolor: if LMC_NULL,  use:
	 * glDisable(GL_COLOR_MATERIAL);
	 * LMC_NULL: use glDisable(GL_COLOR_MATERIAL);
	 */
	    glDisable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    break;
	default:
	    glDisable(GL_COLOR_MATERIAL);
	    break;
    }
#endif

}

/************************************************************************/

gsd_circ(x, y, rad)
float x, y, rad;
{
#ifdef USE_OGL
	/* OGLXXX See gluDisk man page. */
    { GLUquadricObj *qobj = gluNewQuadric(); gluQuadricDrawStyle(qobj, GLU_SILHOUETTE); glPushMatrix(); glTranslatef(x,  y, 0.); gluDisk( qobj, 0.,  rad, 32, 1); glPopMatrix(); gluDeleteQuadric(qobj); };
#endif
}


/************************************************************************/
gsd_sphere(center, siz)
float center[3], siz;
{
#ifndef USE_OGL
float params[4];
static int first=1;

    if(first){
        /* set sphere mode */
        sphmode(SPH_ORIENT, FALSE);
        first = 0;
    }
    
    params[0]=center[0];
    params[1]=center[1];
    params[2]=center[2];
    params[3]=siz;
    sphdraw(params);
#else
static int first=1;
static GLUquadricObj *QOsphere;

    if(first){
	QOsphere = gluNewQuadric();
	if(QOsphere){
	    gluQuadricNormals(QOsphere, GLU_SMOOTH); /* default */
	    gluQuadricTexture(QOsphere, GL_FALSE); /* default */
	    gluQuadricOrientation(QOsphere, GLU_OUTSIDE); /* default */
	    gluQuadricDrawStyle(QOsphere, GLU_FILL); 
	}
        first = 0;
    }
    glPushMatrix();
    glTranslatef(center[0],  center[1],  center[2]);
    gluSphere(QOsphere, (double)siz, 24, 24);
    glPopMatrix();
#endif
}
/************************************************************************/
gsd_zwritemask(n)
unsigned long n;
{
#ifdef USE_OGL
	/* OGLXXX glDepthMask is boolean only */
    glDepthMask((GLboolean)(n));
#endif
}

/************************************************************************/
gsd_backface(n)
int n;
{
#ifdef USE_OGL
    glCullFace(!(n) ? GL_FRONT : GL_BACK); (n) ? glEnable(GL_CULL_FACE):glDisable(GL_CULL_FACE);
#endif
}

/************************************************************************/
gsd_frontface(n)
int n;
{
#ifdef USE_OGL
    glCullFace((n) ? GL_FRONT : GL_BACK); (n) ? glEnable(GL_CULL_FACE):glDisable(GL_CULL_FACE);
#endif
}

/************************************************************************/
gsd_linewidth(n)
short n;
{
#ifdef USE_OGL
    glLineWidth((GLfloat)(n));
#endif
}

/************************************************************************/
gsd_bgntmesh()
{
#ifdef USE_OGL
    glBegin(GL_TRIANGLE_STRIP);
/*
    glBegin(GL_TRIANGLE_FAN);
*/
#endif
}

/************************************************************************/
gsd_endtmesh()
{
#ifdef USE_OGL
    glEnd();
#endif
}

/************************************************************************/
gsd_swaptmesh()
{
#ifdef USE_OGL
	/* OGLXXX
	 * swaptmesh not supported, maybe glBegin(GL_TRIANGLE_FAN)
	 * swaptmesh()
	 */
    /*DELETED*/;
#endif
}

/************************************************************************/
gsd_bgnpolygon()
{
#ifdef USE_OGL
	/* OGLXXX
	 * special cases for polygons:
	 * 	independant quads: use GL_QUADS
	 * 	independent triangles: use GL_TRIANGLES
	 */
    glBegin(GL_POLYGON);
#endif
}

/************************************************************************/
gsd_endpolygon()
{
#ifdef USE_OGL
    glEnd();
#endif
}

/************************************************************************/
gsd_bgnline()
{
#ifdef USE_OGL
	/* OGLXXX for multiple, independent line segments: use GL_LINES */
    glBegin(GL_LINE_STRIP);
#endif
}

/************************************************************************/
gsd_endline()
{
#ifdef USE_OGL
    glEnd();
#endif
}

/************************************************************************/
gsd_shademodel(bool)
int bool;
{
    Shade = bool;
#ifdef USE_OGL
    if(bool)
	glShadeModel(GL_SMOOTH);
    else
	glShadeModel(GL_FLAT);
#endif
}

/************************************************************************/
gsd_getshademodel()
{
    return(Shade);
}

/************************************************************************/
gsd_bothbuffer()
{
#ifdef USE_OGL
	/* OGLXXX frontbuffer: other possibilities include GL_FRONT_AND_BACK */
    glDrawBuffer(GL_FRONT_AND_BACK);
#endif
}

/************************************************************************/
gsd_frontbuffer(bool)
int bool;
{
#ifdef USE_OGL
	/* OGLXXX frontbuffer: other possibilities include GL_FRONT_AND_BACK */
    glDrawBuffer((bool) ? GL_FRONT : GL_BACK);
#endif
}

/************************************************************************/
gsd_backbuffer(bool)
int bool;
{
#ifdef USE_OGL
	/* OGLXXX backbuffer: other possibilities include GL_FRONT_AND_BACK */
    glDrawBuffer((bool) ? GL_BACK : GL_FRONT);
#endif
}

/************************************************************************/
gsd_swapbuffers()
{
#ifdef USE_OGL
    /* OGLXXX swapbuffers: 
    glXSwapBuffers(*display, window);
    replace display and window */

    Swap_func();
#endif
}

/************************************************************************/
gsd_popmatrix()
{
#ifdef USE_OGL
    glPopMatrix();
#endif
}

/************************************************************************/
gsd_pushmatrix()
{
#ifdef USE_OGL
    glPushMatrix();
#endif
}

/************************************************************************/
gsd_scale(xs,ys,zs)
float xs, ys, zs;
{
#ifdef USE_OGL
    glScalef(xs,  ys,  zs);
#endif
}

/************************************************************************/
gsd_translate(dx,dy,dz)
float dx, dy, dz;
{
#ifdef USE_OGL
    glTranslatef(dx,  dy,  dz);
#endif
}

/************************************************************************/
gsd_rot(angle, axis)
float angle;
char axis;
{
#ifdef USE_OGL
	/* OGLXXX You can do better than this. */
    glRotatef(angle, ( axis)=='x'||( axis)=='X', ( axis)=='y'||( axis)=='Y', ( axis)=='z'||( axis)=='Z');
#endif
}

/************************************************************************/
gsd_litvert_func(norm, col, pt)
float norm[3];
unsigned long col;
float pt[3];
{
#ifdef USE_OGL
    glNormal3fv(norm);
    gsd_color_func(col);
    glVertex3fv(pt);
#endif
}

/************************************************************************/
gsd_vert_func(pt)
float pt[3];
{
#ifdef USE_OGL
    glVertex3fv(pt);
#endif
}

/************************************************************************/


#define RED_MASK 0x000000FF
#define GRN_MASK 0x0000FF00
#define BLU_MASK 0x00FF0000
#define ALP_MASK 0xFF000000

#define INT_TO_RED(i, r)    (r = (i & RED_MASK))
#define INT_TO_GRN(i, g)    (g = (i & GRN_MASK) >> 8)
#define INT_TO_BLU(i, b)    (b = (i & BLU_MASK) >> 16)
#define INT_TO_ALP(i, a)    (a = (i & ALP_MASK) >> 24)

gsd_color_func(col)
unsigned col;
{
#ifdef USE_OGL
GLbyte r, g, b, a;

	/* OGLXXX
	 * cpack: if argument is not a variable
	 * might need to be:
	 * 	glColor4b(($1)&0xff, ($1)>>8&0xff, ($1)>>16&0xff, ($1)>>24&0xff)
	 */
    INT_TO_RED(col, r);
    INT_TO_GRN(col, g);
    INT_TO_BLU(col, b);
    INT_TO_ALP(col, a);
    glColor4ub(r, g, b, a);

#endif
}

#ifdef USE_OGL
static float ogl_light_amb[MAX_LIGHTS][4];
static float ogl_light_diff[MAX_LIGHTS][4];
static float ogl_light_spec[MAX_LIGHTS][4];
static float ogl_light_pos[MAX_LIGHTS][4];
static float ogl_mat_amb[4];
static float ogl_mat_diff[4];
static float ogl_mat_spec[4];
static float ogl_mat_emis[4];
static float ogl_mat_shin;
static float ogl_lmodel[4];

static GLuint LightList[MAX_LIGHTS];
#endif
/************************************************************************/
gsd_init_lightmodel()
{
#ifdef USE_OGL
    
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE); /* normal vector renormalization */
	/* OGLXXX
	 * Ambient:
	 * 	If this is a light model lmdef, then use glLightModelf and GL_LIGHT_MODEL_AMBIENT.
	 * Include ALPHA parameter with ambient
	 */
    ogl_mat_amb[0] = 0.1;
    ogl_mat_amb[1] = 0.1;
    ogl_mat_amb[2] = 0.1;
    ogl_mat_amb[3] = 0.5;

    ogl_mat_diff[0] = 0.8;
    ogl_mat_diff[1] = 0.8;
    ogl_mat_diff[2] = 0.8;
    ogl_mat_diff[3] = 0.8;

    ogl_mat_spec[0] = 0.45;
    ogl_mat_spec[1] = 0.45;
    ogl_mat_spec[2] = 0.45;
    ogl_mat_spec[3] = 0.45;

    ogl_mat_emis[0] = 0.0;
    ogl_mat_emis[1] = 0.0;
    ogl_mat_emis[2] = 0.0;
    ogl_mat_emis[3] = 0.0;

    ogl_mat_shin = 25.0;

	/* OGLXXX
	 * attenuation: see glLightf man page:
	 * Add GL_LINEAR_ATTENUATION.
    sgi_lmodel[0] = GL_CONSTANT_ATTENUATION;
    sgi_lmodel[1] = 1.0;
    sgi_lmodel[2] = 0.0;
    sgi_lmodel[3] = ;
	 */
	/* OGLXXX
	 * lmdef other possibilities include:
	 * 	glLightf(light, pname, *params);
	 * 	glLightModelf(pname, param);
	 * Check list numbering.
	 * Translate params as needed.
	 */

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ogl_mat_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ogl_mat_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ogl_mat_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ogl_mat_emis);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ogl_mat_shin);

	/* OGLXXX lmbind: check object numbering. */
	/* OGLXXX
	 * lmbind: check object numbering.
	 * Use GL_FRONT in call to glMaterialf.
	 * Use GL_FRONT in call to glMaterialf.
    if(1) {glCallList(1); glEnable(LMODEL);} else glDisable(LMODEL);
    if(1) {glCallList(1); glEnable(GL_FRONT);} else glDisable(GL_FRONT);
	 */

#endif
}

/************************************************************************/
gsd_set_material(set_shin, set_emis, sh, em, emcolor)
int set_shin, set_emis;  /* flags */
float sh, em; /* sh & em should be 0. - 1. */
int emcolor;  /* packed colors to use for emission */
{
    if(set_shin){
	ogl_mat_spec[0] = sh;
	ogl_mat_spec[1] = sh;
	ogl_mat_spec[2] = sh;
	ogl_mat_spec[3] = sh;

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ogl_mat_spec);

	ogl_mat_shin = 60. + (int)(sh * 68.);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, ogl_mat_shin);
    }

    if(set_emis){
	ogl_mat_emis[0] = (em * (emcolor & 0x0000FF))/255.;
	ogl_mat_emis[1] = (em * ((emcolor & 0x00FF00)>>8))/255.;
	ogl_mat_emis[2] = (em * ((emcolor & 0xFF0000)>>16))/255.;

	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ogl_mat_emis);
    }

}

/************************************************************************/
gsd_deflight(num, vals)
int num;
struct lightdefs *vals;
{
#ifdef USE_OGL
    if(num > 0 && num <= MAX_LIGHTS){
	ogl_light_pos[num - 1][0] = vals->position[X];
	ogl_light_pos[num - 1][1] = vals->position[Y];
	ogl_light_pos[num - 1][2] = vals->position[Z];
	ogl_light_pos[num - 1][3] = vals->position[W];

	glLightfv(GL_LIGHT0 + num, GL_POSITION, ogl_light_pos[num-1]);

	ogl_light_diff[num - 1][0] = vals->color[0];
	ogl_light_diff[num - 1][1] = vals->color[1];
	ogl_light_diff[num - 1][2] = vals->color[2];
	ogl_light_diff[num - 1][3] = .3;

	glLightfv(GL_LIGHT0 + num, GL_DIFFUSE, ogl_light_diff[num-1]);

	ogl_light_amb[num - 1][0] = vals->ambient[0];
	ogl_light_amb[num - 1][1] = vals->ambient[1];
	ogl_light_amb[num - 1][2] = vals->ambient[2];
	ogl_light_amb[num - 1][3] = .3;

	glLightfv(GL_LIGHT0 + num, GL_AMBIENT, ogl_light_amb[num-1]);

    }
#endif

}

/************************************************************************/
/* on = 0 turns them off */
gsd_switchlight(num, on)
int num, on;
{
#ifdef USE_OGL
short defin;

    defin = on? num: 0 ;  
    if(defin) 
	glEnable(GL_LIGHT0 + num); 
    else 
	glDisable(GL_LIGHT0 + num);
#endif

}

/************************************************************************/
gsd_getimage(pixbuf, xsize, ysize)
unsigned long **pixbuf;
int *xsize, *ysize;
{
#ifdef USE_OGL
GLuint l, r, b, t;

	/* OGLXXX
	 * get GL_VIEWPORT:
	 * You can probably do better than this.
	 */
    
    /*GLint tmp[4];

    glGetIntegerv(GL_VIEWPORT, tmp);
    l=tmp[0];
    r=tmp[0]+tmp[2]-1;
    b=tmp[1];
    t=tmp[1]+tmp[3]-1;
     
    
    *xsize = r - l + 1;
    *ysize = t - b + 1; */
    
    *xsize = W_width;
    *ysize = W_height;

    if(NULL == (*pixbuf = 
	    (unsigned long *)malloc(*xsize * *ysize * sizeof(unsigned long))))
	return (0);

    
    gsd_swapbuffers();    
    
    glReadBuffer(GL_FRONT);
    
    /*   glReadPixels(l, b, (r)-(l)+1, (t)-(b)+1, GL_RGBA, GL_BYTE,  *pixbuf);*/
    glReadPixels(0, 0, W_width, W_height, GL_RGBA, GL_BYTE,  *pixbuf);

    return(1);
#endif
    return(0);
}

/************************************************************************/
gsd_blend(yesno)
int yesno;
{
#ifdef USE_OGL
    if(yesno){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else{
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
    }
#endif
}

/************************************************************************/
gsd_def_clipplane(num, params)
int num;
float *params;
{
#ifdef USE_OGL
	/* OGLXXX see man page for glClipPlane equation */
    glClipPlane( GL_CLIP_PLANE0+(num), params); 
/*
    if( 2) glEnable(GL_CLIP_PLANE+(num)); else glDisable(GL_CLIP_PLANE0+(num));
*/
#endif
}

/************************************************************************/
gsd_set_clipplane(num, able)
int num, able;
{
float params[4]; /* ignored */
#ifdef USE_OGL
	/* OGLXXX see man page for glClipPlane equation */
    if (able)
	glEnable(GL_CLIP_PLANE0+(num)); 
    else 
	glDisable(GL_CLIP_PLANE0+(num));
#endif
}
/************************************************************************/
/************************************************************************/


