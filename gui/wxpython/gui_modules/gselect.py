"""
@package gselect

@brief Custom control that selects elements

Classes:
 - Select
 - TreeCrtlComboPopup
 - VectorDBInfo
 - LayerSelect
 - DriverSelect
 - DatabaseSelect
 - ColumnSelect

(C) 2007-2008 by the GRASS Development Team This program is free
software under the GNU General Public License (>=v2). Read the file
COPYING that comes with GRASS for details.

@author Michael Barton
@author Martin Landa <landa.martin gmail.com>
"""

import os
import sys

import wx
import wx.combo

import grass.script as grass

import globalvar
import gcmd
import utils
from preferences import globalSettings as UserSettings

class Select(wx.combo.ComboCtrl):
    def __init__(self, parent, id, size,
                 type=None, multiple=False, mapsets=None, exceptOf=[]):
        """
        Custom control to create a ComboBox with a tree control
        to display and select GIS elements within acessible mapsets.
        Elements can be selected with mouse. Can allow multiple selections, when
        argument multiple=True. Multiple selections are separated by commas.
        """
        wx.combo.ComboCtrl.__init__(self, parent=parent, id=id, size=size)
        self.GetChildren()[0].SetName("Select")
        self.GetChildren()[0].type = type
        
        self.tcp = TreeCtrlComboPopup()

        self.SetPopupControl(self.tcp)
        self.SetPopupExtents(0,100)
        if type:
            self.tcp.GetElementList(type, mapsets, exceptOf)
            self.tcp.SetData(type = type, mapsets = mapsets,
                             exceptOf = exceptOf, multiple = multiple)

    def SetElementList(self, type, mapsets = None, exceptOf = []):
        self.tcp.seltree.DeleteAllItems()
        self.tcp.GetElementList(type)
        self.tcp.SetData(type = type, mapsets = mapsets,
                         exceptOf = exceptOf)
        
class TreeCtrlComboPopup(wx.combo.ComboPopup):
    """
    Create a tree ComboBox for selecting maps and other GIS elements
    in accessible mapsets within the current location
    """

    # overridden ComboPopup methods
    def Init(self):
        self.value = [] # for multiple is False -> len(self.value) in [0,1]
        self.curitem = None
        self.multiple = False
        self.type = None
        self.mapsets = []
        self.exceptOf = []

    def Create(self, parent):
        self.seltree = wx.TreeCtrl(parent, style=wx.TR_HIDE_ROOT
                                   |wx.TR_HAS_BUTTONS
                                   |wx.TR_SINGLE
                                   |wx.TR_LINES_AT_ROOT
                                   |wx.SIMPLE_BORDER
                                   |wx.TR_FULL_ROW_HIGHLIGHT)
        self.seltree.Bind(wx.EVT_MOTION, self.OnMotion)
        self.seltree.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.seltree.Bind(wx.EVT_TREE_ITEM_EXPANDING, self.mapsetExpanded)
        self.seltree.Bind(wx.EVT_TREE_ITEM_COLLAPSED, self.mapsetCollapsed)
        self.seltree.Bind(wx.EVT_TREE_ITEM_ACTIVATED, self.mapsetActivated)
        self.seltree.Bind(wx.EVT_TREE_SEL_CHANGED, self.mapsetSelected)
        self.seltree.Bind(wx.EVT_TREE_DELETE_ITEM, lambda x: None)

    # the following dummy handler are needed to keep tree events from propagating up to
    # the parent GIS Manager layer tree
    def mapsetExpanded(self, event):
        pass

    def mapsetCollapsed(self, event):
        pass

    def mapsetActivated(self, event):
        pass

    def mapsetSelected(self, event):
        pass
    # end of dummy events

    def GetControl(self):
        return self.seltree

    def GetStringValue(self):
        str = ""
        for value in self.value:
            str += value + ","
        str = str.rstrip(',')
        
        return str

    def OnPopup(self):
        """Limited only for first selected"""
        # update list
        self.seltree.DeleteAllItems()
        self.GetElementList(self.type, self.mapsets, self.exceptOf)

        if len(self.value) > 0:
            root = self.seltree.GetRootItem()
            if not root:
                return
            item = self.FindItem(root, self.value[0])
            try:
                self.seltree.EnsureVisible(item)
                self.seltree.SelectItem(item)
            except:
                pass
            
    def SetStringValue(self, value):
        # this assumes that item strings are unique...
        root = self.seltree.GetRootItem()
        if not root:
            return
        found = self.FindItem(root, value)
        if found:
            self.value.append(found)
            self.seltree.SelectItem(found)

    def GetAdjustedSize(self, minWidth, prefHeight, maxHeight):
        return wx.Size(minWidth, min(200, maxHeight))

    def GetElementList(self, element, mapsets=None, exceptOf=[]):
        """
        Get list of GIS elements in accessible mapsets and display as tree
        with all relevant elements displayed beneath each mapset branch
        """
        # get current mapset
        curr_mapset = grass.gisenv()['MAPSET']
        
        # list of mapsets in current location
        if mapsets is None:
            mapsets = utils.ListOfMapsets()

        # map element types to g.mlist types
        elementdict = {'cell':'rast',
                       'raster':'rast',
                       'rast':'rast',
                       'raster files':'rast',
                       'grid3':'rast3d',
                       'rast3d':'rast3d',
                       'raster3D':'rast3d',
                       'raster3D files':'rast3d',
                       'vector':'vect',
                       'vect':'vect',
                       'binary vector files':'vect',
                       'dig':'oldvect',
                       'oldvect':'oldvect',
                       'old vector':'oldvect',
                       'dig_ascii':'asciivect',
                       'asciivect':'asciivect',
                       'asciivector':'asciivect',
                       'ascii vector files':'asciivect',
                       'icons':'icon',
                       'icon':'icon',
                       'paint icon files':'icon',
                       'paint/labels':'labels',
                       'labels':'labels',
                       'label':'labels',
                       'paint label files':'labels',
                       'site_lists':'sites',
                       'sites':'sites',
                       'site list':'sites',
                       'site list files':'sites',
                       'windows':'region',
                       'region':'region',
                       'region definition':'region',
                       'region definition files':'region',
                       'windows3d':'region3d',
                       'region3d':'region3d',
                       'region3D definition':'region3d',
                       'region3D definition files':'region3d',
                       'group':'group',
                       'imagery group':'group',
                       'imagery group files':'group',
                       '3d.view':'3dview',
                       '3dview':'3dview',
                       '3D viewing parameters':'3dview',
                       '3D view parameters':'3dview'}

        if element not in elementdict:
            self.AddItem(_('Not selectable element'))
            return

        # get directory tree nodes
        # reorder mapsets based on search path (TODO)
        for i in range(len(mapsets)):
            if i > 0 and mapsets[i] == curr_mapset:
                mapsets[i] = mapsets[0]
                mapsets[0] = curr_mapset
        
        if globalvar.have_mlist:
            filesdict = grass.mlist_grouped(elementdict[element])
        else:
            filesdict = grass.list_grouped(elementdict[element])
        
        first_dir = None
        for dir in mapsets:
            dir_node = self.AddItem('Mapset: ' + dir)
            if not first_dir:
                first_dir = dir_node
            
            self.seltree.SetItemTextColour(dir_node, wx.Colour(50, 50, 200))
            try:
                elem_list = filesdict[dir]
                elem_list.sort(key=str.lower)
                for elem in elem_list:
                    if elem != '':
                        fullqElem = elem + '@' + dir
                        if len(exceptOf) > 0 and fullqElem in exceptOf:
                            continue
                        self.AddItem(fullqElem, parent=dir_node)
            except:
                continue

            if self.seltree.ItemHasChildren(dir_node):
                sel = UserSettings.Get(group='general', key='elementListExpand',
                                       subkey='selection')
                collapse = True

                if sel == 0: # collapse all except PERMANENT and current
                    if dir in ('PERMANENT', curr_mapset):
                        collapse = False
                elif sel == 1: # collapse all except PERMANENT
                    if dir == 'PERMANENT':
                        collapse = False
                elif sel == 2: # collapse all except current
                    if dir == curr_mapset:
                        collapse = False
                elif sel == 3: # collapse all
                    pass
                elif sel == 4: # expand all
                    collapse = False
                
                if collapse:
                    self.seltree.Collapse(dir_node)
                else:
                    self.seltree.Expand(dir_node)
        
        if first_dir:
            # select first mapset (MSW hack)
            self.seltree.SelectItem(first_dir)
    
    # helpers
    def FindItem(self, parentItem, text):
        item, cookie = self.seltree.GetFirstChild(parentItem)
        while item:
            if self.seltree.GetItemText(item) == text:
                return item
            if self.seltree.ItemHasChildren(item):
                item = self.FindItem(item, text)
            item, cookie = self.seltree.GetNextChild(parentItem, cookie)
        return wx.TreeItemId()


    def AddItem(self, value, parent=None):
        if not parent:
            root = self.seltree.GetRootItem()
            if not root:
                root = self.seltree.AddRoot("<hidden root>")
            parent = root

        item = self.seltree.AppendItem(parent, text=value)
        return item

    def OnMotion(self, evt):
        # have the selection follow the mouse, like in a real combobox
        item, flags = self.seltree.HitTest(evt.GetPosition())
        if item and flags & wx.TREE_HITTEST_ONITEMLABEL:
            self.seltree.SelectItem(item)
            self.curitem = item
        evt.Skip()

    def OnLeftDown(self, evt):
        # do the combobox selection
        item, flags = self.seltree.HitTest(evt.GetPosition())
        if item and flags & wx.TREE_HITTEST_ONITEMLABEL:
            self.curitem = item

            if self.seltree.GetRootItem() == self.seltree.GetItemParent(item):
                self.value = [] # cannot select mapset item
            else:
                if self.multiple is True:
                    # text item should be unique
                    self.value.append(self.seltree.GetItemText(item))
                else:
                    self.value = [self.seltree.GetItemText(item), ]

            self.Dismiss()

        evt.Skip()

    def SetData(self, **kargs):
        """Set object properties"""
        if kargs.has_key('type'):
            self.type = kargs['type']
        if kargs.has_key('mapsets'):
            self.mapsets = kargs['mapsets']
        if kargs.has_key('exceptOf'):
            self.exceptOf = kargs['exceptOf']
        if kargs.has_key('multiple'):
            self.multiple = kargs['multiple']
        
class VectorDBInfo:
    """Class providing information about attribute tables
    linked to a vector map"""
    def __init__(self, map):
        self.map = map

        # dictionary of layer number and associated (driver, database, table)
        self.layers = {}
         # dictionary of table and associated columns (type, length, values, ids)
        self.tables = {}
        
        if not self.__CheckDBConnection(): # -> self.layers
            return

        self.__DescribeTables() # -> self.tables

    def __CheckDBConnection(self):
        """Check DB connection"""
        nuldev = file(os.devnull, 'w+')
        self.layers = grass.vector_db(map=self.map, stderr=nuldev)
        nuldev.close()
        
        if (len(self.layers.keys()) == 0):
            return False

        return True
    
    def __DescribeTables(self):
        """Describe linked tables"""
        for layer in self.layers.keys():
            # determine column names and types
            table = self.layers[layer]["table"]
            columnsCommand = gcmd.Command (cmd=["db.describe",
                                                "-c", "--q",
                                                "table=%s" % self.layers[layer]["table"],
                                                "driver=%s" % self.layers[layer]["driver"],
                                                "database=%s" % self.layers[layer]["database"]],
                                           rerr = None)


            columns = {} # {name: {type, length, [values], [ids]}}

            if columnsCommand.returncode == 0:
                # skip nrows and ncols
                i = 0
                for line in columnsCommand.ReadStdOutput()[2:]:
                    num, name, type, length = line.strip().split(':')
                    # FIXME: support more datatypes
                    if type.lower() == "integer":
                        ctype = int
                    elif type.lower() == "double precision":
                        ctype = float
                    else:
                        ctype = str

                    columns[name.strip()] = { 'index'  : i,
                                              'type'   : type.lower(),
                                              'ctype'  : ctype,
                                              'length' : int(length),
                                              'values' : [],
                                              'ids'    : []}
                    i += 1
            else:
                return False

            # check for key column
            # v.db.connect -g/p returns always key column name lowercase
            if self.layers[layer]["key"] not in columns.keys():
                for col in columns.keys():
                    if col.lower() == self.layers[layer]["key"]:
                        self.layers[layer]["key"] = col.upper()
                        break
            
            self.tables[table] = columns
            
        return True
    
    def Reset(self):
        """Reset"""
        for layer in self.layers:
            table = self.layers[layer]["table"] # get table desc
            columns = self.tables[table]
            for name in self.tables[table].keys():
                self.tables[table][name]['values'] = []
                self.tables[table][name]['ids']    = []
        
    def GetName(self):
        """!Get vector name"""
        return self.map
    
    def GetKeyColumn(self, layer):
        """!Get key column of given layer
        
        @param layer vector layer number
        """
        return self.layers[layer]['key']
    
    def GetTable(self, layer):
        """!Get table name of given layer
        
        @param layer vector layer number
        """
        return self.layers[layer]['table']
    
    def GetDbSettings(self, layer):
        """!Get database settins

        @param layer layer number
        
        @return (driver, database)
        """
        return self.layers[layer]['driver'], self.layers[layer]['database']
    
    def GetTableDesc(self, table):
        """!Get table columns

        @param table table name
        """
        return self.tables[table]

class LayerSelect(wx.Choice):
    """
    Creates combo box for selecting data layers defined for vector.
    The 'layer' terminology is likely to change for GRASS 7
    """
    def __init__(self, parent,
                 id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=globalvar.DIALOG_LAYER_SIZE,
                 vector=None, choices=[], initial=['1'], default=None):

        super(LayerSelect, self).__init__(parent, id, pos=pos, size=size,
                                          choices=choices)

        # initial choices (force '1' to be included)
        self.initial = initial
        if '1' not in self.initial:
            self.initial.append('1')
        
        # default value
        self.default = default
        
        self.SetName("LayerSelect")
        
        if len(choices) > 1:
            return
        
        if vector:
            self.InsertLayers(vector)
        else:
            self.SetItems(self.initial)
            self.SetStringSelection('1')
        
    def InsertLayers(self, vector):
        """Insert layers for a vector into the layer combobox"""
        layerchoices = utils.GetVectorNumberOfLayers(vector)
        for layer in self.initial:
            if layer in layerchoices:
                continue
            layerchoices.append(layer)

        # sort list of available layers
        utils.ListSortLower(layerchoices)
        
        if len(layerchoices) > 1:
            self.SetItems(layerchoices)
            self.SetStringSelection('1')
        else:
            self.SetItems(['1'])
            self.SetStringSelection('1')
        
        if self.default:
            self.SetStringSelection(str(self.default))
        
class DriverSelect(wx.ComboBox):
    """
    Creates combo box for selecting database driver.
    """
    def __init__(self, parent, choices, value,
                 id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=globalvar.DIALOG_LAYER_SIZE, **kargs):

        super(DriverSelect, self).__init__(parent, id, value, pos, size,
                                           choices, style=wx.CB_READONLY)
        
        self.SetName("DriverSelect")
        
        self.SetStringSelection(value)

class DatabaseSelect(wx.TextCtrl):
    """
    Creates combo box for selecting database driver.
    """
    def __init__(self, parent, value='',
                 id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=globalvar.DIALOG_TEXTCTRL_SIZE, **kargs):
        
        super(DatabaseSelect, self).__init__(parent, id, value, pos, size)
                               
        self.SetName("DatabaseSelect")

class TableSelect(wx.ComboBox):
    """
    Creates combo box for selecting attribute tables from the database
    """
    def __init__(self, parent,
                 id=wx.ID_ANY, value='', pos=wx.DefaultPosition,
                 size=globalvar.DIALOG_COMBOBOX_SIZE,
                 choices=[]):

        super(TableSelect, self).__init__(parent, id, value, pos, size, choices,
                                          style=wx.CB_READONLY)

        self.SetName("TableSelect")

        if not choices:
            self.InsertTables()
                
    def InsertTables(self, driver=None, database=None):
        """Insert attribute tables into combobox"""
        items = []
        cmd = ['db.tables',
               '-p']
        if driver:
            cmd.append('driver=%s' % driver)
        if database:
            cmd.append('database=%s' % database)
        
        try:
            tableCmd = gcmd.Command(cmd)
        except gcmd.CmdError:
            tableCmd = None

        
        if tableCmd and \
                tableCmd.returncode == 0:
            for table in tableCmd.ReadStdOutput():
                items.append(table)
            
        self.SetItems(items)
        self.SetValue('')
        
class ColumnSelect(wx.ComboBox):
    """
    Creates combo box for selecting columns in the attribute table for a vector.
    The 'layer' terminology is likely to change for GRASS 7
    """
    def __init__(self, parent,
                 id = wx.ID_ANY, value = '',
                 size = globalvar.DIALOG_COMBOBOX_SIZE,
                 vector = None, layer = 1, choices = [], readonly = False,
                 **kwargs):
        if readonly:
            if kwargs.has_key('style'):
                style |= wx.CB_READONLY
            else:
                style = wx.CB_READONLY
        
        super(ColumnSelect, self).__init__(parent, id, value = value, choices = choices, size = size,
                                           **kwargs)
        
        self.SetName("ColumnSelect")

        if vector:
            self.InsertColumns(vector, layer)
    
    def InsertColumns(self, vector, layer):
        """Insert columns for a vector attribute table into the columns combobox"""
        dbInfo = VectorDBInfo(vector)
        
        try:
            table = dbInfo.layers[int(layer)]['table']
            columnchoices = dbInfo.tables[table]
            columns = len(columnchoices.keys()) * ['']
            for key, val in columnchoices.iteritems():
                columns[val['index']] = key
        except (KeyError, ValueError):
            columns = []

        self.SetItems(columns)
        self.SetValue('')

    def InsertTableColumns(self, table, driver=None, database=None):
        """Insert table columns"""
        columns = []
        
        ret = gcmd.RunCommand('db.columns',
                              read = True,
                              driver = driver,
                              database = database,
                              table = table)
        
        if ret:
            columns = ret.splitlines()

        self.SetItems(columns)
        
class DbColumnSelect(wx.ComboBox):
    """
    Creates combo box for selecting columns from any table.
    """
    def __init__(self, parent,
                 id=wx.ID_ANY, value='', pos=wx.DefaultPosition,
                 size=wx.DefaultSize, choices=[''], **kargs):

        super(ColumnSelect, self).__init__(parent, id, value, pos, size, choices)

        dbtable = kargs['table'] # table to check for columns
        dbdriver = kargs['driver'] # driver for table
        dbdatabase = kargs['database'] # database for table

        if dbtable == '':
            return
        else:
            self.InsertColumns(dbtable)
                
    def InsertColumns(self, table):
        """insert columns for a table into the columns combobox"""
        if table == '' : return
        
        cmd = ['db.columns',
               'table=%s' % table]
        
        if dbdriver:
            cmd.append('driver=%s' % dbdriver)
        if dbdatabase:
            cmd.append('database=%s' % dbdatabase)
        
        try:
            columnchoices = gcmd.Command(cmd).ReadStdOutput()
        except gcmd.CmdError:
            columnchoices = []

        # columnchoices.sort()
        self.SetItems(columnchoices)
    
