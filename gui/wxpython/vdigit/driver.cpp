/**
   \file driver.cpp
   
   \brief Experimental C++ wxWidgets display driver

   This driver is designed for wxPython GRASS GUI (digitization tool).
   Draw vector map layer to PseudoDC.

   This program is free software under the GNU General Public
   License (>=v2). Read the file COPYING that comes with GRASS
   for details.

   \author (C) by the GRASS Development Team
   Martin Landa <landa.martin gmail.com>

   \date 2007-2008 
*/

#include "driver.h"

/**
   \brief Initialize driver

   Allocate given structures.
   
   \param[in,out] PseudoDC device where to draw vector objects
   
   \return
*/
DisplayDriver::DisplayDriver(void *device)
{
    G_gisinit(""); /* GRASS functions */

    mapInfo = NULL;

    dc = (wxPseudoDC *) device;

    points = Vect_new_line_struct();
    pointsScreen = new wxList();
    cats = Vect_new_cats_struct();
    
    selected = Vect_new_list();

    drawSegments = false;

    // avoid GUI crash when G_fatal_error() is called (opening the vector map)
    Vect_set_fatal_error(GV_FATAL_PRINT);
    // G_set_error_routine(print_error);
}

/**
   \brief Destroy driver

   Close the map, deallocate given structures.

   \param

   \return
*/
DisplayDriver::~DisplayDriver()
{
    if (mapInfo)
	CloseMap();

    Vect_destroy_line_struct(points);
    delete pointsScreen;
    Vect_destroy_cats_struct(cats);
    Vect_destroy_list(selected);
}

/**
   \brief Set device for drawing
   
   \param[in,out] PseudoDC device where to draw vector objects

   \return
*/
void DisplayDriver::SetDevice(void *device)
{
    dc = (wxPseudoDC *) device;

    return;
}

/**
   \brief Draw content of the vector map to device
   
   \return number of lines which were drawn
   \return -1 on error
 */
int DisplayDriver::DrawMap(bool force)
{
    if (!mapInfo || !dc)
	return -1;

    int nlines;
    BOUND_BOX mapBox;
    struct ilist *listLines;

    // ids.clear();
    listLines = Vect_new_list();

    ResetTopology();

    /* nlines = Vect_get_num_lines(mapInfo); */

    Vect_get_map_box(mapInfo, &mapBox);

    // draw lines inside of current display region
    nlines = Vect_select_lines_by_box(mapInfo, &(region.box),
     				      GV_POINTS | GV_LINES, // fixme
				      listLines);

    G_debug(3, "wxDriver.DrawMap(): region: w=%f, e=%f, s=%f, n=%f",
	    region.box.W, region.box.E, region.box.S, region.box.N);

    bool inBox;
    dc->BeginDrawing();
    for (int i = 0; i < listLines->n_values; i++) {
	DrawLine(listLines->value[i]);
    }
    dc->EndDrawing();

    // PrintIds();

    Vect_destroy_list(listLines);

    return listLines->n_values;
}	

/**
   \brief Draw selected vector objects to the device
 
   \param[in] line id
   \return 1 on success
   \return -1 on failure (vector object is dead, etc.)
*/
int DisplayDriver::DrawLine(int line)
{
    if (!dc || !Vect_line_alive (mapInfo, line))
	return -1;

    int dcId;    // 0 | 1 | segment id
    int type;    // line type
    double x, y, z; // screen coordinates
    bool draw;   // draw object ?

    wxPen *pen;

    // read line
    type = Vect_read_line (mapInfo, points, cats, line);

    // add ids
    // -> node1, line1, vertex1, line2, ..., node2
    // struct lineDesc desc = {points->n_points, dcId};
    // ids[line] = desc;
    // update id for next line
    // dcId += points->n_points * 2 - 1;

    if (IsSelected(line)) { // line selected ?
	pen = new wxPen(settings.highlight, settings.lineWidth, wxSOLID);
	draw = true;
	dcId = 1;
	topology.highlight++;
    }
    else {
	dcId = 0;
	if (type & GV_LINES) {
	    switch (type) {
	    case GV_LINE:
		pen = new wxPen(settings.line.color, settings.lineWidth, wxSOLID);
		topology.line++;
		draw = settings.line.enabled;
		break;
	    case GV_BOUNDARY:
		int left, right;
		Vect_get_line_areas(mapInfo, line,
				    &left, &right);
		if (left == 0 && right == 0) {
		    pen = new wxPen(settings.boundaryNo.color, settings.lineWidth, wxSOLID);
		    topology.boundaryNo++;
		    draw = settings.boundaryNo.enabled;
		}
		else if (left > 0 && right > 0) {
		    pen = new wxPen(settings.boundaryTwo.color, settings.lineWidth, wxSOLID);
		    topology.boundaryTwo++;
		    draw = settings.boundaryTwo.enabled;
		}
		else {
		    pen = new wxPen(settings.boundaryOne.color, settings.lineWidth, wxSOLID);
		    topology.boundaryOne++;
		    draw = settings.boundaryOne.enabled;
		}
		break;
	    default:
		draw = false;
		break;
	    }
	}
	else if (type & GV_POINTS) {
	    if (type == GV_POINT && settings.point.enabled) {
		pen = new wxPen(settings.point.color, settings.lineWidth, wxSOLID);
		topology.point++;
		draw = settings.point.enabled;
	    }
	    else if (type == GV_CENTROID) {
		int cret = Vect_get_centroid_area(mapInfo, line);
		if (cret > 0) { // -> area
		    draw = settings.centroidIn.enabled;
		    pen = new wxPen(settings.centroidIn.color, settings.lineWidth, wxSOLID);
		    topology.centroidIn++;
		}
		else if (cret == 0) {
		    draw = settings.centroidOut.enabled;
		    pen = new wxPen(settings.centroidOut.color, settings.lineWidth, wxSOLID);
		    topology.centroidOut++;
		}
		else {
		    draw = settings.centroidDup.enabled;
		    pen = new wxPen(settings.centroidDup.color, settings.lineWidth, wxSOLID);
		    topology.centroidDup++;
		}
	    }
	}
    }
    
    // clear screen points & convert EN -> xy
    pointsScreen->Clear();
    for (int i = 0; i < points->n_points; i++) {
	Cell2Pixel(points->x[i], points->y[i], points->z[i],
		   &x, &y, &z);
	pointsScreen->Append((wxObject*) new wxPoint((int) x, (int) y)); /* TODO: 3D */
    }
    
    dc->SetId(dcId); /* 0 | 1 (selected) */
    dc->SetPen(*pen);

    if (draw) {
	if (type & GV_POINTS) {
	    DrawCross(line, (const wxPoint *) pointsScreen->GetFirst()->GetData());
	}
	else {
	    // long int startId = ids[line].startId + 1;
	    if (dcId > 0 && drawSegments) {
		dcId = 2; // first segment
		for (int i = 0; i < pointsScreen->GetCount() - 1; dcId += 2) {
		wxPoint *point_beg = (wxPoint *) pointsScreen->Item(i)->GetData();
		wxPoint *point_end = (wxPoint *) pointsScreen->Item(++i)->GetData();

		// set bounds for line
		// wxRect rect (*point_beg, *point_end);
		// dc->SetIdBounds(startId, rect);
		
		dc->SetId(dcId); // set unique id & set bbox for each segment
		dc->SetPen(*pen);
		wxRect rect (*point_beg, *point_end);
		dc->SetIdBounds(dcId, rect);
		dc->DrawLine(point_beg->x, point_beg->y,
			     point_end->x, point_end->y);
		}
	    }
	    else {
		wxPoint points[pointsScreen->GetCount()];
		for (int i = 0; i < pointsScreen->GetCount(); i++) {
		    wxPoint *point_beg = (wxPoint *) pointsScreen->Item(i)->GetData();
		    points[i] = *point_beg;
		}
		dc->DrawLines(pointsScreen->GetCount(), points);
	    }
	}
    }

    if (type & GV_LINES) {
	DrawLineVerteces(line); // draw vertices
	DrawLineNodes(line);    // draw nodes
    }

    delete pen;

    return 1;
}

/**
   \brief Draw line verteces to the device
 
   Except of first and last vertex, see DrawLineNodes().

   \param line id

   \return number of verteces which were drawn
   \return -1 if drawing vertices is disabled
*/
int DisplayDriver::DrawLineVerteces(int line)
{
    int dcId;
    wxPoint *point;
    wxPen *pen;

    if (!IsSelected(line) && !settings.vertex.enabled)
	return -1;

    // determine color
    if (!IsSelected(line)) {
	pen = new wxPen(settings.vertex.color, settings.lineWidth, wxSOLID);
	dcId = 0;
    }
    else {
	pen = new wxPen(settings.highlight, settings.lineWidth, wxSOLID);
	if (drawSegments) {
	    dcId = 3; // first vertex
	}
	else {
	    dcId = 1;
	}
    }

    dc->SetId(dcId); /* 0 | 1 (selected) */
    dc->SetPen(*pen);

    for (int i = 1; i < pointsScreen->GetCount() - 1; i++, dcId += 2) {
	point = (wxPoint*) pointsScreen->Item(i)->GetData();

	if (IsSelected(line) && drawSegments) {
	    dc->SetId(dcId);
	    dc->SetPen(*pen);
	    wxRect rect (*point, *point);
	    dc->SetIdBounds(dcId, rect);
	}
	
	if (settings.vertex.enabled) {
	    DrawCross(line, (const wxPoint*) pointsScreen->Item(i)->GetData());
	    topology.vertex++;
	}
    }

    delete pen;

    return pointsScreen->GetCount() - 2;
}

/**
   \brief Draw line nodes to the device
 
   \param line id

   \return 1
   \return -1 if no nodes were drawn
*/
int DisplayDriver::DrawLineNodes(int line)
{
    int dcId;
    int node;
    double east, north, depth;
    double x, y, z;
    int nodes [2];
    bool draw;

    wxPen *pen;

    // draw nodes??
    if (!settings.nodeOne.enabled && !settings.nodeTwo.enabled)
	return -1;

    // get nodes
    Vect_get_line_nodes(mapInfo, line, &(nodes[0]), &(nodes[1]));
        
    for (int i = 0; i < sizeof(nodes) / sizeof(int); i++) {
	node = nodes[i];
	// get coordinates
	Vect_get_node_coor(mapInfo, node,
			   &east, &north, &depth);

	// convert EN->xy
	Cell2Pixel(east, north, depth,
		   &x, &y, &z);

	// determine color
	if (IsSelected(line)) {
	    pen = new wxPen(settings.highlight, settings.lineWidth, wxSOLID);
	    draw = true;
	    if (!drawSegments) {
		dcId = 1;
	    }
	    else {
		// node1, line1, vertex1, line2, vertex2, ..., node2
		if (i == 0) // first node
		    dcId = 1; 
		else // last node
		    dcId = 2 * points->n_points - 1;
	    }
	}
	else {
	    dcId = 0;
	    if (Vect_get_node_n_lines(mapInfo, node) == 1) {
		pen = new wxPen(settings.nodeOne.color, settings.lineWidth, wxSOLID);
		topology.nodeOne++;
		draw = settings.nodeOne.enabled;
	    }
	    else {
		pen = new wxPen(settings.nodeTwo.color, settings.lineWidth, wxSOLID);
		topology.nodeTwo++;
		draw = settings.nodeTwo.enabled;
	    }
	}
	
	wxPoint point((int) x, (int) y);
	if (IsSelected(line) && drawSegments) {
	    wxRect rect (point, point);
	    dc->SetIdBounds(dcId, rect);
	}

	// draw node if needed
	if (draw) {
	    dc->SetId(dcId);
	    dc->SetPen(*pen);
	    DrawCross(line, &point);
	}
    }
    
    delete pen;

    return 1;
}


/*
  \brief Close vector map layer
  
  \param void

  \return 0 on success
  \return non-zero on error
*/
int DisplayDriver::CloseMap()
{
    int ret;

    ret = -1;
    if (mapInfo) {
	if (mapInfo->mode == GV_MODE_RW) {
	    /* rebuild topology */
	    Vect_build(mapInfo, NULL);
	}
	ret = Vect_close(mapInfo);
	G_free ((void *) mapInfo);
	mapInfo = NULL;
    }
    
    return ret;
}

/**
   \brief Open vector map layer
 
   \param[in] mapname name of vector map
   \param[in] mapset name of mapset where the vector map layer is stored
   
   \return topo level
   \return -1 on error
*/
int DisplayDriver::OpenMap(const char* mapname, const char *mapset, bool update)
{
    int ret;

    if (!mapInfo)
	mapInfo = (struct Map_info *) G_malloc (sizeof (struct Map_info));

    // define open level (level 2: topology)
    Vect_set_open_level(2);

    // avoid GUI crash when G_fatal_error() is called (opening the vector map)
    Vect_set_fatal_error(GV_FATAL_PRINT);

    // open existing map
    if (!update) {
	ret = Vect_open_old(mapInfo, (char*) mapname, (char *) mapset);
    }
    else {
	ret = Vect_open_update(mapInfo, (char*) mapname, (char *) mapset);
    }

    if (ret == -1) { // error
	G_free((void *) mapInfo);
	mapInfo = NULL;
    }

    return ret;
}

/**
   \brief Reload vector map layer

   Close and open again. Needed for modification using v.edit.

   TODO: Get rid of that...

   \param
   
   \return
*/
void DisplayDriver::ReloadMap()
{
    // char* name   = G_store(Vect_get_map_name(mapInfo)); ???
    char* name   = G_store(mapInfo->name);
    char* mapset = G_store(Vect_get_mapset(mapInfo));

    Vect_close(mapInfo);
    mapInfo = NULL;

    OpenMap(name, mapset, false); // used only for v.edit
    //Vect_build_partial(mapInfo, GV_BUILD_NONE, stderr);
    //Vect_build(mapInfo, stderr);

    return;
}

/*
  \brief Conversion from geographic coordinates (east, north)
  to screen (x, y)
  
  TODO: 3D stuff...

  \param[in] east,north,depth geographical coordinates
  \param[out] x, y, z screen coordinates
  
  \return 
*/
void DisplayDriver::Cell2Pixel(double east, double north, double depth,
			       double *x, double *y, double *z)
{
    double n, w;
    /*
    *x = int((east  - region.map_west) / region.map_res);
    *y = int((region.map_north - north) / region.map_res);
    */
    w = region.center_easting  - (region.map_width / 2)  * region.map_res;
    n = region.center_northing + (region.map_height / 2) * region.map_res;

    /*
    *x = int((east  - w) / region.map_res);
    *y = int((n - north) / region.map_res);
    */
    *x = (east  - w) / region.map_res;
    *y = (n - north) / region.map_res;

    *z = 0;

    return;
}

/**
   \brief Set geographical region
 
   Region must be upgraded because of Cell2Pixel().
   
   \param[in] north,south,east,west,ns_res,ew_res region settings
 
   \return
*/
void DisplayDriver::SetRegion(double north, double south, double east, double west,
			      double ns_res, double ew_res,
			      double center_easting, double center_northing,
			      double map_width, double map_height)
{
    region.box.N  = north;
    region.box.S  = south;
    region.box.E  = east;
    region.box.W  = west;
    region.box.T  = PORT_DOUBLE_MAX;
    region.box.B  = -PORT_DOUBLE_MAX;
    region.ns_res = ns_res;
    region.ew_res = ew_res;

    region.center_easting = center_easting;
    region.center_northing = center_northing;

    region.map_width  = map_width;
    region.map_height = map_height;

    // calculate real region
    region.map_res = (region.ew_res > region.ns_res) ? region.ew_res : region.ns_res;

    region.map_west  = region.center_easting - (region.map_width / 2.) * region.map_res;
    region.map_north = region.center_northing + (region.map_height / 2.) * region.map_res;

    return;
}

/**
   \brief Draw cross symbol of given size to device content
   
   Used for points, nodes, vertices

   \param[in] point coordinates of center
   \param[in] size size of the cross symbol
   
   \return 1 on success
   \return -1 on failure
*/
int DisplayDriver::DrawCross(int line, const wxPoint* point, int size)
{
    if (!dc || !point)
	return -1;

    dc->DrawLine(point->x - size, point->y, point->x + size, point->y);
    dc->DrawLine(point->x, point->y - size, point->x, point->y + size);
    return 1;
}

/*
  \brief Set settings for displaying vector feature
 
  E.g. line width, color, ...
  
  \param[in] lineWidth,... settgings
  
  \return 
*/
void DisplayDriver::UpdateSettings(unsigned long highlight,
				   bool ePoint,       unsigned long cPoint, /* enabled, color */
				   bool eLine,        unsigned long cLine,
				   bool eBoundaryNo,  unsigned long cBoundaryNo,
				   bool eBoundaryOne, unsigned long cBoundaryOne,
				   bool eBoundaryTwo, unsigned long cBoundaryTwo,
				   bool eCentroidIn,  unsigned long cCentroidIn,
				   bool eCentroidOut, unsigned long cCentroidOut,
				   bool eCentroidDup, unsigned long cCentroidDup,
				   bool eNodeOne,     unsigned long cNodeOne,
				   bool eNodeTwo,     unsigned long cNodeTwo,
				   bool eVertex,      unsigned long cVertex,
				   int lineWidth)
{
    settings.highlight.Set(highlight);

    settings.point.enabled = ePoint;
    settings.point.color.Set(cPoint);

    settings.line.enabled = eLine;
    settings.line.color.Set(cLine);

    settings.boundaryNo.enabled = eBoundaryNo;
    settings.boundaryNo.color.Set(cBoundaryNo);
    settings.boundaryOne.enabled = eBoundaryOne;
    settings.boundaryOne.color.Set(cBoundaryOne);
    settings.boundaryTwo.enabled = eBoundaryTwo;
    settings.boundaryTwo.color.Set(cBoundaryTwo);


    settings.centroidIn.enabled = eCentroidIn;
    settings.centroidIn.color.Set(cCentroidIn);
    settings.centroidOut.enabled = eCentroidOut;
    settings.centroidOut.color.Set(cCentroidOut);
    settings.centroidDup.enabled = eCentroidDup;
    settings.centroidDup.color.Set(cCentroidDup);

    settings.nodeOne.enabled = eNodeOne;
    settings.nodeOne.color.Set(cNodeOne);
    settings.nodeTwo.enabled = eNodeTwo;
    settings.nodeTwo.color.Set(cNodeTwo);

    settings.vertex.enabled = eVertex;
    settings.vertex.color.Set(cVertex);

    settings.lineWidth = lineWidth;
}

/**
   \brief Prints gId: dcIds

   Useful for debugging purposes.

   \param

   \return
*/
void DisplayDriver::PrintIds()
{
    std::cerr << "topology.highlight: " << topology.highlight << std::endl;

    std::cerr << "topology.point: " << topology.point << std::endl;
    std::cerr << "topology.line: " << topology.line << std::endl;

    std::cerr << "topology.boundaryNo: " << topology.boundaryNo << std::endl;
    std::cerr << "topology.boundaryOne: " << topology.boundaryOne << std::endl;
    std::cerr << "topology.boundaryTwo: " << topology.boundaryTwo << std::endl;

    std::cerr << "topology.centroidIn: " << topology.centroidIn << std::endl;
    std::cerr << "topology.centroidOut: " << topology.centroidOut << std::endl;
    std::cerr << "topology.centroidDup: " << topology.centroidDup << std::endl;

    std::cerr << "topology.nodeOne: " << topology.nodeOne << std::endl;
    std::cerr << "topology.nodeTwo: " << topology.nodeTwo << std::endl;

    std::cerr << "topology.vertex: " << topology.vertex << std::endl;

    std::cerr << std::endl << "nobjects: "
	      << topology.point * 2 + // cross
      topology.line + 
      topology.boundaryNo +
      topology.boundaryOne +
      topology.boundaryTwo +
      topology.centroidIn * 2 +
      topology.centroidOut * 2 +
      topology.centroidDup * 2 +
      topology.nodeOne * 2 +
      topology.nodeTwo * 2 +
      topology.vertex * 2 << std::endl;

    std::cerr << "selected: ";

    for (int i = 0; i < selected->n_values; i++) {
	std::cerr << selected->value[i] << " ";
    }
    std::cerr << std::endl;

    return;
}

/**
   \brief Select vector objects by given bounding box
   
   If line id is already in the list of selected lines, then it will
   be excluded from this list.

   \param[in] x1,y1,x2,y2 corners coordinates of bounding box

   \return number of selected features
   \return -1 on error
*/
int DisplayDriver::SelectLinesByBox(double x1, double y1, double x2, double y2)
{
    if (!mapInfo)
	return -1;

    int type, line;
    double dx, dy;

    struct ilist *list;
    struct line_pnts *bbox;

    type = -1; // all types

    list = Vect_new_list();
    bbox = Vect_new_line_struct();

    dx = std::fabs(x2 - x1);
    dy = std::fabs(y2 - y1);
        
    Vect_append_point(bbox, x1, y1, -PORT_DOUBLE_MAX);
    Vect_append_point(bbox, x2, y1,  PORT_DOUBLE_MAX);
    Vect_append_point(bbox, x2, y2, -PORT_DOUBLE_MAX);
    Vect_append_point(bbox, x1, y2,  PORT_DOUBLE_MAX);
    Vect_append_point(bbox, x1, y1, -PORT_DOUBLE_MAX);
        
    Vect_select_lines_by_polygon(mapInfo, bbox,
				 0, NULL,
				 type, list);
	
    for (int i = 0; i < list->n_values; i++) {
	line = list->value[i];
	if (!IsSelected(line)) {
	    // selected.push_back(line);
	    Vect_list_append(selected, line);
	}
	else {
	    // selected.erase(GetSelectedIter(line));
	    Vect_list_delete(selected, line);
	}
    }

    // remove all duplicate ids
    // sort(selected.begin(), selected.end());
    // selected.erase(unique(selected.begin(), selected.end()), selected.end());

    Vect_destroy_line_struct(bbox);
    Vect_destroy_list(list);

    // return selected.size();
    return selected->n_values;
}

/**
   \brief Select vector feature by given point in given
   threshold
   
   Only one vector object can be selected. Bounding boxes of
   all segments are stores.

   \param[in] x,y point of searching
   \param[in] thresh threshold value where to search
   \param[in] onlyType select vector object of given type

   \return point on line if line found
*/
std::vector<double> DisplayDriver::SelectLineByPoint(double x, double y, double thresh,
						     int type)
{
    long int line;
    int ftype;
    double px, py, pz;

    std::vector<double> p;

    if (type == -1) {
	ftype = GV_POINTS | GV_LINES;
    }
    else if (type == 0) {
	ftype = GV_POINTS;
    }
    else if (type == 1) {
	ftype = GV_LINES;
    }

    line = Vect_find_line(mapInfo, x, y, 0.0,
			  ftype, thresh, 0, 0);

    if (line > 0) {
	// selected.push_back(line);
	Vect_list_append(selected, line);
	type = Vect_read_line (mapInfo, points, cats, line);
	Vect_line_distance (points, x, y, 0.0, WITHOUT_Z,
			    &px, &py, &pz,
			    NULL, NULL, NULL);
	p.push_back(px);
	p.push_back(py);
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
    // if (GetSelectedIter(line) != selected.end())
    if (Vect_val_in_list(selected, line))
	return true;

    return false;
}

/**
   \brief Is vector object selected?
   
   \param[in] line id

   \return item iterator
   \return selected.end() if object is not selected
*/
/*
std::vector<int>::iterator DisplayDriver::GetSelectedIter(int line)
{
    for(std::vector<int>::iterator i = selected.begin(), e = selected.end();
	i != e; ++i) {
	if (line == *i)
	    return i;
    }

    return selected.end();
}
*/
/**
   \brief Get ids of selected objects

   \param[in] grassId if true return GRASS line ids
   if false return PseudoDC ids
   
   \return list of ids of selected vector objects
*/
std::vector<int> DisplayDriver::GetSelected(bool grassId)
{
    if (grassId)
	return ListToVector(selected);

    std::vector<int> dc_ids;
    long int line;

    if (!drawSegments) {
	dc_ids.push_back(1);
    }
    else {
	int npoints;
	Vect_read_line(mapInfo, points, NULL, selected->value[0]);
	npoints = points->n_points;
	for (int i = 1; i < 2 * npoints; i++) {
	  dc_ids.push_back(i);
	}
    }

    /*
    for(std::vector<int>::const_iterator i = selected.begin(), e = selected.end();
	i != e; ++i) {
	line = *i;
	ids_map::const_iterator ii = ids.find(line);
	if (ii != ids.end()) { // line found
	    long int endId = ii->second.npoints * 2 - 1 + ii->second.startId;
	    int type, i;
	    int vx, vy, vz;
	    type = Vect_read_line (mapInfo, points, cats, line);
	    i = 0;
 	    for (long int id = ii->second.startId; id < endId; id++) {
		dc_ids.push_back(id);
                // set bounding boxes for all selected objects (only nodes)
		if (id % 2) {
		    Cell2Pixel(points->x[i], points->y[i], points->z[i],
			       &vx, &vy, &vz);
		    wxRect rect (wxPoint (vx, vy), wxPoint (vx, vy));
		    dc->SetIdBounds(id, rect);

		    i++;
		}

	    }
	}
    }
    */

    return dc_ids;
}

/**
   \brief Set selected vector objects
   
   \param[in] list of GRASS ids to be set

   \return 1
*/
int DisplayDriver::SetSelected(std::vector<int> id)
{
    // selected = id;
    VectorToList(selected, id);

    if (selected->n_values <= 0)
	drawSegments = false;

    return 1;
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
    if (selected->n_values != 1 || !drawSegments) 
	return returnId;

    startId = 1;
    line = selected->value[0];

    type = Vect_read_line (mapInfo, points, cats, line);
        
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

/**
   \brief Reset topology structure.

   \return
*/
void DisplayDriver::ResetTopology()
{
    topology.highlight = 0;
    
    topology.point = 0;
    topology.line = 0;
    
    topology.boundaryNo = 0;
    topology.boundaryOne = 0;
    topology.boundaryTwo = 0;
    
    topology.centroidIn = 0;
    topology.centroidOut = 0;
    topology.centroidDup = 0;
    
    topology.nodeOne = 0;
    topology.nodeTwo = 0;
    
    topology.vertex = 0;

    return;
}

/**
   \brief Convert vect list to std::vector

   \param list vect list

   \return std::vector
*/
std::vector<int> DisplayDriver::ListToVector(struct ilist *list)
{
    std::vector<int> vect;

    if (!list)
	return vect;

    for (int i = 0; i < list->n_values; i++) {
	vect.push_back(list->value[i]);
    }

    return vect;
}

/**
   \brief Convert std::vector to vect list

   \param list vect list
   \param vec  std::vector instance

   \return number of items
   \return -1 on error
*/
int DisplayDriver::VectorToList(struct ilist *list, const std::vector<int>& vec)
{
    if (!list)
	return -1;

    Vect_reset_list(list);

    for (std::vector<int>::const_iterator i = vec.begin(), e = vec.end();
	 i != e; ++i) {
	Vect_list_append(list, *i);
    }

    return list->n_values;
}

/**
   \brief Error messages handling 

   \param msg message
   \param type type message (MSG, WARN, ERR)

   \return 0
*/
int print_error(const char *msg, int type)
{
    fprintf(stderr, "%s", msg);
    
    return 0;
}
