/*
 * This code is preliminary. I don't know if it is even
 * correct.
 */

/*
 * From "Map Projections" by Peter Richardus and Ron K. Alder, 1972
 * (526.8 R39m in Map & Geography Library)
 * page  19, formula 2.17
 *
 * Formula is the equation of a geodesic from (lat1,lon1) to (lat2,lon2)
 * Input is lon, output is lat (all in degrees)
 *
 * Note formula only works if 0 < abs(lon2-lon1) < 180
 * If lon1 == lon2 then geodesic is the merdian lon1 
 * (and the formula will fail)
 * if lon2-lon1=180 then the geodesic is either meridian lon1 or lon2
 */

#include "pi.h"

extern double sin(), cos(), tan(), atan();

static double A, B;
#define swap(a,b) temp=a;a=b;b=temp

G_begin_geodesic_equation (lon1, lat1, lon2, lat2)
    double lon1, lat1, lon2, lat2;
{
    double sin21, tan1,tan2;

    adjust_lon (&lon1);
    adjust_lon (&lon2);
    adjust_lat (&lat1);
    adjust_lat (&lat2);
    if (lon1 > lon2)
    {	
	register double temp;
	swap(lon1,lon2);
	swap(lat1,lat2);
    }
    if (lon1 == lon2)
    {
	A = B = 0.0;
	return 0;
    }
    lon1 = Radians(lon1);
    lon2 = Radians(lon2);
    lat1 = Radians(lat1);
    lat2 = Radians(lat2);

    sin21 = sin (lon2-lon1);
    tan1  = tan (lat1);
    tan2  = tan (lat2);

    A = ( tan2 * cos(lon1) - tan1 * cos(lon2) ) /sin21;
    B = ( tan2 * sin(lon1) - tan1 * sin(lon2) ) /sin21;

    return 1;
}

/* only works if lon1 < lon < lon2 */

double
G_geodesic_lat_from_lon (lon)
    double lon;
{
    adjust_lon (&lon);
    lon = Radians(lon);
    return Degrees (atan(A * sin(lon) - B * cos(lon)));
}

static
adjust_lon (lon)
    double *lon;
{
    while (*lon > 180.0)
	*lon -= 360.0;
    while (*lon < -180.0)
	*lon += 360.0;
}

static
adjust_lat (lat)
    double *lat;
{
    if (*lat >  90.0) *lat =  90.0;
    if (*lat < -90.0) *lat = -90.0;
}
