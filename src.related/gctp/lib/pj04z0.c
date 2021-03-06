/* pj04z0.f -- translated by f2c (version of 11 May 1990  14:38:39).
 * C version and Fortran dependencies removed by Sean Dougherty and
 * Philip Thompson (phils@athena.mit.edu), 10/5/90.
 * Computer Resource Lab., Dept. Architecture and Urban Planning,
 * MIT, Cambridge MA  02139
 */

#include <stdio.h>
#include <math.h>
#include "gctp.h"

/* Table of constant values */
static int c__5 = 5;
static int c__0 = 0;

/**********************************************************************/
/** NOAA/USGS GENERAL MAP PROJECTION PACKAGE ..... DR. A. A. ELASSAL **/
/**          MATHEMATICAL ANALYSIS BY JOHN SNYDER                    **/
/** GCTP/II                 VERSION 1.0.2           SEPTEMBER 1,1986 **/
/**********************************************************************/
/* LAMBERT CONFORMAL CONIC  */
/**********************************************************************/

int pj04z0_0_(n__, zone, data, ipfile, iflg, geog, proj)
int n__;
int *zone;
double *data;
FILE *ipfile;
int *iflg;
double *geog, *proj;
{
    double halfpi = 1.57079632679489661923;
    double epsln = 1e-10;
    double one = 1.0;
    static int switch_ = 0;
    double d_1;
    char angs[16 * 4];
    double b, x, y, theta;
    double rh, ts, cosphi, sinphi, ms1, ms2, ts0, ts1, ts2, con;
    static double a, e, es, lat1, lat2, lon0, lat0, x0, y0, ns, f, rh0;

    /**** PARAMETERS **** A,E,ES,LAT1,LAT2,LON0,LAT0,X0,Y0,NS,F,RH0 ***/
    /* Parameter adjustments */
    if (data)
        --data;
    if (geog)
        --geog;
    if (proj)
        --proj;
    /* Function Body */
    switch (n__) {
    case 1:
        goto L_is04z0;
    case 2:
        goto L_pf04z0;
    case 3:
        goto L_pi04z0;
    }

    /* .  INITIALIZATION OF PROJECTION PARAMETERS (ENTRY INPUT)  . */

  L_is04z0:
    *iflg = 0;
    if (switch_ != 0 && switch_ == *zone)
        return 0;
    if (data[1] <= 0.0)
        goto L100;
    a = data[1];
    b = data[2];
    if (b > 0.0)
        goto L40;
    e = 0.0;
    es = 0.0;
    goto L120;
  L40:
    if (b > 1.0)
        goto L60;
    e = sqrt(b);
    es = b;
    goto L120;
  L60:
    /* Computing 2nd power */
    d_1 = b / a;
    es = 1.0 - d_1 * d_1;
    e = sqrt(es);
    goto L120;
  L100:
    a = ellpz0.az;
    e = ellpz0.ez;
    es = ellpz0.esz;
  L120:
    unitz0_(&data[3], &c__5, &lat1, &c__0, ipfile, iflg);
    if (*iflg != 0)
        return 0;
    unitz0_(&data[4], &c__5, &lat2, &c__0, ipfile, iflg);
    if (*iflg != 0)
        return 0;
    if ((d_1 = lat1 + lat2, fabs(d_1)) >= epsln)
        goto L130;
    if (ipfile != NULL) {
        fprintf(ipfile, "pj04z0_0_: Equal latitudes for st. parallels");
    fprintf(ipfile, " on opposite sides of equator\n");
        }
    *iflg = 401;
    return 0;
  L130:
    unitz0_(&data[5], &c__5, &lon0, &c__0, ipfile, iflg);
    if (*iflg != 0)
        return 0;
    unitz0_(&data[6], &c__5, &lat0, &c__0, ipfile, iflg);
    if (*iflg != 0)
        return 0;
    x0 = data[7];
    y0 = data[8];
    sinphi = sin(lat1);
    con = sinphi;
    cosphi = cos(lat1);
    ms1 = msfnz0_(&e, &sinphi, &cosphi);
    ts1 = tsfnz0_(&e, &lat1, &sinphi);
    sinphi = sin(lat2);
    cosphi = cos(lat2);
    ms2 = msfnz0_(&e, &sinphi, &cosphi);
    ts2 = tsfnz0_(&e, &lat2, &sinphi);
    sinphi = sin(lat0);
    ts0 = tsfnz0_(&e, &lat0, &sinphi);
    if ((d_1 = lat1 - lat2, fabs(d_1)) >= epsln)
        goto L140;
    ns = con;
    goto L150;
  L140:
    ns = log(ms1 / ms2) / log(ts1 / ts2);
  L150:
    f = ms1 / (ns * pow_dd(&ts1, &ns));
    rh0 = a * f * pow_dd(&ts0, &ns);

    /* LIST RESULTS OF PARAMETER INITIALIZATION. */
    dmslz0_(&lat1, &c__0, angs, ipfile, iflg);
    dmslz0_(&lat2, &c__0, angs + 16, ipfile, iflg);
    dmslz0_(&lon0, &c__0, angs + 32, ipfile, iflg);
    dmslz0_(&lat0, &c__0, angs + 48, ipfile, iflg);
    if (ipfile != NULL) {
        fprintf(ipfile, "Initialization parameters");
    fprintf(ipfile, " (lambert conformal conic projection):\n");
        fprintf(ipfile, "\tSemi-major axis of ellipsoid = %16.4f meters\n", a);
        fprintf(ipfile, "\tEccentricity squared         = %16.13f\n", es);
        fprintf(ipfile, "\tLatitude of 1st st. parallel = %16s\n", angs);
        fprintf(ipfile, "\tLatitude of 2nd st. parallel = %16s\n", angs + 16);
        fprintf(ipfile, "\tLongitude of origin          = %16s\n", angs + 32);
        fprintf(ipfile, "\tLatitude of origin           = %16s\n", angs + 48);
        fprintf(ipfile, "\tFalse easting                = %16.4f meters\n", x0);
        fprintf(ipfile, "\tFalse northing               = %16.4f meters\n", y0);
    }
    switch_ = *zone;
    return 0;

    /* .  FORWARD TRANSFORMATION  . */
  L_pf04z0:
    *iflg = 0;
    if (switch_ != 0)
        goto L200;
    if (ipfile != NULL)
        fprintf(ipfile, "pj04z0_0_: Uninitialized transformation\n");
    *iflg = 400;
    return 0;
  L200:
    con = (d_1 = fabs(geog[2]) - halfpi, fabs(d_1));
    if (con > epsln)
        goto L220;
    con = geog[2] * ns;
    if (con > 0.0)
        goto L210;
    if (ipfile != NULL)
        fprintf(ipfile, "pj04z0_0_: Point cannot be projected\n");
    *iflg = 402;
    return 0;
  L210:
    rh = 0.0;
    goto L230;
  L220:
    sinphi = sin(geog[2]);
    ts = tsfnz0_(&e, &geog[2], &sinphi);
    rh = a * f * pow_dd(&ts, &ns);
  L230:
    d_1 = geog[1] - lon0;
    theta = ns * adjlz0_(&d_1);
    proj[1] = x0 + rh * sin(theta);
    proj[2] = y0 + rh0 - rh * cos(theta);
    return 0;

    /* .  INVERSE TRANSFORMATION  . */
  L_pi04z0:
    *iflg = 0;
    if (switch_ != 0)
        goto L240;
    if (ipfile != NULL)
        fprintf(ipfile, "pj04z0_0_: Uninitialized transformation\n");
    *iflg = 400;
    return 0;
  L240:
    x = proj[1] - x0;
    y = rh0 - proj[2] + y0;
    d_1 = sqrt(x * x + y * y);
    rh = d_sign(&d_1, &ns);
    theta = 0.0;
    con = d_sign(&one, &ns);
    if (rh != 0.0)
        theta = atan2(con * x, con * y);
    if (rh != 0.0 || ns > 0.0)
        goto L250;
    geog[2] = -halfpi;
    goto L260;
  L250:
    con = 1.0 / ns;
    d_1 = rh / (a * f);
    ts = pow_dd(&d_1, &con);
    geog[2] = phi2z0_(&e, &ts, ipfile, iflg);
    if (*iflg != 0) {
        return 0;
    }
  L260:
    d_1 = theta / ns + lon0;
    geog[1] = adjlz0_(&d_1);
    return 0;
} /* pj04z0_ */


int pj04z0_()
{
    return pj04z0_0_(0, (int*)0, (double*)0, NULL, (int*)0,
            (double*)0, (double*)0);
}

int is04z0_(zone, data, ipfile, iflg)
int *zone;
double *data;
FILE *ipfile;
int *iflg;
{
    return pj04z0_0_(1, zone, data, ipfile, iflg,(double*)0,(double*)0);
}

int pf04z0_(geog, proj, iflg)
double *geog, *proj;
int *iflg;
{
    return pj04z0_0_(2, (int*)0, (double*)0, NULL, iflg, geog, proj);
}

int pi04z0_(proj, geog, iflg)
double *proj, *geog;
int *iflg;
{
    return pj04z0_0_(3, (int*)0, (double*)0, NULL, iflg, geog, proj);
}

/*** end pj04z0.c ***/
