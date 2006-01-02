/***************************************************************************
*
* MODULE:       r3.in.ascii
*
* AUTHOR(S):    Roman Waupotitsch, Michael Shapiro, Helena Mitasova, Bill Brown, 
* 		Lubos Mitas, Jaro Hofierka 
*
* PURPOSE:      Convert a 3D ASCII raster text file into a (binary) 3D raster map layer 
*
* COPYRIGHT:    (C) 2005 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*               License (>=v2). Read the file COPYING that comes with GRASS
*               for details.
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gis.h"
#include "G3d.h"
#include "glocale.h"

/*---------------------------------------------------------------------------*/
/*- prototypes --------------------------------------------------------------*/

static void fatalError (char *errorMsg);       /*Simple Error message */

static void setParams ();                     /*Fill the paramType structure */

/*Puts the userdefined parameters into easier handable variables*/
static void getParams (char **input, char **output, int *convertNull, double *nullValu);

/*reads a g3d ascii-file headerfile-string*/
static void readHeaderString (FILE *fp, char *valueString, double *value);

static FILE * openAscii (char *asciiFile, G3D_Region *region); /*open the g3d ascii file*/

/*This function does all the work, it reads the values from the g3d ascii-file and put 
it into an g3d-map*/
static void asciiToG3d (FILE *fp, G3D_Region *region, int convertNull, double nullValue);





/*---------------------------------------------------------------------------*/

void *map = NULL;
extern void * G3d_openNewParam ();

/*---------------------------------------------------------------------------*/
static void
fatalError  (char *errorMsg)

{
  if (map != NULL) {
    /* should unopen map here! */
    G3d_closeCell (map);
  }

  G3d_fatalError (errorMsg);
}

/*---------------------------------------------------------------------------*/

typedef	struct {
  struct Option *input, *output, *nv;
} paramType;

static paramType param;

static void
setParams ()

{
  param.input = G_define_option();
  param.input->key = "input";
  param.input->type = TYPE_STRING;
  param.input->required = YES;
  param.input->description = _("Ascii raster file to be imported");

  param.output = G_define_option();
  param.output->key = "output";
  param.output->type = TYPE_STRING;
  param.output->required = YES;
  param.output->multiple = NO ;
  param.output->gisprompt = _("any,grid3,3d raster");
  param.output->description = _("Name for G3d raster map");

  param.nv = G_define_option();
  param.nv->key = "nv";
  param.nv->type = TYPE_STRING;
  param.nv->required = NO;
  param.nv->multiple = NO;
  param.nv->answer = "none";
  param.nv->description = 
    _("String representing NULL value data cell (use 'none' if no such value)");
}

/*---------------------------------------------------------------------------*/

static void
getParams  (char **input, char **output, int *convertNull, double *nullValue)

{
 *input = param.input->answer;
 *output = param.output->answer; 
 *convertNull = (strcmp(param.nv->answer, "none") != 0);
 if (*convertNull)
   if (sscanf (param.nv->answer, "%lf", nullValue) != 1)
     fatalError ("getParams: NULL-value value invalid");

 G_debug (3, "getParams: input: %s, output: %s", *input, *output);
}

/*---------------------------------------------------------------------------*/

static void
readHeaderString  (FILE *fp, char *valueString, double *value)

{
  static char format[100];

  snprintf (format, 100, "%s %%lf", valueString); /*to avoid bufferoverflows we use snprintf*/
  if (fscanf (fp, format, value) != 1)
    fatalError ("readHeaderString: header value invalid");

  while (fgetc (fp) != '\n');
}

/*---------------------------------------------------------------------------*/

static FILE *
openAscii  (char *asciiFile, G3D_Region *region)

{
  FILE *fp;
  double tmp;

 G_debug (3, "openAscii: opens the ascii file and reads the header");

  fp = fopen (asciiFile, "r");
  if (fp == NULL) {
    perror(asciiFile);
    G_usage ();
    exit (-1) ;
  }

  G3d_getWindow (region);

  readHeaderString (fp, "north:", &(region->north));
  readHeaderString (fp, "south:", &(region->south));
  readHeaderString (fp, "east:", &(region->east));
  readHeaderString (fp, "west:", &(region->west));
  readHeaderString (fp, "top:", &(region->top));
  readHeaderString (fp, "bottom:", &(region->bottom));
  readHeaderString (fp, "rows:", &tmp); region->rows = tmp;
  readHeaderString (fp, "cols:", &tmp); region->cols = tmp;
  readHeaderString (fp, "levels:", &tmp); region->depths = tmp;

  return fp;
}

/*---------------------------------------------------------------------------*/

#define MAX(a,b) (a > b ? a : b)

static void
asciiToG3d  (FILE *fp, G3D_Region *region, int convertNull, double nullValue)

{
  int x, y, z;
  double value;
/*
  int tileX, tileY, tileZ;
  
  G3d_getTileDimensionsMap (map, &tileX, &tileY, &tileZ);
  G3d_minUnlocked (map, G3D_USE_CACHE_X);

  G3d_autolockOn (map);
  G3d_unlockAll (map);
fprintf(stderr,"rows=%d cols=%d depths=%d\n",region->rows,region->cols,region->depths);
*/

 G_debug (3, "asciiToG3d: writing the g3d map, with rows %i cols %i depths %i", region->rows, region->cols, region->depths);

  for (z = 0; z < region->depths; z++) {
/*
    if ((z % tileZ) == 0) G3d_unlockAll (map);
*/
    for (y = region->rows-1; y >= 0; y--)    /* go north to south */
      for (x = 0; x < region->cols; x++) {
	if (fscanf (fp, "%lf", &value) != 1)
	  fatalError ("asciiToG3d: read failed");
	
	if (convertNull && (value == nullValue))
	  G3d_setNullValue (&value, 1, G3D_DOUBLE);
	G3d_putDouble (map, x, y, z, value);
      }
/*
    if (! G3d_flushTilesInCube (map, 
				0, 0, MAX (0, z - tileZ),
				region->rows - 1,
				region->cols - 1, z))
      fatalError ("asciiTog3d: error flushing tiles");
*/
  }
/*
  if (! G3d_flushAllTiles (map))  
    fatalError ("asciiTog3d: error flushing tiles");

  G3d_autolockOff (map);
  G3d_unlockAll (map);
*/
}

/*---------------------------------------------------------------------------*/

int
main  (int argc, char *argv[])

{
  char *input, *output;
  int convertNull;
  double nullValue;
  int useTypeDefault, type, useLzwDefault, doLzw, useRleDefault, doRle;
  int usePrecisionDefault, precision, useDimensionDefault, tileX, tileY, tileZ;
  G3D_Region region;
  FILE *fp;
  struct GModule *module;

  map = NULL;

  G_gisinit(argv[0]);
  module = G_define_module();
  module->description =
   _("Convert a 3D ASCII raster text file into a (binary) 3D raster map layer ");

  setParams ();
  G3d_setStandard3dInputParams ();

  if (G_parser (argc, argv)) exit(1);

  getParams (&input, &output, &convertNull, &nullValue);
  if (! G3d_getStandard3dParams (&useTypeDefault, &type, 
				 &useLzwDefault, &doLzw, 
				 &useRleDefault, &doRle, 
				 &usePrecisionDefault, &precision, 
				 &useDimensionDefault, &tileX, &tileY, &tileZ))
    fatalError ("main: error getting standard parameters");

  fp = openAscii (input, &region);
  
  /*Open the new G3D map*/
  map = G3d_openNewParam (output, G3D_DOUBLE, G3D_USE_CACHE_XY,
			  &region,
			  type, doLzw, doRle, precision, tileX, tileY, tileZ);

  if (map == NULL) fatalError ("main: error opening g3d file");

  /*Create the new G3D Map*/
  asciiToG3d (fp, &region, convertNull, nullValue);

  if (! G3d_closeCell (map)) 
    fatalError ("main: error closing new g3d file");
    
  map = NULL;
  if (fclose (fp)) fatalError ("main: error closing ascii file");

  return 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
