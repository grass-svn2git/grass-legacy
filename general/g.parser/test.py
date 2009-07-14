#!/usr/bin/python

# g.parser demo script for python programing

import os
import sys

#%Module
#%  description: g.parser test script (python)
#%End
#%flag
#%  key: f
#%  description: A flag
#%end
#%option
#% key: raster
#% type: string
#% gisprompt: old,cell,raster
#% description: Raster input map
#% required : yes
#%end
#%option
#% key: vector
#% type: string
#% gisprompt: old,vector,vector
#% description: Vector input map
#% required : yes
#%end
#%option
#% key: option1
#% type: string
#% description: An option
#% required : yes
#%end

def main():

    #add your code here

    print ""
    if ( os.getenv('GIS_FLAG_F') == "1" ):
        print "Flag -f set"
    else:
        print "Flag -f not set"

    #test if parameter present:
    if ( os.getenv("GIS_OPT_OPTION1") != "" ):
        print "Value of GIS_OPT_OPTION1: '%s'" % os.getenv('GIS_OPT_OPTION1')

    print "Value of GIS_OPT_RASTER: '%s'" % os.getenv('GIS_OPT_RASTER')
    print "Value of GIS_OPT_VECTOR: '%s'" % os.getenv('GIS_OPT_VECTOR')

    #end of your code 
    
if __name__ == "__main__":
    args = ""
    for arg in sys.argv:
        args += arg+" "

    if !os.getenv("GISBASE"):
        print >> sys.stderr, "You must be in GRASS GIS to run this program."
        sys.exit(0)

    try:
        if ( sys.argv[1] != "@ARGS_PARSED@" ):
            os.system("g.parser %s " % (args))
    except IndexError:
        os.system("g.parser %s" % (args))

    if sys.argv[1] == "@ARGS_PARSED@":
        main();



