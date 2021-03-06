#include "gis.h"
#define PI	3.14159265358979323846
#define Radians(x) ((x) * PI/180.0)
#define Degrees(x) ((x) * 180.0/PI)


parse_line (key, s, e1, n1, e2, n2, projection)
    char *key, **s;
    double *e1, *n1, *e2, *n2;
{
    int err;
    double sin(), cos();
    double distance, azimuth;

    err = 0;
    if(!G_scan_easting (s[0], e1, projection))
	err |= 1;
    if(!G_scan_northing (s[1], n1, projection))
	err |= 2;
    if(sscanf (s[2],  "%lf", &azimuth) != 1)
	err |= 4;
    if(sscanf (s[3],  "%lf", &distance) != 1 || distance < 0.0)
	err |= 8;

    if(err)
    {
	fprintf (stderr, "%s=", key);
	fprintf (stderr, "%s%s%s,",
	    err&1?"<":"", s[0], err&1?">":"");
	fprintf (stderr, "%s%s%s,",
	    err&2?"<":"", s[1], err&2?">":"");
	fprintf (stderr, "%s%s%s,",
	    err&4?"<":"", s[2], err&4?">":"");
	fprintf (stderr, "%s%s%s",
	    err&8?"<":"", s[3], err&8?">":"");
	fprintf (stderr, " ** invalid values(s) **\n");
	return err;
    }

    *e2 = *e1 + distance * sin (Radians(azimuth));
    *n2 = *n1 + distance * cos (Radians(azimuth));

    return 0;
}
