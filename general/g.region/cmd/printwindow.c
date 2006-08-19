#include <string.h>
#include <stdlib.h>
#include <grass/gis.h>
#include <grass/gprojects.h>
#include <grass/glocale.h>

#include "local_proto.h"

int print_window(struct Cell_head *window, int print_flag, int dist_flag,
		 int z_flag, int g_flag, int b_flag)
{
    char *prj, *datum, *ellps;
    int x;
    char north[30], south[30], east[30], west[30], nsres[30], ewres[30],
	nsres3[30], ewres3[30], tbres[30];
    /* BOB */
    double EW_DIST1, EW_DIST2, NS_DIST1, NS_DIST2;
    double longitude, latitude;
    double lo1, la1, lo2, la2, lo3, la3, lo4, la4;	/* for map center and bash non-LL proj in lat/long */
    double mid_n_lo, mid_n_la, mid_s_lo, mid_s_la, mid_w_lo, mid_w_la, mid_e_lo,
	mid_e_la;
    double sh_ll_w, sh_ll_e, sh_ll_n, sh_ll_s;	/*Lat- and Longditude information for bash output of non-LL projections (b_flag) */
    char buf[50];
    /*double sh_ll_rows, sh_ll_cols; *//*Calculated bounding box rows and cols for the b_flag */

    if (print_flag == 2)
	x = -1;
    else
	x = window->proj;

    G_format_northing(window->north, north, x);
    G_format_northing(window->south, south, x);
    G_format_easting(window->east, east, x);
    G_format_easting(window->west, west, x);
    G_format_resolution(window->ew_res, ewres, x);
    G_format_resolution(window->ew_res3, ewres3, x);
    G_format_resolution(window->ns_res, nsres, x);
    G_format_resolution(window->ns_res3, nsres3, x);
    G_format_resolution(window->tb_res, tbres, x);
    G_begin_distance_calculations();
    /* EW Dist at North edge */
    EW_DIST1 =
	G_distance(window->east, window->north, window->west, window->north);
    /* EW Dist at South Edge */
    EW_DIST2 =
	G_distance(window->east, window->south, window->west, window->south);
    /* NS Dist at East edge */
    NS_DIST1 =
	G_distance(window->east, window->north, window->east, window->south);
    /* NS Dist at West edge */
    NS_DIST2 =
	G_distance(window->west, window->north, window->west, window->south);

    if (dist_flag == 1) {
	sprintf(ewres, "%.8f", ((EW_DIST1 + EW_DIST2) / 2) / window->cols);
	G_trim_decimal(ewres);
	sprintf(ewres3, "%.8f", ((EW_DIST1 + EW_DIST2) / 2) / window->cols3);
	G_trim_decimal(ewres3);
	sprintf(nsres, "%.8f", ((NS_DIST1 + NS_DIST2) / 2) / window->rows);
	G_trim_decimal(nsres);
	sprintf(nsres3, "%.8f", ((NS_DIST1 + NS_DIST2) / 2) / window->rows3);
	G_trim_decimal(nsres3);
	sprintf(tbres, "%.8f", (window->top - window->bottom) / window->depths);
	G_trim_decimal(tbres);
    }
    if (print_flag == 1) {	/* flag.print */
	prj = G_database_projection_name();
	if (!prj)
	    prj = "** unknown **";
	fprintf(stdout, "%-11s %d (%s)\n", "projection:", window->proj, prj);
	fprintf(stdout, "%-11s %d\n", "zone:", window->zone);
	/* don't print datum/ellipsoid in XY-Locations */
	if (window->proj != 0) {
	    datum = G_database_datum_name();
	    if (!datum)
		datum = "** unknown (default: WGS84) **";
	    ellps = G_database_ellipse_name();
	    if (!ellps)
		ellps = "** unknown (default: WGS84) **";

	    fprintf(stdout, "%-11s %s\n", "datum:", datum);
	    fprintf(stdout, "%-11s %s\n", "ellipsoid:", ellps);
	}
	fprintf(stdout, "%-11s %s\n", "north:", north);
	fprintf(stdout, "%-11s %s\n", "south:", south);
	fprintf(stdout, "%-11s %s\n", "west:", west);
	fprintf(stdout, "%-11s %s\n", "east:", east);
	if (z_flag) {
	    fprintf(stdout, "%-11s %.8f\n", "top:", window->top);
	    fprintf(stdout, "%-11s %.8f\n", "bottom:", window->bottom);
	}
	fprintf(stdout, "%-11s %s\n", "nsres:", nsres);
	if (z_flag) {
	    fprintf(stdout, "%-11s %s\n", "nsres3:", nsres3);
	}
	fprintf(stdout, "%-11s %s\n", "ewres:", ewres);
	if (z_flag) {
	    fprintf(stdout, "%-11s %s\n", "ewres3:", ewres3);
	    fprintf(stdout, "%-11s %s\n", "tbres:", tbres);
	}

	fprintf(stdout, "%-11s %d\n", "rows:", window->rows);
	if (z_flag) {
	    fprintf(stdout, "%-11s %d\n", "rows3:", window->rows3);
	}
	fprintf(stdout, "%-11s %d\n", "cols:", window->cols);
	if (z_flag) {
	    fprintf(stdout, "%-11s %d\n", "cols3:", window->cols3);
	    fprintf(stdout, "%-11s %d\n", "depths:", window->depths);
	}
	fprintf(stdout, "%-11s %lld\n", "cells:", (long long) window->rows * window->cols);
	if (z_flag)
	  fprintf(stdout, "%-11s %lld\n", "3dcells:",
		  (long long) window->rows3 * window->cols3 * window -> depths);
    }
    else if (print_flag == 3) {	/* flag.lprint: show boundaries in lat/long  MN 2001 */
	/* if coordinates are not in lat/long format, transform them: */
	if ((G_projection() != PROJECTION_LL) && window->proj != 0) {
	    struct Key_Value *in_proj_info, *in_unit_info;	/* projection information of input map */
	    struct pj_info iproj;	/* input map proj parameters  */
	    struct pj_info oproj;	/* output map proj parameters  */

	    /* read current projection info */
	    if ((in_proj_info = G_get_projinfo()) == NULL)
		G_fatal_error(_
			      ("Can't get projection info of current location"));

	    if ((in_unit_info = G_get_projunits()) == NULL)
		G_fatal_error(_
			      ("Can't get projection units of current location"));

	    if (pj_get_kv(&iproj, in_proj_info, in_unit_info) < 0)
		G_fatal_error(_
			      ("Can't get projection key values of current location"));

	    G_free_key_value(in_proj_info);
	    G_free_key_value(in_unit_info);

	    /* set output projection to lat/long w/ same ellipsoid as input */
	    oproj.zone = 0;
	    oproj.meters = 1.;
	    sprintf(oproj.proj, "ll");
	    if ((oproj.pj = pj_latlong_from_proj(iproj.pj)) == NULL)
		G_fatal_error(_
			      ("Unable to set up lat/long projection parameters"));

	    /* do the transform
	     * syntax: pj_do_proj(outx, outy, in_info, out_info) 
	     *
	     *  1 ------ 2
	     *  |        |  map corners
	     *  |        |
	     *  4--------3
	     */

	    latitude = window->north;
	    longitude = window->west;
	    if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		G_fatal_error(_
			      ("Error in pj_do_proj (projection of input coordinate pair)"));

	    lo1 = longitude;
	    la1 = latitude;
	    fprintf(stdout, "long: %.8f lat: %.8f (north/west corner)\n", lo1,
		    la1);

	    latitude = window->north;
	    longitude = window->east;
	    if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		G_fatal_error(_
			      ("Error in pj_do_proj (projection of input coordinate pair)"));

	    lo2 = longitude;
	    la2 = latitude;
	    fprintf(stdout, "long: %.8f lat: %.8f (north/east corner)\n", lo2,
		    la2);

	    latitude = window->south;
	    longitude = window->east;
	    if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		G_fatal_error(_
			      ("Error in pj_do_proj (projection of input coordinate pair)"));

	    lo3 = longitude;
	    la3 = latitude;
	    fprintf(stdout, "long: %.8f lat: %.8f (south/east corner)\n", lo3,
		    la3);

	    latitude = window->south;
	    longitude = window->west;
	    if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		G_fatal_error(_
			      ("Error in pj_do_proj (projection of input coordinate pair)"));

	    lo4 = longitude;
	    la4 = latitude;
	    fprintf(stdout, "long: %.8f lat: %.8f (south/west corner)\n", lo4,
		    la4);

	    fprintf(stdout, "%-11s %d\n", "rows:", window->rows);
	    fprintf(stdout, "%-11s %d\n", "cols:", window->cols);

	    /* 
	     * map corners and side mids:
	     *          mid_n
	     *       1 ---+---2
	     *       |        |
	     * mid_w +        + mid_e 
	     *       |        |
	     *       4----+---3
	     *          mid_s
	     *
	     * lo: longitude
	     * la: latitude
	     */

	    /* side mids for easting, northing center: */
	    mid_n_lo = (lo2 + lo1) / 2.;
	    mid_n_la = (la2 + la1) / 2.;
	    mid_s_lo = (lo3 + lo4) / 2.;
	    mid_s_la = (la3 + la4) / 2.;
	    mid_w_lo = (lo1 + lo4) / 2.;	/* not needed */
	    mid_w_la = (la1 + la4) / 2.;	/* not needed */
	    mid_e_lo = (lo3 + lo2) / 2.;	/* not needed */
	    mid_e_la = (la3 + la2) / 2.;	/* not needed */

	    G_debug(3, "mid_n_lo %f", mid_n_lo);
	    G_debug(3, "mid_s_lo %f", mid_s_lo);
	    G_debug(3, "mid_n_la %f", mid_n_la);
	    G_debug(3, "mid_s_la %f", mid_s_la);
	    G_debug(3, "mid_w_lo %f", mid_w_lo);
	    G_debug(3, "mid_e_lo %f", mid_e_lo);
	    G_debug(3, "mid_w_la %f", mid_w_la);
	    G_debug(3, "mid_e_la %f", mid_e_la);

	    G_format_easting((mid_n_lo + mid_s_lo) / 2., buf, PROJECTION_LL);
	    fprintf(stdout, "Center longitude: %11s [%.5f]\n", buf,
		    (mid_n_lo + mid_s_lo) / 2.);

	    G_format_northing((mid_n_la + mid_s_la) / 2., buf, PROJECTION_LL);
	    fprintf(stdout, "Center latitude:  %11s [%.5f]\n", buf,
		    (mid_n_la + mid_s_la) / 2.);

	}			/* transform to LL */
	else /* in lat/long already */ if (window->proj != 0)
	    fprintf(stderr,
		    "You are already in lat/long. Use -p flag instead.\n");
	else
	    fprintf(stderr,
		    "You are in xy location (no projection possible, use -p flag instead).\n");
    }
    else if (print_flag == 4) {	/* flag.center: print coordinates of map center  MN 2001 */
	if ((G_projection() == PROJECTION_LL))
	    fprintf(stdout,
		    "Decimal degree (East/North positive, West/South negative):\n");
	fprintf(stdout, "%-11s %f\n", "region center northing:",
		((window->north - window->south) / 2. + window->south));
	fprintf(stdout, "%-11s %f\n", "region center easting: ",
		((window->west - window->east) / 2. + window->east));
    }
    else if (print_flag == 5) {	/* flag.eprint: print region extent  MN 2003 */
	if (g_flag) {
	    if ((G_projection() == PROJECTION_LL))
		fprintf(stderr, "Values in decimal degree:\n");
	    fprintf(stdout, "ns_extent=%f\n", window->north - window->south);
	    fprintf(stdout, "ew_extent=%f\n", window->east - window->west);
	}
	else {
	    if ((G_projection() == PROJECTION_LL))
		fprintf(stdout, "Values in decimal degree:\n");
	    fprintf(stdout, "%-11s %f\n", "region north-south extent:",
		    window->north - window->south);
	    fprintf(stdout, "%-11s %f\n", "region east-west extent:",
		    window->east - window->west);
	}
    }
    else {			/*print the output in shell style (g-flag) */

	fprintf(stdout, "n=%s\n", north);
	fprintf(stdout, "s=%s\n", south);
	fprintf(stdout, "w=%s\n", west);
	fprintf(stdout, "e=%s\n", east);
	if (z_flag) {
	    fprintf(stdout, "t=%g\n", window->top);
	    fprintf(stdout, "b=%g\n", window->bottom);
	}
	fprintf(stdout, "nsres=%s\n", nsres);
	if (z_flag) {
	    fprintf(stdout, "nsres3=%s\n", nsres3);
	}
	fprintf(stdout, "ewres=%s\n", ewres);
	if (z_flag) {
	    fprintf(stdout, "ewres3=%s\n", ewres3);
	    fprintf(stdout, "tbres=%s\n", tbres);
	}
	fprintf(stdout, "rows=%d\n", window->rows);
	fprintf(stdout, "cols=%d\n", window->cols);
	if (z_flag) {
	    fprintf(stdout, "rows3=%d\n", window->rows3);
	    fprintf(stdout, "cols3=%d\n", window->cols3);
	    fprintf(stdout, "depths=%d\n", window->depths);
	}

	/* ***************************************************** */
	/*Calculate the largest boudingbox in lat-lon coordinates */
	/*and print it to stdout ******************************* */
	/* ***************************************************** */
	if (b_flag) {		/*print only if requested, needs -g flag*/
	    if ((G_projection() != PROJECTION_XY)) {

		/*Needed to calculate the LL bounding box */
		struct Key_Value *in_proj_info, *in_unit_info, *out_proj_info, *out_unit_info;	/* projection information of input and output map */
		struct pj_info iproj;	/* input map proj parameters  */
		struct pj_info oproj;	/* output map proj parameters  */
		char buff[100], dum[100];

		/* read current projection info */
		if ((in_proj_info = G_get_projinfo()) == NULL)
		    G_fatal_error(_
				  ("Can't get projection info of current location"));

		if ((in_unit_info = G_get_projunits()) == NULL)
		    G_fatal_error(_
				  ("Can't get projection units of current location"));

		if (pj_get_kv(&iproj, in_proj_info, in_unit_info) < 0)
		    G_fatal_error(_
				  ("Can't get projection key values of current location"));

		/* set output projection to lat/long  and wgs84 ellipsoid */
		out_proj_info = G_create_key_value();
		out_unit_info = G_create_key_value();

		G_set_key_value("proj", "ll", out_proj_info);

		if (G_get_datumparams_from_projinfo(in_proj_info, buff, dum) <
		    0)
		    G_fatal_error(_
				  ("WGS84 output not possible as this location does not contain "
				   "datum transformation parameters. Try running g.setproj."));
		else
		    G_set_key_value("datum", "wgs84", out_proj_info);

		G_set_key_value("unit", "degree", out_unit_info);
		G_set_key_value("units", "degrees", out_unit_info);
		G_set_key_value("meters", "1.0", out_unit_info);

		if (pj_get_kv(&oproj, out_proj_info, out_unit_info) < 0)
		    G_fatal_error(_
				  ("Unable to set up lat/long projection parameters"));

		G_free_key_value(in_proj_info);
		G_free_key_value(in_unit_info);
		G_free_key_value(out_proj_info);
		G_free_key_value(out_unit_info);


		/* set output projection to lat/long w/ same ellipsoid as input */
		/*
		 * oproj.zone = 0;
		 * oproj.meters = 1.;
		 * sprintf(oproj.proj, "ll");
		 * if ((oproj.pj = pj_latlong_from_proj(iproj.pj)) == NULL)
		 * G_fatal_error(_("Unable to set up lat/long projection parameters"));            
		 */
		/* do the transform
		 * syntax: pj_do_proj(outx, outy, in_info, out_info) 
		 *
		 *  1 ------ 2
		 *  |        |  map corners
		 *  |        |
		 *  4--------3
		 */

		latitude = window->north;
		longitude = window->west;
		if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		    G_fatal_error(_
				  ("Error in pj_do_proj (projection of input coordinate pair)"));

		lo1 = longitude;
		la1 = latitude;

		latitude = window->north;
		longitude = window->east;
		if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		    G_fatal_error(_
				  ("Error in pj_do_proj (projection of input coordinate pair)"));

		lo2 = longitude;
		la2 = latitude;

		latitude = window->south;
		longitude = window->east;
		if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		    G_fatal_error(_
				  ("Error in pj_do_proj (projection of input coordinate pair)"));

		lo3 = longitude;
		la3 = latitude;

		latitude = window->south;
		longitude = window->west;
		if (pj_do_proj(&longitude, &latitude, &iproj, &oproj) < 0)
		    G_fatal_error(_
				  ("Error in pj_do_proj (projection of input coordinate pair)"));

		lo4 = longitude;
		la4 = latitude;

		/*Calculate the largest bounding box */
		if (fabs(lo3) > fabs(lo4)) {
		    if (fabs(lo4) < fabs(lo1))
			sh_ll_w = lo4;
		    else
			sh_ll_w = lo1;
		    if (fabs(lo3) > fabs(lo2))
			sh_ll_e = lo3;
		    else
			sh_ll_e = lo2;
		}
		else {
		    if (fabs(lo4) > fabs(lo1))
			sh_ll_w = lo4;
		    else
			sh_ll_w = lo1;
		    if (fabs(lo3) < fabs(lo2))
			sh_ll_e = lo3;
		    else
			sh_ll_e = lo2;
		}

		if (fabs(la4) < fabs(la1)) {
		    if (fabs(la1) > fabs(la2))
			sh_ll_n = la1;
		    else
			sh_ll_n = la2;
		    if (fabs(la4) < fabs(la3))
			sh_ll_s = la4;
		    else
			sh_ll_s = la3;
		}
		else {
		    if (fabs(la1) < fabs(la2))
			sh_ll_n = la1;
		    else
			sh_ll_n = la2;
		    if (fabs(la4) > fabs(la3))
			sh_ll_s = la4;
		    else
			sh_ll_s = la3;
		}

		/*print the largset bounding box */
		fprintf(stdout, "LL_W=%.8f\n", sh_ll_w);
		fprintf(stdout, "LL_E=%.8f\n", sh_ll_e);
		fprintf(stdout, "LL_N=%.8f\n", sh_ll_n);
		fprintf(stdout, "LL_S=%.8f\n", sh_ll_s);
		/*It should be calculated which number of rows and cols we have in LL */
		/*fprintf (stdout, "LL_ROWS=%f \n",sh_ll_rows);
		 * fprintf (stdout, "LL_COLS=%f \n",sh_ll_cols); */
	    }
	    else {
		/*fprintf(stderr, "No LatLong information for XY-Projection"); */
	    }
	}
    }

    return 0;
}
