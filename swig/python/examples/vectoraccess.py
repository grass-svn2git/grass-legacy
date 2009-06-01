#!/usr/bin/python

# run within GRASS Spearfish session
# run this before starting python to append module search path:
#   export PYTHONPATH=/usr/src/grass70/swig/python
#   check with "import sys; sys.path"
# or:
#   sys.path.append("/usr/src/grass70/swig/python")
# FIXME: install the grass bindings in $GISBASE/lib/ ?

import os, sys
from grass.lib import grass
from grass.lib import vector as grassvect

if not os.environ.has_key("GISBASE"):
    print "You must be in GRASS GIS to run this program."
    sys.exit(1)

if len(sys.argv)==2:
  input = sys.argv[1]
else:
  input = raw_input("Vector Map Name? ")

# initialize
grass.G_gisinit('')

# find map in search path
mapset = grass.G_find_vector2(input,'')

# define map structure
map = grassvect.Map_info()

# define open level (level 2: topology)
grassvect.Vect_set_open_level (2)

# open existing map
grassvect.Vect_open_old(map, input, mapset)

# query
print 'Vect map: ', input
print 'Vect is 3D: ', grassvect.Vect_is_3d (map)
print 'Vect DB links: ', grassvect.Vect_get_num_dblinks(map)
print 'Map Scale:  1:', grassvect.Vect_get_scale(map)

# vector box tests
box = grassvect.bound_box()
c_easting1  =  599505.0
c_northing = 4921010.0
c_easting2  =  4599505.0

grassvect.Vect_get_map_box(map, box)
print 'Position 1 in box? ', grassvect.Vect_point_in_box(c_easting1, c_northing, 0, box)
print 'Position 2 in box? ', grassvect.Vect_point_in_box(c_easting2, c_northing, 0, box)
print 'Vector line 2 in box? ', grassvect.Vect_get_line_box(map, 2, box)
# misleading:
# print 'Number of lines:', grassvect.Vect_get_num_lines(map)
# how to access GV_POINT?
# print 'Number of points: ', grassvect.Vect_get_num_primitives(map,GV_POINT)
# confusing:
#print 'Number of lines: ', grassvect.Vect_get_num_primitives(map,GV_LINE)
#print 'Number of areas:', grassvect.Vect_get_num_primitives(map,GV_AREA)
print 'Number of areas:', grassvect.Vect_get_num_areas(map)

# close map
grassvect.Vect_close(map)
## end of the python script

