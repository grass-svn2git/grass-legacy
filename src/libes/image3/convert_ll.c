/*======================================================================
			imagery_lib supplimental

  convert_ll.c --

    convert_to_ll (east, north, lat, lon)

    convert_from_ll (lat, lon, east, north)


    Convert Control Points to/from the target location 
    coordinate system to/from (lat/lon).
    
    Conversion routines used base on target location PROJECTION CODE:


======================================================================*/

#include "ortho_image.h"
#include "gprojects.h"
#include "string.h"

/*-----------------------------------------------------------------------
 * Convert Target coordinates (east, north) to (lat, lon)
 * 
 *
 * RETURNS: 
 *-----------------------------------------------------------------------*/
int convert_to_ll (Control_Points_LL *cpll, Control_Points_2D *cptemp)
{
  int i;
  double lat,  lon;
  double east, north;

  struct pj_info targ_proj_info;       /* target location proj info */
  struct pj_info ll_proj_info;         /* info for lat/lon */
  struct Key_Value  *targ_proj_keys, *targ_unit_keys;
  struct Key_Value  *ll_proj_keys,   *ll_unit_keys;
  double a, es;    /* target ellps paramet */
  char buf[50];
  char *tmp_bob;

   
  /* initialize */
  cpll->count  = 0;
  cpll->e1     = NULL;
  cpll->n1     = NULL;
  cpll->lat2   = NULL;
  cpll->lon2   = NULL;
  cpll->status = NULL;


  /* switch the context to the target location */
  select_target_env();

  /* get the target location projection infor */
  /* G_get_window (&tmp_window); */
  targ_proj_keys = G_get_projinfo();
  if (targ_proj_keys == NULL) {
	exit (0);
  }


  targ_unit_keys = G_get_projunits();
  if (targ_unit_keys == NULL) {
	exit (0);
  }

  /*   There is no longer a pj_get_kv() in libproj.a   Sep-15-1999 */
  if (pj_get_kv(&targ_proj_info, targ_proj_keys, targ_unit_keys) < 0) {
	exit (0);
  }

tmp_bob = G_find_key_value("proj", targ_proj_keys);

fprintf(stderr, "BOBBY %s\n", tmp_bob);

if (!strcmp(tmp_bob, "ll")) return 0;


  /* save the target ellps parameter */
  G_get_ellipsoid_parameters(&a, &es);

  /* set up ll_proj_keys */
  ll_proj_keys = G_create_key_value();
  ll_unit_keys = G_create_key_value();
  G_set_key_value ("name", "Lat/Lon", ll_proj_keys);
  G_set_key_value ("proj", "ll", ll_proj_keys);
  sprintf(buf, "%.16g", a);
  G_set_key_value("a", buf, ll_proj_keys);
  sprintf(buf, "%.16g", es);
  G_set_key_value("es", buf, ll_proj_keys);

  G_set_key_value("unit", "degree", ll_unit_keys);
  G_set_key_value("units", "degrees", ll_unit_keys);
  G_set_key_value("meters", "1.0", ll_unit_keys);
   
  /*   There is no longer a pj_get_kv() in libproj.a   Sep-15-1999 */
  if (pj_get_kv(&ll_proj_info, ll_proj_keys, ll_unit_keys) < 0) {
	exit (0);
  }


  /* free the keys */
  G_free_key_value (targ_proj_keys);
  G_free_key_value (targ_unit_keys);
  G_free_key_value (ll_proj_keys);
  G_free_key_value (ll_unit_keys);
   

  /* loop through all the temp points */
  for (i = 0; i < cptemp->count; i++) {

fprintf(stderr, "BOB -- 3\n");

    /* allocate an empty control point */
    I_new_con_point_ll (cpll, 0.0, 0.0, 0.0, 0.0,  0);

fprintf(stderr, "BOB -- 4\n");

    /* e1, n1, and status remaing the same */
    cpll->status[i] = cptemp->status[i];
    cpll->e1[i] = cptemp->e1[i];
    cpll->n1[i] = cptemp->n1[i];

    east  = cptemp->e2[i];
    north = cptemp->n2[i];

fprintf(stderr, "BOB -- 5\n");

    /* There is no pj_do_proj() function in libproj.a Sep-15-1999 */
    if (pj_do_proj (&east, &north, &targ_proj_info, &ll_proj_info) <0) {
       G_fatal_error ("Error in pj_do_proj\n");
    } 

    lon = east ;
    lat = north;

fprintf(stderr, "BOB1 -- %f %f\n", lon, lat);

    /** set the converted postions into the temp points **/
    cpll->lon2[i] = lon;
    cpll->lat2[i] = lat;

  } /* i loop */

  /* switch the context back to current location */
  select_current_env();

  return 0;
}


/*-----------------------------------------------------------------------*
 * Convert from (lat/lon) to Target coordinates (east, north)
 * 
 *
 * RETRUNS: 
 *-----------------------------------------------------------------------*/
int 
convert_from_ll (Control_Points_LL *cpll, Control_Points_2D *cptemp)
{
  int i;
  double lat,  lon;
  double east, north;

  struct pj_info targ_proj_info;       /* target location proj info */
  struct pj_info ll_proj_info;         /* info for lat/lon */
  struct Key_Value  *targ_proj_keys, *targ_unit_keys;
  struct Key_Value  *ll_proj_keys,   *ll_unit_keys;
  double a, es;    /* target ellps paramet */
  char buf[50];


  /* initialize */
  cptemp->count  = 0;
  cptemp->e1     = NULL;
  cptemp->n1     = NULL;
  cptemp->e2     = NULL;
  cptemp->n2     = NULL;
  cptemp->status = NULL;


  /* switch the context to the target location */
  select_target_env();

  /* get the target location projection infor */
  /* G_get_window (&tmp_window); */

  targ_proj_keys = G_get_projinfo();
  if (targ_proj_keys == NULL) {
	exit (0);
  }

  targ_unit_keys = G_get_projunits();
  if (targ_unit_keys == NULL) {
	exit (0);
  }

  /*   There is no longer a pj_get_kv() in libproj.a   Sep-15-1999 */
  if (pj_get_kv(&targ_proj_info, targ_proj_keys, targ_unit_keys) < 0) {
	exit (0);
  }

  /* save the target ellps parameter */
  G_get_ellipsoid_parameters(&a, &es);

  /* set up ll_proj_keys */
  ll_proj_keys = G_create_key_value();
  ll_unit_keys = G_create_key_value();
  G_set_key_value ("name", "Lat/Lon", ll_proj_keys);
  G_set_key_value ("proj", "ll", ll_proj_keys);
  sprintf(buf, "%.16g", a);
  G_set_key_value("a", buf, ll_proj_keys);
  sprintf(buf, "%.16g", es);
  G_set_key_value("es", buf, ll_proj_keys);

  /*   There is no longer a pj_get_kv() in libproj.a   Sep-15-1999 */
  if (pj_get_kv(&ll_proj_info, ll_proj_keys, ll_unit_keys) < 0) {
	exit (0);
  }

  /* free the keys */
  G_free_key_value (targ_proj_keys);
  G_free_key_value (targ_unit_keys);
  G_free_key_value (ll_proj_keys);
  G_free_key_value (ll_unit_keys);   

   
  for (i = 0; i < cpll->count; i++) {

    /* allocate an empty control point */
    I_new_ref_point (cptemp, 0.0, 0.0, 0.0, 0.0, (int) 0);  
				/* uses ref_point for 2D */

    /* e1, n1, and status remaing the same */
    cptemp->status[i] = cpll->status[i];
    cptemp->e1[i] = cpll->e1[i];
    cptemp->n1[i] = cpll->n1[i];

    /* get the lat and lon */
    lon = cpll->lon2[i];
    lat = cpll->lat2[i];

    /* project from ll to target */
    if (pj_do_proj (&lon, &lat, &ll_proj_info, &targ_proj_info) <0) {
       G_fatal_error ("Error in pj_do_proj\n");
    }

    east  = lon;
    north = lat;


fprintf(stderr, "BOB2 -- %f %f\n", east, north);

    /** set the converted postions into teh temp points **/
    cptemp->e2[i] = east;
    cptemp->n2[i] = north;

  } /* i -loop */

  /* switch the context to the target location */
  select_current_env();

  return 0;
}
