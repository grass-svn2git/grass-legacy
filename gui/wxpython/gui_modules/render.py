"""
@package render

Rendering map layers into image

Classes:
 - MapLayer
 - Map

C) 2006-2008 by the GRASS Development Team
This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Michael Barton, Jachym Cepicky, Martin Landa

@date 2006-2008
"""

import os
import sys
import glob
import math
try:
    import subprocess
except:
    compatPath = os.path.join(globalvar.ETCWXDIR, "compat")
    sys.path.append(compatPath)
    import subprocess

import wx

import globalvar
import utils
import gcmd
from debug import Debug as Debug

#
# use g.pnmcomp for creating image composition or
# wxPython functionality
#
USE_GPNMCOMP = True

class MapLayer(object):
    """Stores information about map layers or overlays to be displayed"""
    def __init__(self, type, cmd, name=None,
                 active=True, hidden=False, opacity=1.0):
        """
        @param type layer type (raster, vector, overlay, command, etc.)
        @param cmd GRASS command for rendering layer, given as list, e.g. ['d.rast', 'map=elevation@PERMANENT']
        @param name layer name, e.g. 'elevation@PERMANENT'
        @param active layer is active, will be rendered only if True
        @param hidden layer is hidden, won't be listed in Layer Manager if True
        @param opacity layer opacity <0;1>
        """
        self.type    = type
        self.name    = name
        self.cmdlist = cmd

        self.active  = active
        self.hidden  = hidden
        self.opacity = opacity

        Debug.msg (3, "MapLayer.__init__(): type=%s, cmd='%s', name=%s, " \
                   "active=%d, opacity=%d, hidden=%d" % \
                   (self.type, self.GetCmd(string=True), self.name, self.active,
                    self.opacity, self.hidden))

        # generated file for layer
        gtemp = utils.GetTempfile()
        self.maskfile = gtemp + ".pgm"
        if self.type == "overlay":
            self.mapfile = gtemp + ".png"
        else:
            self.mapfile = gtemp + ".ppm"

    def __del__(self):
        Debug.msg (3, "MapLayer.__del__(): layer=%s, cmd='%s'" %
                   (self.name, self.GetCmd(string=True)))

    def Render(self):
        """Render map layer to image

        @return name of file with rendered image or None
        """

        Debug.msg (3, "MapLayer.Render(): type=%s" % \
                   (self.type))

        #
        # to be sure, set temporary file with layer and mask
        #
        gtemp = utils.GetTempfile()
        self.maskfile = gtemp + ".pgm"
        if self.type == 'overlay':
            self.mapfile  = gtemp + ".png"
        else:
            self.mapfile  = gtemp + ".ppm"

        #
        # prepare command for each layer
        #
        layertypes = ['raster', 'rgb', 'his', 'shaded', 'rastarrow', 'rastnum',
                      'vector','thememap','themechart',
                      'grid', 'geodesic', 'rhumb', 'labels',
                      'command',
                      'overlay']

        if self.type not in layertypes:
            raise gcmd.GStdError(_("<%(name)s>: layer type <%(type)s> is not supported yet.") % \
                                     {'type' : self.type, 'name' : self.name})
        
        #
        # start monitor
        #
        os.environ["GRASS_PNGFILE"] = self.mapfile
        
        #
        # execute command
        #
        try:
            runcmd = gcmd.Command(cmd=self.cmdlist + ['--q'],
                                  stderr=None)
        except gcmd.CmdError, e:
            print e
        
        if runcmd.returncode != 0:
            self.mapfile = None
            self.maskfile = None
            return None

        #
        # stop monitor
        #
        os.unsetenv("GRASS_PNGFILE")

        return self.mapfile

    def GetMapset(self):
        """
        @return mapset name of the layer
        """
        if not self.name:
            return ''

        return self.name.split('@')[1]

    def GetCmd(self, string=False):
        """
        @param string get command as string if True otherwise list

        @return command list/string
        """
        if string:
            return ' '.join(self.cmdlist)
        else:
            return self.cmdlist

    def GetOpacity(self, float=False):
        """
        Get opacity level
        @param float get opacity level in <0,1> otherwise <0,100>

        @return opacity level
        """
        if float:
            return self.opacity
        
        return int (self.opacity * 100)

class Map(object):
    """
    Map composition (stack of map layers)
    """
    def __init__(self):
        # 
        # region/extent settigns
        #
        self.wind      = {}    # WIND settings (wind file)
        self.region    = {}    # region settings (g.region)
        self.width     = 640   # map width
        self.height    = 480   # map height

        #
        # list of layers
        #
        self.layers    = []  # stack of available GRASS layer

        self.overlays  = []  # stack of available overlays
        self.ovlookup  = {}  # lookup dictionary for overlay items and overlays

        #
        # environment settings
        #
        self.env       = {}  # enviroment variables, like MAPSET, LOCATION_NAME, etc.
        self.verbosity = 0   # --q

        # 
        # generated file for rendering the map
        #
        self.mapfile   = utils.GetTempfile()

        # setting some initial env. variables
        self.InitGisEnv() # g.gisenv
        self.InitRegion()
        os.environ["GRASS_TRANSPARENT"]     = "TRUE"
        os.environ["GRASS_BACKGROUNDCOLOR"] = "ffffff"
        os.environ["GRASS_HEIGHT"]          = str(self.height)
        os.environ["GRASS_WIDTH"]           = str(self.width)
        os.environ["GRASS_MESSAGE_FORMAT"]  = "gui"
        os.environ["GRASS_PNG_AUTO_WRITE"]  = "TRUE"
        os.environ["GRASS_TRUECOLOR"]       = "TRUE"
        os.environ["GRASS_COMPRESSION"]     = "0"
        os.environ["GRASS_VERBOSE"]         = str(self.verbosity)

    def InitRegion(self):
        """
        Initialize current region settings.

        Set up 'self.region' using g.region command and
        self.wind according to the wind file.

        Adjust self.region based on map window size.
        """

        #
        # setting region ('g.region -upg')
        #
        self.region = self.GetRegion()

        #
        # read WIND file
        #
        self.GetWindow()

        #
        # setting resolution
        #
        self.SetRegion()

    def InitGisEnv(self):
        """
        Stores GRASS variables (g.gisenv) to self.env variable
        """

        if not os.getenv("GISBASE"):
            print >> sys.stderr, _("GISBASE not set. You must be in GRASS GIS to run this program.")
            sys.exit(1)

        gisenvCmd = gcmd.Command(["g.gisenv"])

        for line in gisenvCmd.ReadStdOutput():
            line = line.strip()
            key, val = line.split("=")
            val = val.replace(";","")
            val = val.replace("'","")
            self.env[key] = val

    def GetWindow(self):
        """Read WIND file and set up self.wind dictionary"""
        # FIXME: duplicated region WIND == g.region (at least some values)
        windfile = os.path.join (self.env['GISDBASE'],
                                 self.env['LOCATION_NAME'],
                                 self.env['MAPSET'],
                                 "WIND")
        try:
            windfile = open (windfile, "r")
        except StandardError, e:
            sys.stderr.write("%s %<s>: %s" % (_("Unable to open file"), windfile, e))
            sys.exit(1)

        for line in windfile.readlines():
            line = line.strip()
            key, value = line.split(":",1)
            key = key.strip()
            value = value.strip()
            self.wind[key] = value
            
        windfile.close()

        return self.wind
        
    def __adjustRegion(self):
        """
        Adjusts display resolution to match monitor size in pixels.
        Maintains constant display resolution, not related to computational
        region. Do NOT use the display resolution to set computational
        resolution. Set computational resolution through g.region.
        """

        mapwidth    = abs(self.region["e"] - self.region["w"])
        mapheight   = abs(self.region['n'] - self.region['s'])

        self.region["nsres"] =  mapheight / self.height
        self.region["ewres"] =  mapwidth  / self.width
        self.region['rows']  = round(mapheight / self.region["nsres"])
        self.region['cols']  = round(mapwidth / self.region["ewres"])
        self.region['cells'] = self.region['rows'] * self.region['cols']

        Debug.msg (3, "Map.__adjustRegion(): %s" % self.region)

        return self.region

    def AlignResolution(self):
        """
        Sets display extents to even multiple of
        current resolution defined in WIND file from SW corner.
        This must be done manually as using the -a flag
        can produce incorrect extents.
        """

        # new values to use for saving to region file
        new = {}
        n = s = e = w = 0.0
        nwres = ewres = 0.0

        # Get current values for region and display
        nsres = self.GetRegion()['nsres']
        ewres = self.GetRegion()['ewres']

        n = float(self.region['n'])
        s = float(self.region['s'])
        e = float(self.region['e'])
        w = float(self.region['w'])

        # Calculate rows, columns, and extents
        new['rows'] = math.fabs(round((n-s)/nsres))
        new['cols'] = math.fabs(round((e-w)/ewres))

        # Calculate new extents
        new['s'] = nsres * round(s/nsres)
        new['w'] = ewres * round(w/ewres)
        new['n'] = new['s'] + (new['rows'] * nsres)
        new['e'] = new['w'] + (new['cols'] * ewres)

        return new

    def AlignExtentFromDisplay(self):
        """Sets display extents (n,s,e,w) to even multiple of
        current display resolution from center point"""

        # calculate new bounding box based on center of display
        if self.region["ewres"] > self.region["nsres"]:
            res = self.region["ewres"]
        else:
            res = self.region["nsres"]

        Debug.msg(3, "Map.AlignExtentFromDisplay(): width=%d, height=%d, res=%f, center=%f,%f" % \
                      (self.width, self.height, res, self.region['center_easting'],
                       self.region['center_northing']))
            
        ew = (self.width / 2) * res
        ns = (self.height / 2) * res

        self.region['n'] = self.region['center_northing'] + ns
        self.region['s'] = self.region['center_northing'] - ns
        self.region['e'] = self.region['center_easting'] + ew
        self.region['w'] = self.region['center_easting'] - ew

    def ChangeMapSize(self, (width, height)):
        """Change size of rendered map.
        
        @param width,height map size

        @return True on success
        @return False on failure
        """
        try:
            self.width  = int(width)
            self.height = int(height)
            Debug.msg(2, "Map.ChangeMapSize(): width=%d, height=%d" % \
                          (self.width, self.height))
            return True
        except:
            self.width  = 640
            self.height = 480
            return False

    def GetRegion(self, rast=None, vect=None,
                  n=None, s=None, e=None, w=None):
        """
        Get region settings

        Optionaly raster or vector map layer can be given.

        @param rast raster name or None
        @param vect vector name or None
        
        @return region settings as directory, e.g. {
        "n":"4928010", "s":"4913700", "w":"589980",...}
        """

        region = {}

        tmpreg = os.getenv("GRASS_REGION")
        os.unsetenv("GRASS_REGION")

        # do not update & shell style output
        cmdList = ["g.region", "-u", "-g", "-p", "-c"]

        if n:
            cmdList.append('n=%s' % n)
        if s:
            cmdList.append('s=%s' % s)
        if e:
            cmdList.append('e=%s' % e)
        if w:
            cmdList.append('w=%s' % w)

        if rast:
            cmdList.append('rast=%s' % rast)
        if vect:
            cmdList.append('vect=%s' % vect)

        try:
            cmdRegion = gcmd.Command(cmdList)
        except gcmd.CmdError, e:
            if rast:
                e.message = _("Unable to zoom to raster map <%s>.") % rast + \
                '%s%s' % (os.linesep, os.linesep) + e.message
            elif vect:
                e.message = _("Unable to zoom to vector map <%s>.") % vect + \
                '%s%s' % (os.linesep, os.linesep) + e.message

            print e
            return self.region

        for reg in cmdRegion.ReadStdOutput():
            key, val = reg.split("=", 1)
            try:
                region[key] = float(val)
            except ValueError:
                region[key] = val

        # restore region
        if tmpreg:
            os.environ["GRASS_REGION"] = tmpreg

        Debug.msg (3, "Map.GetRegion(): %s" % region)

        return region

    def SetRegion(self, windres=False):
        """
        Render string for GRASS_REGION env. variable, so that the images will be rendered
        from desired zoom level.

        @param windres If windres set to True, uses resolution from
        WIND file rather than display (for modules that require set
        resolution like d.rast.num)

        @return String usable for GRASS_REGION variable or None
        """
        grass_region = ""

        # adjust region settigns to match monitor
        self.region = self.__adjustRegion()

        #        newextents = self.AlignResolution()
        #        self.region['n'] = newextents['n']
        #        self.region['s'] = newextents['s']
        #        self.region['e'] = newextents['e']
        #        self.region['w'] = newextents['w']

        # read values from wind file
        try:
            for key in self.wind.keys():
                if key == 'north':
                    grass_region += "north: %s; " % \
                        (self.region['n'])
                    continue
                elif key == "south":
                    grass_region += "south: %s; " % \
                        (self.region['s'])
                    continue
                elif key == "east":
                    grass_region += "east: %s; " % \
                        (self.region['e'])
                    continue
                elif key == "west":
                    grass_region += "west: %s; " % \
                        (self.region['w'])
                    continue
                elif key == "e-w resol":
                    grass_region += "e-w resol: %f; " % \
                        (self.region['ewres'])
                    continue
                elif key == "n-s resol":
                    grass_region += "n-s resol: %f; " % \
                        (self.region['nsres'])
                    continue
                elif key == "cols":
                    grass_region += 'cols: %d; ' % \
                        (self.width)
                    continue
                elif key == "rows":
                    grass_region += 'rows: %d; ' % \
                        (self.height)
                    continue
                else:
                    grass_region += key + ": "  + self.wind[key] + "; "

            Debug.msg (3, "Map.SetRegion(): %s" % grass_region)

            return grass_region

        except:
            return None

    def ProjInfo(self):
        """
        Return region projection and map units information
        """

        projinfo = {}

        p = gcmd.Command(['g.proj', '-p'])

        if p.returncode == 0:
            for line in p.ReadStdOutput():
                if ':' in line:
                    key,val = line.split(':')
                    key = key.strip()
                    val = val.strip()
                    projinfo[key] = val
                elif "XY location (unprojected)" in line:
                    projinfo['proj'] = "xy"
            return projinfo
        else:
            return None

    def GetListOfLayers(self, l_type=None, l_mapset=None, l_name=None,
                        l_active=None, l_hidden=None):
        """
        Returns list of layers of selected properties or list of all
        layers. 

        @param l_type layer type, e.g. raster/vector/wms/overlay
        @param l_mapset all layers from given mapset
        @param l_name all layers with given name
        @param l_active only layers with 'active' attribute set to True or False
        @param l_hidden only layers with 'hidden' attribute set to True or False

        @return list of selected layers
        """

        selected = []

        # ["raster", "vector", "wms", ... ]
        for layer in self.layers + self.overlays:
            # specified type only
            if l_type != None and layer.type != l_type:
                continue

            # mapset
            if l_mapset != None and layer.GetMapset() != l_mapset:
                continue

            # name
            if l_name != None and layer.name != l_name:
                continue

            # hidden and active layers
            if l_active != None and \
                   l_hidden != None:
                if layer.active == l_active and \
                       layer.hidden == l_hidden:
                    selected.append(layer)

            # active layers
            elif l_active != None:
                if layer.active == l_active:
                    selected.append(layer)

            # hidden layers
            elif l_hidden != None:
                if layer.hidden == l_hidden:
                    selected.append(layer)

            # all layers
            else:
                selected.append(layer)

        Debug.msg (3, "Map.GetListOfLayers(): numberof=%d" % len(selected))

        return selected

    def Render(self, force=False, mapWindow=None):
        """
        Creates final image composite

        This function can conditionaly use high-level tools, which
        should be avaliable in wxPython library
        
        @param force force rendering
        @param reference for MapFrame instance (for progress bar)

        @return name of file with rendered image or None
        """

        maps = []
        masks =[]
        opacities = []

        tmp_region = os.getenv("GRASS_REGION")
        os.environ["GRASS_REGION"] = self.SetRegion()
        os.environ["GRASS_WIDTH"]  = str(self.width)
        os.environ["GRASS_HEIGHT"] = str(self.height)

        # render map layers
        for layer in self.layers + self.overlays:
            # skip if not active
            if layer == None or layer.active == False:
                continue
            
            # render if there is no mapfile
            if layer.mapfile == None:
                layer.Render()
                    
            # process bar
            if mapWindow is not None:
                mapWindow.onRenderCounter += 1

            wx.Yield()
            # redraw layer content
            if force:
                if not layer.Render():
                    continue

            # add image to compositing list
            if layer.type != "overlay":
                maps.append(layer.mapfile)
                masks.append(layer.maskfile)
                opacities.append(str(layer.opacity))
                
            Debug.msg (3, "Map.Render() type=%s, layer=%s " % (layer.type, layer.name))

	# ugly hack for MSYS
	if not subprocess.mswindows:
	    mapstr = ",".join(maps)
	    maskstr = ",".join(masks)
            mapoutstr = self.mapfile
	else:
	    mapstr = ""
	    for item in maps:
                mapstr += item.replace('\\', '/')		
	    mapstr = mapstr.rstrip(',')
	    maskstr = ""
            for item in masks:
		maskstr += item.replace('\\', '/')
	    maskstr = maskstr.rstrip(',')
	    mapoutstr = self.mapfile.replace('\\', '/')

        compstring = "g.pnmcomp" + globalvar.EXT_BIN + \
            " in=" + mapstr + \
            " mask=" + maskstr + \
            " opacity=" + ",".join(opacities)+ \
            " background=255:255:255" + \
            " width=" + str(self.width) + \
            " height=" + str(self.height) + \
            " output=" + mapoutstr

        # compose command
        complist = ["g.pnmcomp",
                   "in=%s" % ",".join(maps),
	           "mask=%s" % ",".join(masks),
                   "opacity=%s" % ",".join(opacities),
                   "background=255:255:255",
                   "width=%s" % str(self.width),
                   "height=%s" % str(self.height),
                   "output=%s" % self.mapfile]


        # render overlays

        os.unsetenv("GRASS_REGION")

        if tmp_region:
            os.environ["GRASS_REGION"] = tmp_region

        # run g.pngcomp to get composite image
        try:
            gcmd.Command(complist)
	    # os.system(compstring)
        except gcmd.CmdError, e:
            print e
            return None

        Debug.msg (2, "Map.Render() force=%s file=%s" % (force, self.mapfile))

        return self.mapfile

    def AddLayer(self, type, command, name=None,
                 l_active=True, l_hidden=False, l_opacity=1.0, l_render=False):
        """
        Adds generic display command layer to list of layers

        @param item reference to item in layer tree
        @param type layer type
        @param name layer name
        @param cmd  GRASS command to render layer
        @param l_active checked/not checked for display in layer tree
        @param l_hidden not used here
        @param l_opacity opacity leve range from 0(transparent)-1(not transparent)
        @param l_render render an image if False

        @return new layer on success
        @return None on failure

        """
        # l_opacity must be <0;1>
        if l_opacity < 0: l_opacity = 0
        elif l_opacity > 1: l_opacity = 1

        layer = MapLayer(type=type, name=name, cmd=command,
                         active=l_active, hidden=l_hidden, opacity=l_opacity)

        # add maplayer to the list of layers
        self.layers.append(layer)

        Debug.msg (3, "Map.AddLayer(): layer=%s" % layer.name)
        if l_render:
            if not layer.Render():
                raise gcmd.GStdError(_("Unable to render map layer <%s>.") % (name))

        return self.layers[-1]

    def DeleteLayer(self, layer):
        """
        Removes layer from list of layers,
        defined by reference to MapLayer instance

        Returns:
            Removed layer on success or None
        """

        Debug.msg (3, "Map.DeleteLayer(): name=%s" % layer.name)
        if layer in self.layers:
            if layer.mapfile:
                base = os.path.split(layer.mapfile)[0]
                mapfile = os.path.split(layer.mapfile)[1]
                tempbase = mapfile.split('.')[0]
                basefile = os.path.join(base,tempbase)+r'.*'
                for f in glob.glob(basefile):
                    os.remove(f)
            self.layers.remove(layer)
            return layer

        return None

    def ReorderLayers(self, layerList):
        """
        Make a new reordered list to match reordered
        layer tree - for drag and drop
        """
        self.layers = layerList

        layerNameList = ""
        for layer in self.layers:
            if layer.name:
                layerNameList += layer.name + ','
        Debug.msg (4, "Map.ReoderLayers(): layers=%s" % \
                   (layerNameList))

    def ChangeLayer(self, layer, type, command, name=None,
                    l_active=True, l_hidden=False, l_opacity=1, l_render=False):
        """
        Change the command and other other options for a layer
        """

        # l_opacity must be <0;1>
        if l_opacity < 0: l_opacity = 0
        elif l_opacity > 1: l_opacity = 1

        Debug.msg (3, "Map.ChangeLayer():")

        newlayer = MapLayer(type=type, cmd=command, name=name,
                            active=l_active, hidden=l_hidden, opacity=l_opacity)

        oldlayerindex = self.layers.index(layer)

        # add maplayer to the list of layers
        if layer:
            self.layers[oldlayerindex] = newlayer

        if l_render and not layer.Render():
            raise gcmd.GException(_("Unable to render map layer <%s>.") % 
                                  (name))

        return self.layers[-1]

    def ChangeOpacity(self, layer, l_opacity):
        """
        Changes opacity value of map layer

        @param layer layer instance
        @param l_opacity opacity level <0;1>
        """
        # l_opacity must be <0;1>
        if l_opacity < 0: l_opacity = 0
        elif l_opacity > 1: l_opacity = 1

        layer.opacity = l_opacity
        Debug.msg (3, "Map.ChangeOpacity(): layer=%s, opacity=%f" % \
                   (layer.name, layer.opacity))

    def ChangeLayerActive(self, layer, active):
        """
        Change the active state of a layer

        @param layer layer instance
        @param active to be rendered (True)
        """
        layer.active = active

        Debug.msg (3, "Map.ChangeLayerActive(): name='%s' -> active=%d" % \
                   (layer.name, layer.active))

    def ChangeLayerName (self, layer, name):
        """
        Change name of the layer

        @param layer layer instance
        @param name  layer name to set up
        """
        Debug.msg (3, "Map.ChangeLayerName(): from=%s to=%s" % \
                   (layer.name, name))
        layer.name =  name

    def RemoveLayer(self, name=None, id=None):
        """
        Removes layer from layer list of layers

        Layer is defined by name@mapset or id.

        @param name layer name (must be unique)
        @param id layer index in layer list

        @return removed layer on success
        @return None on failure
        """

        # delete by name
        if name:
            retlayer = None
            for layer in self.layers:
                if layer.name == name:
                    retlayer = layer
                    os.remove(layer.mapfile)
                    os.remove(layer.maskfile)
                    self.layers.remove(layer)
                    return layer
        # del by id
        elif id != None:
            return self.layers.pop(id)

        return None

    def GetLayerIndex(self, layer):
        """
        Returns index of layer in layer list.

        @param layer layer instace

        @return layer index
        @return None
        """

        if layer in self.layers:
            return self.layers.index(layer)
        else:
            return None

    def AddOverlay(self, ovltype=None, type='overlay', command=None,
                   l_active=True, l_hidden=False, l_opacity=1, l_render=False):
        """
        Adds overlay (grid, barscale, others?) to list of overlays

        @param ovltype overlay type
        @param command GRASS command to render overlay
        @param l_active overlay activated (True) or disabled (False)
        @param l_render render an image (True)

        @return new layer on success
        @retutn None on failure
        """

        Debug.msg (2, "Map.AddOverlay(): cmd=%s, render=%d" % (command, l_render))
        overlay = MapLayer(type='overlay', name=None, cmd=command,
                           active=l_active, hidden=l_hidden, opacity=l_opacity)

        # add maplayer to the list of layers
        self.overlays.append(overlay)

        if l_render and command != '' and not overlay.Render():
            raise gcmd.GException(_("Unable render overlay <%s>.") % 
                                  (name))

        self.ovlookup[ovltype] = overlay

        return self.overlays[-1]

    def ChangeOverlay(self, ovltype, type, name, command,
                      l_active=True, l_hidden=False, l_opacity=1, l_render=False):
        """
        Change overlay properities

        @param ovltype overlay type
        @param type layer type
        @param command GRASS command to render an overlay
        @param l_active overlay is active (True) or disabled (False)
        @param l_hidded not used here
        @param l_opacity opacity level <0,1>
        @param l_render render overlay (True)

        @return new overlay instance
        """

        newoverlay = MapLayer(type='overlay', name=name, cmd=command,
                              active=l_active, hidden=l_hidden, opacity=l_opacity)

        oldovlindex = self.overlays.index(self.ovlookup[ovltype])

        # add overlay to the list of layers
        if self.ovlookup[ovltype]:
            self.overlays[oldovlindex] = newoverlay
            self.ovlookup[ovltype] = newoverlay

        if l_render and command != '' and not overlay.Render():
            raise gcmd.GException(_("Unable render overlay <%s>.") % 
                                  (name))

        return self.overlays[-1]

    def changeOverlayActive(self, ovltype, activ):
        """
        Change active status of overlay
        """
        try:
            overlay = self.ovlookup[ovltype]
            overlay.active = activ
            Debug.msg (3, "Map.changeOverlayActive(): type=%d, active=%d" % (type, activ))
        except:
            sys.stderr.write("Cannot change status of overlay index [%d]\n" % type)

    def Clean(self):
        """
        Go trough all layers and remove them from layer list
        Removes also l_mapfile and l_maskfile

        @return 1 on faulure
        @return None on success
        """
        try:
            for layer in self.layers:
                if layer.mapfile:
                    base = os.path.split(layer.mapfile)[0]
                    mapfile = os.path.split(layer.mapfile)[1]
                    tempbase = mapfile.split('.')[0]
                    basefile = os.path.join(base,tempbase)+r'.*'
                    for f in glob.glob(basefile):
                        os.remove(f)
                self.layers.remove(layer)
            for overlay in self.overlays:
                if overlay.ovlfile:
                    base = os.path.split(overlay.ovlfile)[0]
                    mapfile = os.path.split(overlay.ovlfile)[1]
                    tempbase = mapfile.split('.')[0]
                    basefile = os.path.join(base,tempbase)+r'.*'
                    for f in glob.glob(basefile):
                        os.remove(f)
                self.overlays.remove(overlay)
            return None
        except:
            return 1
        self.layers = []

    def ReverseListOfLayers(self):
        """Reverse list of layers"""
        return self.layers.reverse()

if __name__ == "__main__":
    """
    Test of Display class.
    Usage: display=Render()
    """

    print "Initializing..."
    os.system("g.region -d")

    map = Map()
    map.width = 640
    map.height = 480

    map.AddLayer(item=None, type="raster", name="elevation.dem", command = "d.rast elevation.dem@PERMANENT catlist=1000-1500 -i", l_opacity=.7)

    map.AddLayer(item=None, type="vector", name="streams", command = "d.vect streams@PERMANENT color=red width=3 type=line")

    image = map.Render(force=True)

    if image:
        os.system("display %s" % image)
