"""
MODULE: digit

CLASSES:
 * VEdit
 * VDigit
 * DisplayDriver
 * SettingsDialog
 
PURPOSE: Digitization tool wxPython GUI prototype

         Note: Initial version under development

         Progress:
          (1) v.edit called on the background (class VEdit)
          (2) Reimplentation of v.digit (VDigit)

AUTHORS: The GRASS Development Team
         Martin Landa <landa.martin gmail.com>

COPYRIGHT: (C) 2007 by the GRASS Development Team
           This program is free software under the GNU General Public
           License (>=v2). Read the file COPYING that comes with GRASS
           for details.
"""

import sys
import string

import wx
import wx.lib.colourselect as csel

import cmd
import dbm
from debug import Debug as Debug

try:
    # replace with the path to the GRASS SWIG-Python interface
    g6libPath = "/hardmnt/schiele0/ssi/landa/src/grass6/swig/python"
    # g6libPath = "/usr/src/gis/grass6/swig/python"
    sys.path.append(g6libPath)
    import python_grass6 as g6lib
except:
    print >> sys.stderr, "For running digitization tool you need to enable GRASS SWIG-Python interface.\n" \
          "This only TEMPORARY solution (display driver based on SWIG-Python interface is EXTREMELY SLOW!\n" \
          "Will be replaced by C/C++ display driver."

class AbstractDigit:
    """
    Abstract digitization class
    """
    def __init__(self, settings=None):
        self.map    = None
        self.driver = None

        if not settings:
            self.settings = {}
            # symbology
            self.settings["symbolBackground"] = (None, "white") # enabled, color
            self.settings["symbolHighlight"] = (None, "yellow")
            self.settings["symbolPoint"] = (True, "black")
            self.settings["symbolLine"] = (True, "black")
            self.settings["symbolBoundaryNo"] = (True, "grey")
            self.settings["symbolBoundaryOne"] = (True, "orange")
            self.settings["symbolBoundaryTwo"] = (True, "green")
            self.settings["symbolCentroidIn"] = (True, "blue")
            self.settings["symbolCentroidOut"] = (True, "brown")
            self.settings["symbolCentroidDup"] = (True, "violet")
            self.settings["symbolNodeOne"] = (True, "red")
            self.settings["symbolNodeTwo"] = (True, "dark green")
            self.settings["symbolVertex"] = (True, "pink")
            
            # display
            self.settings["snapping"] = (10, "screen pixels") # value, unit
            self.settings["lineWidth"] = (2, "screen pixels")
            # digitize new record
            self.settings["addRecord"] = True
            self.settings["layer"] = 1
            self.settings["category"] = 1
            self.settings["categoryMode"] = "Next to use"
        else:
            self.settings = settings

    def SetCategoryNextToUse(self):
        """Find maximum category number in the map layer
        and update Digit.settings['category']

        Returns 'True' on success, 'False' on failure
        """

        if self.map:
            categoryCmd = cmd.Command(cmd=["v.category", "-g", "--q",
                                           "input=%s" % self.map, 
                                           "option=report",
                                           "layer=%d" % self.settings["layer"]])

            if categoryCmd.returncode != 0:
                return
        
            for line in categoryCmd.ReadStdOutput():
                if "all" in line:
                    try:
                        maxCat = int(line.split(' ')[-1]) + 1
                        self.settings['category'] = maxCat
                    except:
                        return False
                    return True
        else:
            self.settings["category"] = 1

    def SetCategory(self):
        """Return category number to use (according Settings)"""
        if self.settings["categoryMode"] == "No category":
            self.settings["category"] = "None"
        elif self.settings["categoryMode"] == "Next to use":
            self.SetCategoryNextToUse()
        else:
            if self.settings["category"] == "None":
                self.SetCategoryNextToUse()

        return self.settings["category"]

    def ReInitialize(self, map=None, mapwindow=None):
        """Re-initialize settings according selected map layer"""
        self.map = map
        
        if self.map:
            Debug.msg (3, "AbstractDigit.ReInitialize(): map=%s" % \
                       map)

            self.SetCategory()
            self.driver = DisplayDriver(map, mapwindow)
        else:
            del self.driver
            self.driver = None

class VEdit(AbstractDigit):
    """
    Prototype of digitization class based on v.edit command

    Note: This should be replaced by VDigit class.
    """
    def __init__(self, settings=None):
        AbstractDigit.__init__(self, settings)

    def AddPoint (self, map, type, x, y, z=None):
        """
        Add point/centroid to the vector map layer
        """
        if type == "centroid":
            key = "C"
        else:
            key = "P"

        layer = self.settings["layer"]
        cat   = self.SetCategory()
        
        addstring="""%s 1 1
                    %f %f\n%d %d""" % (key, x, y, layer, cat)

        Debug.msg (3, "VEdit.AddPoint(): map=%s, type=%s, layer=%d, cat=%d, x=%f, y=%f" % \
                   (map, type, layer, cat, x, y))
        Debug.msg (4, "Vline.AddPoint(): input=%s" % addstring)
                
        self._AddFeature (map=map, input=addstring, flags=['-s'])

    def AddLine (self, map, type, coords):
        """
        Add line/boundary to the vector map layer
        """
        if len(coords) < 2:
            return

        layer = self.settings["layer"]
        cat   = self.SetCategory()
        
        if type == "boundary":
            key = "B"
            flags = ['-c', '-s'] # close boundaries
        else:
            key = "L"
            flags = ['-s']
            
        addstring="""%s %d 1\n""" % (key, len(coords))
        for point in coords:
            addstring += """%f %f\n""" % \
                (float(point[0]), float(point [1]))

        addstring += "%d %d" % (layer, cat)

        Debug.msg (3, "Vline.AddLine(): type=%s, layer=%d, cat=%d coords=%s" % \
                   (key, layer, cat, coords))
        Debug.msg (4, "Vline.AddLine(): input=%s" % addstring)

        self._AddFeature (map=map, input=addstring, flags=flags)

    def _AddFeature (self, map, input, flags):
        """
        General method which adds feature to the vector map
        """
        threshold = Digit.settings["snapping"][0]
        if Digit.settings["snapping"][1] == "screen pixels":
            # pixel -> cell
            print "#", threshold
            threshold = self.driver.mapwindow.Distance(beginpt=(0,0),
                                                       endpt=(threshold,0))
                                                       

        print "#", threshold

        command = ["v.edit", "-n", "--q", 
                   "map=%s" % map,
                   "tool=add",
                   "thresh=%f" % threshold]
        # additional flags
        for flag in flags:
            command.append(flag)

        # run the command
        vedit = cmd.Command(cmd=command, stdin=input)

        # redraw map
        self.driver.ReDrawMap(map)
        
    def DeleteSelectedLines(self):
        """Delete vector feature from map"""

        if not self.driver.highlighted:
            Debug.msg(4, "Digit.DeleteSelectedLines(): ids=%s" % \
                      self.driver.highligted)
            return False
        
        ids = ",".join(["%d" % v for v in self.driver.highlighted])
        command = ["v.edit", "-n", "--q",
                   "map=%s" % self.map,
                   "tool=delete",
                   "ids=%s" % ids]

        # run the command
        vedit = cmd.Command(cmd=command)

        # redraw map
        self.driver.ReDrawMap(self.map)

        return True
    
class VDigit(AbstractDigit):
    """
    Prototype of digitization class based on v.digit reimplementation

    Under development (wxWidgets C/C++ background)
    """
    pass

class DisplayDriver:
    """
    Experimental display driver implemented in Python using
    GRASS-Python SWIG interface

    Note: This will be in the future rewritten in C/C++
    """
    def __init__(self, map, mapwindow):
        self.mapwindow   = mapwindow
        self.ids         = {}   # dict[g6id] = [DCid]
        self.highlighted = [] # list of highlighted objects (g6id)

        g6lib.G_gisinit('')

        name, mapset = map.split('@')

        Debug.msg(4, "DisplayDriver.__init__(): name=%s, mapset=%s" % \
                      (name, mapset))

        # define map structure
        self.mapInfo = g6lib.Map_info()
        # define open level (level 2: topology)
        g6lib.Vect_set_open_level(2)

        # open existing map
        # python 2.5 (unicode) -> str()
        g6lib.Vect_open_old(self.mapInfo, str(name), str(mapset))

        # auxilary structures
        self.points = g6lib.Vect_new_line_struct();
        self.cats = g6lib.Vect_new_cats_struct();

    def __del__(self):
        if self.mapInfo:
            g6lib.Vect_close(self.mapInfo)
        if self.points:
            g6lib.Vect_destroy_line_struct(self.points)
        if self.cats:
            g6lib.Vect_destroy_cats_struct(self.cats)

    def __SetPen(self, line, symbol):
        # see include/vect/dig_defines.h
        # define GV_POINT      0x01
        # define GV_LINE       0x02
        # define GV_BOUNDARY   0x04
        # define GV_CENTROID   0x08
        # define GV_FACE       0x10
        # define GV_KERNEL     0x20
        # define GV_AREA       0x40
        # define GV_VOLUME     0x80

        if line in self.highlighted:
            symbol = "symbolHighlight"
        
        width = Digit.settings["lineWidth"][0]
        
        if Digit.settings[symbol][0] in [True, None]:
            self.mapwindow.pen = self.mapwindow.polypen = wx.Pen(colour=Digit.settings[symbol][1],
                                                                 width=width, style=wx.SOLID)
        else:
            self.mapwindow.pen = self.mapwindow.polypen = None

    def SetHighlighted(self, ids):
        """Set highlighted objects in PseudoDC

        For disable highlighting set ids=[]
        """
        # reset
        self.highlighted = []
        
        for line in ids:
            self.highlighted.append(line)

        Debug.msg(4, "DisplayDriver.SetHightlighted(): dcIds=%s, lineIds=%s" % \
                      (ids, self.highlighted))

    def SelectLinesByBox(self, rect):
        """Alternative method for selecting vector feature
        in the map by given bounding box.

        rect = ((x1, y1), (x2, y2))
        This method should be in the future reimplemented using
        PseudoDC.FindObjectInsideBBox()"""

        type = 255 # all types (see include/vect/dig_defines.h)
        list = g6lib.Vect_new_list()
        bbox = g6lib.Vect_new_line_struct()

        x1, y1 = rect[0]
        x2, y2 = rect[1]
        dx = abs(x2 - x1)
        dy = abs(y2 - y1)
        
        g6lib.Vect_append_point(bbox, x1, y1, 0)
        g6lib.Vect_append_point(bbox, x2, y1, 0)
        g6lib.Vect_append_point(bbox, x2, y2, 0)
        g6lib.Vect_append_point(bbox, x1, y2, 0)
        g6lib.Vect_append_point(bbox, x1, y1, 0)
        
        g6lib.Vect_select_lines_by_polygon(self.mapInfo, bbox,
                                           0, None,
                                           type, list)

        self.highligted = []
        for idx in range(list.n_values):
            line = g6lib.intArray_getitem(list.value, idx)
            if line not in self.highlighted:
                self.highlighted.append(line)

        Debug.msg(4, "DisplayDriver.SelectLinesByBox(%s): n=%d, ids=%s" % \
                  (rect, list.n_values, self.highlighted))
        
        g6lib.Vect_destroy_line_struct(bbox)
        g6lib.Vect_destroy_list(list)

        return self.highlighted
        
    def ReDrawMap(self, map):
        """Reopen map and draw its content in PseudoDC"""
        # close map
        if self.mapInfo:
            g6lib.Vect_close(self.mapInfo)
        
        # re-open map
        name, mapset = map.split('@')
        g6lib.Vect_open_old(self.mapInfo, str(name), str(mapset))

        self.DrawMap()
        
    def DrawMap(self):
        """Draw map content in PseudoDC"""
        Debug.msg(4, "DisplayDriver.DrawMap()")
        self.DisplayLines()

    def DisplayLines(self):
        """Display all lines in PseudoDC"""
        nlines = g6lib.Vect_get_num_lines(self.mapInfo)
        Debug.msg(4, "DisplayDriver.DisplayLines(): nlines=%d" % nlines)
        #symb_set_driver_color ( SYMB_HIGHLIGHT );
        for line in range(1, nlines + 1):
            #symb = LineSymb[i];
            #if ( !Symb[symb].on ) continue;
            self.DisplayLine(line)

        return nlines
    
    def DisplayLine(self, line):
        """Display line (defined by id) in PseudoDC"""
        Debug.msg(4, "DisplayDriver.DisplayLine(): line=%d" % line)
        
        if not g6lib.Vect_line_alive (self.mapInfo, line):
            return
        
        type = g6lib.Vect_read_line (self.mapInfo, self.points, self.cats, line)

        Debug.msg (4, "DisplayDriver.DisplayLine(): line=%d type=%d" % \
                   (line, type))

        # add id
        self.ids[line] = []
        self.DisplayPoints(line, type, self.points)

    def DisplayNodes(self, line):
        """Display nodes of the given line"""
        Debug.msg(4, "DisplayDriver.DisplayNodes(): line=%d" % line)

        n1 = g6lib.new_intp()
        n2 = g6lib.new_intp()
        x = g6lib.new_doublep()
        y = g6lib.new_doublep()
        z = g6lib.new_doublep()

        nodes = [n1, n2]
        g6lib.Vect_get_line_nodes(self.mapInfo, line, n1, n2)
        
        for nodep in nodes:
            node = g6lib.intp_value(nodep)
            g6lib.Vect_get_node_coor(self.mapInfo, node,
                                     x, y, z)
            coords = (self.mapwindow.Cell2Pixel(g6lib.doublep_value(x),
                                                g6lib.doublep_value(y)))

            if g6lib.Vect_get_node_n_lines(self.mapInfo, node) == 1:
                self.__SetPen(line, "symbolNodeOne") # one line
            else:
                self.__SetPen(line, "symbolNodeTwo") # two lines

            self.ids[line].append(self.mapwindow.DrawCross(coords, size=5))

        g6lib.delete_doublep(x)
        g6lib.delete_doublep(y)
        g6lib.delete_doublep(z)
        g6lib.delete_intp(n1)
        g6lib.delete_intp(n2)

    def DisplayPoints(self, line, type, points):
        """Draw points in PseudoDC"""

        coords = []
        npoints = points.n_points
        Debug.msg(4, "DisplayDriver.DisplayPoints() npoints=%d" % npoints);

        for idx in range(npoints):
            x = g6lib.doubleArray_getitem(self.points.x, idx)
            y = g6lib.doubleArray_getitem(self.points.y, idx)
            z = g6lib.doubleArray_getitem(self.points.z, idx)
            coords.append(self.mapwindow.Cell2Pixel(x, y))

        if len(coords) > 1: # -> line
            # draw line
            if type == g6lib.GV_BOUNDARY:
                leftp = g6lib.new_intp()
                rightp = g6lib.new_intp()
                g6lib.Vect_get_line_areas(self.mapInfo, line,
                                          leftp, rightp)
                left, right = g6lib.intp_value(leftp), g6lib.intp_value(rightp)
                if left == 0 and right == 0:
                    self.__SetPen(line, "symbolBoundaryNo")
                elif left > 0 and right > 0:
                    self.__SetPen(line, "symbolBoundaryTwo")
                else:
                    self.__SetPen(line, "symbolBoundaryOne")
                g6lib.delete_intp(leftp)
                g6lib.delete_intp(rightp)
            else: # GV_LINE
                self.__SetPen(line, "symbolLine")
            self.ids[line].append(self.mapwindow.DrawLines(coords))
            # draw verteces
            self.__SetPen(line, "symbolVertex")
            for idx in range(1, npoints-1):
                self.ids[line].append(self.mapwindow.DrawCross(coords[idx], size=4))
            # draw nodes
            self.DisplayNodes(line)
        else:
            if type == g6lib.GV_CENTROID:
                cret = g6lib.Vect_get_centroid_area(self.mapInfo, line)
                if cret > 0: # -> area
                    self.__SetPen(line, "symbolCentroidIn")
                elif cret == 0:
                    self.__SetPen(line, "symbolCentroidOut")
                else:
                    self.__SetPen(line, "symbolCentroidDup")
            else:
                self.__SetPen(line, "symbolPoint")
            self.ids[line].append(self.mapwindow.DrawCross(coords[0], size=5))
        
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
        Digit.SetCategory() # update category number (next to use)
        self.__CreateSettingsPage(notebook)
        
        # buttons
        btnApply = wx.Button(self, wx.ID_APPLY, _("Apply") )
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        btnOk = wx.Button(self, wx.ID_OK, _("OK") )
        btnOk.SetDefault()

        # bindigs
        btnApply.Bind(wx.EVT_BUTTON, self.OnApply)
        btnOk.Bind(wx.EVT_BUTTON, self.OnOK)

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
                                      colour=Digit.settings[key][1], size=(25, 25))
            isEnabled = Digit.settings[key][0]
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
        # snapping
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Snapping threshold"))
        self.snappingValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(50, -1),
                                         value=str(Digit.settings["snapping"][0]), min=1, max=1e6)
        self.snappingUnit = wx.ComboBox(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                         choices=["screen pixels", "map units"])
        self.snappingUnit.SetValue(Digit.settings["snapping"][1])
        flexSizer.Add(text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.snappingValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(self.snappingUnit, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)
        # line width
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Line width"))
        self.lineWidthValue = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(50, -1),
                                          value=str(Digit.settings["lineWidth"][0]),
                                          min=1, max=1e6)
        self.lineWidthUnit = wx.ComboBox(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                         choices=["screen pixels", "map units"])
        self.lineWidthUnit.SetValue(Digit.settings["lineWidth"][1])
        flexSizer.Add(text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(self.lineWidthValue, proportion=0, flag=wx.ALIGN_CENTER | wx.FIXED_MINSIZE)
        flexSizer.Add(self.lineWidthUnit, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=5)

        #
        # attributes
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Digitize new feature"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        # checkbox
        self.addRecord = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                     label=_("Add new record into table"))
        self.addRecord.SetValue(Digit.settings["addRecord"])
        sizer.Add(item=self.addRecord, proportion=0, flag=wx.ALL | wx.EXPAND, border=1)
        # settings
        flexSizer = wx.FlexGridSizer(cols=2, hgap=3, vgap=3)
        flexSizer.AddGrowableCol(0)
        settings = ((_("Layer"), 1), (_("Category"), 1), (_("Mode"), _("Next to use")))
        # layer
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Layer"))
        self.layer = wx.TextCtrl(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                 value=str(Digit.settings["layer"])) # TODO: validator
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.layer, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        # category number
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Category number"))
        self.category = wx.TextCtrl(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                    value=str(Digit.settings["category"])) # TODO: validator
        if Digit.settings["categoryMode"] != "Manual entry":
            self.category.SetEditable(False)
            self.category.Enable(False)
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.category, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)
        # category mode
        text = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Category mode"))
        self.categoryMode = wx.ComboBox(parent=panel, id=wx.ID_ANY,
                                        style=wx.CB_SIMPLE | wx.CB_READONLY, size=(125, -1),
                                        choices=[_("Next to use"), _("Manual entry"), _("No category")])
        self.categoryMode.SetValue(Digit.settings["categoryMode"])
        flexSizer.Add(item=text, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(item=self.categoryMode, proportion=0,
                      flag=wx.FIXED_MINSIZE | wx.ALIGN_CENTER_VERTICAL)

        sizer.Add(item=flexSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=1)
        border.Add(item=sizer, proportion=0,
                   flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        # bindings
        self.Bind(wx.EVT_CHECKBOX, self.OnChangeAddRecord, self.addRecord)
        self.Bind(wx.EVT_COMBOBOX, self.OnChangeCategoryMode, self.categoryMode)
        
        panel.SetSizer(border)
        
        return panel

    def __SymbologyData(self):
        """
        Data for __CreateSymbologyPage()

        label | checkbox | color
        """

        return (
            ("Background", "symbolBackground"),
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
        Digit.settings["categoryMode"] = mode
        if mode == "Manual entry": # enable
            self.category.Enable(True)
            self.category.SetEditable(True)
        elif self.category.IsEnabled(): # disable
            self.category.SetEditable(False)
            self.category.Enable(False)

        if mode == "No category" and self.addRecord.IsChecked():
            self.addRecord.SetValue(False)
        Digit.SetCategory()
        self.category.SetValue(str(Digit.settings['category']))

    def OnChangeAddRecord(self, event):
        """Checkbox 'Add new record' status changed"""
        self.category.SetValue(str(Digit.SetCategory()))
            
    def OnOK(self, event):
        """Button 'OK' clicked"""
        self.UpdateSettings()
        self.Close()

    def OnApply(self, event):
        """Button 'Apply' clicked"""
        self.UpdateSettings()

    def UpdateSettings(self):
        """Update Digit.settings"""
        try:
            # symbology
            for key, (enabled, color) in self.symbology.iteritems():
                if enabled:
                    Digit.settings[key] = (enabled.IsChecked(), color.GetColour())
                else:
                    Digit.settings[key] = (None, color.GetColour())
            # display
            Digit.settings["snapping"] = (int(self.snappingValue.GetValue()), # value
                                          self.snappingUnit.GetValue()) # unit
            Digit.settings["lineWidth"] = (int(self.lineWidthValue.GetValue()),
                                           self.lineWidthUnit.GetValue())
            # digitize new feature
            Digit.settings["addRecord"] = self.addRecord.IsChecked()
            Digit.settings["layer"] = int(self.layer.GetValue())
            if Digit.settings["categoryMode"] == "No category":
                Digit.settings["category"] = None
            else:
                Digit.settings["category"] = int(self.category.GetValue())
            Digit.settings["categoryMode"] = self.categoryMode.GetValue()
        except:
            pass
    
##############################
# digitization class instance
##############################

Digit = VEdit()
