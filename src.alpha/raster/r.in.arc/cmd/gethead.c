#include "gis.h"
gethead (fd, cellhd, missingval)
FILE *fd;
struct Cell_head *cellhd;
int *missingval;
{
	int ok ;
	int nodata,res,s,w,r,c;
	char label[100], value[100];
	char buf[1024];
	char *err, *G_adjust_Cell_head();
	int G_scan_northing();
	int G_scan_easting();
	int G_scan_resolution();
	int scan_int();
	int scan_res();

	s=nodata=res=w=r=c=0;

	cellhd->zone = G_zone();
	cellhd->proj = G_projection();

	while (nodata == 0 || s == 0 || res == 0 || w == 0 || r == 0 || c == 0)
	{
		if (!G_getl(buf,sizeof buf, fd)) break;
		*label = *value = NULL;
		sscanf (buf, "%s %s", label, value);
		if (*label == NULL) continue;	/* ignore blank lines */


		if (strcmp (label, "ncols") == 0)
		{
			if(!extract (c++, label, value, &cellhd->cols, cellhd->proj,
			    scan_int)) ok = 0;
			continue;
		}

		if (strcmp (label, "nrows") == 0)
		{
			if(!extract (r++, label, value, &cellhd->rows, cellhd->proj,
			    scan_int)) ok = 0;
			continue;
		}

		if (strcmp (label, "xllcorner") == 0)
		{
			if(!extract (w++, label, value, &cellhd->west, cellhd->proj,
			    G_scan_easting)) ok = 0;
			continue;
		}

		if (strcmp (label, "yllcorner") == 0)
		{
			if(!extract (s++, label, value, &cellhd->south, cellhd->proj,
			    G_scan_northing)) ok = 0;
			continue;
		}

		if (strcmp (label, "cellsize") == 0)
		{
			if(!extract (res++, label, value, &cellhd->ew_res, cellhd->proj,
			    G_scan_resolution)) ok = 0;
			    
		        cellhd->ns_res=cellhd->ew_res;
			cellhd->north = cellhd->south + (cellhd->ns_res * cellhd->rows);
			cellhd->east = cellhd->west + (cellhd->ew_res * cellhd->cols);
			
			continue;
		}

                if (strcmp (label, "NODATA_value") == 0)
		{
			if(!extract (nodata++, label, value, missingval, cellhd->proj,
			    scan_res)) ok = 0;
			continue;
		}

		error ("illegal line in header");
		error (buf);
		
		missing(s,"yllcorner") ;
		missing(w,"xllcorner") ;
		missing(r,"nrows") ;
		missing(c,"ncols") ;
		missing(res,"cellsize") ;
		missing(nodata,"NODATA_value") ;
		return 0 ;
	}

	if (err = G_adjust_Cell_head (cellhd, 1, 1))
	{
		error (err);
		return 0;
	}

	return 1;
}

static
scan_int (s, i, proj)
char *s;
int *i;
{
	char dummy[3];

	*dummy = 0;

	if (sscanf (s, "%d%1s", i, dummy) != 1)
		return 0;
	if (*dummy)
		return 0;
	if (*i <= 0)
		return 0;
	return 1;
}

static
scan_res (s, i, proj)
char *s;
int *i;
{
	char dummy[3];

	*dummy = 0;

	if (sscanf (s, "%d%1s", i, dummy) != 1)
		return 0;
	if (*dummy)
		return 0;
	if (*i <= -9999999)
    	        return 0;
    	return 1;
}



static
extract (count, label, value, data, proj, scanner)
char *label;
char *value;
char *data;
int (*scanner)();
{
	char msg[1024];
	if (count)
	{
		sprintf (msg, "duplicate \"%s\" field in header", label);
		error (msg);
		return 0;
	}
	if (scanner (value, data, proj))
		return 1;
	sprintf (msg, "illegal \"%s\" value in header", label);
	error (msg);
	sprintf (msg, "  %s: %s", label, value);
	error (msg);
	return 0;
}

static
missing (count, label)
char *label;
{
	char msg[200];
	if (count) return 0;
	sprintf (msg, "\"%s\" field missing from header", label);
	error(msg);
	return 1;
}

static
error(msg)
char *msg;
{
	static int first = 1;
	char *G_program_name();

	if (first)
	{
		fprintf (stderr, "%s: ** errors detected in header section **\n\n",
		    G_program_name());
		first = 0;
	}
	fprintf (stderr, "  %s\n", msg);
}
