"""
MODULE:     wxgui_utils.py

CLASSES:
    * AbstractLayer
    * Layer
    * LayerTree
    * LoadMapLayersDialog

PURPOSE:    Utility classes for GRASS wxPython GUI. Main functions include tree control
            for GIS map layer management, command console, and command parsing.

AUTHORS:    The GRASS Development Team
            Michael Barton (Arizona State University)
            Jachym Cepicky (Mendel University of Agriculture)
            Martin Landa <landa.martin gmail.com>

COPYRIGHT:  (C) 2007-2008 by the GRASS Development Team
            This program is free software under the GNU General Public
            License (>=v2). Read the file COPYING that comes with GRASS
            for details.
"""

import os
import sys
import string

import wx
import wx.lib.customtreectrl as CT
import wx.combo
import wx.lib.newevent

import globalvar
import menuform
import mapdisp
import render
import gcmd
import grassenv
import histogram
import utils
from debug import Debug as Debug
from icon import Icons as Icons
from preferences import globalSettings as UserSettings
try:
    import subprocess
except:
    from compat import subprocess

TREE_ITEM_HEIGHT = 25

# define event for GRASS console (running GRASS command in separate thread)
(UpdateGMConsoleEvent, EVT_UPDATE_GMCONSOLE) = wx.lib.newevent.NewEvent()

class LayerTree(CT.CustomTreeCtrl):
    """
    Creates layer tree structure
    """
    #	def __init__(self, parent, id, pos, size, style):
    def __init__(self, parent,
                 id=wx.ID_ANY, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.SUNKEN_BORDER,
                 ctstyle=CT.TR_HAS_BUTTONS | CT.TR_HAS_VARIABLE_ROW_HEIGHT |
                 CT.TR_HIDE_ROOT | CT.TR_ROW_LINES | CT.TR_FULL_ROW_HIGHLIGHT |
                 CT.TR_EDIT_LABELS | CT.TR_MULTIPLE,
                 idx=None, gismgr=None, notebook=None, auimgr=None, showMapDisplay=True):
        CT.CustomTreeCtrl.__init__(self, parent, id, pos, size, style, ctstyle)

        ### SetAutoLayout() causes that no vertical scrollbar is displayed
        ### when some layers are not visible in layer tree
        self.SetAutoLayout(True)
        self.SetGradientStyle(1)
        self.EnableSelectionGradient(True)
        self.SetFirstGradientColour(wx.Colour(100, 100, 100))
        self.SetSecondGradientColour(wx.Colour(150, 150, 150))

        self.Map = render.Map()    # instance of render.Map to be associated with display
        self.root = None           # ID of layer tree root node
        self.groupnode = 0         # index value for layers
        self.optpage = {}          # dictionary of notebook option pages for each map layer
        self.layer_selected = None # ID of currently selected layer
        self.saveitem = {}         # dictionary to preserve layer attributes for drag and drop
        self.first = True          # indicates if a layer is just added or not
        self.drag = False          # flag to indicate a drag event is in process
        self.disp_idx = idx
        self.gismgr = gismgr
        self.notebook = notebook   # GIS Manager notebook for layer tree
        self.treepg = parent       # notebook page holding layer tree
        self.auimgr = auimgr       # aui manager

        # init associated map display
        self.mapdisplay = mapdisp.MapFrame(self,
                                           id=wx.ID_ANY, pos=wx.DefaultPosition, size=(640,480),
                                           style=wx.DEFAULT_FRAME_STYLE,
                                           tree=self, notebook=self.notebook,
                                           gismgr=self.gismgr, page=self.treepg,
                                           Map=self.Map, auimgr=self.auimgr)

        # title
        self.mapdisplay.SetTitle(_("GRASS GIS Map Display: " +
                                   str(self.disp_idx) + 
                                   " - Location: " + grassenv.GetGRASSVariable("LOCATION_NAME")))

        # show new display
        if showMapDisplay is True:
            self.mapdisplay.Show()
            self.mapdisplay.Refresh()
            self.mapdisplay.Update()

        self.root = self.AddRoot(_("Map Layers"))
        self.SetPyData(self.root, (None,None))

        #create image list to use with layer tree
        il = wx.ImageList(16, 16, mask=False)

        trart = wx.ArtProvider.GetBitmap(wx.ART_FOLDER_OPEN, wx.ART_OTHER, (16, 16))
        self.folder_open = il.Add(trart)
        trart = wx.ArtProvider.GetBitmap(wx.ART_FOLDER, wx.ART_OTHER, (16, 16))
        self.folder = il.Add(trart)

        bmpsize = (16, 16)
        trgif = Icons["addrast"].GetBitmap(bmpsize)
        self.rast_icon = il.Add(trgif)

        trgif = Icons["addrgb"].GetBitmap(bmpsize)
        self.rgb_icon = il.Add(trgif)

        trgif = Icons["addhis"].GetBitmap(bmpsize)
        self.his_icon = il.Add(trgif)

        trgif = Icons["addshaded"].GetBitmap(bmpsize)
        self.shaded_icon = il.Add(trgif)

        trgif = Icons["addrarrow"].GetBitmap(bmpsize)
        self.rarrow_icon = il.Add(trgif)

        trgif = Icons["addrnum"].GetBitmap(bmpsize)
        self.rnum_icon = il.Add(trgif)

        trgif = Icons["addvect"].GetBitmap(bmpsize)
        self.vect_icon = il.Add(trgif)

        trgif = Icons["addthematic"].GetBitmap(bmpsize)
        self.theme_icon = il.Add(trgif)

        trgif = Icons["addchart"].GetBitmap(bmpsize)
        self.chart_icon = il.Add(trgif)

        trgif = Icons["addgrid"].GetBitmap(bmpsize)
        self.grid_icon = il.Add(trgif)

        trgif = Icons["addgeodesic"].GetBitmap(bmpsize)
        self.geodesic_icon = il.Add(trgif)

        trgif = Icons["addrhumb"].GetBitmap(bmpsize)
        self.rhumb_icon = il.Add(trgif)

        trgif = Icons["addlabels"].GetBitmap(bmpsize)
        self.labels_icon = il.Add(trgif)

        trgif = Icons["addcmd"].GetBitmap(bmpsize)
        self.cmd_icon = il.Add(trgif)

        self.AssignImageList(il)

        # use when groups implemented
        ## self.tree.SetItemImage(self.root, fldridx, wx.TreeItemIcon_Normal)
        ## self.tree.SetItemImage(self.root, fldropenidx, wx.TreeItemIcon_Expanded)

        self.Bind(wx.EVT_TREE_ITEM_EXPANDING,   self.OnExpandNode)
        self.Bind(wx.EVT_TREE_ITEM_COLLAPSED,   self.OnCollapseNode)
        self.Bind(wx.EVT_TREE_ITEM_ACTIVATED,   self.OnActivateLayer)
        self.Bind(wx.EVT_TREE_SEL_CHANGED,      self.OnChangeSel)
        self.Bind(CT.EVT_TREE_ITEM_CHECKED,     self.OnLayerChecked)
        self.Bind(wx.EVT_TREE_DELETE_ITEM,      self.OnDeleteLayer)
        self.Bind(wx.EVT_TREE_BEGIN_DRAG,       self.OnBeginDrag)
        self.Bind(wx.EVT_TREE_END_DRAG,         self.OnEndDrag)
        self.Bind(wx.EVT_TREE_ITEM_RIGHT_CLICK, self.OnLayerContextMenu)
        self.Bind(wx.EVT_TREE_END_LABEL_EDIT,   self.OnChangeLayerName)
        self.Bind(wx.EVT_KEY_UP,                self.OnKeyUp)
        # self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

    def OnKeyUp(self, event):
        """Key pressed"""
        key = event.GetKeyCode()
        if key == wx.WXK_DELETE and self.gismgr:
            self.gismgr.OnDeleteLayer(None)

        event.Skip()

    def OnChangeLayerName (self, event):
        """Change layer name"""
        Debug.msg (3, "LayerTree.OnChangeLayerName: name=%s" % event.GetLabel())

    def OnLayerContextMenu (self, event):
        """Contextual menu for item/layer"""
        if not self.layer_selected:
            event.Skip()
            return

        ltype = self.GetPyData(self.layer_selected)[0]['type']

        Debug.msg (4, "LayerTree.OnContextMenu: layertype=%s" % \
                       ltype)

        ## pos = event.GetPosition()
        ## pos = self.ScreenToClient(pos)

        if not hasattr (self, "popupID1"):
            self.popupID1 = wx.NewId()
            self.popupID2 = wx.NewId()
            self.popupID3 = wx.NewId()
            self.popupID4 = wx.NewId()
            self.popupID5 = wx.NewId()
            self.popupID6 = wx.NewId()
            self.popupID7 = wx.NewId()
            self.popupID8 = wx.NewId()
            self.popupID9 = wx.NewId()
                        
        self.popupMenu = wx.Menu()
        # general item
        self.popupMenu.Append(self.popupID1, text=_("Remove"))
        self.Bind(wx.EVT_MENU, self.gismgr.OnDeleteLayer, id=self.popupID1)

        if ltype != "command": # rename
            self.popupMenu.Append(self.popupID2, text=_("Rename"))
            self.Bind(wx.EVT_MENU, self.RenameLayer, id=self.popupID2)

        # map layer items
        if ltype != "group" and \
                ltype != "command": # properties
            self.popupMenu.AppendSeparator()
            self.popupMenu.Append(self.popupID8, text=_("Change opacity level"), kind=wx.ITEM_CHECK)
            if self.FindWindowById(self.GetPyData(self.layer_selected)[0]['ctrl']).GetName() == 'spinCtrl':
                checked = True
            else:
                checked = False
            self.popupMenu.Check(self.popupID8, checked)
            self.Bind(wx.EVT_MENU, self.OnPopupOpacityLevel, id=self.popupID8)
            self.popupMenu.Append(self.popupID3, text=_("Properties"))
            self.Bind(wx.EVT_MENU, self.OnPopupProperties, id=self.popupID3)
            self.popupMenu.Append(self.popupID9, text=_("Zoom to selected map"))
            self.Bind(wx.EVT_MENU, self.mapdisplay.MapWindow.ZoomToMap, id=self.popupID9)

        # specific items
        try:
            mltype = self.GetPyData(self.layer_selected)[0]['type']
        except:
            mltype = None
        # vector specific items
        if mltype and mltype == "vector":
            self.popupMenu.AppendSeparator()
            self.popupMenu.Append(self.popupID4, text=_("Show attribute table"))
            self.Bind (wx.EVT_MENU, self.gismgr.OnShowAttributeTable, id=self.popupID4)

            self.popupMenu.Append(self.popupID5, text=_("Start editing"))
            self.popupMenu.Append(self.popupID6, text=_("Stop editing"))
            self.popupMenu.Enable(self.popupID6, False)
            self.Bind (wx.EVT_MENU, self.OnStartEditing, id=self.popupID5)
            self.Bind (wx.EVT_MENU, self.OnStopEditing,  id=self.popupID6)

            layer = self.GetPyData(self.layer_selected)[0]['maplayer']
            # enable editing only for vector map layers available in the current mapset
            digit = self.mapdisplay.digittoolbar
            if layer.GetMapset() != grassenv.GetGRASSVariable("MAPSET"):
                # only vector map in current mapset can be edited
                self.popupMenu.Enable (self.popupID5, False)
                self.popupMenu.Enable (self.popupID6, False)
            elif digit and digit.layerSelectedID != None:
                # vector map already edited
                if digit.layers[digit.layerSelectedID] == layer:
                    self.popupMenu.Enable (self.popupID5, False)
                    self.popupMenu.Enable(self.popupID6, True)
                    self.popupMenu.Enable(self.popupID1, False)
                else:
                    self.popupMenu.Enable(self.popupID5, False)
                    self.popupMenu.Enable(self.popupID6, False)
            self.popupMenu.Append(self.popupID7, _("Metadata"))
            self.Bind (wx.EVT_MENU, self.OnMetadata, id=self.popupID7)


        # raster
        elif mltype and mltype == "raster":
            self.popupMenu.AppendSeparator()
            self.popupMenu.Append(self.popupID4, _("Histogram"))
            self.Bind (wx.EVT_MENU, self.OnHistogram, id=self.popupID4)
            self.popupMenu.Append(self.popupID5, _("Metadata"))
            self.Bind (wx.EVT_MENU, self.OnMetadata, id=self.popupID5)

        ## self.PopupMenu(self.popupMenu, pos)
        self.PopupMenu(self.popupMenu)
        self.popupMenu.Destroy()

    def OnMetadata(self, event):
        """Print metadata of raster/vector map layer
        TODO: Dialog to modify metadata
        """
        mapLayer = self.GetPyData(self.layer_selected)[0]['maplayer']
        mltype = self.GetPyData(self.layer_selected)[0]['type']

        if mltype == 'raster':
            cmd = ['r.info']
        elif mltype == 'vector':
            cmd = ['v.info']
        cmd.append('map=%s' % mapLayer.name)

        # print output to command log area
        self.gismgr.goutput.RunCmd(cmd)

    def OnHistogram(self, event):
        """
        Plot histogram for given raster map layer
        """
        mapLayer = self.GetPyData(self.layer_selected)[0]['maplayer']
        if not mapLayer.name:
            dlg = wx.MessageDialog(self, _("Unable to display histogram of "
                                           "raster map."),
                                   _("Error"), wx.OK | wx.ICON_ERROR)
            dlg.ShowModal()
            dlg.Destroy()
            return False

        if not hasattr (self, "histogramFrame"):
            self.histogramFrame = None

        if hasattr (self.mapdisplay, "histogram") and self.mapdisplay.histogram:
            self.histogramFrame = self.mapdisplay.histogram

        if not self.histogramFrame:
            self.histogramFrame = histogram.HistFrame(self,
                                                      id=wx.ID_ANY,
                                                      pos=wx.DefaultPosition, size=(400, 300),
                                                      style=wx.DEFAULT_FRAME_STYLE)
            # show new display
            self.histogramFrame.Show()

        self.histogramFrame.SetHistLayer(['d.histogram', 'map=%s' % mapLayer.name])
        self.histogramFrame.HistWindow.UpdateHist()
        self.histogramFrame.Refresh()
        self.histogramFrame.Update()

        return True

    def OnStartEditing (self, event):
        """
        Start editing vector map layer requested by the user
        """
        try:
            maplayer = self.GetPyData(self.layer_selected)[0]['maplayer']
        except:
            event.Skip()
            return

        if not self.mapdisplay.digittoolbar: # enable tool
            self.mapdisplay.AddToolbar("digit")
        else: # tool already enabled
            pass

        # mark layer as 'edited'
        self.mapdisplay.digittoolbar.StartEditing (maplayer)

    def OnStopEditing (self, event):
        """
        Stop editing the current vector map layer
        """
        try:
            maplayer = self.GetPyData(self.layer_selected)[0]['maplayer']
        except:
            event.Skip()
            return

        self.mapdisplay.digittoolbar.OnExit()
        self.mapdisplay.imgVectorMap = None

    def OnPopupProperties (self, event):
        """Popup properties dialog"""
        self.PropertiesDialog(self.layer_selected)

    def OnPopupOpacityLevel(self, event):
        """Popup opacity level indicator"""
        if not self.GetPyData(self.layer_selected)[0]['ctrl']:
            return

        win = self.FindWindowById(self.GetPyData(self.layer_selected)[0]['ctrl'])
        type = win.GetName()

        self.layer_selected.DeleteWindow()

        opacity = self.GetPyData(self.layer_selected)[0]['maplayer'].GetOpacity()
        if type == 'staticText':
            ctrl = wx.SpinCtrl(self, id=wx.ID_ANY, value="",
                               style=wx.SP_ARROW_KEYS, initial=100, min=0, max=100,
                               name='spinCtrl')
            ctrl.SetValue(opacity)
            self.Bind(wx.EVT_SPINCTRL, self.OnOpacity, ctrl)
        else:
            ctrl = wx.StaticText(self, id=wx.ID_ANY,
                                 name='staticText')
            if opacity < 100:
                ctrl.SetLabel('   (' + str(opacity) + '%)')
                
        self.GetPyData(self.layer_selected)[0]['ctrl'] = ctrl.GetId()
        self.layer_selected.SetWindow(ctrl)

        self.RefreshSelected()
        self.Refresh()
        
    def RenameLayer (self, event):
        """Rename layer"""
        self.EditLabel(self.layer_selected)

    def AddLayer(self, ltype, lname=None, lchecked=None, lopacity=None, lcmd=None, lgroup=None):
        """Add new item to the layer tree, create corresponding MapLayer instance.
        Launch property dialog if needed (raster, vector, etc.)

        Note: lcmd is given as a list
        """

        self.first = True
        params = {} # no initial options parameters

        # deselect active item
        if self.layer_selected:
            self.SelectItem(self.layer_selected, select=False)

        Debug.msg (3, "LayerTree().AddLayer(): ltype=%s" % (ltype))
        
        if ltype == 'command':
            # generic command item
            ctrl = wx.TextCtrl(self, id=wx.ID_ANY, value='',
                               pos=wx.DefaultPosition, size=(250,25),
                               style=wx.TE_MULTILINE|wx.TE_WORDWRAP)
            ctrl.Bind(wx.EVT_TEXT_ENTER, self.OnCmdChanged)
            ctrl.Bind(wx.EVT_TEXT,       self.OnCmdChanged)
        elif ltype == 'group':
            # group item
            ctrl = None
            grouptext = _('Layer group:') + str(self.groupnode)
            self.groupnode += 1
        else:
            # all other items (raster, vector, ...)
            if UserSettings.Get(group='general', key='changeOpacityLevel', subkey='enabled'):
                ctrl = wx.SpinCtrl(self, id=wx.ID_ANY, value="",
                                   style=wx.SP_ARROW_KEYS, initial=100, min=0, max=100,
                                   name='spinCtrl')
                
                self.Bind(wx.EVT_SPINCTRL, self.OnOpacity, ctrl)
            else:
                ctrl = wx.StaticText(self, id=wx.ID_ANY,
                                     name='staticText')
        # add layer to the layer tree
        if self.layer_selected and self.layer_selected != self.GetRootItem():
            if self.GetPyData(self.layer_selected)[0]['type'] != 'group':
                if lgroup is False:
                    # last child of root
                    layer = self.AppendItem(parentId=self.root,
                                            text='', ct_type=1, wnd=ctrl)
                elif lgroup is None or lgroup is True:
                    # insert item on given position
                    parent = self.GetItemParent(self.layer_selected)
                    layer = self.InsertItem(parentId=parent, input=self.GetPrevSibling(self.layer_selected),
                                            text='', ct_type=1, wnd=ctrl)

            else: # group (first child of self.layer_selected)
                layer = self.PrependItem(parent=self.layer_selected,
                                         text='', ct_type=1, wnd=ctrl)
                self.Expand(self.layer_selected)
        else: # add first layer to the layer tree (first child of root)
            layer = self.PrependItem(parent=self.root, text='', ct_type=1, wnd=ctrl)

        # layer is initially unchecked as inactive (beside 'command')
        # use predefined value if given
        if lchecked is not None:
            checked = lchecked
        else:
            checked = True

        self.CheckItem(layer, checked=checked)

        # select new item
        self.SelectItem(layer, select=True)
        self.layer_selected = layer

        # add text and icons for each layer ltype
        if ltype == 'raster':
            self.SetItemImage(layer, self.rast_icon)
            self.SetItemText(layer, '%s %s' % (_('raster'), _('(double click to set properties)')))
        elif ltype == 'rgb':
            self.SetItemImage(layer, self.rgb_icon)
            self.SetItemText(layer, '%s %s' % (_('RGB'), _('(double click to set properties)')))
        elif ltype == 'his':
            self.SetItemImage(layer, self.his_icon)
            self.SetItemText(layer, '%s %s' % (_('HIS'), _('(double click to set properties)')))
        elif ltype == 'shaded':
            self.SetItemImage(layer, self.shaded_icon)
            self.SetItemText(layer, '%s %s' % (_('Shaded relief'), _('(double click to set properties)')))
        elif ltype == 'rastnum':
            self.SetItemImage(layer, self.rnum_icon)
            self.SetItemText(layer, '%s %s' % (_('raster cell numbers'), _('(double click to set properties)')))
        elif ltype == 'rastarrow':
            self.SetItemImage(layer, self.rarrow_icon)
            self.SetItemText(layer, '%s %s' % (_('raster flow arrows'), _('(double click to set properties)')))
        elif ltype == 'vector':
            self.SetItemImage(layer, self.vect_icon)
            self.SetItemText(layer, '%s %s' % (_('vector'), _('(double click to set properties)')))
        elif ltype == 'thememap':
            self.SetItemImage(layer, self.theme_icon)
            self.SetItemText(layer, '%s %s' % (_('thematic map'), _('(double click to set properties)')))
        elif ltype == 'themechart':
            self.SetItemImage(layer, self.chart_icon)
            self.SetItemText(layer, '%s %s' % (_('thematic charts'), _('(double click to set properties)')))
        elif ltype == 'grid':
            self.SetItemImage(layer, self.grid_icon)
            self.SetItemText(layer, '%s %s' % (_('grid'), _('(double click to set properties)')))
        elif ltype == 'geodesic':
            self.SetItemImage(layer, self.geodesic_icon)
            self.SetItemText(layer, '%s %s' % (_('geodesic line'), _('(double click to set properties)')))
        elif ltype == 'rhumb':
            self.SetItemImage(layer, self.rhumb_icon)
            self.SetItemText(layer, '%s %s' % (_('rhumbline'), _('(double click to set properties)')))
        elif ltype == 'labels':
            self.SetItemImage(layer, self.labels_icon)
            self.SetItemText(layer, '%s %s' % (_('vector labels'), _('(double click to set properties)')))
        elif ltype == 'command':
            self.SetItemImage(layer, self.cmd_icon)
        elif ltype == 'group':
            self.SetItemImage(layer, self.folder)
            self.SetItemText(layer, grouptext)

        self.first = False

        if ltype != 'group':
            if lopacity:
                opacity = lopacity
                if UserSettings.Get(group='general', key='changeOpacityLevel', subkey='enabled'):
                    ctrl.SetValue(int(lopacity * 100))
                else:
                    if opacity < 1.0:
                        ctrl.SetLabel('   (' + str(int(opacity * 100)) + '%)')
            else:
                opacity = 1.0
            if lcmd and len(lcmd) > 1:
                cmd = lcmd
                render = False
                name = utils.GetLayerNameFromCmd(lcmd)
            else:
                cmd = []
                render = False
                name = None

            if ctrl:
                ctrlId = ctrl.GetId()
            else:
                ctrlId = None
                
            # add a data object to hold the layer's command (does not apply to generic command layers)
            self.SetPyData(layer, ({'cmd': cmd,
                                    'type' : ltype,
                                    'ctrl' : ctrlId,
                                    'maplayer' : None,
                                    'prowin' : None}, 
                                   None))

            maplayer = self.Map.AddLayer(type=ltype, command=self.GetPyData(layer)[0]['cmd'], name=name,
                                         l_active=checked, l_hidden=False,
                                         l_opacity=opacity, l_render=render)
            self.GetPyData(layer)[0]['maplayer'] = maplayer

            # run properties dialog if no properties given
            if len(cmd) == 0:
                self.PropertiesDialog(layer, show=True)

        else: # group
            self.SetPyData(layer, ({'cmd': None,
                                    'type' : ltype,
                                    'ctrl' : None,
                                    'maplayer' : None,
                                    'prowin' : None}, 
                                   None))

        # use predefined layer name if given
        if lname:
            if ltype != 'command':
                self.SetItemText(layer, lname)
            else:
                ctrl.SetValue(lname)

        # updated progress bar range (mapwindow statusbar)
        if checked is True:
            self.mapdisplay.onRenderGauge.SetRange(len(self.Map.GetListOfLayers(l_active=True)))

        # layer.SetHeight(TREE_ITEM_HEIGHT)

        return layer

    def PropertiesDialog (self, layer, show=True):
        """Launch the properties dialog"""
        if self.GetPyData(layer)[0].has_key('propwin') and \
                self.GetPyData(layer)[0]['propwin'] is not None:
            # recycle GUI dialogs
            if self.GetPyData(layer)[0]['propwin'].IsShown():
                self.GetPyData(layer)[0]['propwin'].SetFocus()
            else:
                self.GetPyData(layer)[0]['propwin'].Show()
            return
        
        completed = ''
        params = self.GetPyData(layer)[1]
        ltype  = self.GetPyData(layer)[0]['type']
                
        Debug.msg (3, "LayerTree.PropertiesDialog(): ltype=%s" % \
                   ltype)

        if self.GetPyData(layer)[0]['cmd']:
            cmdValidated = menuform.GUI().ParseCommand(self.GetPyData(layer)[0]['cmd'],
                                                       completed=(self.GetOptData,layer,params),
                                                       parentframe=self, show=show)
            self.GetPyData(layer)[0]['cmd'] = cmdValidated
        elif ltype == 'raster':
            cmd = ['d.rast']
            if UserSettings.Get(group='cmd', key='rasterOverlay', subkey='enabled'):
                cmd.append('-o')
            menuform.GUI().ParseCommand(cmd, completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'rgb':
            menuform.GUI().ParseCommand(['d.rgb'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'his':
            menuform.GUI().ParseCommand(['d.his'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'shaded':
            menuform.GUI().ParseCommand(['d.shadedmap'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'rastarrow':
            menuform.GUI().ParseCommand(['d.rast.arrow'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'rastnum':
            menuform.GUI().ParseCommand(['d.rast.num'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'vector':
            menuform.GUI().ParseCommand(['d.vect'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'thememap':
            # -s flag requested, otherwise only first thematic category is displayed
            # should be fixed by C-based d.thematic.* modules
            menuform.GUI().ParseCommand(['d.vect.thematic', '-s'], 
                                        completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'themechart':
            menuform.GUI().ParseCommand(['d.vect.chart'],
                                        completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'grid':
            menuform.GUI().ParseCommand(['d.grid'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'geodesic':
            menuform.GUI().ParseCommand(['d.geodesic'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'rhumb':
            menuform.GUI().ParseCommand(['d.rhumbline'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'labels':
            menuform.GUI().ParseCommand(['d.labels'], completed=(self.GetOptData,layer,params),
                                        parentframe=self)
        elif ltype == 'cmdlayer':
            pass
        elif ltype == 'group':
            pass

    def OnActivateLayer(self, event):
        """Click on the layer item.
        Launch property dialog, or expand/collapse group of items, etc."""
        
        layer = event.GetItem()
        self.layer_selected = layer

        self.PropertiesDialog (layer)

        if self.GetPyData(layer)[0]['type'] == 'group':
            if self.IsExpanded(layer):
                self.Collapse(layer)
            else:
                self.Expand(layer)

    def OnDeleteLayer(self, event):
        """Remove selected layer item from the layer tree"""

        item = event.GetItem()

        try:
            item.properties.Close(True)
        except:
            pass

        if item != self.root:
            Debug.msg (3, "LayerTree.OnDeleteLayer(): name=%s" % \
                           (self.GetItemText(item)))
        else:
            self.root = None

        # unselect item
        self.Unselect()
        self.layer_selected = None

        try:
            if self.GetPyData(item)[0]['type'] != 'group':
                self.Map.DeleteLayer( self.GetPyData(item)[0]['maplayer'])
        except:
            pass

        # redraw map if auto-rendering is enabled
        if self.mapdisplay.autoRender.GetValue(): 
            self.mapdisplay.OnRender(None)

        if self.mapdisplay.digittoolbar:
            self.mapdisplay.digittoolbar.UpdateListOfLayers (updateTool=True)

        # update progress bar range (mapwindow statusbar)
        self.mapdisplay.onRenderGauge.SetRange(len(self.Map.GetListOfLayers(l_active=True)))

        event.Skip()

    def OnLayerChecked(self, event):
        """Enable/disable given layer item"""
        item    = event.GetItem()
        checked = item.IsChecked()
        
        if self.drag == False and self.first == False:
            # change active parameter for item in layers list in render.Map
            if self.GetPyData(item)[0]['type'] == 'group':
                child, cookie = self.GetFirstChild(item)
                while child:
                    self.CheckItem(child, checked)
                    self.Map.ChangeLayerActive(self.GetPyData(child)[0]['maplayer'], checked)
                    child = self.GetNextSibling(child)
            else:
                self.Map.ChangeLayerActive(self.GetPyData(item)[0]['maplayer'], checked)

        # update progress bar range (mapwindow statusbar)
        self.mapdisplay.onRenderGauge.SetRange(len(self.Map.GetListOfLayers(l_active=True)))

        # redraw map if auto-rendering is enabled
        if self.mapdisplay.autoRender.GetValue(): 
            self.mapdisplay.OnRender(None)

    def OnCmdChanged(self, event):
        """Change command string"""
        ctrl = event.GetEventObject().GetId()
        cmd = event.GetString()
        layer = None

        layer = self.GetFirstVisibleItem()

        while layer and layer.IsOk():
            if self.GetPyData(layer)[0]['ctrl'] == ctrl:
                break
            
            layer = self.GetNextVisible(layer)

        # change parameters for item in layers list in render.Map
        if layer and self.drag == False:
            self.ChangeLayer(layer)
            self.GetPyData(layer)[0]['cmd'] = cmd.split(' ')
            maplayer = self.GetPyData(layer)[0]['maplayer']
            for option in maplayer.GetCmd():
                if 'map=' in option:
                    mapname = option.split('=')[1]
                    self.Map.ChangeLayerName(maplayer, mapname)

        event.Skip()

    def OnOpacity(self, event):
        """
        Set opacity level for map layer
        """
        Debug.msg (3, "LayerTree.OnOpacity(): %s" % event.GetInt())

        ctrl = event.GetEventObject().GetId()
        maplayer = None

        vislayer = self.GetFirstVisibleItem()

        layer = None
        for item in range(0, self.GetCount()):
            if self.GetPyData(vislayer)[0]['ctrl'] == ctrl:
                layer = vislayer

            if not self.GetNextVisible(vislayer):
                break
            else:
                vislayer = self.GetNextVisible(vislayer)

        if layer:
            maplayer = self.GetPyData(layer)[0]['maplayer']

        opacity = event.GetInt() / 100.
        # change opacity parameter for item in layers list in render.Map
        if maplayer and self.drag == False:
            self.Map.ChangeOpacity(maplayer, opacity)

        # redraw map if auto-rendering is enabled
        if self.mapdisplay.autoRender.GetValue(): 
            self.mapdisplay.OnRender(None)

    def OnChangeSel(self, event):
        """Selection changed"""
        oldlayer = event.GetOldItem()
        layer = event.GetItem()
        self.layer_selected = layer
        try:
            self.RefreshLine(oldlayer)
            self.RefreshLine(layer)
        except:
            pass

    def OnCollapseNode(self, event):
        """
        Collapse node
        """
        if self.GetPyData(self.layer_selected)[0]['type'] == 'group':
            self.SetItemImage(self.layer_selected, self.folder)

    def OnExpandNode(self, event):
        """
        Expand node
        """
        self.layer_selected = event.GetItem()
        if self.GetPyData(self.layer_selected)[0]['type'] == 'group':
            self.SetItemImage(self.layer_selected, self.folder_open)

    def OnBeginDrag(self, event):
        """
        Drag and drop of tree nodes
        """

        item  = event.GetItem()
        Debug.msg (3, "LayerTree.OnBeginDrag(): layer=%s" % \
                   (self.GetItemText(item)))

        event.Allow()
        self.drag = True
        self.DoSelectItem(item, unselect_others=True)

        # save everthing associated with item to drag
        self.dragItem = item

    def RecreateItem (self, event, oldItem, parent=None):
        """
        Recreate item (needed for OnEndDrag())
        """
        Debug.msg (4, "LayerTree.RecreateItem(): layer=%s" % \
                   self.GetItemText(oldItem))

        # fetch data (olditem)
        text    = self.GetItemText(oldItem)
        image   = self.GetItemImage(oldItem, 0)
        if self.GetPyData(oldItem)[0]['ctrl']:
            oldctrl = self.FindWindowById(self.GetPyData(oldItem)[0]['ctrl'])
        else:
            oldctrl = None
        checked = self.IsItemChecked(oldItem)
        
        # recreate spin/text control for layer
        if self.GetPyData(oldItem)[0]['type'] == 'command':
            newctrl = wx.TextCtrl(self, id=wx.ID_ANY, value='',
                                  pos=wx.DefaultPosition, size=(250,25),
                                  style=wx.TE_MULTILINE|wx.TE_WORDWRAP)
            try:
                newctrl.SetValue(self.GetPyData(oldItem)[0]['maplayer'].GetCmd(string=True))
            except:
                pass
            newctrl.Bind(wx.EVT_TEXT_ENTER, self.OnCmdChanged)
            newctrl.Bind(wx.EVT_TEXT,       self.OnCmdChanged)
        elif self.GetPyData(oldItem)[0]['type'] == 'group' or oldctrl is None:
            newctrl = None
        else:
            opacity = self.GetPyData(oldItem)[0]['maplayer'].GetOpacity()
            if oldctrl.GetName() == 'staticText':
                newctrl = wx.StaticText(self, id=wx.ID_ANY,
                                        name='staticText')
                if opacity < 100:
                    newctrl.SetLabel('   (' + str(opacity) + '%)')
            else:
                newctrl = wx.SpinCtrl(self, id=wx.ID_ANY, value="",
                                      style=wx.SP_ARROW_KEYS, min=0, max=100,
                                      name='spinCtrl')
                newctrl.SetValue(opacity)
                self.Bind(wx.EVT_SPINCTRL, self.OnOpacity, newctrl)

        # decide where to put new layer and put it there
        if not parent:
            flag = self.HitTest(event.GetPoint())[1]
        else:
            flag = 0

        if self.GetPyData(oldItem)[0]['type'] == 'group':
            windval = None
            data    = None
        else:
            windval = self.GetPyData(self.layer_selected)[0]['maplayer'].GetOpacity()
            data    = self.GetPyData(oldItem)

        # create GenericTreeItem instance
        if flag & wx.TREE_HITTEST_ABOVE:
            newItem = self.PrependItem(self.root, text=text, \
                                   ct_type=1, wnd=newctrl, image=image, \
                                   data=data)
        elif (flag &  wx.TREE_HITTEST_BELOW) or (flag & wx.TREE_HITTEST_NOWHERE) \
                 or (flag & wx.TREE_HITTEST_TOLEFT) or (flag & wx.TREE_HITTEST_TORIGHT):
            newItem = self.AppendItem(self.root, text=text, \
                                  ct_type=1, wnd=newctrl, image=image, \
                                  data=data)
        else:
            if parent:
                afteritem = parent
            else:
                afteritem = event.GetItem()

            if  self.GetPyData(afteritem)[0]['type'] == 'group':
                parent = afteritem
                newItem = self.AppendItem(parent, text=text, \
                                      ct_type=1, wnd=newctrl, image=image, \
                                      data=data)
                self.Expand(afteritem)
            else:
                parent = self.GetItemParent(afteritem)
                newItem = self.InsertItem(parent, afteritem, text=text, \
                                      ct_type=1, wnd=newctrl, image=image, \
                                      data=data)

        # add layer at new position
        self.SetPyData(newItem, self.GetPyData(oldItem))
        if newctrl:
            self.GetPyData(newItem)[0]['ctrl'] = newctrl.GetId()
        else:
            self.GetPyData(newItem)[0]['ctrl'] = None
            
        self.CheckItem(newItem, checked=checked)

        # newItem.SetHeight(TREE_ITEM_HEIGHT)

        return newItem

    def OnEndDrag(self, event):
        """
        Insert copy of layer in new
        position and delete original at old position
        """

        self.drag = True
        try:
            old = self.dragItem  # make sure this member exists
        except:
            return

        Debug.msg (4, "LayerTree.OnEndDrag(): layer=%s" % \
                   (self.GetItemText(self.dragItem)))

        newItem  = self.RecreateItem (event, self.dragItem)

        if  self.GetPyData(newItem)[0]['type'] == 'group':
            (child, cookie) = self.GetFirstChild(self.dragItem)
            if child:
                while child:
                    self.RecreateItem(event, child, parent=newItem)
                    self.Delete(child)
                    child = self.GetNextChild(old, cookie)[0]

            self.Expand(newItem)

        # delete layer at original position
        try:
            self.Delete(old) # entry in render.Map layers list automatically deleted by OnDeleteLayer handler
        except AttributeError:
            # FIXME being ugly (item.SetWindow(None))
            pass

        # reorder layers in render.Map to match new order after drag and drop
        self.ReorderLayers()

        # select new item
        self.SelectItem(newItem)

        # completed drag and drop
        self.drag = False

    def GetOptData(self, dcmd, layer, params, propwin):
        """Process layer data"""

        # set layer text to map name
        if dcmd:
            mapname = utils.GetLayerNameFromCmd(dcmd)
            self.SetItemText(layer, mapname)

        # update layer data
        if params:
            self.SetPyData(layer, (self.GetPyData(layer)[0], params))
        if dcmd:
            self.GetPyData(layer)[0]['cmd'] = dcmd
        self.GetPyData(layer)[0]['propwin'] = propwin
        
        # check layer as active
        # self.CheckItem(layer, checked=True)

        # change parameters for item in layers list in render.Map
        self.ChangeLayer(layer)

    def ReorderLayers(self):
        """Add commands from data associated with
        any valid layers (checked or not) to layer list in order to
        match layers in layer tree."""

        # make a list of visible layers
        treelayers = []

        vislayer = self.GetFirstVisibleItem()

        if not vislayer:
            return

        itemList = ""

        for item in range(0, self.GetCount()):
            itemList += self.GetItemText(vislayer) + ','
            if self.GetPyData(vislayer)[0]['type'] != 'group':
                treelayers.append(self.GetPyData(vislayer)[0]['maplayer'])

            if not self.GetNextVisible(vislayer):
                break
            else:
                vislayer = self.GetNextVisible(vislayer)

        Debug.msg (4, "LayerTree.ReoderLayers(): items=%s" % \
                   (itemList))

        # reorder map layers
        treelayers.reverse()
        self.Map.ReorderLayers(treelayers)

    def ChangeLayer(self, item):
        """Change layer"""

        type = self.GetPyData(item)[0]['type']

        if type == 'command':
            win = self.FindWindowById(self.GetPyData(item)[0]['ctrl'])
            if win.GetValue() != None:
                cmdlist = win.GetValue().split(' ')
                opac = 1.0
                chk = self.IsItemChecked(item)
                hidden = not self.IsVisible(item)
        elif type != 'group':
            if self.GetPyData(item)[0] is not None:
                cmdlist = self.GetPyData(item)[0]['cmd']
                opac = self.GetPyData(item)[0]['maplayer'].GetOpacity(float=True)
                chk = self.IsItemChecked(item)
                hidden = not self.IsVisible(item)
        maplayer = self.Map.ChangeLayer(layer=self.GetPyData(item)[0]['maplayer'], type=type,
                                        command=cmdlist, name=self.GetItemText(item),
                                        l_active=chk, l_hidden=hidden, l_opacity=opac, l_render=False)

        self.GetPyData(item)[0]['maplayer'] = maplayer

        # if digitization tool enabled -> update list of available vector map layers
        if self.mapdisplay.digittoolbar:
            self.mapdisplay.digittoolbar.UpdateListOfLayers(updateTool=True)

        # redraw map if auto-rendering is enabled
        if self.mapdisplay.autoRender.GetValue(): 
            self.mapdisplay.OnRender(None)

        # item.SetHeight(TREE_ITEM_HEIGHT)

    def setNotebookPage(self,pg):
        self.parent.notebook.SetSelection(pg)

    def OnCloseWindow(self, event):
        pass
        # self.Map.Clean()

    def FindItemByData(self, key, value):
        """Find item based on key and value (see PyData[0])"""
        item = self.GetFirstChild(self.root)[0]
        return self.__FindSubItemByData(item, key, value)

    def __FindSubItemByData(self, item, key, value):
        """Support method for FindItemByValue"""
        while item and item.IsOk():
            itemValue = self.GetPyData(item)[0][key]
            if value == itemValue:
                return item
            if self.GetPyData(item)[0]['type'] == 'group':
                subItem = self.GetFirstChild(item)[0]
                found = self.__FindSubItemByData(subItem, key, value)
                if found:
                    return found
            item = self.GetNextSibling(item)

        return None

class LoadMapLayersDialog(wx.Dialog):
    """Load selected map layers (raster, vector) into layer tree"""
    def __init__(self, parent, title, style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER):
        wx.Dialog.__init__(self, parent=parent, id=wx.ID_ANY, title=title, style=style)

        self.parent = parent # GMFrame

        #
        # dialog body
        #
        self.bodySizer = self.__createDialogBody()
        # update list of layer to be loaded
        self.LoadMapLayers(self.layerType.GetStringSelection()[:4],
                           self.mapset.GetStringSelection())
        #
        # buttons
        #
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        btnOk = wx.Button(self, wx.ID_OK, _("Load") )
        btnOk.SetDefault()

        #
        # bindigs
        #
        #btnOk.Bind(wx.EVT_BUTTON, self.OnOK)
        #btnCancel.Bind(wx.EVT_BUTTON, self.OnCancel)

        #
        # sizers & do layout
        #
        btnSizer = wx.StdDialogButtonSizer()
        btnSizer.AddButton(btnCancel)
        btnSizer.AddButton(btnOk)
        btnSizer.Realize()
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(item=self.bodySizer, proportion=1,
                      flag=wx.EXPAND | wx.ALL, border=5)
        mainSizer.Add(item=btnSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)

        # set dialog min size
        self.SetMinSize(self.GetSize())
        
    def __createDialogBody(self):
        bodySizer = wx.GridBagSizer(vgap=3, hgap=3)
        bodySizer.AddGrowableCol(1)
        bodySizer.AddGrowableRow(2)
        
        # layer type
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("Map layer type:")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(0,0))

        self.layerType = wx.Choice(parent=self, id=wx.ID_ANY,
                                   choices=['raster', 'vector'], size=(200,-1))
        self.layerType.SetSelection(0)
        bodySizer.Add(item=self.layerType,
                      pos=(0,1))
        
        # mapset filter
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("Mapset:")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(1,0))

        self.mapset = wx.ComboBox(parent=self, id=wx.ID_ANY,
                                  style=wx.CB_SIMPLE | wx.CB_READONLY,
                                  choices=UserSettings.Get(group='general', key='mapsetPath', subkey='value', internal=True),
                                  size=(200,-1))
        self.mapset.SetStringSelection(grassenv.GetGRASSVariable("MAPSET"))
        bodySizer.Add(item=self.mapset,
                      pos=(1,1))

        # layer list 
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("List of maps:")),
                      flag=wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_TOP,
                      pos=(2,0))
        self.layers = wx.CheckListBox(parent=self, id=wx.ID_ANY,
                                      size=(250, 100),
                                      choices=[])
        bodySizer.Add(item=self.layers,
                      flag=wx.EXPAND,
                      pos=(2,1))

        # bindings
        self.layerType.Bind(wx.EVT_CHOICE, self.OnChangeParams)
        self.mapset.Bind(wx.EVT_COMBOBOX, self.OnChangeParams)
        self.layers.Bind(wx.EVT_RIGHT_DOWN, self.OnMenu)
        
        return bodySizer

    def LoadMapLayers(self, type, mapset):
        """Load list of map layers

        @param type layer type ('raster' or 'vector')
        @param mapset mapset name
        """
        list = gcmd.Command(['g.mlist',
                             'type=%s' % type,
                             'mapset=%s' % mapset])

        maps = []
        for map in list.ReadStdOutput():
            maps.append(map)
            
        self.layers.Set(maps)
        
        # check all items by default
        for item in range(self.layers.GetCount()):
            self.layers.Check(item)

    def OnChangeParams(self, event):
        """Filter parameters changed by user"""
        # update list of layer to be loaded
        self.LoadMapLayers(self.layerType.GetStringSelection()[:4],
                           self.mapset.GetStringSelection())
    
        event.Skip()

    def OnMenu(self, event):
        """Table description area, context menu"""
        if not hasattr(self, "popupID1"):
            self.popupDataID1 = wx.NewId()
            self.popupDataID2 = wx.NewId()

            self.Bind(wx.EVT_MENU, self.OnSelectAll,   id=self.popupDataID1)
            self.Bind(wx.EVT_MENU, self.OnDeselectAll, id=self.popupDataID2)

        # generate popup-menu
        menu = wx.Menu()
        menu.Append(self.popupDataID1, _("Select all"))
        menu.Append(self.popupDataID2, _("Deselect all"))

        self.PopupMenu(menu)
        menu.Destroy()

    def OnSelectAll(self, event):
        """Select all map layer from list"""
        for item in range(self.layers.GetCount()):
            self.layers.Check(item, True)

    def OnDeselectAll(self, event):
        """Select all map layer from list"""
        for item in range(self.layers.GetCount()):
            self.layers.Check(item, False)

    def GetMapLayers(self):
        """Return list of checked map layers"""
        layerNames = []
        for indx in self.layers.GetSelections():
            # layers.append(self.layers.GetStringSelec(indx))
            pass

        # return fully qualified map names
        mapset = self.mapset.GetStringSelection()
        for item in range(self.layers.GetCount()):
            if not self.layers.IsChecked(item):
                continue
            layerNames.append(self.layers.GetString(item) + '@' + mapset)

        return layerNames
    
    def GetLayerType(self):
        """Get selected layer type"""
        return self.layerType.GetStringSelection()
    
