/**
   \file line.cpp

   \brief Line manipulation

   This program is free software under the GNU General Public
   License (>=v2). Read the file COPYING that comes with GRASS
   for details.

   (C) 2008 by The GRASS development team

   \author Martin Landa <landa.martin gmail.com>

   \date 2008 
*/

extern "C" {
#include <grass/vedit.h>
#include <grass/dbmi.h>
}
#include "driver.h"
#include "digit.h"

/**
   \brief Add new vector feature

   \param type   feature type
   \param coords pairs of coordinates list (2D or 3D map)
   \param layer  layer number
   \param cat    category number
   \param bgmap  map of background map or NULL
   \param snap   snapping mode (see vedit.h)
   \param thresh threshold value for snapping

   \return 0 on success
   \return -1 on failure
*/
int Digit::AddLine(int type, std::vector<double> coords, int layer, int cat,
		   const char *bgmap, int snap, double threshold)
{
    int i, npoints;
    int newline;

    struct line_pnts *Points;
    struct line_cats *Cats;

    struct Map_info **BgMap; /* backgroud vector maps */
    int nbgmaps;             /* number of registrated background maps */

    if (!display->mapInfo) {
	return -1;
    }

    npoints = coords.size() / (Vect_is_3d(display->mapInfo) ? 3 : 2);
    if (coords.size() != npoints * (Vect_is_3d(display->mapInfo) ? 3 : 2)) {
	return -1;
    }

    G_debug(2, "wxDigit.AddLine(): npoints=%d, layer=%d, cat=%d, snap=%d",
	    npoints, layer, cat, snap);

    /* TODO: 3D */
    if (!(type & GV_POINTS) && !(type & GV_LINES)) {
	return -1;
    }

    if (layer < -1) {
	return -1;
    }

    BgMap = NULL;
    nbgmaps = 0;
    if (bgmap && strlen(bgmap) > 0) {
	BgMap = OpenBackgroundVectorMap(bgmap);
	if (!BgMap) {
	    return -1;
	}
	else {
	    nbgmaps = 1;
	}
    }

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    Vect_cat_set(Cats, layer, cat);

    if (cat > GetCategory(layer)) {
	SetCategory(layer, cat); /* set up max category for layer */
    }

    i = 0;
    while (i < coords.size()) {
	if (Vect_is_3d(display->mapInfo)) {
	    Vect_append_point(Points, coords[i], coords[i+1], coords[i+2]);
	    i += 3;
	}
	else {
	    Vect_append_point(Points, coords[i], coords[i+1], 0.0);
	    i += 2;
	}
    }

    if (type & GV_BOUNDARY) { /* close boundary */
	int last = Points->n_points-1;
	if (Vect_points_distance(Points->x[0], Points->x[0], Points->z[0],
				 Points->x[last], Points->x[last], Points->z[last],
				 Vect_is_3d(display->mapInfo)) <= threshold) {
	    Points->x[last] = Points->x[0];
	    Points->y[last] = Points->y[0];
	    Points->z[last] = Points->z[0];
	    G_debug(3, "wxDigit.AddLine(): boundary closed");
	}
    }

    if (snap != NO_SNAP) { /* apply snapping (node or vertex) */
	Vedit_snap_line(display->mapInfo, BgMap, nbgmaps,
			-1, Points,
			threshold, (SNAP) ? 0 : 1); 
    }

    if (Vect_write_line(display->mapInfo, type, Points, Cats) < 0) {
	return -1;
    }

    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);

    if (BgMap && BgMap[0]) {
	Vect_close(BgMap[0]);
    }

    G_debug(2, "wxDigit.AddLine(): line=%d written", newline);

    return 0;
}

/**
   \brief Rewrite given line

   \param line line id
   \param coords line geometry

   \return new line id
   \return -1 error
*/
int Digit::RewriteLine(int line, std::vector<double> coords)
{
    int ret, type, dim;
    double x, y, z;
    struct line_pnts *points;
    struct line_cats *cats;

    if (!display->mapInfo) {
	return -1;
    }

    /* line alive ? */
    if (!Vect_line_alive(display->mapInfo, line)) {
	return -1;
    }
    
    ret = 0;
    points = Vect_new_line_struct();
    cats = Vect_new_cats_struct();

    /* read line */
    type = Vect_read_line(display->mapInfo, NULL, cats, line);
    if (type < 0) {
	ret = -1;
    }

    /* define line geometry */
    if (Vect_is_3d(display->mapInfo)) {
	dim = 3;
    }
    else {
	dim = 2;
    }
    for(size_t i = dim - 1; i < coords.size(); i += dim) {
	if (dim == 2) {
	    Vect_append_point(points, coords[i-1], coords[i], 0.0);
	}
	else {
	    Vect_append_point(points, coords[i-2], coords[i-1], coords[i]);
	}
    }

    /* rewrite line */
    if (ret == 0) {
	if (Vect_rewrite_line(display->mapInfo, line, type, points, cats) < 0) {
	    ret = -1;
	}
    }

    Vect_destroy_line_struct(points);
    Vect_destroy_cats_struct(cats);

    return ret;
}

/**
   \brief Split/break selected line

   Shape of line is not changed.

   \param x,y,z coordinates (z is used only if map is 3d)
   \param thresh threshold value to find a point on line
*/
int Digit::SplitLine(double x, double y, double z,
		     double thresh)
{
    int ret;
    struct line_pnts *point;

    if (!display->mapInfo)
	return -1;

    point = Vect_new_line_struct();
    Vect_append_point(point, x, y, z);

    ret = Vedit_split_lines(display->mapInfo, display->selected,
			    point, thresh, NULL);

    Vect_destroy_line_struct(point);

    return ret;
}

/**
   \brief Delete selected vector features

   \param delete_records delete also attribute records

   \return number of deleted lines
   \return -1 on error
*/
int Digit::DeleteLines(bool delete_records)
{
    int ret;
    int n_dblinks;
    struct line_cats *Cats, *Cats_del;
    // struct ilist *List;

    if (!display->mapInfo) {
	return -1;
    }

    n_dblinks = Vect_get_num_dblinks(display->mapInfo);
    Cats_del = NULL;
    // List = NULL;

    /* collect categories if needed */
    if (delete_records) {
	Cats = Vect_new_cats_struct();
	// List = Vect_new_list();
	Cats_del = Vect_new_cats_struct();
	for (int i = 0; i < display->selected->n_values; i++) {
	    if (Vect_read_line(display->mapInfo, NULL, Cats, display->selected->value[i]) < 0) {
		Vect_destroy_cats_struct(Cats_del);
		//Vect_destroy_list(List);
		return -1;
	    }
	    for (int j = 0; j < Cats->n_cats; j++) {
		/*
		  To find other vector objects with the same category,
		  category index is need to be updated (i.e. to
		  rebuild topo, sidx, cidx). This can be time-consuming
		  task for large vector maps.
		*/
		/*
		  Vect_build(display->mapInfo, NULL);
		  Vect_cidx_find_all(display->mapInfo, Cats->field[j],
		  GV_POINTS | GV_LINES, Cats->cat[j],
		  List);
		if (List->n_values == 1 &&
		    List->value[0] == display->selected->value[i]) {
		    Vect_cat_set(Cats_del, Cats->field[j], Cats->cat[j]);
		}
		*/
		Vect_cat_set(Cats_del, Cats->field[j], Cats->cat[j]);
	    }
	}
	Vect_destroy_cats_struct(Cats);
    }

    ret = Vedit_delete_lines(display->mapInfo, display->selected);

    if (ret > 0 && delete_records) {
	struct field_info *fi;
	char buf[GSQL_MAX];
	dbDriver *driver;
	dbHandle handle;
	dbString stmt;

	for (int dblink = 0; dblink < n_dblinks; dblink++) {
	    fi = Vect_get_dblink(display->mapInfo, dblink);
	    if (fi == NULL) {
		return -1;
	    }

	    driver = db_start_driver(fi->driver);
	    if (driver == NULL) {
		return -1;
	    }

	    db_init_handle (&handle);
	    db_set_handle (&handle, fi->database, NULL);
	    if (db_open_database(driver, &handle) != DB_OK) {
		return -1;
	    }

	    db_init_string (&stmt);
	    sprintf (buf, "DELETE FROM %s WHERE", fi->table);
	    db_set_string(&stmt, buf);
	    int n_cats = 0;
	    for (int c = 0; c < Cats_del->n_cats; c++) {
		if (Cats_del->field[c] == fi->number) {
		    if (n_cats > 0) {
			sprintf (buf, " or");
			db_append_string(&stmt, buf);
		    }
		    sprintf (buf, " %s = %d", fi->key, Cats_del->cat[c]);
		    db_append_string(&stmt, buf);
		    n_cats++;
		}
	    }

	    Vect_cat_del(Cats_del, fi->number);

	    if (n_cats &&
		db_execute_immediate (driver, &stmt) != DB_OK ) {
		return -1;
	    }
	    
	    db_close_database(driver);
	    db_shutdown_driver(driver);
	}
    }

    /* update category settings */
    // InitCats();

    if (Cats_del) {
	Vect_destroy_cats_struct(Cats_del);
    }

    /*
    if(List) {
	Vect_destroy_list(List);
    }
    */
    return ret;
}


/** 
    \brief Move selected vector features

    \param move_x,move_y,move_z move direction (move_z is used only if map is 3D)
    \param bgmap  map of background map or NULL
    \param snap snapping move (see vedit.h)
    \param thresh threshold value for snapping

    \return number of moved features
    \return -1 on error
*/
int Digit::MoveLines(double move_x, double move_y, double move_z,
		     const char *bgmap, int snap, double thresh)
{
    int ret;
    struct Map_info **BgMap; /* backgroud vector maps */
    int nbgmaps;             /* number of registrated background maps */

    if (!display->mapInfo)
	return -1;

    BgMap = NULL;
    nbgmaps = 0;
    if (bgmap && strlen(bgmap) > 0) {
	BgMap = OpenBackgroundVectorMap(bgmap);
	if (!BgMap) {
	    return -1;
	}
	else {
	    nbgmaps = 1;
	}
    }

    ret = Vedit_move_lines(display->mapInfo, BgMap, nbgmaps,
			   display->selected,
			   move_x, move_y, move_z,
			   snap, thresh);

    if (BgMap && BgMap[0]) {
	Vect_close(BgMap[0]);
    }

    return ret;
}

/**
   \brief Flip selected lines/boundaries

   \return number of modified lines
   \return -1 on error
*/
int Digit::FlipLines()
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    /*
      ret = Vedit_flip_lines(display->mapInfo, display->selected);
    */

    return ret;
}

/**
   \brief Merge selected lines/boundaries

   \return number of modified lines
   \return -1 on error
*/
int Digit::MergeLines()
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    ret = Vedit_merge_lines(display->mapInfo, display->selected);

    return ret;
}

/**
   \brief Breaks selected lines/boundaries

   \return number of modified lines
   \return -1 on error
*/
int Digit::BreakLines()
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    ret = Vect_break_lines_list(display->mapInfo, display->selected,
				GV_LINES, NULL, NULL);

    return ret;
}

/**
   \brief Snap selected lines/boundaries

   \param thresh threshold value for snapping

   \return number of modified lines
   \return -1 on error
*/
int Digit::SnapLines(double thresh)
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    Vect_snap_lines_list (display->mapInfo, display->selected,
			  thresh, NULL, NULL);

    return ret;
}

/**
   \brief Connect two selected lines/boundaries

   \return 1 lines connected
   \return 0 lines not connected
   \return -1 on error
*/
int Digit::ConnectLines(double thresh)
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    ret = Vedit_connect_lines(display->mapInfo, display->selected,
			      thresh);

    return ret;
}

/**
   \brief Automated labeling (z coordinate assignment) of vector lines (contours).

   Modified vector map must be 3D.

   \param x1,y1,x2,y2 line nodes for intersection
   \param start starting value
   \param step  step value for labeling

   \return number of modified lines
   \return -1 on error
*/
int Digit::ZBulkLabeling(double x1, double y1, double x2, double y2,
			 double start, double step)
{
    int ret;

    if (!display->mapInfo) {
	return -1;
    }

    ret = Vedit_bulk_labeling (display->mapInfo, display->selected,
			       x1, y1, x2, y2, start, step);

    return ret;
}

/**
   \brief Copy vector features

   \param ids line ids to be copied (if not given selected are used)
   \param bgmap name of background map (if not given, copy features from input)

   \return number of copied features
   \return -1 on error
*/
int Digit::CopyLines(std::vector<int> ids, const char* bgmap_name)
{
    int ret;
    struct Map_info *bgMap;
    struct ilist *list;

    bgMap = NULL;
    list = NULL;

    if (!display->mapInfo) {
	return -1;
    }


    if (bgmap_name) {
	const char *mapset;
	bgMap = (struct Map_info *) G_malloc(sizeof (struct Map_info));
	mapset = G_find_vector2 (bgmap_name, ""); 
	Vect_open_old(bgMap, (char *) bgmap_name, (char *) mapset); /* TODO */
    }

    if (!ids.empty()) {
	list = Vect_new_list();
	for (std::vector<int>::const_iterator b = ids.begin(), e = ids.end();
	     b != e; ++b) {
	    Vect_list_append(list, *b);
	}
    }
    else {
	list = display->selected;
    }

    Vedit_copy_lines (display->mapInfo, bgMap,
		      list);

    if (list != display->selected) {
	Vect_destroy_list(list);
    }

    if (bgMap) {
	Vect_close(bgMap);
	G_free ((void *) bgMap);
    }

    return ret;
}

/**
   \brief Open background vector map

   @todo support more background maps then only one

   \param bgmap pointer to vector map name

   \return vector map array
   \return NULL on error
*/
struct Map_info** Digit::OpenBackgroundVectorMap(const char *bgmap)
{
    char name[GNAME_MAX];
    char mapset[GMAPSET_MAX];
    int nbgmaps;
    struct Map_info** BgMap;

    if (!display->mapInfo) {
	return NULL;
    }

    if (G_find_vector2 (bgmap, "") == NULL) {
	return NULL;
    }
    
    nbgmaps = 0;

    if (!G__name_is_fully_qualified(bgmap, name, mapset)) {
	G_strncpy(name, bgmap, GNAME_MAX);
	mapset[0] = '\0';
    }
    if (strcmp(G_fully_qualified_name((const char*) display->mapInfo->name, (const char*) G_mapset()),
	       G_fully_qualified_name((const char*) bgmap, (const char*) mapset))) {
	nbgmaps = 1;
	BgMap = (struct Map_info**) G_malloc (nbgmaps * sizeof(struct Map_info*));
	BgMap[nbgmaps-1] = (struct Map_info *) G_malloc (sizeof(struct Map_info));
	
	// avoid GUI crash
	Vect_set_fatal_error(GV_FATAL_PRINT);
	
	if (Vect_open_old(BgMap[nbgmaps-1], name, mapset) == -1) {
	    G_free ((void *) BgMap[nbgmaps-1]);
	    BgMap = NULL;
	}
    }
    
    return BgMap;
}

/**
   \brief Type conversion of selected features

   Supported conversions:
    - point <-> centroid
    - line <-> boundary

   \return number of modified features
   \return -1 on error
*/
int Digit::TypeConvLines()
{
    int nlines, line;
    int type, newtype;
    struct line_pnts *Points;
    struct line_cats *Cats;

    if (!display->mapInfo) {
	return -1;
    }

    nlines = 0;

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    for (int i = 0; i < display->selected->n_values; i++) {
	line = display->selected->value[i];
	if (!Vect_line_alive(display->mapInfo, line))
	    continue;
	type = Vect_read_line(display->mapInfo, Points, Cats, line);
	if (type < 0) {
	    return -1;
	}
	switch (type) {
	case GV_POINT:
	    newtype = GV_CENTROID;
	    break;
	case GV_CENTROID:
	    newtype = GV_POINT;
	    break;
	case GV_LINE:
	    newtype = GV_BOUNDARY;
	    break;
	case GV_BOUNDARY:
	    newtype = GV_LINE;
	    break;
	default:
	    newtype = -1;
	    break;
	}
	
	G_debug(3, "Digit.TypeConvLines(): line=%d, from_type=%d, to_type=%d",
		line, type, newtype);
	
	if (newtype > 0) {
	    if (Vect_rewrite_line(display->mapInfo, line, newtype, Points, Cats) < 0) {
		return -1;
	    }
	    nlines++;
	}
    }
    
    Vect_destroy_line_struct(Points);
    Vect_destroy_cats_struct(Cats);

    return nlines;
}
