"""
MODULE: digit

CLASSES:
 * DigitError
 * AbstractDigit 
 * VEdit
 * VDigit
 * AbstractDisplayDriver
 * CDisplayDriver
 * DigitSettingsDialog
 * DigitCategoryDialog
 * DigitZBulkDialog

PURPOSE: Digitization tool wxPython GUI prototype

         Note: Initial version under development

         Progress:
          (1) v.edit called on the background (class VEdit)
          (2) Reimplentation of v.digit (VDigit)

         Import:
          from digit import Digit as Digit
          
AUTHORS: The GRASS Development Team
         Martin Landa <landa.martin gmail.com>

COPYRIGHT: (C) 2007-2008 by the GRASS Development Team
           This program is free software under the GNU General Public
           License (>=v2). Read the file COPYING that comes with GRASS
           for details.
"""

import os
import sys
import string
import copy

import wx
import wx.lib.colourselect as csel
import wx.lib.mixins.listctrl as listmix

import gcmd
import dbm
from debug import Debug as Debug
import gselect 
try:
    digitPath = os.path.join(os.getenv("GISBASE"), "etc", "wx", "vdigit")
    sys.path.append(digitPath)
    import grass6_wxvdigit as vdigit
    GV_LINES = vdigit.GV_LINES
except ImportError, err:
    GV_LINES = None
    print >> sys.stderr, "%sWARNING: Digitization tool is disabled (%s). " \
          "Detailed information in README file." % \
          (os.linesep, err)

#
# Use v.edit on background or experimental C++ interface (not yet completed)
#
USEVEDIT = True
if USEVEDIT is True and GV_LINES is not None:
    print >> sys.stderr, "%sWARNING: Digitization tool uses v.edit interface by default. " \
        "This can significantly slow down some operations especially for " \
        "middle-large vector maps. "\
        "You can enable experimental vdigit interface by setting " \
          "USEVEDIT to False in digit.py file." % \
          os.linesep


class AbstractDigit:
    """
    Abstract digitization class
    """
    def __init__(self, mapwindow, settings=None):
        """Initialization

        @param mapwindow reference to mapwindow (MapFrame) instance
        @param settings  initial settings of digitization tool
        """
        self.map       = None

        Debug.msg (3, "AbstractDigit.__init__(): map=%s" % \
                   self.map)

        #self.SetCategory()

        # is unique for map window instance
        if not settings:
            self.settings = {}
            # symbology
            # self.settings["symbolBackground"] = (None, (255,255,255, 255)) # white
            self.settings["symbolHighlight"] = (None, (255, 255, 0, 255)) #yellow
            self.settings["symbolPoint"] = (True, (0, 0, 0, 255)) # black
            self.settings["symbolLine"] = (True, (0, 0, 0, 255)) # black
            self.settings["symbolBoundaryNo"] = (True, (126, 126, 126, 255)) # grey
            self.settings["symbolBoundaryOne"] = (True, (0, 255, 0, 255)) # green
            self.settings["symbolBoundaryTwo"] = (True, (255, 135, 0, 255)) # orange
            self.settings["symbolCentroidIn"] = (True, (0, 0, 255, 255)) # blue
            self.settings["symbolCentroidOut"] = (True, (165, 42, 42, 255)) # brown
            self.settings["symbolCentroidDup"] = (True, (156, 62, 206, 255)) # violet
            self.settings["symbolNodeOne"] = (True, (255, 0, 0, 255)) # red
            self.settings["symbolNodeTwo"] = (True, (0, 86, 45, 255)) # dark green
            self.settings["symbolVertex"] = (False, (255, 20, 147, 255)) # deep pink
            
            # display
            self.settings["lineWidth"] = (2, "screen pixels")

            # snapping
            self.settings["snapping"] = (10, "screen pixels") # value, unit
            self.settings["snapToVertex"] = False
            self.settings["backgroundMap"] = ''

            # digitize new record
            self.settings["addRecord"] = True
            self.settings["layer"] = 1
            self.settings["category"] = 1
            self.settings["categoryMode"] = "Next to use"
            
            # delete existing feature(s)
            self.settings["delRecord"] = True

            # query tool
            self.settings["query"]       = ("length", True)   # name, select by box
            self.settings["queryLength"] = ("shorter than", 0) # gt or lt, threshold
            self.settings["queryDangle"] = ("shorter than", 0)
            # select feature (point, line, centroid, boundary)
            self.settings["selectFeature"] = {}
            self.settings["selectFeature"]["point"] = {'id': None, 'val': True}
            self.settings["selectFeature"]["line"] = {'id': None, 'val': True}
            self.settings["selectFeature"]["centroid"] = {'id': None, 'val': True}
            self.settings["selectFeature"]["boundary"] = {'id': None, 'val': True} 
        else:
            self.settings = settings

        self.driver = CDisplayDriver(self, mapwindow)

    def SetCategoryNextToUse(self):
        """Find maximum category number in the map layer
        and update Digit.settings['category']

        @return 'True' on success, 'False' on failure
        """
        # vector map layer without categories, reset to '1'
        self.settings['category'] = 1

        if self.map:
            if USEVEDIT:
                categoryCmd = gcmd.Command(cmd=["v.category", "-g", "--q",
                                                "input=%s" % self.map, 
                                                "option=report"])

                if categoryCmd.returncode != 0:
                    return False
        
                for line in categoryCmd.ReadStdOutput():
                    if "all" in line:
                        if self.settings['layer'] != int(line.split(' ')[0]):
                            continue
                        try:
                            maxCat = int(line.split(' ')[-1]) + 1
                            self.settings['category'] = maxCat
                        except:
                            return False
                        return True
            else:
                self.settings['category'] = self.digit.GetCategory(self.settings['layer']) + 1
    
    def SetCategory(self):
        """Return category number to use (according Settings)"""
        if self.settings["categoryMode"] == "No category":
            self.settings["category"] = 1
        elif self.settings["categoryMode"] == "Next to use":
            self.SetCategoryNextToUse()

        return self.settings["category"]

    def SetMapName(self, map):
        """Set map name

        @param map map name to be set up or None (will close currently edited map)
        """
        Debug.msg (3, "AbstractDigit.SetMapName map=%s" % map)
        self.map = map

        ret = self.driver.Reset(self.map)
        if map and ret == -1:
            raise gcmd.DigitError(_('Unable to open vector map <%s> for editing. The vector map is probably broken. '
                               'Try to run v.build for rebuilding the topology.') % map)
        if not map and ret != 0:
            raise gcmd.DigitError(_('Closing vector map <%s> failed. The vector map is probably broken. '
                               'Try to run v.build for rebuilding the topology.') % map)
            
        if not USEVEDIT:
            try:
                self.digit.InitCats()
            except:
                pass

    def SelectLinesByQueryThresh(self):
        """Generic method used for SelectLinesByQuery()
        -- to get threshold value"""
        thresh = 0.0
        if self.settings['query'][0] == "length":
            thresh = self.settings['queryLength'][1]
            if self.settings["queryLength"][0] == "shorter than":
                thresh = -1 * thresh
        else:
            thresh = self.settings['queryDangle'][1]
            if self.settings["queryDangle"][0] == "shorter than":
                thresh = -1 * thresh

        return thresh

    def GetSelectType(self):
        """Get type(s) to be selected

        Used by SelectLinesByBox() and SelectLinesByPoint()"""

        type = 0
        for feature in (('point', vdigit.GV_POINT),
                        ('line', vdigit.GV_LINE),
                        ('centroid', vdigit.GV_CENTROID),
                        ('boundary', vdigit.GV_BOUNDARY)):
            if self.settings['selectFeature'][feature[0]]['val'] is True:
                type |= feature[1]

        return type

class VEdit(AbstractDigit):
    """
    Prototype of digitization class based on v.edit command

    Note: This should be replaced by VDigit class.
    """
    def __init__(self, mapwindow, settings=None):
        """Initialization

        @param mapwindow reference to mapwindow (MapFrame) instance
        @param settings  initial settings of digitization tool
        """
        AbstractDigit.__init__(self, mapwindow, settings)

    def AddPoint (self, map, point, x, y, z=None):
        """Add point/centroid

        @param map   map name
        @param point feature type (True for point, otherwise centroid)
        @param x,y,z coordinates
        """
        if point:
            key = "P"
        else:
            key = "C"

        layer = self.settings["layer"]
        cat   = self.SetCategory()
        
        if layer > 0 and cat != "None":
            addstring =  "%s 1 1\n" % (key)
        else:
            addstring =  "%s 1\n" % (key)

        addstring += "%f %f\n" % (x, y)

        if layer > 0 and cat != "None":
            addstring += "%d %d\n" % (layer, cat)
            Debug.msg (3, "VEdit.AddPoint(): map=%s, type=%s, layer=%d, cat=%d, x=%f, y=%f" % \
                           (map, type, layer, cat, x, y))
        else:
            Debug.msg (3, "VEdit.AddPoint(): map=%s, type=%s, x=%f, y=%f" % \
                           (map, type, x, y))

        Debug.msg (4, "Vline.AddPoint(): input=%s" % addstring)
                
        self.__AddFeature (map=map, input=addstring)

    def AddLine (self, map, line, coords):
        """Add line/boundary

        @param map  map name
        @param line feature type (True for line, otherwise boundary)
        @param list of coordinates
        """
        if len(coords) < 2:
            return

        layer = self.settings["layer"]
        cat   = self.SetCategory()
        
        if line:
            key = "L"
            flags = []
        else:
            key = "B"
            flags = ['-c'] # close boundaries
            
        if layer > 0 and cat != "None":
            addstring = "%s %d 1\n" % (key, len(coords))
        else:
            addstring = "%s %d\n" % (key, len(coords))

        for point in coords:
            addstring += "%f %f\n" % \
                (float(point[0]), float(point [1]))

        if layer > 0 and cat != "None":
            addstring += "%d %d\n" % (layer, cat)
            Debug.msg (3, "Vline.AddLine(): type=%s, layer=%d, cat=%d coords=%s" % \
                           (key, layer, cat, coords))
        else:
            Debug.msg (3, "Vline.AddLine(): type=%s, coords=%s" % \
                           (key, coords))

        Debug.msg (4, "VEdit.AddLine(): input=%s" % addstring)

        self.__AddFeature (map=map, input=addstring, flags=flags)

    def __AddFeature (self, map, input, flags=[]):
        """Generic method to add new vector feature

        @param map   map name
        @param input feature definition in GRASS ASCII format
        @param flags additional flags
        """
        if self.settings['snapping'][0] <= 0.0:
            snap = "no"
        else:
            if self.settings['snapToVertex']:
                snap = "vertex"
            else:
                snap = "node"

        command = ["v.edit", "-n", "--q", 
                   "map=%s" % map,
                   "tool=add",
                   "thresh=%f" % self.driver.GetThreshold(),
                   "snap=%s" % snap]

        if self.settings['backgroundMap'] != '':
            command.append("bgmap=%s" % self.settings['backgroundMap'])

        # additional flags
        for flag in flags:
            command.append(flag)

        # run the command
        Debug.msg(4, "VEdit.AddFeature(): input=%s" % input)
        vedit = gcmd.Command(cmd=command, stdin=input, stderr=None)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()
        
    def DeleteSelectedLines(self):
        """Delete selected features"""
        selected = self.driver.GetSelected() # grassId

        if len(selected) <= 0:
            return False

        ids = ",".join(["%d" % v for v in selected])

        Debug.msg(4, "Digit.DeleteSelectedLines(): ids=%s" % \
                      ids)

        # delete also attributes if requested
        if self.settings['delRecord'] is True:
            layerCommand = gcmd.Command(cmd=["v.db.connect",
                                             "-g", "--q",
                                             "map=%s" % self.map],
                                        rerr=None, stderr=None)
            if layerCommand.returncode == 0:
                layers = {}
                for line in layerCommand.ReadStdOutput():
                    lineList = line.split(' ')
                    layers[int(lineList[0])] = { "table"    : lineList[1],
                                                 "key"      : lineList[2],
                                                 "database" : lineList[3],
                                                 "driver"   : lineList[4] }
                for layer in layers.keys():
                    printCats = gcmd.Command(['v.category',
                                              '--q',
                                              'input=%s' % self.map,
                                              'layer=%d' % layer,
                                              'option=print',
                                              'id=%s' % ids])
                    sql = 'DELETE FROM %s WHERE' % layers[layer]['table']
                    n_cats = 0
                    for cat in printCats.ReadStdOutput():
                        for c in cat.split('/'):
                            sql += ' cat = %d or' % int(c)
                            n_cats += 1
                    sql = sql.rstrip(' or')
                    if n_cats > 0:
                        gcmd.Command(['db.execute',
                                      '--q',
                                      'driver=%s' % layers[layer]['driver'],
                                      'database=%s' % layers[layer]['database']],
                                     stdin=sql,
                                     rerr=None, stderr=None)

        command = [ "v.edit",
                    "map=%s" % self.map,
                    "tool=delete",
                    "ids=%s" % ids]

        # run the command
        vedit = gcmd.Command(cmd=command, stderr=None)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()

        return True

    def MoveSelectedLines(self, move):
        """Move selected features

        @param move X,Y direction
        """
        return self.__MoveFeature("move", None, move)

    def MoveSelectedVertex(self, coords, move):
        """Move selected vertex

        Feature geometry is changed.

        @param coords click coordinates
        @param move   X,Y direction
        """
        return self.__MoveFeature("vertexmove", coords, move)

    def __MoveFeature(self, tool, coords, move):
        """Move selected vector feature (line, vertex)

        @param tool   tool for v.edit
        @param coords click coordinates
        @param move   direction (x, y)
        """
        selected = self.driver.GetSelected()

        if len(selected) <= 0:
            return False

        ids = ",".join(["%d" % v for v in selected])

        Debug.msg(4, "Digit.MoveSelectedLines(): ids=%s, move=%s" % \
                      (ids, move))

        if self.settings['snapping'][0] <= 0:
            snap = "no"
        else:
            if self.settings['snapToVertex']:
                snap = "vertex"
            else:
                snap = "node"


        command = ["v.edit", "--q", 
                   "map=%s" % self.map,
                   "tool=%s" % tool,
                   "ids=%s" % ids,
                   "move=%f,%f" % (float(move[0]),float(move[1])),
                   "thresh=%f" % self.driver.GetThreshold(),
                   "snap=%s" % snap]

        if tool == "vertexmove":
            command.append("coords=%f,%f" % (float(coords[0]), float(coords[1])))
            command.append("-1") # modify only first selected
                                             
        # run the command
        vedit = gcmd.Command(cmd=command, stderr=None)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()

        return True

    def AddVertex(self, coords):
        """Add new vertex to the selected line/boundary on position 'coords'

        @param coords coordinates to add vertex
        """
        return self.__ModifyVertex(coords, "vertexadd")

    def RemoveVertex(self, coords):
        """Remove vertex from the selected line/boundary on position 'coords'

        @param coords coordinates to remove vertex
        """
        return self.__ModifyVertex(coords, "vertexdel")
    
    def __ModifyVertex(self, coords, action):
        """Generic method for vertex manipulation

        @param coords coordinates
        @param action operation to perform
        """
        try:
            line = self.driver.GetSelected()[0]
        except:
            return False

        command = ["v.edit", "--q",
                   "map=%s" % self.map,
                   "tool=%s" % action,
                   "ids=%s" % line,
                   "coords=%f,%f" % (float(coords[0]),float(coords[1])),
                   "thresh=%f" % self.driver.GetThreshold()]

        # run the command
        vedit = gcmd.Command(cmd=command, stderr=None)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()
        
        return True

    def SplitLine(self, coords):
        """Split selected line/boundary on position 'coords'

        @param coords coordinates to split line
        """
        try:
            line = self.driver.GetSelected()[0]
        except:
            return False

        command = ["v.edit", "--q",
                   "map=%s" % self.map,
                   "tool=break",
                   "ids=%s" % line,
                   "coords=%f,%f" % (float(coords[0]),float(coords[1])),
                   "thresh=%f" % self.driver.GetThreshold()]

        # run the command
        vedit = gcmd.Command(cmd=command, stderr=None)

        # redraw map
        self.driver.ReloadMap()
        
        return True

    def EditLine(self, line, coords):
        """Edit existing line/boundary

        @param line id of line to be modified
        @param coords list of coordinates of modified line
        """
        # remove line
        vEditDelete = gcmd.Command(['v.edit',
                                   '--q',
                                   'map=%s' % self.map,
                                   'tool=delete',
                                   'ids=%s' % line], stderr=None)

        # add line
        if len(coords) > 0:
            self.AddLine(self.map, "line", coords)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()

    def __ModifyLines(self, tool):
        """Generic method to modify selected lines/boundaries

        @param tool operation to be performed by v.edit
        """
        ids = self.driver.GetSelected()

        if len(ids) <= 0:
            return False

        vEdit = ['v.edit',
                 '--q',
                 'map=%s' % self.map,
                 'tool=%s' % tool,
                 'ids=%s' % ",".join(["%d" % v for v in ids])]

        if tool in ['snap', 'connect']:
            vEdit.append("thresh=%f" % self.driver.GetThreshold())

        runCmd = gcmd.Command(vEdit)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()
                        
        return True

    def FlipLine(self):
        """Flip selected lines/boundaries"""
        return self.__ModifyLines('flip')

    def MergeLine(self):
        """Merge selected lines/boundaries"""
        return self.__ModifyLines('merge')

    def BreakLine(self):
        """Break selected lines/boundaries"""
        return self.__ModifyLines('break')

    def SnapLine(self):
        """Snap selected lines/boundaries"""
        return self.__ModifyLines('snap')

    def ConnectLine(self):
        """Connect selected lines/boundaries"""
        return self.__ModifyLines('connect')


    def ZBulkLine(self, pos1, pos2, value, step):
        """Provide z bulk-labeling (automated assigment of z coordinate
        to 3d lines

        @param pos1,pos2 bounding box definition for selecting lines to be labeled
        @param value starting value
        @param step  step value
        """
        gcmd.Command(['v.edit',
                      '--q',
                      'map=%s' % self.map,
                      'tool=zbulk',
                      'bbox=%f,%f,%f,%f' % (pos1[0], pos1[1], pos2[0], pos2[1]),
                      'zbulk=%f,%f' % (value, step)])


    def CopyLine(self, ids=None):
        """Copy features from (background) vector map

        @param ids list of line ids to be copied
        """
        if not ids:
            ids = self.driver.GetSelected()

        if len(ids) <= 0:
            return False

        vEdit = ['v.edit',
                 '--q',
                 'map=%s' % self.map,
                 'tool=copy',
                 'ids=%s' % ",".join(["%d" % v for v in ids])]

        if self.settings['backgroundMap'] != '':
            vEdit.append('bgmap=%s' % self.settings['backgroundMap'])

        runCmd = gcmd.Command(vEdit)

        # reload map (needed for v.edit)
        self.driver.ReloadMap()
                        
        return True

    def CopyCats(self, cats, ids):
        """Copy given categories to objects with id listed in ids

        @param cats list of cats to be copied
        @param ids  ids of lines to be modified
        """
        if len(cats) == 0 or len(ids) == 0:
            return False

        # collect cats
        gcmd.Command(['v.edit',
                     '--q',
                     'map=%s' % self.map,
                     'tool=catadd',
                     'cats=%s' % ",".join(["%d" % v for v in cats]),
                     'ids=%s' % ",".join(["%d" % v for v in ids])])
        
        # reload map (needed for v.edit)
        self.driver.ReloadMap()

        return True

    def SelectLinesFromBackgroundMap(self, pos1, pos2):
        """Select features from background map

        @param pos1,pos2 bounding box defifinition
        """

        if self.settings['backgroundMap'] == '':
            Debug.msg(4, "VEdit.SelectLinesFromBackgroundMap(): []")
            return []

        x1, y1 = pos1
        x2, y2 = pos2

        vEditCmd = gcmd.Command(['v.edit',
                                 '--q',
                                 'map=%s' % self.settings['backgroundMap'],
                                 'tool=select',
                                 'bbox=%f,%f,%f,%f' % (pos1[0], pos1[1], pos2[0], pos2[1])])
                                 #'polygon=%f,%f,%f,%f,%f,%f,%f,%f,%f,%f' % \
                                 #    (x1, y1, x2, y1, x2, y2, x1, y2, x1, y1)])
                                             
        try:
            output = vEditCmd.ReadStdOutput()[0] # first line
            ids = output.split(',') 
            ids = map(int, ids) # str -> int
        except:
            return []

        Debug.msg(4, "VEdit.SelectLinesFromBackgroundMap(): %s" % \
                      ",".join(["%d" % v for v in ids]))
        
        return ids

    def SelectLinesByQuery(self, pos1, pos2):
        """Select features by query

        @param pos1, pos2 bounding box definition
        """
        thresh = self.SelectLinesByQueryThresh()
        
        w, n = pos1
        e, s = pos2

        if self.settings['query'][1] == False: # select globaly
            vInfo = gcmd.Command(['v.info',
                                  'map=%s' % self.map,
                                  '-g'])
            for item in vInfo.ReadStdOutput():
                if 'north' in item:
                    n = float(item.split('=')[1])
                elif 'south' in item:
                    s = float(item.split('=')[1])
                elif 'east' in item:
                    e = float(item.split('=')[1])
                elif 'west' in item:
                    w = float(item.split('=')[1])

        vEdit = (['v.edit',
                  '--q',
                  'map=%s' % self.map,
                  'tool=select',
                  'bbox=%f,%f,%f,%f' % (w, n, e, s),
                  'query=%s' % self.settings['query'][0],
                  'thresh=%f' % thresh])

        vEditCmd = gcmd.Command(vEdit)
        
        try:
            output = vEditCmd.ReadStdOutput()[0] # first line
            ids = output.split(',') 
            ids = map(int, ids) # str -> int
        except:
            return []

        Debug.msg(4, "VEdit.SelectLinesByQuery(): %s" % \
                      ",".join(["%d" % v for v in ids]))
        
        return ids

    def GetLayers(self):
        """Return list of layers"""
        layerCommand = gcmd.Command(cmd=["v.db.connect",
                                             "-g", "--q",
                                             "map=%s" % self.map],
                                        rerr=None, stderr=None)
        if layerCommand.returncode == 0:
            layers = []
            for line in layerCommand.ReadStdOutput():
                lineList = line.split(' ')
                layers.append(int(lineList[0]))
            return layers

        return [1]

class VDigit(AbstractDigit):
    """
    Prototype of digitization class based on v.digit reimplementation

    Under development (wxWidgets C/C++ background)
    """
    def __init__(self, mapwindow, settings=None):
        """Initialization

        @param mapwindow reference to mapwindow (MapFrame) instance
        @param settings  initial settings of digitization tool
        """
        AbstractDigit.__init__(self, mapwindow, settings)

        try:
            self.digit = vdigit.Digit(self.driver.GetDevice())
        except (ImportError, NameError):
            self.digit = None

    def AddPoint (self, map, point, x, y, z=None):
        """Add new point/centroid

        @param map   map name (unused, for compatability with VEdit)
        @param point feature type (if true point otherwise centroid)
        @param x,y,z coordinates
        """
        layer = self.settings["layer"]
        cat   = self.SetCategory()
        
        if point:
            type = vdigit.GV_POINT 
        else:
            type = vdigit.GV_CENTROID 

        snap, thresh = self.__getSnapThreshold()

        if z:
            ret = self.digit.AddLine(type, [x, y, z], layer, cat,
                                     str(self.settings["backgroundMap"]), snap, thresh)
        else:
            ret = self.digit.AddLine(type, [x, y], layer, cat,
                                     str(self.settings["backgroundMap"]), snap, thresh)

        if ret == -1:
            raise gcmd.DigitError, _("Adding new feature to vector map <%s> failed.") % map
        
    def AddLine (self, map, line, coords):
        """Add line/boundary

        @param map    map name (unused, for compatability with VEdit)
        @param line   feature type (if True line, otherwise boundary)
        @param coords list of coordinates
        """
        if len(coords) < 2:
            return

        layer = self.settings["layer"]
        cat   = self.SetCategory()

        if line:
            type = vdigit.GV_LINE
        else:
            type = vdigit.GV_BOUNDARY

        listCoords = []
        for c in coords:
            for x in c:
                listCoords.append(x)

        snap, thresh = self.__getSnapThreshold()

        ret = self.digit.AddLine(type, listCoords, layer, cat,
                                 str(self.settings["backgroundMap"]), snap, thresh)

        if ret == -1:
            raise gcmd.DigitError, _("Adding new feature to vector map <%s> failed.") % map


    def DeleteSelectedLines(self):
        """Delete selected features

        @return number of deleted lines
        """
        nlines = self.digit.DeleteLines(self.settings['delRecord'])

        return nlines

    def MoveSelectedLines(self, move):
        """Move selected features

        @param move direction (x, y)
        """
        snap, thresh = self.__getSnapThreshold()

        nlines = self.digit.MoveLines(move[0], move[1], 0.0, # TODO 3D
                                      snap, thresh) 

        return nlines

    def MoveSelectedVertex(self, coords, move):
        """Move selected vertex of the line

        @param coords click coordinates
        @param move   X,Y direction

        @return 1 vertex moved
        @return 0 vertex not moved (not found, line is not selected)
        """
        snap, thresh = self.__getSnapThreshold()

        return self.digit.MoveVertex(coords[0], coords[1], 0.0, # TODO 3D
                                     move[0], move[1], 0.0,
                                     snap, thresh)

    def AddVertex(self, coords):
        """Add new vertex to the selected line/boundary on position 'coords'

        @param coords coordinates to add vertex
        """
        return self.digit.ModifyLineVertex(1, coords[0], coords[1], 0.0, # TODO 3D
                                           self.driver.GetThreshold())

    def RemoveVertex(self, coords):
        """Remove vertex from the selected line/boundary on position 'coords'

        @param coords coordinates to remove vertex
        """
        return self.digit.ModifyLineVertex(0, coords[0], coords[1], 0.0, # TODO 3D
                                           self.driver.GetThreshold())

    def SplitLine(self, coords):
        """Split selected line/boundary on position 'coords'

        @param coords coordinates to split line

        @return 1 line splitted
        @return 0 no action
        @return -1 error
        """
        return self.digit.SplitLine(coords[0], coords[1], 0.0, # TODO 3D
                                    self.driver.GetThreshold())

    def EditLine(self, line, coords):
        """Edit existing line/boundary

        @param line id of line to be modified
        @param coords list of coordinates of modified line
        """
        try:
            lineid = line[0]
        except:
            lineid = -1

        listCoords = []
        for c in coords:
            for x in c:
                listCoords.append(x)

        return self.digit.RewriteLine(lineid, listCoords)

    def FlipLine(self):
        """Flip selected lines/boundaries"""
        return self.digit.FlipLines()

    def MergeLine(self):
        """Merge selected lines/boundaries"""
        return self.digit.MergeLines()

    def BreakLine(self):
        """Break selected lines/boundaries"""
        return self.digit.BreakLines()

    def SnapLine(self):
        """Snap selected lines/boundaries"""
        snap, thresh = self.__getSnapThreshold()
        return self.digit.SnapLines(thresh)

    def ConnectLine(self):
        """Connect selected lines/boundaries"""
        snap, thresh = self.__getSnapThreshold()
        return self.digit.ConnectLines(thresh)

    def CopyLine(self, ids=None):
        """Copy features from (background) vector map

        @param ids list of line ids to be copied
        """
        return self.digit.CopyLines(ids, self.settings['backgroundMap'])

    def CopyCats(self, cats, ids):
        """Copy given categories to objects with id listed in ids

        @param cats list of cats to be copied
        @param ids  ids of lines to be modified
        """
        if len(cats) == 0 or len(ids) == 0:
            return False

        return self.digit.CopyCats(cats, ids)

    def SelectLinesByQuery(self, pos1, pos2):
        """Select features by query

        @param pos1, pos2 bounding box definition
        """
        thresh = self.SelectLinesByQueryThresh()
        
        w, n = pos1
        e, s = pos2

        query = vdigit.QUERY_UNKNOWN
        if self.settings['query'][0] == 'length':
            query = vdigit.QUERY_LENGTH
        elif self.settings['query'][0] == 'dangle':
            query = vdigit.QUERY_DANGLE

        type = vdigit.GV_POINTS | vdigit.GV_LINES # TODO: 3D
        
        ids = self.digit.SelectLinesByQuery(w, n, 0.0, e, s, 1000.0, self.settings['query'][1],
                                            query, type, thresh)

        Debug.msg(4, "VDigit.SelectLinesByQuery(): %s" % \
                      ",".join(["%d" % v for v in ids]))
        
        return ids

    def GetLineCats(self, line=-1):
        """Get layer/category pairs from given (selected) line
        
        @param line feature id (-1 for first selected line)
        """
        return self.digit.GetLineCats(line)

    def SetLineCats(self, line, layer, cats, add=True):
        """Set categories for given line and layer

        @param line feature id
        @param layer layer number (-1 for first selected line)
        @param cats list of categories
        @param add if True to add, otherwise do delete categories
        """
        return self.digit.SetLineCats(line, layer, cats, add)

    def GetLayers(self):
        """Get list of layers"""
        return self.digit.GetLayers()

    def __getSnapThreshold(self):
        """Get snap mode and threshold value

        @return (snap, thresh)
        """
        thresh = self.driver.GetThreshold()

        if thresh > 0.0:
            if self.settings['snapToVertex']:
                snap = vdigit.SNAPVERTEX
            else:
                snap = vdigit.SNAP
        else:
            snap = v.digit.NO_SNAP

        return (snap, thresh)

if USEVEDIT:
    class Digit(VEdit):
        """Default digit class"""
        def __init__(self, mapwindow):
            VEdit.__init__(self, mapwindow)
            self.type = 'vedit'
else:
    class Digit(VDigit):
        """Default digit class"""
        def __init__(self, mapwindow):
            VDigit.__init__(self, mapwindow)
            self.type = 'vdigit'

class AbstractDisplayDriver:
    """Abstract classs for display driver"""
    def __init__(self, parent, mapwindow):
        """Initialization

        @param parent
        @param mapwindow reference to mapwindow (MFrame)
        """
        self.parent      = parent
        self.mapwindow   = mapwindow
        
        self.ids         = {}   # dict[g6id] = [pdcId]
        self.selected    = []   # list of selected objects (grassId!)

    def GetThreshold(self, value=None, units=None):
        """Return threshold in map units

        @param value threshold to be set up
        @param units units (map, screen)
        """
        if not value:
            value = self.parent.settings["snapping"][0]

        if not units:
            units = self.parent.settings["snapping"][1]

        if units == "screen pixels":
            # pixel -> cell
            reg = self.mapwindow.Map.region
            if reg['nsres'] > reg['ewres']:
                res = reg['nsres']
            else:
                res = reg['ewres']
                
            threshold = value * res
        else:
            threshold = value

        Debug.msg(4, "AbstractDisplayDriver.GetThreshold(): thresh=%f" % threshold)
        
        return threshold

class CDisplayDriver(AbstractDisplayDriver):
    """
    Display driver using grass6_wxdriver module
    """
    def __init__(self, parent, mapwindow):
        """Initialization

        @param parent
        @param mapwindow reference to mapwindow (MFrame)
        """
        AbstractDisplayDriver.__init__(self, parent, mapwindow)

        self.mapWindow = mapwindow

        # initialize wx display driver
        try:
            self.__display = vdigit.DisplayDriver(mapwindow.pdcVector)
        except:
            self.__display = None
            
        settings = self.parent.settings
        self.UpdateSettings()

    def GetDevice(self):
        """Get device"""
        return self.__display
    
    def SetDevice(self, pdc):
        """Set device for driver

        @param pdc wx.PseudoDC instance
        """
        self.__display.SetDevice(pdc)
            
    def Reset(self, map):
        """Reset map

        Open or close the vector map by driver.

        @param map map name or None to close the map

        @return 0 on success (close map)
        @return topo level on success (open map)
        @return non-zero (close map)
        @return -1 on error (open map)
        """
        if map:
            name, mapset = map.split('@')
            try:
                if USEVEDIT:
                    ret = self.__display.OpenMap(str(name), str(mapset), False)
                else:
                    ret = self.__display.OpenMap(str(name), str(mapset), True)
            except SystemExit:
                ret = -1
        else:
            ret = self.__display.CloseMap()

        return ret
    
    def ReloadMap(self):
        """Reload map (close and re-open).

        Needed for v.edit, TODO: get rid of that..."""
        
        Debug.msg(4, "CDisplayDriver.ReloadMap():")
        self.__display.ReloadMap()

    def DrawMap(self):
        """Draw vector map layer content

        @return wx.Image instance
        """
        import time
        start = time.clock()
        nlines = self.__display.DrawMap(True) # force
        stop = time.clock()
        Debug.msg(3, "CDisplayDriver.DrawMap(): nlines=%d, sec=%f" % \
                      (nlines, stop-start))

        return nlines

    def SelectLinesByBox(self, begin, end, type=0):
        """Select vector features by given bounding box.

        If type is given, only vector features of given type are selected.

        @param begin,end bounding box definition
        @param type      select only objects of given type
        """
        x1, y1 = begin
        x2, y2 = end

        nselected = self.__display.SelectLinesByBox(x1, y1, -1.0 * vdigit.PORT_DOUBLE_MAX,
                                                    x2, y2, vdigit.PORT_DOUBLE_MAX,
                                                    type)
        Debug.msg(4, "CDisplayDriver.SelectLinesByBox(): selected=%d" % \
                      nselected)

        return nselected

    def SelectLineByPoint(self, point, type=0):
        """Select vector feature by coordinates of click point (in given threshold).

        If type is given, only vector features of given type are selected.

        @param point click coordinates (bounding box given by threshold)
        @param type  select only objects of given type
        """
        pointOnLine = self.__display.SelectLineByPoint(point[0], point[1], 0.0,
                                                       self.GetThreshold(),
                                                       type, 0); # without_z

        if len(pointOnLine) > 0:
            Debug.msg(4, "CDisplayDriver.SelectLineByPoint(): pointOnLine=%f,%f" % \
                          (pointOnLine[0], pointOnLine[1]))
            return pointOnLine
        else:
            Debug.msg(4, "CDisplayDriver.SelectLineByPoint(): no line found")
            return None
        
    def GetSelected(self, grassId=True):
        """Return ids of selected vector features
        
        @param grassId if grassId is True returns GRASS ids, otherwise
        internal ids of objects drawn in PseudoDC"""
        if grassId:
            selected = self.__display.GetSelected(True)
        else:
            selected = self.__display.GetSelected(False)
            
        Debug.msg(4, "CDisplayDriver.GetSelected(): grassId=%d, ids=%s" % \
                      (grassId, (",".join(["%d" % v for v in selected]))))
            
        return selected

    def GetSelectedVertex(self, coords):
        """Get PseudoDC id(s) of vertex (of selected line)
        on position 'coords'

        @param coords click position
        """
        x, y = coords

        id = self.__display.GetSelectedVertex(x, y, self.GetThreshold())

        Debug.msg(4, "CDisplayDriver.GetSelectedVertex(): id=%s" % \
                      (",".join(["%d" % v for v in id])))

        return id 

    def SetSelected(self, id):
        """Set selected vector features

        @param id line id to be selected
        """
        Debug.msg(4, "CDisplayDriver.SetSelected(): id=%s" % \
                  ",".join(["%d" % v for v in id]))

        self.__display.SetSelected(id)

    def UpdateRegion(self):
        """Set geographical region
        
        Needed for 'cell2pixel' conversion"""
        
        map = self.mapwindow.Map
        reg = map.region
        
        self.__display.SetRegion(reg['n'],
                                 reg['s'],
                                 reg['e'],
                                 reg['w'],
                                 reg['nsres'],
                                 reg['ewres'],
                                 reg['center_easting'],
                                 reg['center_northing'],
                                 map.width, map.height)
    
    def UpdateSettings(self):
        """Update display driver settings"""
        settings = self.parent.settings
        # TODO map units

        if not self.__display:
            return
        
        self.__display.UpdateSettings (wx.Color(settings['symbolHighlight'][1][0],
                                                settings['symbolHighlight'][1][1],
                                                settings['symbolHighlight'][1][2],
                                                255).GetRGB(),
                                       settings['symbolPoint'][0],
                                       wx.Color(settings['symbolPoint'][1][0],
                                                settings['symbolPoint'][1][1],
                                                settings['symbolPoint'][1][2],
                                                255).GetRGB(),
                                       settings['symbolLine'][0],
                                       wx.Color(settings['symbolLine'][1][0],
                                                settings['symbolLine'][1][1],
                                                settings['symbolLine'][1][2],
                                           255).GetRGB(),
                                       settings['symbolBoundaryNo'][0],
                                       wx.Color(settings['symbolBoundaryNo'][1][0],
                                                settings['symbolBoundaryNo'][1][1],
                                                settings['symbolBoundaryNo'][1][2],
                                                255).GetRGB(),
                                       settings['symbolBoundaryOne'][0],
                                       wx.Color(settings['symbolBoundaryOne'][1][0],
                                                settings['symbolBoundaryOne'][1][1],
                                                settings['symbolBoundaryOne'][1][2],
                                                255).GetRGB(),
                                       settings['symbolBoundaryTwo'][0],
                                       wx.Color(settings['symbolBoundaryTwo'][1][0],
                                                settings['symbolBoundaryTwo'][1][1],
                                                settings['symbolBoundaryTwo'][1][2],
                                                255).GetRGB(),
                                       settings['symbolCentroidIn'][0],
                                       wx.Color(settings['symbolCentroidIn'][1][0],
                                                settings['symbolCentroidIn'][1][1],
                                                settings['symbolCentroidIn'][1][2],
                                                255).GetRGB(),
                                       settings['symbolCentroidOut'][0],
                                       wx.Color(settings['symbolCentroidOut'][1][0],
                                                settings['symbolCentroidOut'][1][1],
                                                settings['symbolCentroidOut'][1][2],
                                                255).GetRGB(),
                                       settings['symbolCentroidDup'][0],
                                       wx.Color(settings['symbolCentroidDup'][1][0],
                                                settings['symbolCentroidDup'][1][1],
                                                settings['symbolCentroidDup'][1][2],
                                                255).GetRGB(),
                                       settings['symbolNodeOne'][0],
                                       wx.Color(settings['symbolNodeOne'][1][0],
                                                settings['symbolNodeOne'][1][1],
                                                settings['symbolNodeOne'][1][2],
                                                255).GetRGB(),
                                       settings['symbolNodeTwo'][0],
                                       wx.Color(settings['symbolNodeTwo'][1][0],
                                                settings['symbolNodeTwo'][1][1],
                                                settings['symbolNodeTwo'][1][2],
                                                255).GetRGB(),
                                       settings['symbolVertex'][0],
                                       wx.Color(settings['symbolVertex'][1][0],
                                                settings['symbolVertex'][1][1],
                                                settings['symbolVertex'][1][2],
                                                255).GetRGB(),
                                       settings['lineWidth'][0])

class DigitSettingsDialog(wx.Dialog):
    """
    Standard settings dialog for digitization purposes
    """
    def __init__(self, parent, title, style=wx.DEFAULT_DIALOG_STYLE):
        wx.Dialog.__init__(self, parent=parent, id=wx.ID_ANY, title=title, style=style)

        self.parent = parent # mapdisplay.BufferedWindow class instance

        # notebook
        notebook = wx.Notebook(parent=self, id=wx.ID_ANY, style=wx.BK_DEFAULT)
        self.__CreateSymbologyPage(notebook)
        parent.digit.SetCategory() # update category number (next to use)
        self.__CreateSettingsPage(notebook)
        self.__CreateQueryPage(notebook)

        # buttons
        btnApply = wx.Button(self, wx.ID_APPLY, _("Apply") )
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        btnOk = wx.Button(self, wx.ID_OK, _("OK") )
        btnOk.SetDefault()

        # bindigs
        btnApply.Bind(wx.EVT_BUTTON, self.OnApply)
        btnOk.Bind(wx.EVT_BUTTON, self.OnOK)
        btnCancel.Bind(wx.EVT_BUTTON, self.OnCancel)

        # sizers
        btnSizer = wx.StdDialogButtonSizer()
        btnSizer.AddButton(btnCancel)
        btnSizer.AddButton(btnApply)
        btnSizer.AddButton(btnOk)
        btnSizer.Realize()
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(item=notebook, proportion=1, flag=wx.EXPAND | wx.ALL, border=5)
        mainSizer.Add(item=btnSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)

    def __CreateSymbologyPage(self, notebook):
        """Create notebook page concerning with symbology settings"""

        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Symbology"))

        sizer = wx.BoxSizer(wx.VERTICAL)
        
        flexSizer = wx.FlexGridSizer (cols=3, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)

        self.symbology = {}
        for label, key in self.__SymbologyData():
            textLabel = wx.StaticText(panel, wx.ID_ANY, label)
            color = csel.ColourSelect(panel, id=wx.ID_ANY,
                                      colour=self.parent.digit.settings[key][1], size=(25, 25))
            isEnabled = self.parent.digit.settings[key][0]
            if isEnabled is not None:
                enabled = wx.CheckBox(panel, id=wx.ID_ANY, label="")
                enabled.SetValue(isEnabled)
                self.symbology[key] = (enabled, color)
            else:
                enabled = (1, 1)
                self.symbology[key] = (None, color)
            
            flexSizer.Add(textLabel, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
            flexSizer.Add(enabled, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
            flexSizer.Add(color, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=10)
        
        panel.SetSizer(sizer)
        
        return panel

    def __CreateSettingsPage(self, notebook):
        """Create notebook page concerning with symbology settings"""

        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Settings"))

        border = wx.BoxSizer(wx.VERTICAL)
        
        #
        # display section
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Display"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        flexSizer = wx.FlexGridSizer (cols=3, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)
        # line width
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Line width"))
        self.lineWidthValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(50, -1),
                                          initial=self.parent.digit.settings["lineWidth"][0],
                                          min=1, max=1e6)
        self.lineWidthUnit = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                       choices=["screen pixels", "map units"])
        self.lineWidthUnit.SetStringSelection(self.parent.digit.settings["lineWidth"][1])
        flexSizer.Add(text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.lineWidthValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(self.lineWidthUnit, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=5)

        #
        # snapping section
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Snapping"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        flexSizer1 = wx.FlexGridSizer (cols=3, hgap=5, vgap=5)
        flexSizer1.AddGrowableCol(0)
        flexSizer2 = wx.FlexGridSizer (cols=2, hgap=5, vgap=5)
        flexSizer2.AddGrowableCol(0)
        # snapping
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Snapping threshold"))
        self.snappingValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(50, -1),
                                         initial=self.parent.digit.settings["snapping"][0],
                                         min=1, max=1e6)
        self.snappingValue.Bind(wx.EVT_SPINCTRL, self.OnChangeSnappingValue)
        self.snappingUnit = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                      choices=["screen pixels", "map units"])
        self.snappingUnit.SetStringSelection(self.parent.digit.settings["snapping"][1])
        self.snappingUnit.Bind(wx.EVT_CHOICE, self.OnChangeSnappingUnits)
        flexSizer1.Add(text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer1.Add(self.snappingValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer1.Add(self.snappingUnit, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)
        # background map
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Backgroud vector map"))
        self.backgroundMap = gselect.Select(parent=panel, id=wx.ID_ANY, size=(200,-1),
                                           type="vector")
        self.backgroundMap.SetValue(self.parent.digit.settings["backgroundMap"])
        self.backgroundMap.Bind(wx.EVT_TEXT, self.OnChangeBackgroundMap)
        flexSizer2.Add(text, proportion=1, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer2.Add(self.backgroundMap, proportion=1, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        #flexSizer.Add(self.snappingUnit, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        vertexSizer = wx.BoxSizer(wx.VERTICAL)
        self.snapVertex = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                      label=_("Snap also to vertex"))
        self.snapVertex.SetValue(self.parent.digit.settings["snapToVertex"])
        vertexSizer.Add(item=self.snapVertex, proportion=0, flag=wx.EXPAND)
        self.mapUnits = self.parent.MapWindow.Map.ProjInfo()['units']
        self.snappingInfo = wx.StaticText(parent=panel, id=wx.ID_ANY,
                                          label=_("Snapping threshold is %.1f %s") % \
                                              (self.parent.digit.driver.GetThreshold(),
                                               self.mapUnits))
        vertexSizer.Add(item=self.snappingInfo, proportion=0,
                        flag=wx.ALL | wx.EXPAND, border=1)

        sizer.Add(item=flexSizer1, proportion=1, flag=wx.TOP | wx.LEFT | wx.EXPAND, border=1)
        sizer.Add(item=flexSizer2, proportion=1, flag=wx.TOP | wx.LEFT | wx.EXPAND, border=1)
        sizer.Add(item=vertexSizer, proportion=1, flag=wx.BOTTOM | wx.LEFT | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0, flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        #
        # attributes
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Digitize new feature"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        # checkbox
        self.addRecord = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                     label=_("Add new record into table"))
        self.addRecord.SetValue(self.parent.digit.settings["addRecord"])
        sizer.Add(item=self.addRecord, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        # settings
        flexSizer = wx.FlexGridSizer(cols=2, hgap=3, vgap=3)
        flexSizer.AddGrowableCol(0)
        settings = ((_("Layer"), 1), (_("Category"), 1), (_("Mode"), _("Next to use")))
        # layer
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Layer"))
        if self.parent.digit.map:
            layers = map(str, self.parent.digit.GetLayers())
        else:
            layers = [str(self.parent.digit.settings["layer"])]
        self.layer = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                               choices=layers)
        self.layer.SetStringSelection(str(self.parent.digit.settings["layer"]))
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.layer, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        # category number
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Category number"))
        self.category = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                    initial=self.parent.digit.settings["category"],
                                    min=-1e9, max=1e9) 
        if self.parent.digit.settings["categoryMode"] != "Manual entry":
            self.category.Enable(False)
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.category, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        # category mode
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Category mode"))
        self.categoryMode = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                      choices=[_("Next to use"), _("Manual entry"), _("No category")])
        self.categoryMode.SetStringSelection(self.parent.digit.settings["categoryMode"])
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.categoryMode, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0,
                   flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)
        # delete existing feature
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Delete existing feature(s)"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        # checkbox
        self.deleteRecord = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                        label=_("Delete record from table"))
        self.deleteRecord.SetValue(self.parent.digit.settings["delRecord"])
        sizer.Add(item=self.deleteRecord, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0,
                   flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        # bindings
        self.Bind(wx.EVT_CHECKBOX, self.OnChangeAddRecord, self.addRecord)
        self.Bind(wx.EVT_CHOICE, self.OnChangeCategoryMode, self.categoryMode)
        self.Bind(wx.EVT_CHOICE, self.OnChangeLayer, self.layer)

        panel.SetSizer(border)
        
        return panel

    def __CreateQueryPage(self, notebook):
        """Create notebook page for query tool"""

        settings = self.parent.digit.settings

        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Query tool"))

        border = wx.BoxSizer(wx.VERTICAL)

        #
        # query tool box
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Choose query tool"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        LocUnits = self.parent.MapWindow.Map.ProjInfo()['units']

        self.queryBox = wx.CheckBox(parent=panel, id=wx.ID_ANY, label=_("Select by box"))
        self.queryBox.SetValue(settings["query"][1])

        sizer.Add(item=self.queryBox, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        sizer.Add((0, 5))

        #
        # length
        #
        self.queryLength = wx.RadioButton(parent=panel, id=wx.ID_ANY, label=_("length"))
        self.queryLength.Bind(wx.EVT_RADIOBUTTON, self.OnChangeQuery)
        sizer.Add(item=self.queryLength, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        flexSizer = wx.FlexGridSizer (cols=4, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)
        txt = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Select lines"))
        self.queryLengthSL = wx.Choice (parent=panel, id=wx.ID_ANY, 
                                        choices = [_("shorter than"), _("longer than")])
        self.queryLengthSL.SetStringSelection(settings["queryLength"][0])
        self.queryLengthValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(100, -1),
                                            initial=1,
                                            min=0, max=1e6)
        self.queryLengthValue.SetValue(settings["queryLength"][1])
        units = wx.StaticText(parent=panel, id=wx.ID_ANY, label="%s" % LocUnits)
        flexSizer.Add(txt, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.queryLengthSL, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(self.queryLengthValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(units, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        sizer.Add(item=flexSizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)

        #
        # dangle
        #
        self.queryDangle = wx.RadioButton(parent=panel, id=wx.ID_ANY, label=_("dangle"))
        self.queryDangle.Bind(wx.EVT_RADIOBUTTON, self.OnChangeQuery)
        sizer.Add(item=self.queryDangle, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        flexSizer = wx.FlexGridSizer (cols=4, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)
        txt = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Select dangles"))
        self.queryDangleSL = wx.Choice (parent=panel, id=wx.ID_ANY, 
                                        choices = [_("shorter than"), _("longer than")])
        self.queryDangleSL.SetStringSelection(settings["queryDangle"][0])
        self.queryDangleValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(100, -1),
                                       initial=1,
                                       min=0, max=1e6)
        self.queryDangleValue.SetValue(settings["queryDangle"][1])
        units = wx.StaticText(parent=panel, id=wx.ID_ANY, label="%s" % LocUnits)
        flexSizer.Add(txt, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.queryDangleSL, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(self.queryDangleValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(units, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        sizer.Add(item=flexSizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)

        if settings["query"][0] == "length":
            self.queryLength.SetValue(True)
        else:
            self.queryDangle.SetValue(True)

        # enable & disable items
        self.OnChangeQuery(None)

        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=5)

        #
        # select box
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Select vector features"))
        sizer = wx.StaticBoxSizer(box, wx.HORIZONTAL)
        for feature in ('point', 'line',
                        'centroid', 'boundary'):
            chkbox = wx.CheckBox(parent=panel, label=feature)
            settings['selectFeature'][feature]['id'] = chkbox.GetId()
            chkbox.SetValue(settings['selectFeature'][feature]['val'])
            sizer.Add(item=chkbox, proportion=0,
                      flag=wx.EXPAND | wx.ALL, border=3)

        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=5)

        panel.SetSizer(border)
        
        return panel


    def __SymbologyData(self):
        """
        Data for __CreateSymbologyPage()

        label | checkbox | color
        """

        return (
            #            ("Background", "symbolBackground"),
            ("Highlight", "symbolHighlight"),
            ("Point", "symbolPoint"),
            ("Line", "symbolLine"),
            ("Boundary (no area)", "symbolBoundaryNo"),
            ("Boundary (one area)", "symbolBoundaryOne"),
            ("Boundary (two areas)", "symbolBoundaryTwo"),
            ("Centroid (in area)", "symbolCentroidIn"),
            ("Centroid (outside area)", "symbolCentroidOut"),
            ("Centroid (duplicate in area)", "symbolCentroidDup"),
            ("Node (one line)", "symbolNodeOne"),
            ("Node (two lines)", "symbolNodeTwo"),
            ("Vertex", "symbolVertex"))

    def OnChangeCategoryMode(self, event):
        """Change category mode"""

        mode = event.GetString()
        self.parent.digit.settings["categoryMode"] = mode
        if mode == "Manual entry": # enable
            self.category.Enable(True)
        elif self.category.IsEnabled(): # disable
            self.category.Enable(False)

        if mode == "No category" and self.addRecord.IsChecked():
            self.addRecord.SetValue(False)
        self.parent.digit.SetCategory()
        self.category.SetValue(self.parent.digit.settings['category'])

    def OnChangeLayer(self, event):
        """Layer changed"""
        layer = int(event.GetString())
        if layer > 0:
            self.parent.digit.settings['layer'] = layer
            self.parent.digit.SetCategory()
            self.category.SetValue(self.parent.digit.settings['category'])
            
        event.Skip()

    def OnChangeAddRecord(self, event):
        """Checkbox 'Add new record' status changed"""
        self.category.SetValue(str(self.parent.digit.SetCategory()))
            
    def OnChangeSnappingValue(self, event):
        """Change snapping value - update static text"""
        value = self.snappingValue.GetValue()
        if self.snappingUnit.GetStringSelection() == "map units":
            threshold = value
        else:
            threshold = self.parent.digit.driver.GetThreshold(value)

        self.snappingInfo.SetLabel(_("Snapping threshold is %.1f %s") % 
                                   (threshold,
                                    self.mapUnits))

        event.Skip()

    def OnChangeSnappingUnits(self, event):
        """Snapping units change -> update static text"""
        value = self.snappingValue.GetValue()
        units = self.snappingUnit.GetStringSelection()
        threshold = self.parent.digit.driver.GetThreshold(value, units)

        if units == "map units":
            self.snappingInfo.SetLabel(_("Snapping threshold is %.1f %s") % \
                                           (value,
                                            self.mapUnits))
        else:
            self.snappingInfo.SetLabel(_("Snapping threshold is %.1f %s") % \
                                           (threshold,
                                            self.mapUnits))
            
        event.Skip()

    def OnChangeBackgroundMap(self, event):
        """Change background map"""
        map = self.backgroundMap.GetValue()
        
        self.parent.digit.settings['backgroundMap'] = map
        
    def OnChangeQuery(self, event):
        """Change query"""
        if self.queryLength.GetValue():
            # length
            self.queryLengthSL.Enable(True)
            self.queryLengthValue.Enable(True)
            self.queryDangleSL.Enable(False)
            self.queryDangleValue.Enable(False)
        else:
            # dangle
            self.queryLengthSL.Enable(False)
            self.queryLengthValue.Enable(False)
            self.queryDangleSL.Enable(True)
            self.queryDangleValue.Enable(True)

    def OnOK(self, event):
        """Button 'OK' clicked"""
        self.UpdateSettings()
        self.parent.digittoolbar.settingsDialog = None
        self.Close()

    def OnApply(self, event):
        """Button 'Apply' clicked"""
        self.UpdateSettings()

    def OnCancel(self, event):
        """Button 'Cancel' clicked"""
        self.parent.digittoolbar.settingsDialog = None
        self.Close()

    def UpdateSettings(self):
        """Update self.parent.digit.settings"""

        # symbology
        for key, (enabled, color) in self.symbology.iteritems():
            if enabled:
                self.parent.digit.settings[key] = (enabled.IsChecked(), color.GetColour())
            else:
                self.parent.digit.settings[key] = (None, color.GetColour())
        # display
        self.parent.digit.settings["lineWidth"] = (int(self.lineWidthValue.GetValue()),
                                                   self.lineWidthUnit.GetStringSelection())

        # snapping
        self.parent.digit.settings["snapping"] = (int(self.snappingValue.GetValue()), # value
                                      self.snappingUnit.GetStringSelection()) # unit
        self.parent.digit.settings["snapToVertex"] = self.snapVertex.IsChecked()
        
        # digitize new feature
        self.parent.digit.settings["addRecord"] = self.addRecord.IsChecked()
        self.parent.digit.settings["layer"] = int(self.layer.GetStringSelection())
        if self.parent.digit.settings["categoryMode"] == "No category":
            self.parent.digit.settings["category"] = None
        else:
            self.parent.digit.settings["category"] = int(self.category.GetValue())
        self.parent.digit.settings["categoryMode"] = self.categoryMode.GetStringSelection()

        # delete existing feature
        self.parent.digit.settings["delRecord"] = self.deleteRecord.IsChecked()

        # threshold
        try:
            self.parent.digit.threshold = self.parent.digit.driver.GetThreshold()
        except:
            pass

        # query tool
        if self.queryLength.GetValue():
            self.parent.digit.settings["query"] = ("length", self.queryBox.IsChecked())
        else:
            self.parent.digit.settings["query"] = ("dangle", self.queryBox.IsChecked())
        self.parent.digit.settings["queryLength"] = (self.queryLengthSL.GetStringSelection(),
                                                     int(self.queryLengthValue.GetValue()))
        self.parent.digit.settings["queryDangle"] = (self.queryDangleSL.GetStringSelection(),
                                                     int(self.queryDangleValue.GetValue()))

        # select features
        for feature in ('point', 'line',
                        'centroid', 'boundary'):
            self.parent.digit.settings['selectFeature'][feature]['val'] = \
                self.FindWindowById(self.parent.digit.settings["selectFeature"][feature]['id']).IsChecked()

        # update driver settings
        self.parent.digit.driver.UpdateSettings()

        # redraw map if auto-rendering is enabled
        if self.parent.autoRender.GetValue(): 
            self.parent.ReRender(None)

class DigitCategoryDialog(wx.Dialog, listmix.ColumnSorterMixin):
    """
    Dialog used to display/modify categories of vector objects
    
    @param parent
    @param title dialog title
    @param query {coordinates, qdist}    - v.edit/v.what
    @param cats  directory of categories - vdigit
    @param line  line id                 - vdigit
    @param pos
    @param style
    """
    def __init__(self, parent, title,
                 map, query=None, cats=None, line=None,
                 pos=wx.DefaultPosition,
                 style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER):

        # parent
        self.parent = parent # mapdisplay.BufferedWindow class instance

        # map name
        self.map = map

        # line id (if not found remains 'None')
        self.line = None

        # {layer: [categories]}
        self.cats = {}

        # do not display dialog if no line is found (-> self.cats)
        if cats is None:
            if self.__GetCategories(query[0], query[1]) == 0 or not self.line:
                Debug.msg(3, "DigitCategoryDialog(): nothing found!")
                return
        else:
            # self.cats = dict(cats)
            for layer in cats.keys():
                self.cats[layer] = list(cats[layer]) # TODO: tuple to list
            self.line = line

        # make copy of cats (used for 'reload')
        self.cats_orig = copy.deepcopy(self.cats)

        Debug.msg(3, "DigitCategoryDialog(): line=%d, cats=%s" % \
                      (self.line, self.cats))

        wx.Dialog.__init__(self, parent=self.parent, id=wx.ID_ANY, title=title,
                           style=style, pos=pos)

        # list of categories
        box = wx.StaticBox(parent=self, id=wx.ID_ANY,
                           label=" %s " % _("List of categories"))
        listSizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        self.list = CategoryListCtrl(parent=self, id=wx.ID_ANY,
                                     style=wx.LC_REPORT |
                                     wx.BORDER_NONE |
                                     wx.LC_SORT_ASCENDING |
                                     wx.LC_HRULES |
                                     wx.LC_VRULES)
        # sorter
        self.itemDataMap = self.list.Populate()
        listmix.ColumnSorterMixin.__init__(self, 2)

        listSizer.Add(item=self.list, proportion=1, flag=wx.EXPAND)

        # add new category
        box = wx.StaticBox(parent=self, id=wx.ID_ANY,
                           label=" %s " % _("Add new category"))
        addSizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        flexSizer = wx.FlexGridSizer (cols=5, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(3)

        layerNewTxt = wx.StaticText(parent=self, id=wx.ID_ANY,
                                 label="%s:" % _("Layer"))
        self.layerNew = wx.TextCtrl(parent=self, id=wx.ID_ANY, size=(50, -1),
                                    value="1")
        catNewTxt = wx.StaticText(parent=self, id=wx.ID_ANY,
                               label="%s:" % _("Category"))
        try:
            newCat = max(self.cats[1]) + 1
        except:
            newCat = 1
        self.catNew = wx.TextCtrl(parent=self, id=wx.ID_ANY, size=(50, -1),
                                  value=str(newCat))
        btnAddCat = wx.Button(self, wx.ID_ADD)
        flexSizer.Add(item=layerNewTxt, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.layerNew, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=catNewTxt, proportion=0,
                      flag=wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_RIGHT | wx.LEFT,
                      border=10)
        flexSizer.Add(item=self.catNew, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=btnAddCat, proportion=0,
                      flag=wx.EXPAND | wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)
        addSizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)

        # buttons
        btnApply = wx.Button(self, wx.ID_APPLY)
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        #btnReload = wx.Button(self, wx.ID_UNDO, _("&Reload"))
        btnOk = wx.Button(self, wx.ID_OK)
        btnOk.SetDefault()

        # sizers
        btnSizer = wx.StdDialogButtonSizer()
        btnSizer.AddButton(btnCancel)
        #btnSizer.AddButton(btnReload)
        #btnSizer.SetNegativeButton(btnReload)
        btnSizer.AddButton(btnApply)
        btnSizer.AddButton(btnOk)
        btnSizer.Realize()
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(item=listSizer, proportion=1,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
        mainSizer.Add(item=addSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALIGN_CENTER |
                      wx.LEFT | wx.RIGHT | wx.BOTTOM, border=5)
        mainSizer.Add(item=btnSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)
        self.SetAutoLayout(True)

        # set min size for dialog
        self.SetMinSize(self.GetBestSize())

        # bindings
        # buttons
        #btnReload.Bind(wx.EVT_BUTTON, self.OnReload)
        btnApply.Bind(wx.EVT_BUTTON, self.OnApply)
        btnOk.Bind(wx.EVT_BUTTON, self.OnOK)
        btnAddCat.Bind(wx.EVT_BUTTON, self.OnAddCat)
        btnCancel.Bind(wx.EVT_BUTTON, self.OnCancel)

        # list
        # self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected, self.list)
        # self.list.Bind(wx.EVT_RIGHT_DOWN, self.OnRightDown)
        self.list.Bind(wx.EVT_COMMAND_RIGHT_CLICK, self.OnRightUp) #wxMSW
        self.list.Bind(wx.EVT_RIGHT_UP, self.OnRightUp) #wxGTK
        self.Bind(wx.EVT_LIST_BEGIN_LABEL_EDIT, self.OnBeginEdit, self.list)
        self.Bind(wx.EVT_LIST_END_LABEL_EDIT, self.OnEndEdit, self.list)
        self.Bind(wx.EVT_LIST_COL_CLICK, self.OnColClick, self.list)

    def GetListCtrl(self):
        """Used by ColumnSorterMixin"""
        return self.list

    def OnColClick(self, event):
        """Click on column header (order by)"""
        event.Skip()
        
    def OnBeginEdit(self, event):
        """Editing of item started"""
        event.Allow()

    def OnEndEdit(self, event):
        """Finish editing of item"""
        itemIndex = event.GetIndex()
        layerOld = int (self.list.GetItem(itemIndex, 0).GetText())
        catOld = int (self.list.GetItem(itemIndex, 1).GetText())

        if event.GetColumn() == 0:
            layerNew = int(event.GetLabel())
            catNew = catOld
        else:
            layerNew = layerOld
            catNew = int(event.GetLabel())

        try:
            if layerNew not in self.cats.keys():
                self.cats[layerNew] = []
            self.cats[layerNew].append(catNew)
            self.cats[layerOld].remove(catOld)
        except:
            event.Veto()
            self.list.SetStringItem(itemIndex, 0, str(layerNew))
            self.list.SetStringItem(itemIndex, 1, str(catNew))
            dlg = wx.MessageDialog(self, _("Unable to add new layer/category <%s/%s>.\n"
                                           "Layer and category number must be integer.\n"
                                           "Layer number must be greater then zero.") %
                                   (str(self.layerNew.GetValue()), str(self.catNew.GetValue())),
                                   _("Error"), wx.OK | wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            return False

    def OnRightDown(self, event):
        """Mouse right button down"""
        x = event.GetX()
        y = event.GetY()
        item, flags = self.list.HitTest((x, y))

        if item !=  wx.NOT_FOUND and \
                flags & wx.LIST_HITTEST_ONITEM:
            self.list.Select(item)

        event.Skip()

    def OnRightUp(self, event):
        """Mouse right button up"""
        if not hasattr(self, "popupID1"):
            self.popupID1 = wx.NewId()
            self.popupID2 = wx.NewId()
            self.popupID3 = wx.NewId()
            self.Bind(wx.EVT_MENU, self.OnItemDelete,    id=self.popupID1)
            self.Bind(wx.EVT_MENU, self.OnItemDeleteAll, id=self.popupID2)
            self.Bind(wx.EVT_MENU, self.OnReload, id=self.popupID3)

        # generate popup-menu
        menu = wx.Menu()
        menu.Append(self.popupID1, _("Delete selected"))
        if self.list.GetFirstSelected() == -1:
            menu.Enable(self.popupID1, False)

        menu.Append(self.popupID2, _("Delete all"))
        menu.AppendSeparator()
        menu.Append(self.popupID3, _("Reload"))

        self.PopupMenu(menu)
        menu.Destroy()

    def OnItemSelected(self, event):
        """Item selected"""
        event.Skip()

    def OnItemDelete(self, event):
        """Delete selected item(s) from the list (layer/category pair)"""
        item = self.list.GetFirstSelected()
        while item != -1:
            layer = int (self.list.GetItem(item, 0).GetText())
            cat = int (self.list.GetItem(item, 1).GetText())
            self.list.DeleteItem(item)
            self.cats[layer].remove(cat)

            item = self.list.GetFirstSelected()
            
        event.Skip()
        
    def OnItemDeleteAll(self, event):
        """Delete all items from the list"""
        self.list.DeleteAllItems()
        self.cats = {}

        event.Skip()

    def __GetCategories(self, coords, qdist):
        """Get layer/category pairs for all available
        layers

        Return True line found or False if not found"""
        
        cmdWhat = gcmd.Command(cmd=['v.what',
                                   '--q',
                                   'map=%s' % self.map,
                                   'east_north=%f,%f' % \
                                       (float(coords[0]), float(coords[1])),
                                   'distance=%f' % qdist])

        if cmdWhat.returncode != 0:
            return False

        for item in cmdWhat.ReadStdOutput():
            litem = item.lower()
            if "line:" in litem: # get line id
                self.line = int(item.split(':')[1].strip())
            elif "layer:" in litem: # add layer
                layer = int(item.split(':')[1].strip())
                if layer not in self.cats.keys():
                    self.cats[layer] = []
            elif "category:" in litem: # add category
                self.cats[layer].append(int(item.split(':')[1].strip()))

        return True

    def OnReload(self, event):
        """Reload button pressed"""
        # restore original list
        self.cats = copy.deepcopy(self.cats_orig)

        # polulate list
        self.itemDataMap = self.list.Populate(update=True)

        event.Skip()

    def OnCancel(self, event):
        """Cancel button pressed"""
        self.parent.parent.digittoolbar.categoryDialog = None
        self.parent.parent.digit.driver.SetSelected([])
        self.parent.UpdateMap(render=False)
        self.Close()

    def OnApply(self, event):
        """Apply button pressed"""

        # action : (catsFrom, catsTo)
        check = {'catadd': (self.cats,      self.cats_orig),
                 'catdel': (self.cats_orig, self.cats)}

        # add/delete new category
        for action, cats in check.iteritems():
            for layer in cats[0].keys():
                catList = []
                for cat in cats[0][layer]:
                    if layer not in cats[1].keys() or \
                            cat not in cats[1][layer]:
                        catList.append(cat)
                if catList != []:
                    if USEVEDIT is True:
                        vEditCmd = ['v.edit', '--q',
                                    'map=%s' % self.map,
                                    'layer=%d' % layer,
                                    'tool=%s' % action,
                                    'cats=%s' % ",".join(["%d" % v for v in catList]),
                                    'id=%d' % self.line]
            
                        gcmd.Command(vEditCmd)
                    else:
                        if action == 'catadd':
                            add = True
                        else:
                            add = False
                        self.line = self.parent.parent.digit.SetLineCats(-1, layer,
                                                                          catList, add)
                        if self.line < 0:
                            wx.MessageBox(parent=self, message=_("Unable to update vector map."),
                                          caption=_("Error"), style=wx.OK | wx.ICON_ERROR)
        if USEVEDIT is True:           
            # reload map (needed for v.edit)
            self.parent.parent.digit.driver.ReloadMap()

        self.cats_orig = copy.deepcopy(self.cats)

        event.Skip()

    def OnOK(self, event):
        """OK button pressed"""
        self.OnApply(event)
        self.OnCancel(event)

    def OnAddCat(self, event):
        """Button 'Add' new category pressed"""
        try:
            layer = int(self.layerNew.GetValue())
            cat   = int(self.catNew.GetValue())
            if layer <= 0:
                raise ValueError
        except ValueError:
            dlg = wx.MessageDialog(self, _("Unable to add new layer/category <%s/%s>.\n"
                                           "Layer and category number must be integer.\n"
                                           "Layer number must be greater then zero.") %
                                   (str(self.layerNew.GetValue()), str(self.catNew.GetValue())),
                                   _("Error"), wx.OK | wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            return False

        if layer not in self.cats.keys():
            self.cats[layer] = []

        self.cats[layer].append(cat)

        # reload list
        self.itemDataMap = self.list.Populate(update=True)

        # update category number for add
        self.catNew.SetValue(str(cat + 1))

        event.Skip()

        return True

    def GetLine(self):
        """Get id of selected line of 'None' if no line is selected"""
        return self.line

    def UpdateDialog(self, query=None, cats=None, line=None):
        """Update dialog
        
        @param query {coordinates, distance} - v.edit/v.what
        @param cats  directory layer/cats    - vdigit
        Return True if updated otherwise False
        """

        # line id (if not found remains 'None')
        self.line = None

        # {layer: [categories]}
        self.cats = {}

        # do not display dialog if no line is found (-> self.cats)
        if cats is None:
            ret = self.__GetCategories(query[0], query[1])
        else:
            # self.cats = dict(cats)
            for layer in cats.keys():
                self.cats[layer] = list(cats[layer]) # TODO: tuple to list
            self.line = line
            ret = 1
        if ret == 0 or not self.line:
            Debug.msg(3, "DigitCategoryDialog(): nothing found!")
            return False
        
        # make copy of cats (used for 'reload')
        self.cats_orig = copy.deepcopy(self.cats)

        # polulate list
        self.itemDataMap = self.list.Populate(update=True)

        try:
            newCat = max(self.cats[1]) + 1
        except:
            newCat = 1
        self.catNew.SetValue(str(newCat))

        return True

class CategoryListCtrl(wx.ListCtrl,
                       listmix.ListCtrlAutoWidthMixin,
                       listmix.TextEditMixin):
    """List of layers/categories"""

    def __init__(self, parent, id, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        
        self.parent = parent
        
        wx.ListCtrl.__init__(self, parent, id, pos, size, style)

        listmix.ListCtrlAutoWidthMixin.__init__(self)
        listmix.TextEditMixin.__init__(self)

    def Populate(self, update=False):
        """Populate the list"""

        itemData = {} # requested by sorter

        if not update:
            self.InsertColumn(0, _("Layer"))
            self.InsertColumn(1, _("Category"))
        else:
            self.DeleteAllItems()

        i = 1
        for layer in self.parent.cats.keys():
            catsList = self.parent.cats[layer]
            for cat in catsList:
                index = self.InsertStringItem(sys.maxint, str(catsList[0]))
                self.SetStringItem(index, 0, str(layer))
                self.SetStringItem(index, 1, str(cat))
                self.SetItemData(index, i)
                itemData[i] = (str(layer), str(cat))
                i = i + 1

        if not update:
            self.SetColumnWidth(0, 100)
            self.SetColumnWidth(1, wx.LIST_AUTOSIZE)

        self.currentItem = 0

        return itemData

class DigitZBulkDialog(wx.Dialog):
    """
    Dialog used for Z bulk-labeling tool
    """
    def __init__(self, parent, title, nselected, style=wx.DEFAULT_DIALOG_STYLE):
        wx.Dialog.__init__(self, parent=parent, id=wx.ID_ANY, title=title, style=style)

        self.parent = parent # mapdisplay.BufferedWindow class instance

        # panel  = wx.Panel(parent=self, id=wx.ID_ANY)

        border = wx.BoxSizer(wx.VERTICAL)
        
        txt = wx.StaticText(parent=self,
                            label=_("%d lines selected for z bulk-labeling") % nselected);
        border.Add(item=txt, proportion=0, flag=wx.ALL | wx.EXPAND, border=5)

        box   = wx.StaticBox (parent=self, id=wx.ID_ANY, label=" %s " % _("Set value"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        flexSizer = wx.FlexGridSizer (cols=2, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)

        # starting value
        txt = wx.StaticText(parent=self,
                            label=_("Starting value"));
        self.value = wx.SpinCtrl(parent=self, id=wx.ID_ANY, size=(150, -1),
                                 initial=0,
                                 min=-1e6, max=1e6)
        flexSizer.Add(txt, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.value, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)

        # step
        txt = wx.StaticText(parent=self,
                            label=_("Step"))
        self.step  = wx.SpinCtrl(parent=self, id=wx.ID_ANY, size=(150, -1),
                                 initial=0,
                                 min=0, max=1e6)
        flexSizer.Add(txt, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.step, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)

        # buttons
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        btnOk = wx.Button(self, wx.ID_OK)
        btnOk.SetDefault()

        # sizers
        btnSizer = wx.StdDialogButtonSizer()
        btnSizer.AddButton(btnCancel)
        btnSizer.AddButton(btnOk)
        btnSizer.Realize()
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(item=border, proportion=1, flag=wx.EXPAND | wx.ALL, border=5)
        mainSizer.Add(item=btnSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)
