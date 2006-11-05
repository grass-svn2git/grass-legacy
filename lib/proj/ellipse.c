/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:       gproj library
 * AUTHOR(S):    Original Author unknown, probably CERL
 *               Paul Kelly - paul-grass@stjohnspoint.co.uk
 * PURPOSE:      provide functions for reading ellipsoid parameters from the
 *               location database.     
 * COPYRIGHT:    (C) 2000, 2003 by the GRASS Development Team
 *
 *               This program is free software under the GNU General Public
 *               License (>=v2). Read the file COPYING that comes with GRASS
 *               for details.
 *
 *****************************************************************************/

#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>		/* for sqrt() */
#include <grass/gis.h>
#include <grass/glocale.h>
#include <grass/gprojects.h>
#include "local_proto.h"

static int get_a_e2_f(const char *, const char *, double *, double *, double *);

/**
 * This routine returns the ellipsoid parameters from the database.
 * If the PROJECTION_FILE exists in the PERMANENT mapset, read info from
 * that file, otherwise return WGS 84 values.
 *
 * \return 1 ok, 0 default values used.
 *         Dies with diagnostic if there is an error
 **/
int GPJ_get_ellipsoid_params(double *a, double *e2, double *rf)
{
    int ret;
    struct Key_Value *proj_keys = G_get_projinfo();

    if (proj_keys == NULL)
	proj_keys = G_create_key_value();

    ret = GPJ__get_ellipsoid_params(proj_keys, a, e2, rf);
    G_free_key_value(proj_keys);

    return ret;
}

int
GPJ__get_ellipsoid_params(struct Key_Value *proj_keys,
			  double *a, double *e2, double *rf)
{
    struct gpj_ellps estruct;
    struct gpj_datum dstruct;
    char *err, *str, *str1, *str3;

    str = G_find_key_value("datum", proj_keys);

    if ((str != NULL) && (GPJ_get_datum_by_name(str, &dstruct) > 0)) {
	/* If 'datum' key is present, look up correct ellipsoid
	 * from datum.table */

	str = G_store(dstruct.ellps);
	GPJ_free_datum(&dstruct);

    }
    else
	/* else use ellipsoid defined in PROJ_INFO */
	str = G_find_key_value("ellps", proj_keys);

    if (str != NULL) {
	if (GPJ_get_ellipsoid_by_name(str, &estruct) < 0) {
	    G_asprintf(&err, _("invalid ellipsoid %s in file")
		       , str);
	    G_fatal_error(err);
	}
	else {
	    *a = estruct.a;
	    *e2 = estruct.es;
	    *rf = estruct.rf;
	    GPJ_free_ellps(&estruct);
	    return 1;
	}
    }
    else {
	str3 = G_find_key_value("a", proj_keys);
	if (str3 != NULL) {
	    G_asprintf(&str, "a=%s", str3);
	    if ((str3 = G_find_key_value("es", proj_keys)) != NULL)
		G_asprintf(&str1, "e=%s", str3);
	    else if ((str3 = G_find_key_value("f", proj_keys)) != NULL)
		G_asprintf(&str1, "f=1/%s", str3);
	    else if ((str3 = G_find_key_value("rf", proj_keys)) != NULL)
		G_asprintf(&str1, "f=1/%s", str3);
	    else if ((str3 = G_find_key_value("b", proj_keys)) != NULL)
		G_asprintf(&str1, "b=%s", str3);
	    else
		G_fatal_error
		    ("No secondary ellipsoid descriptor (rf, es or b) in file");

	    if (get_a_e2_f(str, str1, a, e2, rf) == 0)
		G_fatal_error
		    ("Invalid ellipsoid descriptors (a, rf, es or b) in file");
	    *rf = 1.0 / *rf;
	    return 1;
	}
	else {
	    str = G_find_key_value("proj", proj_keys);
	    if ((str == NULL) || (strcmp(str, "ll") == 0)) {
		*a = 6378137.0;
		*e2 = .006694385;
		*rf = 298.257223563;
		return 0;
	    }
	    else {
		G_asprintf(&err, _("No ellipsoid info given in file")
		    );
		G_fatal_error(err);
	    }
	}
    }
    return 1;
}


/**
 * \brief looks up ellipsoid in ellipsoid table and returns the
 * a, e2 parameters for the ellipsoid
 *
 * \return 1 if ok,
 *         -1 if not found in table
 */

int GPJ_get_ellipsoid_by_name(const char *name, struct gpj_ellps *estruct)
{
    struct ellps_list *list, *listhead;

    list = listhead = read_ellipsoid_table(0);

    while (list != NULL) {
	if (strcasecmp(name, list->name) == 0) {
	    estruct->name = G_store(list->name);
	    estruct->longname = G_store(list->longname);
	    estruct->a = list->a;
	    estruct->es = list->es;
	    estruct->rf = list->rf;
	    free_ellps_list(listhead);
	    return 1;
	}
	list = list->next;
    }
    free_ellps_list(listhead);
    return -1;
}

static int
get_a_e2_f(const char *s1, const char *s2, double *a, double *e2, double *f)
{
    double b, recipf;

    if (sscanf(s1, "a=%lf", a) != 1)
	return 0;

    if (*a <= 0.0)
	return 0;

    if (sscanf(s2, "e=%lf", e2) == 1) {
	*f = (double)1.0 - sqrt((double)1.0 - *e2);
	return (*e2 >= 0.0);
    }

    if (sscanf(s2, "f=1/%lf", &recipf) == 1) {
	if (recipf <= 0.0)
	    return 0;
	*f = (double)1.0 / (recipf);
	*e2 = *f + *f - *f * *f;
	return (*e2 >= 0.0);
    }

    if (sscanf(s2, "b=%lf", &b) == 1) {
	if (b <= 0.0)
	    return 0;
	if (b == *a) {
	    *f = 0.0;
	    *e2 = 0.0;
	}
	else {
	    *f = ((*a) - b) / (*a);
	    *e2 = *f + *f - *f * *f;
	}
	return (*e2 >= 0.0);
    }
    return 0;
}

struct ellps_list *read_ellipsoid_table(int fatal)
{
    FILE *fd;
    char *file;
    char *errbuf;
    char buf[4096];
    char name[100], descr[1024], buf1[1024], buf2[1024];
    char badlines[1024];
    int line;
    int err;
    struct ellps_list *current = NULL, *outputlist = NULL;
    double a, e2, f;

    int count = 0;

    G_asprintf(&file, "%s%s", G_gisbase(), ELLIPSOIDTABLE);
    fd = fopen(file, "r");

    if (fd == NULL) {
	perror(file);
	G_asprintf(&errbuf, _("unable to open ellipsoid table file: %s"), file);
	fatal ? G_fatal_error(errbuf) : G_warning(errbuf);
	G_free(errbuf);
	return 0;
    }

    err = 0;
    *badlines = 0;
    for (line = 1; G_getl2(buf, sizeof buf, fd); line++) {
	G_strip(buf);
	if (*buf == 0 || *buf == '#')
	    continue;

	if (sscanf(buf, "%s  \"%1023[^\"]\" %s %s", name, descr, buf1, buf2) !=
	    4) {
	    err++;
	    sprintf(buf, " %d", line);
	    if (*badlines)
		G_strcat(badlines, ",");
	    G_strcat(badlines, buf);
	    continue;
	}


	if (get_a_e2_f(buf1, buf2, &a, &e2, &f)
	    || get_a_e2_f(buf2, buf1, &a, &e2, &f)) {
	    if (current == NULL)
		current = outputlist = G_malloc(sizeof(struct ellps_list));
	    else
		current = current->next = G_malloc(sizeof(struct ellps_list));
	    current->name = G_store(name);
	    current->longname = G_store(descr);
	    current->a = a;
	    current->es = e2;
	    current->rf = (1 / f);
	    current->next = NULL;
	    count++;
	}
	else {
	    err++;
	    sprintf(buf, " %d", line);
	    if (*badlines)
		G_strcat(badlines, ",");
	    G_strcat(badlines, buf);
	    continue;
	}
    }
    if (!err)
	return outputlist;

    G_asprintf(&errbuf,
	       (err == 1 ? "Line%s of ellipsoid table file <%s> is invalid"
		: "Lines%s of ellipsoid table file <%s> are invalid"),
	       badlines, file);
    fatal ? G_fatal_error(errbuf) : G_warning(errbuf);
    G_free(errbuf);
    G_free(file);
    return outputlist;
}

void GPJ_free_ellps(struct gpj_ellps *estruct)
{
    G_free(estruct->name);
    G_free(estruct->longname);
    return;
}

void free_ellps_list(struct ellps_list *elist)
{
    struct ellps_list *old;

    while (elist != NULL) {
	G_free(elist->name);
	G_free(elist->longname);
	old = elist;
	elist = old->next;
	G_free(old);
    }

    return;
}
