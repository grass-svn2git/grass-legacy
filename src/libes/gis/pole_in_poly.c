/***********************************************************
 * G_pole_in_polygon(x, y, n)
 *     double *x, *y, n;
 *
 * For lat-lon coordinates, this routine determines if the polygon
 * defined by the n verticies x,y contain one of the poles
 *
 * returns
 *  -1 if it contains the south pole,
 *   1 if it contains the north pole,
 *   0 no pole
 *
 * Note: don't use this routine if the projection isn't PROJECTION_LL
 *       no check is made by this routine for valid projection
 ***********************************************************/


G_pole_in_polygon(x, y, n)
    double *x, *y;
{
    int i;
    double len, area, total_len, total_area;

    if (n <= 1)
	return 0;

    mystats (x[n-1],y[n-1],x[0],y[0],&total_len, &total_area);
    for (i = 1; i < n; i++)
    {
	mystats (x[i-1],y[i-1],x[i],y[i],&len, &area);
	total_len += len;
	total_area += area;
    }

/* if polygon contains a pole then the x-coordinate length of
 * the perimeter should compute to 0, otherwise it should be about 360
 * (or -360, depending on the direction of perimeter traversal)
 *
 * instead of checking for exactly 0, check from -1 to 1 to avoid
 * roundoff error.
 */
    if (total_len < 1.0 && total_len > -1.0)
	return 0.0;
    
    return total_area >= 0.0 ? 1 : -1;
}

static
mystats (x0,y0,x1,y1,len,area)
    double x0, y0, x1, y1, *len, *area;
{
    if (x1 > x0)
	while (x1-x0 > 180)
	    x0 += 360;
    else if (x0 > x1)
	while (x0-x1 > 180)
	    x0 -= 360;
    
    *len = x0 - x1;

    if (x0 > x1)
	*area = (x0-x1) * (y0+y1)/2.0;
    else
	*area = (x1-x0) * (y1+y0)/2.0;
}
