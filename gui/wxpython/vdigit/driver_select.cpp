/**
   \file driver_selected.cpp
   
   \brief Experimental C++ wxWidgets display driver

   This driver is designed for wxPython GRASS GUI (digitization tool).
   Draw vector map layer to PseudoDC.

   (C) by the GRASS Development Team
   This program is free software under the GNU General Public
   License (>=v2). Read the file COPYING that comes with GRASS
   for details.

   \author Martin Landa <landa.martin gmail.com>

   \date 2007-2008 
*/

#include <cmath>

#include "driver.h"

/**
   \brief Select vector objects by given bounding box
   
   If line id is already in the list of selected lines, then it will
   be excluded from this list.


   \param[in] x1,y1,z1,x2,y2,z3 bounding box definition
   \param[in] type feature type
   \param[in] onlyInside if true select only features inside
   of bounding box (not overlapping)

   \return number of selected features
   \return -1 on error
*/
int DisplayDriver::SelectLinesByBox(double x1, double y1, double z1, 
				    double x2, double y2, double z2,
				    int type, bool onlyInside)
{
    if (!mapInfo)
	return -1;

    int line;

    struct ilist *list;
    struct line_pnts *bbox;

    drawSegments = false;
    drawSelected = true;

    selected.isId = true;

    list = Vect_new_list();
    bbox = Vect_new_line_struct();

    Vect_append_point(bbox, x1, y1, z1);
    Vect_append_point(bbox, x2, y1, z2);
    Vect_append_point(bbox, x2, y2, z1);
    Vect_append_point(bbox, x1, y2, z2);
    Vect_append_point(bbox, x1, y1, z1);
        
    Vect_select_lines_by_polygon(mapInfo, bbox,
				 0, NULL, /* isles */
				 type, list);

    for (int i = 0; i < list->n_values; i++) {
	line = list->value[i];
	if (onlyInside) {
	    bool inside;
	    inside = true;
	    Vect_read_line(mapInfo, points, cats, line);
	    for (int p = 0; p < points->n_points; p++) {
		if (!Vect_point_in_poly(points->x[p], points->y[p],
					bbox)) {
		    inside = false;
		    break;
		}
	    }
	    if (!inside)
		continue; /* skip lines just overlapping bbox */
	}
	
	if (!IsSelected(line)) {
	    Vect_list_append(selected.values, line);
	}
	else {
	    Vect_list_delete(selected.values, line);
	}
    }

    Vect_destroy_line_struct(bbox);
    Vect_destroy_list(list);

    return list->n_values;
}

/**
   \brief Select vector feature by given point in given
   threshold
   
   Only one vector object can be selected. Bounding boxes of
   all segments are stores.

   \param[in] x,y point of searching
   \param[in] thresh threshold value where to search
   \param[in] type select vector object of given type

   \return point on line if line found
*/
std::vector<double> DisplayDriver::SelectLineByPoint(double x, double y, double z,
						     double thresh, int type, int with_z)
{
    long int line;
    double px, py, pz;

    std::vector<double> p;

    drawSelected = true;

    selected.isId = true;

    line = Vect_find_line(mapInfo, x, y, z,
			  type, thresh, with_z, 0);

    if (line > 0) {
	if (!IsSelected(line)) {
	    Vect_list_append(selected.values, line);
	}
	else {
	    Vect_list_delete(selected.values, line);
	}

	type = Vect_read_line (mapInfo, points, cats, line);
	Vect_line_distance (points, x, y, z, with_z,
			    &px, &py, &pz,
			    NULL, NULL, NULL);
	p.push_back(px);
	p.push_back(py);
	if (with_z) {
	    p.push_back(pz);
	}
    }

    drawSegments = true;

    return p;
}

/**
   \brief Is vector object selected?
   
   \param[in] line id

   \return true if vector object is selected
   \return false if vector object is not selected
*/
bool DisplayDriver::IsSelected(int line)
{
    if (selected.isId) {
	if (Vect_val_in_list(selected.values, line))
	    return true;
    }
    else {
	for (int i = 0; i < cats->n_cats; i++) {
	    if (cats->field[i] == 1 &&
		Vect_val_in_list(selected.values, cats->cat[i])) 
		return true;
	}
    }
    
    return false;
}

/**
   \brief Get ids of selected objects

   \param[in] grassId if true return GRASS line ids
   if false return PseudoDC ids
   
   \return list of ids of selected vector objects
*/
std::vector<int> DisplayDriver::GetSelected(bool grassId)
{
    if (grassId)
	return ListToVector(selected.values);

    std::vector<int> dc_ids;

    if (!drawSegments) {
	dc_ids.push_back(1);
    }
    else {
	int npoints;
	Vect_read_line(mapInfo, points, NULL, selected.values->value[0]);
	npoints = points->n_points;
	for (int i = 1; i < 2 * npoints; i++) {
	  dc_ids.push_back(i);
	}
    }

    return dc_ids;
}

/**
   \brief Get feature (grass) ids of duplicated objects

   \return list of ids
*/
std::map<int, std::vector <int> > DisplayDriver::GetDuplicates()
{
    std::map<int, std::vector<int> > ids;

    struct line_pnts *APoints, *BPoints;
 
    int line;

    APoints = Vect_new_line_struct();
    BPoints = Vect_new_line_struct();

    Vect_reset_list(selected.valuesDupl);

    for (int i = 0; i < selected.values->n_values; i++) {
	line = selected.values->value[i];
	if (IsDuplicated(line))
	    continue;
	
	Vect_read_line(mapInfo, APoints, NULL, line);
	
	for (int j = 0; j < selected.values->n_values; j++) {
	    if (i == j || IsDuplicated(selected.values->value[j]))
		continue;
	    
	    Vect_read_line(mapInfo, BPoints, NULL, selected.values->value[j]);
	    
	    if (Vect_line_check_duplicate (APoints, BPoints, WITHOUT_Z)) {
		if (ids.find(i) == ids.end()) {
		    ids[i] = std::vector<int> ();
		    ids[i].push_back(selected.values->value[i]);
		    Vect_list_append(selected.valuesDupl, selected.values->value[i]);
		}
		ids[i].push_back(selected.values->value[j]);
		Vect_list_append(selected.valuesDupl, selected.values->value[j]);
	    }
	}
    }
    
    Vect_destroy_line_struct(APoints);
    Vect_destroy_line_struct(BPoints);
    
    return ids;
}

/**
   \brief Check for already marked duplicates

   \param line line id

   \return 1 line already marked as duplicated
   \return 0 not duplicated
*/
bool DisplayDriver::IsDuplicated(int line)
{
    if (Vect_val_in_list(selected.valuesDupl, line))
	return true;
    
    return false;
}

/**
   \brief Set selected vector objects
   
   \param[in] list of GRASS ids to be set
   \param[in] cat if True expect categories instead of feature ids

   \return 1
*/
int DisplayDriver::SetSelected(std::vector<int> id, bool cat)
{
    drawSelected = true;

    VectorToList(selected.values, id);
    
    selected.isId = !cat;

    if (selected.values->n_values <= 0)
	drawSegments = false;

    return 1;
}

/**
   \brief Unselect selected vector features
   
   \param[in] list of GRASS feature ids

   \return number of selected features
*/
int DisplayDriver::UnSelect(std::vector<int> id)
{
    bool checkForDupl;

    checkForDupl = false;

    for (std::vector<int>::const_iterator i = id.begin(), e = id.end();
	 i != e; ++i) {
	if (IsSelected(*i)) {
	    Vect_list_delete(selected.values, *i);
	}
	if (settings.highlightDupl.enabled && IsDuplicated(*i)) {
	    checkForDupl = true;
	}
    }

    if (checkForDupl) {
	GetDuplicates();
    }

    return selected.values->n_values;
}

/**
   \brief Get PseudoDC vertex id of selected line

   Set bounding box for vertices of line.

   \param[in] x,y coordinates of click
   \param[in] thresh threshold value

   \return id of center, left and right vertex

   \return 0 no line found
   \return -1 on error
*/
std::vector<int> DisplayDriver::GetSelectedVertex(double x, double y, double thresh)
{
    int startId;
    int line, type;
    int Gid, DCid;
    double vx, vy, vz;      // vertex screen coordinates

    double dist, minDist;

    std::vector<int> returnId;

    // only one object can be selected
    if (selected.values->n_values != 1 || !drawSegments) 
	return returnId;

    startId = 1;
    line = selected.values->value[0];

    type = Vect_read_line (mapInfo, points, cats, line);

    minDist = 0.0;
    Gid = -1;
    // find the closest vertex (x, y)
    DCid = 1;
    for(int idx = 0; idx < points->n_points; idx++) {
	dist = Vect_points_distance(x, y, 0.0,
				    points->x[idx], points->y[idx], points->z[idx], 0);
	
	if (idx == 0) {
	    minDist = dist;
	    Gid  = idx;
	}
	else {
	    if (minDist > dist) {
		minDist = dist;
		Gid = idx;
	    }
	}

	Cell2Pixel(points->x[idx], points->y[idx], points->z[idx],
		   &vx, &vy, &vz);
	wxRect rect (wxPoint ((int) vx, (int) vy), wxPoint ((int) vx, (int) vy));
	dc->SetIdBounds(DCid, rect);
	DCid+=2;
    }	

    if (minDist > thresh)
	return returnId;

    // desc = &(ids[line]);

    // translate id
    DCid = Gid * 2 + 1;

    // add selected vertex
    returnId.push_back(DCid);
    // left vertex
    if (DCid == startId) {
	returnId.push_back(-1);
    }
    else {
	returnId.push_back(DCid - 2);
    }

    // right vertex
    if (DCid == (points->n_points - 1) * 2 + startId) {
	returnId.push_back(-1);
    }
    else {
	returnId.push_back(DCid + 2);
    }

    return returnId;
}

/*!
  \brief Get minimal region extent of selected features

  \return n,s,w,e
*/
std::vector<int> DisplayDriver::GetRegionSelected()
{
    std::vector<int> region;

    BOUND_BOX region_box, line_box;
    struct ilist *list, *list_tmp;

    list = list_tmp = NULL;

    G_zero(&region_box, sizeof(region_box));
    
    if (!selected.isId) { /* -> cats */
	list = Vect_new_list();
	list_tmp = Vect_new_list();
	/* can't use here
	 *
	  Vect_cidx_find_all(mapInfo, 1, GV_POINTS | GV_LINES,
	  selected.values->value[i],
	  list_tmp);
	*/
	int type;
	bool found;
	for (int line = 1; line < Vect_get_num_lines(mapInfo); line++) {
	    type = Vect_read_line (mapInfo, NULL, cats, line);
	    if (!(type & (GV_POINTS | GV_LINES)))
		continue;
	    
	    found = false;
	    for (int i = 0; i < cats->n_cats && !found; i++) {
		for (int j = 0; j < selected.values->n_values && !found; j++) {
		    if (cats->cat[i] == selected.values->value[j])
			found = true;
		}
	    }
	    if (found)
		Vect_list_append(list, line);
	}
    }
    else {
	list = selected.values;
    }
    
    for (int i = 0; i < list->n_values; i++) {
	if (!Vect_get_line_box(mapInfo, list->value[i], &line_box))
	    continue;

	if (i == 0) {
	    Vect_box_copy(&region_box, &line_box);
	}
	else {
	    Vect_box_extend(&region_box, &line_box);
	}
    }

    if (list && list != selected.values) {
	Vect_destroy_list(list);
    }
    if (list_tmp)
	Vect_destroy_list(list_tmp);
	
    region.push_back(region_box.N);
    region.push_back(region_box.S);
    region.push_back(region_box.W);
    region.push_back(region_box.E);

    return region;
}
