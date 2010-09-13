"""
@package gdialogs.py

@brief Common dialog used in wxGUI.

List of classes:
 - NewVectorDialog
 - SavedRegion
 - DecorationDialog
 - TextLayerDialog 
 - LoadMapLayersDialog
 - MultiImportDialog
 - LayersList (used by MultiImport) 
 - SetOpacityDialog

(C) 2008 by the GRASS Development Team

This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Martin Landa <landa.martin gmail.com>
"""

import os
import sys
import re
import glob

import wx
import wx.lib.filebrowsebutton as filebrowse
import wx.lib.mixins.listctrl as listmix

from grass.script import core as grass

import gcmd
import grassenv
import globalvar
import gselect
import menuform
import utils
from preferences import globalSettings as UserSettings

class NewVectorDialog(wx.Dialog):
    """Create new vector map layer"""
    def __init__(self, parent, id, title, disableAdd=False, disableTable=False,
                style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER):

        wx.Dialog.__init__(self, parent, id, title, style=style)

        self.panel = wx.Panel(parent=self, id=wx.ID_ANY)

        self.btnCancel = wx.Button(self.panel, wx.ID_CANCEL)
        self.btnOK = wx.Button(self.panel, wx.ID_OK)
        self.btnOK.SetDefault()
        self.btnOK.Enable(False)

        self.label = wx.StaticText(parent=self.panel, id=wx.ID_ANY,
                                   label=_("Name for new vector map:"))
        self.mapName = gselect.Select(parent=self.panel, id=wx.ID_ANY, size=globalvar.DIALOG_GSELECT_SIZE,
                                      type='vector', mapsets=[grassenv.GetGRASSVariable('MAPSET'),])
        self.mapName.SetFocus()
        
        self.table = wx.CheckBox(parent=self.panel, id=wx.ID_ANY,
                                 label=_("Create attribute table"))
        self.table.SetValue(True)
        if disableTable:
            self.table.Enable(False)
        
        self.addbox = wx.CheckBox(parent=self.panel,
                                  label=_('Add created map into layer tree'), style = wx.NO_BORDER)
        if disableAdd:
            self.addbox.SetValue(True)
            self.addbox.Enable(False)
        else:
            self.addbox.SetValue(UserSettings.Get(group='cmd', key='addNewLayer', subkey='enabled'))
        
        self.mapName.Bind(wx.EVT_TEXT, self.OnMapName)
        
        self.__Layout()

        self.SetMinSize(self.GetSize())

    def OnMapName(self, event):
        """Name for vector map layer given"""
        if len(event.GetString()) > 0:
            self.btnOK.Enable(True)
        else:
            self.btnOK.Enable(False)

    def __Layout(self):
        """Do layout"""
        sizer = wx.BoxSizer(wx.VERTICAL)

        dataSizer = wx.BoxSizer(wx.VERTICAL)
        dataSizer.Add(self.label, proportion=0,
                      flag=wx.ALL, border=1)
        dataSizer.Add(self.mapName, proportion=0,
                      flag=wx.EXPAND | wx.ALL, border=1)
        
        dataSizer.Add(self.table, proportion=0,
                      flag=wx.EXPAND | wx.ALL, border=1)

        dataSizer.AddSpacer(5)
        
        dataSizer.Add(item=self.addbox, proportion=0,
                      flag=wx.EXPAND | wx.ALL, border=1)
                
        # buttons
        btnSizer = wx.StdDialogButtonSizer()
        btnSizer.AddButton(self.btnCancel)
        btnSizer.AddButton(self.btnOK)
        btnSizer.Realize()

        sizer.Add(item=dataSizer, proportion=1,
                  flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        sizer.Add(item=btnSizer, proportion=0,
                  flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)
       
        self.panel.SetSizer(sizer)
        sizer.Fit(self)

    def GetName(self):
        """Return (mapName, overwrite)"""
        mapName = self.mapName.GetValue().split('@', 1)[0]

        return mapName
    
def CreateNewVector(parent, cmdDef, title=_('Create new vector map'),
                    exceptMap=None, log=None, disableAdd=False, disableTable=False):
    """Create new vector map layer

    @cmdList tuple/list (cmd list, output paramater)
    
    @return tuple (name of create vector map, add to layer tree)
    @return None of failure
    """
    cmd = cmdDef[0]
    dlg = NewVectorDialog(parent, wx.ID_ANY, title,
                          disableAdd, disableTable)
    if dlg.ShowModal() == wx.ID_OK:
        outmap = dlg.GetName()
        if outmap == exceptMap:
            wx.MessageBox(parent=parent,
                          message=_("Unable to create vector map <%s>.") % outmap,
                          caption=_("Error"),
                          style=wx.ID_OK | wx.ICON_ERROR | wx.CENTRE)
            return (None, None)
        
        if outmap == '': # should not happen
            return (None, None)
        
        cmd.append("%s=%s" % (cmdDef[1], outmap))
        
        try:
            listOfVectors = grass.list_grouped('vect')[grass.gisenv()['MAPSET']]
        except KeyError:
            listOfVectors = []
        
        if not UserSettings.Get(group='cmd', key='overwrite', subkey='enabled') and \
                outmap in listOfVectors:
            dlgOw = wx.MessageDialog(parent, message=_("Vector map <%s> already exists "
                                                       "in the current mapset. "
                                                       "Do you want to overwrite it?") % outmap,
                                     caption=_("Overwrite?"),
                                     style=wx.YES_NO | wx.YES_DEFAULT | wx.ICON_QUESTION)
            if dlgOw.ShowModal() == wx.ID_YES:
                cmd.append('--overwrite')
            else:
                dlgOw.Destroy()
                return (None, None)

        if UserSettings.Get(group='cmd', key='overwrite', subkey='enabled') is True:
            cmd.append('--overwrite')
        
        try:
            gcmd.Command(cmd)
        except gcmd.GException, e:
            gcmd.GError(parent = self,
                        message = e)
            return (None, None)

        #
        # create attribute table
        #
        if dlg.table.IsEnabled() and dlg.table.IsChecked():
            key = UserSettings.Get(group='atm', key='keycolumn', subkey='value')
            sql = 'CREATE TABLE %s (%s INTEGER)' % (outmap, key)
            
            gcmd.RunCommand('db.connect',
                            flags = 'c')
            
            gcmd.RunCommand('db.execute',
                            quiet = True,
                            parent = parent,
                            stdin = sql)
            
            gcmd.RunCommand('v.db.connect',
                            quiet = True,
                            parent = parent,
                            map = outmap,
                            table = outmap,
                            key = key,
                            layer = '1')
        
        # return fully qualified map name
        if '@' not in outmap:
            outmap += '@' + grassenv.GetGRASSVariable('MAPSET')

        if log:
            log.WriteLog(_("New vector map <%s> created") % outmap)

        return (outmap, dlg.addbox.IsChecked())

    return (None, dlg.addbox.IsChecked())

class SavedRegion(wx.Dialog):
    def __init__(self, parent, id, title="", pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE,
                 loadsave='load'):
        """
        Loading and saving of display extents to saved region file
        """
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self.loadsave = loadsave
        self.wind = ''

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        if loadsave == 'load':
            label = wx.StaticText(parent=self, id=wx.ID_ANY, label=_("Load region:"))
            box.Add(item=label, proportion=0, flag=wx.ALIGN_CENTRE | wx.ALL, border=5)
            self.selection = gselect.Select(parent=self, id=wx.ID_ANY, size=globalvar.DIALOG_GSELECT_SIZE,
                                            type='windows')
            self.selection.SetFocus()
            box.Add(item=self.selection, proportion=0, flag=wx.ALIGN_CENTRE | wx.ALL, border=5)
            self.selection.Bind(wx.EVT_TEXT, self.OnSelection)

        elif loadsave == 'save':
            label = wx.StaticText(parent=self, id=wx.ID_ANY, label=_("Save region:"))
            box.Add(item=label, proportion=0, flag=wx.ALIGN_CENTRE | wx.ALL, border=5)
            self.textentry = wx.TextCtrl(parent=self, id=wx.ID_ANY, value="",
                                         size=globalvar.DIALOG_TEXTCTRL_SIZE)
            self.textentry.SetFocus()
            box.Add(item=self.textentry, proportion=0, flag=wx.ALIGN_CENTRE | wx.ALL, border=5)
            self.textentry.Bind(wx.EVT_TEXT, self.OnText)

        sizer.Add(item=box, proportion=0, flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL,
                  border=5)

        line = wx.StaticLine(parent=self, id=wx.ID_ANY, size=(20, -1), style=wx.LI_HORIZONTAL)
        sizer.Add(item=line, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, border=5)

        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(self, wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(self, wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()

        sizer.Add(item=btnsizer, proportion=0, flag=wx.ALIGN_RIGHT | wx.ALL, border=5)

        self.SetSizer(sizer)
        sizer.Fit(self)

    def OnSelection(self, event):
        self.wind = event.GetString()

    def OnText(self, event):
        self.wind = event.GetString()

class DecorationDialog(wx.Dialog):
    """
    Controls setting options and displaying/hiding map overlay decorations
    """
    def __init__(self, parent, ovlId, title, cmd, name=None,
                 pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_DIALOG_STYLE,
                 checktxt='', ctrltxt=''):

        wx.Dialog.__init__(self, parent, wx.ID_ANY, title, pos, size, style)

        self.ovlId   = ovlId   # PseudoDC id
        self.cmd     = cmd
        self.name    = name    # overlay name
        self.parent  = parent  # MapFrame

        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.BoxSizer(wx.HORIZONTAL)
        self.chkbox = wx.CheckBox(parent=self, id=wx.ID_ANY, label=checktxt)
        if self.parent.Map.GetOverlay(self.ovlId) is None:
            self.chkbox.SetValue(True)
        else:
            self.chkbox.SetValue(self.parent.MapWindow.overlays[self.ovlId]['layer'].IsActive())
        box.Add(item=self.chkbox, proportion=0,
                flag=wx.ALIGN_CENTRE|wx.ALL, border=5)
        sizer.Add(item=box, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, border=5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        optnbtn = wx.Button(parent=self, id=wx.ID_ANY, label=_("Set options"))
        box.Add(item=optnbtn, proportion=0, flag=wx.ALIGN_CENTRE|wx.ALL, border=5)
        sizer.Add(item=box, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, border=5)

        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(parent=self, id=wx.ID_ANY,
                              label=_("Drag %s with mouse in pointer mode to position.\n"
                                      "Double-click to change options." % ctrltxt))
        if self.name == 'legend':
            label.SetLabel(label.GetLabel() + _('\nDefine raster map name for legend in '
                                                'properties dialog.'))
        box.Add(item=label, proportion=0,
                flag=wx.ALIGN_CENTRE|wx.ALL, border=5)
        sizer.Add(item=box, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, border=5)

        line = wx.StaticLine(parent=self, id=wx.ID_ANY, size=(20,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(item=line, proportion=0,
                  flag=wx.GROW|wx.ALIGN_CENTER_VERTICAL|wx.ALL, border=5)

        # buttons
        btnsizer = wx.StdDialogButtonSizer()

        self.btnOK = wx.Button(parent=self, id=wx.ID_OK)
        self.btnOK.SetDefault()
        if self.name == 'legend':
            self.btnOK.Enable(False)
        btnsizer.AddButton(self.btnOK)

        btnCancel = wx.Button(parent=self, id=wx.ID_CANCEL)
        btnsizer.AddButton(btnCancel)
        btnsizer.Realize()

        sizer.Add(item=btnsizer, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTER_VERTICAL | wx.ALL, border=5)

        #
        # bindings
        #
        self.Bind(wx.EVT_BUTTON,   self.OnOptions, optnbtn)
        self.Bind(wx.EVT_BUTTON,   self.OnCancel,  btnCancel)
        self.Bind(wx.EVT_BUTTON,   self.OnOK,      self.btnOK)

        self.SetSizer(sizer)
        sizer.Fit(self)

        # create overlay if doesn't exist
        self._CreateOverlay()

        if len(self.parent.MapWindow.overlays[self.ovlId]['cmd']) > 1:
            mapName = utils.GetLayerNameFromCmd(self.parent.MapWindow.overlays[self.ovlId]['cmd'])
            if self.parent.MapWindow.overlays[self.ovlId]['propwin'] is None and mapName:
                # build properties dialog
                menuform.GUI().ParseCommand(cmd=self.cmd,
                                            completed=(self.GetOptData, self.name, ''),
                                            parentframe=self.parent, show=False)
            if mapName:
                # enable 'OK' button
                self.btnOK.Enable()
                if name == 'legend':
                    # set title
                    self.SetTitle(_('Legend of raster map <%s>') % \
                                      mapName)
        
    def _CreateOverlay(self):
        if not self.parent.Map.GetOverlay(self.ovlId):
            overlay = self.parent.Map.AddOverlay(id=self.ovlId, type=self.name,
                                                 command=self.cmd,
                                                 l_active=False, l_render=False, l_hidden=True)

            self.parent.MapWindow.overlays[self.ovlId] = {}
            self.parent.MapWindow.overlays[self.ovlId] = { 'layer' : overlay,
                                                           'params' : None,
                                                           'propwin' : None,
                                                           'cmd' : self.cmd,
                                                           'coords': (10, 10),
                                                           'pdcType': 'image' }
        else:
            if self.parent.MapWindow.overlays[self.ovlId]['propwin'] == None:
                return

            self.parent.MapWindow.overlays[self.ovlId]['propwin'].get_dcmd = self.GetOptData


    def OnOptions(self, event):
        """        self.SetSizer(sizer)
        sizer.Fit(self)

        Sets option for decoration map overlays
        """
        if self.parent.MapWindow.overlays[self.ovlId]['propwin'] is None:
            # build properties dialog
            menuform.GUI().ParseCommand(cmd=self.cmd,
                                        completed=(self.GetOptData, self.name, ''),
                                        parentframe=self.parent)
        else:
            if self.parent.MapWindow.overlays[self.ovlId]['propwin'].IsShown():
                self.parent.MapWindow.overlays[self.ovlId]['propwin'].SetFocus()
            else:
                self.parent.MapWindow.overlays[self.ovlId]['propwin'].Show()

    def OnCancel(self, event):
        """Cancel dialog"""
        self.parent.dialogs['barscale'] = None

        self.Destroy()

    def OnOK(self, event):
        """Button 'OK' pressed"""
        # enable or disable overlay
        self.parent.Map.GetOverlay(self.ovlId).SetActive(self.chkbox.IsChecked())

        # update map
        self.parent.MapWindow.UpdateMap()

        # close dialog
        self.OnCancel(None)

    def GetOptData(self, dcmd, layer, params, propwin):
        """Process decoration layer data"""
        # update layer data
        if params:
            self.parent.MapWindow.overlays[self.ovlId]['params'] = params
        if dcmd:
            self.parent.MapWindow.overlays[self.ovlId]['cmd'] = dcmd
        self.parent.MapWindow.overlays[self.ovlId]['propwin'] = propwin

        # change parameters for item in layers list in render.Map
        # "Use mouse..." (-m) flag causes GUI freeze and is pointless here, trac #119
        
        try:
            self.parent.MapWindow.overlays[self.ovlId]['cmd'].remove('-m')
        except ValueError:
            pass
            
        self.parent.Map.ChangeOverlay(id=self.ovlId, type=self.name,
                                      command=self.parent.MapWindow.overlays[self.ovlId]['cmd'],
                                      l_active=self.parent.MapWindow.overlays[self.ovlId]['layer'].IsActive(),
                                      l_render=False, l_hidden=True)
        if  self.name == 'legend':
            if params and not self.btnOK.IsEnabled():
                self.btnOK.Enable()
            
class TextLayerDialog(wx.Dialog):
    """
    Controls setting options and displaying/hiding map overlay decorations
    """

    def __init__(self, parent, ovlId, title, name='text',
                 pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_DIALOG_STYLE):

        wx.Dialog.__init__(self, parent, wx.ID_ANY, title, pos, size, style)
        from wx.lib.expando import ExpandoTextCtrl, EVT_ETC_LAYOUT_NEEDED

        self.ovlId = ovlId
        self.parent = parent

        if self.ovlId in self.parent.MapWindow.textdict.keys():
            self.currText = self.parent.MapWindow.textdict[self.ovlId]['text']
            self.currFont = self.parent.MapWindow.textdict[self.ovlId]['font']
            self.currClr  = self.parent.MapWindow.textdict[self.ovlId]['color']
            self.currRot  = self.parent.MapWindow.textdict[self.ovlId]['rotation']
            self.currCoords = self.parent.MapWindow.textdict[self.ovlId]['coords']
        else:
            self.currClr = wx.BLACK
            self.currText = ''
            self.currFont = self.GetFont()
            self.currRot = 0.0
            self.currCoords = [10, 10, 10, 10]

        self.sizer = wx.BoxSizer(wx.VERTICAL)
        box = wx.GridBagSizer(vgap=5, hgap=5)

        # show/hide
        self.chkbox = wx.CheckBox(parent=self, id=wx.ID_ANY, \
            label='Show text object')
        if self.parent.Map.GetOverlay(self.ovlId) is None:
            self.chkbox.SetValue(True)
        else:
            self.chkbox.SetValue(self.parent.MapWindow.overlays[self.ovlId]['layer'].IsActive())
        box.Add(item=self.chkbox, span=(1,2),
                flag=wx.ALIGN_LEFT|wx.ALL, border=5,
                pos=(0, 0))

        # text entry
        label = wx.StaticText(parent=self, id=wx.ID_ANY, label=_("Enter text:"))
        box.Add(item=label,
                flag=wx.ALIGN_CENTER_VERTICAL,
                pos=(1, 0))

        self.textentry = ExpandoTextCtrl(parent=self, id=wx.ID_ANY, value="", size=(300,-1))
        self.textentry.SetFont(self.currFont)
        self.textentry.SetForegroundColour(self.currClr)
        self.textentry.SetValue(self.currText)
        # get rid of unneeded scrollbar when text box first opened
        self.textentry.SetClientSize((300,-1))
        
        box.Add(item=self.textentry,
                pos=(1, 1))

        # rotation
        label = wx.StaticText(parent=self, id=wx.ID_ANY, label=_("Rotation:"))
        box.Add(item=label,
                flag=wx.ALIGN_CENTER_VERTICAL,
                pos=(2, 0))
        self.rotation = wx.SpinCtrl(parent=self, id=wx.ID_ANY, value="", pos=(30, 50),
                                    size=(75,-1), style=wx.SP_ARROW_KEYS)
        self.rotation.SetRange(-360, 360)
        self.rotation.SetValue(int(self.currRot))
        box.Add(item=self.rotation,
                flag=wx.ALIGN_RIGHT,
                pos=(2, 1))

        # font
        fontbtn = wx.Button(parent=self, id=wx.ID_ANY, label=_("Set font"))
        box.Add(item=fontbtn,
                flag=wx.ALIGN_RIGHT,
                pos=(3, 1))

        self.sizer.Add(item=box, proportion=1,
                  flag=wx.ALL, border=10)

        # note
        box = wx.BoxSizer(wx.HORIZONTAL)
        label = wx.StaticText(parent=self, id=wx.ID_ANY,
                              label=_("Drag text with mouse in pointer mode "
                                      "to position.\nDouble-click to change options"))
        box.Add(item=label, proportion=0,
                flag=wx.ALIGN_CENTRE | wx.ALL, border=5)
        self.sizer.Add(item=box, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_CENTER | wx.ALL, border=5)

        line = wx.StaticLine(parent=self, id=wx.ID_ANY,
                             size=(20,-1), style=wx.LI_HORIZONTAL)
        self.sizer.Add(item=line, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTRE | wx.ALL, border=5)

        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(parent=self, id=wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(parent=self, id=wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()

        self.sizer.Add(item=btnsizer, proportion=0,
                  flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER, border=5)

        self.SetSizer(self.sizer)
        self.sizer.Fit(self)

        # bindings
        self.Bind(EVT_ETC_LAYOUT_NEEDED, self.OnRefit, self.textentry)
        self.Bind(wx.EVT_BUTTON,     self.OnSelectFont, fontbtn)
        self.Bind(wx.EVT_TEXT,       self.OnText,       self.textentry)
        self.Bind(wx.EVT_SPINCTRL,   self.OnRotation,   self.rotation)

    def OnRefit(self, event):
        """Resize text entry to match text"""
        self.sizer.Fit(self)

    def OnText(self, event):
        """Change text string"""
        self.currText = event.GetString()

    def OnRotation(self, event):
        """Change rotation"""
        self.currRot = event.GetInt()

        event.Skip()

    def OnSelectFont(self, event):
        """Change font"""
        data = wx.FontData()
        data.EnableEffects(True)
        data.SetColour(self.currClr)         # set colour
        data.SetInitialFont(self.currFont)

        dlg = wx.FontDialog(self, data)

        if dlg.ShowModal() == wx.ID_OK:
            data = dlg.GetFontData()
            self.currFont = data.GetChosenFont()
            self.currClr = data.GetColour()

            self.textentry.SetFont(self.currFont)
            self.textentry.SetForegroundColour(self.currClr)

            self.Layout()

        dlg.Destroy()

    def GetValues(self):
        """Get text properties"""
        return { 'text' : self.currText,
                 'font' : self.currFont,
                 'color' : self.currClr,
                 'rotation' : self.currRot,
                 'coords' : self.currCoords,
                 'active' : self.chkbox.IsChecked() }

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
        self.map_layers = [] # list of map layers (full list type/mapset)
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
        bodySizer.AddGrowableRow(3)
        
        # layer type
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("Map layer type:")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(0,0))

        self.layerType = wx.Choice(parent=self, id=wx.ID_ANY,
                                   choices=['raster', 'vector'], size=(100,-1))
        self.layerType.SetSelection(0)
        bodySizer.Add(item=self.layerType,
                      pos=(0,1))
        
        # mapset filter
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("Mapset:")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(1,0))

        self.mapset = wx.ComboBox(parent=self, id=wx.ID_ANY,
                                  style=wx.CB_SIMPLE | wx.CB_READONLY,
                                  choices=utils.ListOfMapsets(get = 'accessible'),
                                  size=(250,-1))
        self.mapset.SetStringSelection(grassenv.GetGRASSVariable("MAPSET"))
        bodySizer.Add(item=self.mapset,
                      pos=(1,1))

        # map name filter
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("Filter:")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(2,0))

        self.filter = wx.TextCtrl(parent=self, id=wx.ID_ANY,
                                  value="",
                                  size=(250,-1))
        bodySizer.Add(item=self.filter,
                      flag=wx.EXPAND,
                      pos=(2,1))

        # layer list 
        bodySizer.Add(item=wx.StaticText(parent=self, label=_("List of maps:")),
                      flag=wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_TOP,
                      pos=(3,0))
        self.layers = wx.CheckListBox(parent=self, id=wx.ID_ANY,
                                      size=(250, 100),
                                      choices=[])
        bodySizer.Add(item=self.layers,
                      flag=wx.EXPAND,
                      pos=(3,1))

        # bindings
        self.layerType.Bind(wx.EVT_CHOICE, self.OnChangeParams)
        self.mapset.Bind(wx.EVT_COMBOBOX, self.OnChangeParams)
        self.layers.Bind(wx.EVT_RIGHT_DOWN, self.OnMenu)
        self.filter.Bind(wx.EVT_TEXT, self.OnFilter)
        return bodySizer

    def LoadMapLayers(self, type, mapset):
        """Load list of map layers

        @param type layer type ('raster' or 'vector')
        @param mapset mapset name
        """
        list = gcmd.Command(['g.mlist',
                             'type=%s' % type,
                             'mapset=%s' % mapset])

        self.map_layers = []
        for map in list.ReadStdOutput():
            self.map_layers.append(map)
            
        self.layers.Set(self.map_layers)
        
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

    def OnFilter(self, event):
        """Apply filter for map names"""
        if len(event.GetString()) == 0:
           self.layers.Set(self.map_layers) 
           return 

        list = []
        for layer in self.map_layers:
            try:
                if re.compile('^' + event.GetString()).search(layer):
                    list.append(layer)
            except:
                pass

        self.layers.Set(list)
        self.OnSelectAll(None)
        
        event.Skip()

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
    
class MultiImportDialog(wx.Dialog):
    """!Import dxf layers"""
    def __init__(self, parent, type,
                 id=wx.ID_ANY, title=_("Multiple import"),
                 link = False,
                 style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER):

        self.parent = parent # GMFrame 
        self.importType = type
        self.link = link     # Link or import data (only for GDAL/OGR) 
        
        self.commandId = -1  # id of running command
       
        wx.Dialog.__init__(self, parent, id, title, style=style)

        self.panel = wx.Panel(parent=self, id=wx.ID_ANY)
        
        self.inputTitle = _("Source name")
        
        self.inputBox = wx.StaticBox(parent=self.panel, id=wx.ID_ANY,
                                     label=" %s " % self.inputTitle)
        self.layerBox = wx.StaticBox(parent=self.panel, id=wx.ID_ANY,
                                     label=_(" List of %s layers ") % self.importType.upper())

        #
        # input
        #
        if self.importType == 'dxf':
            inputFile = filebrowse.FileBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                                     size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                                     dialogTitle=_('Choose DXF file to import'),
                                                     buttonText=_('Browse'),
                                                     startDirectory=os.getcwd(), fileMode=0,
                                                     changeCallback=self.OnSetInput,
                                                     fileMask="DXF File (*.dxf)|*.dxf")
            self.input = { 'file' : [_("DXF file:"),
                                     inputFile,
                                     list()] }
            self.inputType = 'file'
        else:
            self.typeRadio = wx.RadioBox(parent = self.panel, id = wx.ID_ANY,
                                         label = _('Source type'),
                                         style = wx.RA_SPECIFY_COLS,
                                         choices = [_("File"),
                                                    _("Directory"),
                                                    _("Database"),
                                                    _("Protocol")])
            self.typeRadio.SetSelection(0)
            self.Bind(wx.EVT_RADIOBOX, self.OnChangeType)
            
            # input widgets
            if self.importType == 'gdal':
                filemask = 'GeoTIFF (*.tif)|*.tif'
            else:
                filemask = 'ESRI Shapefile (*.shp)|*.shp'
            inputFile = filebrowse.FileBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                                    size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                                    dialogTitle=_('Choose input file'),
                                                    buttonText=_('Browse'),
                                                    startDirectory=os.getcwd(),
                                                    changeCallback=self.OnSetInput,
                                                    fileMask=filemask)
            
            inputDir = filebrowse.DirBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                                  size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                                  dialogTitle=_('Choose input directory'),
                                                  buttonText=_('Browse'),
                                                  startDirectory=os.getcwd(),
                                                  changeCallback=self.OnSetInput)
            inputDir.Hide()

            inputDbFile = filebrowse.FileBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                                      size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                                      dialogTitle=_('Choose file'),
                                                      buttonText=_('Browse'),
                                                      startDirectory=os.getcwd(),
                                                      changeCallback=self.OnSetInput)
            inputDbFile.Hide()
            
            inputDbText = wx.TextCtrl(parent = self.panel, id = wx.ID_ANY)
            inputDbText.Hide()
            inputDbText.Bind(wx.EVT_TEXT, self.OnSetInput)

            inputDbChoice = wx.Choice(parent = self.panel, id = wx.ID_ANY)
            inputDbChoice.Hide()
            inputDbChoice.Bind(wx.EVT_CHOICE, self.OnSetInput)
            
            inputPro = wx.TextCtrl(parent = self.panel, id = wx.ID_ANY)
            inputPro.Hide()
            inputPro.Bind(wx.EVT_TEXT, self.OnSetInput)

            # format widget
            self.formatText = wx.StaticText(self.panel, id=wx.ID_ANY, label=_("Format:"))
            self.format = wx.Choice(parent = self.panel, id = wx.ID_ANY, size=(300, -1))
            self.format.Bind(wx.EVT_CHOICE, self.OnSetFormat)

            if self.importType == 'gdal': 
                ret = gcmd.RunCommand('r.in.gdal',
                                      quiet = True, read = True,
                                      flags = 'f')
            else: # ogr
                ret = gcmd.RunCommand('v.in.ogr',
                                      quiet = True, read = True,
                                      flags = 'f')
            
            self.input = { 'file' : [_("File:"),
                                     inputFile,
                                     list()],
                           'dir'  : [_("Directory:"),
                                     inputDir,
                                     list()],
                           'db'   : [_("Database:"),
                                     inputDbFile,
                                     list()],
                           'pro'  : [_("Protocol:"),
                                     inputPro,
                                     list()],
                           'db-win' : { 'file'   : inputDbFile,
                                        'text'   : inputDbText,
                                        'choice' : inputDbChoice },
                           }
            
            self.formatToExt = {
                # raster
                'GeoTIFF' : 'tif',
                'Erdas Imagine Images (.img)' : '.img',
                'Ground-based SAR Applications Testbed File Format (.gff)' : '.gff',
                'Arc/Info Binary Grid' : 'adf',
                'Portable Network Graphics' : 'png',
                'JPEG JFIF' : 'jpg',
                'Japanese DEM (.mem)' : 'mem',
                'Graphics Interchange Format (.gif)' : 'gif',
                'X11 PixMap Format' : 'xpm',
                'MS Windows Device Independent Bitmap' : 'bmp',
                'SPOT DIMAP' : '.dim',
                'RadarSat 2 XML Product' : 'xml',
                'EarthWatch .TIL' : '.til',
                'ERMapper .ers Labelled' : '.ers',
                'ERMapper Compressed Wavelets' : 'ecw',
                'GRIdded Binary (.grb)' : 'grb',
                'EUMETSAT Archive native (.nat)' : '.nat',
                'Idrisi Raster A.1' : 'rst',
                'Golden Software ASCII Grid (.grd)' : '.grd',
                'Golden Software Binary Grid (.grd)' : 'grd',
                'Golden Software 7 Binary Grid (.grd)' : 'grd',
                'R Object Data Store' : 'r',
                'USGS DOQ (Old Style)' : 'doq',
                'USGS DOQ (New Style)' : 'doq',
                'ENVI .hdr Labelled' : 'hdr',
                'ESRI .hdr Labelled' : 'hdr',
                'Generic Binary (.hdr Labelled)' : 'hdr',
                'PCI .aux Labelled' : 'aux',
                'EOSAT FAST Format' : 'fst',
                'VTP .bt (Binary Terrain) 1.3 Format' : 'bt',
                'FARSITE v.4 Landscape File (.lcp)' : 'lcp',
                'Swedish Grid RIK (.rik)' : 'rik',
                'USGS Optional ASCII DEM (and CDED)' : '.dem',
                'Northwood Numeric Grid Format .grd/.tab' : '',
                'Northwood Classified Grid Format .grc/.tab' : '',
                'ARC Digitized Raster Graphics' : 'arc',
                'Magellan topo (.blx)' : 'blx',
                'SAGA GIS Binary Grid (.sdat)' : 'sdat',
                # vector
                'ESRI Shapefile' : 'shp',
                'UK .NTF'        : 'ntf',
                'SDTS'           : 'ddf',
                'DGN'            : 'dgn',
                'VRT'            : 'vrt',
                'REC'            : 'rec',
                'BNA'            : 'bna',
                'CSV'            : 'csv',
                'GML'            : 'gml',
                'GPX'            : 'gpx',
                'KML'            : 'kml',
                'GMT'            : 'gmt',
                'PGeo'           : 'mdb',
                'XPlane'         : 'dat',
                'AVCBin'         : 'adf',
                'AVCE00'         : 'e00',
                'DXF'            : 'dxf',
                'Geoconcept'     : 'gxt',
                'GeoRSS'         : 'xml',
                'GPSTrackMaker'  : 'gtm',
                'VFK'            : 'vfk' }
            
            if ret:
                for line in ret.splitlines():
                    format = line.strip().rsplit(':', -1)[1].strip()
                    if format in ('Memory', 'Virtual Raster', 'In Memory Raster'):
                        continue
                    if format in ('PostgreSQL', 'SQLite',
                                  'ODBC', 'ESRI Personal GeoDatabase',
                                  'Rasterlite',
                                  'PostGIS WKT Raster driver'):
                        self.input['db'][2].append(format)
                    elif format in ('GeoJSON',
                                    'OGC Web Coverage Service',
                                    'OGC Web Map Service',
                                    'HTTP Fetching Wrapper'):
                        self.input['pro'][2].append(format)
                    else:
                        self.input['file'][2].append(format)
                        self.input['dir'][2].append(format)
            
            self.inputType = 'file'
            
            self.format.SetItems(self.input[self.inputType][2])
            
            if self.importType == 'gdal':
                self.format.SetStringSelection('GeoTIFF')
            elif self.importType == 'ogr':
                self.format.SetStringSelection('ESRI Shapefile')
        
        self.inputText = wx.StaticText(parent = self.panel, id = wx.ID_ANY,
                                       label = self.input[self.inputType][0],
                                       size = (75, -1))
        
        #
        # list of layers
        #
        self.list = LayersList(self.panel)
        self.list.LoadData()

        self.add = wx.CheckBox(parent=self.panel, id=wx.ID_ANY)
        if link:
            self.add.SetLabel(_("Add linked layers into layer tree"))
        else:
            self.add.SetLabel(_("Add imported layers into layer tree"))

        if not link and self.importType in ('gdal', 'ogr'):
            self.overrideCheck = wx.CheckBox(parent=self.panel, id=wx.ID_ANY,
                                             label=_("Override projection (use location's projection)"))
            self.overrideCheck.SetValue(True)
                                             
        self.add.SetValue(UserSettings.Get(group='cmd', key='addNewLayer', subkey='enabled'))

        self.overwrite = wx.CheckBox(parent=self.panel, id=wx.ID_ANY,
                                     label=_("Allow output files to overwrite existing files"))
        self.overwrite.SetValue(UserSettings.Get(group='cmd', key='overwrite', subkey='enabled'))
        
        #
        # buttons
        #
        # cancel
        self.btn_cancel = wx.Button(parent=self.panel, id=wx.ID_CANCEL)
        self.btn_cancel.SetToolTipString(_("Close dialog"))
        self.btn_cancel.Bind(wx.EVT_BUTTON, self.OnCancel)
        # run
        if link:
            self.btn_run = wx.Button(parent=self.panel, id=wx.ID_OK, label= _("&Link"))
            self.btn_run.SetToolTipString(_("Link selected layers"))
        else:
            self.btn_run = wx.Button(parent=self.panel, id=wx.ID_OK, label= _("&Import"))
            self.btn_run.SetToolTipString(_("Import selected layers"))
        self.btn_run.SetDefault()
        self.btn_run.Enable(False)
        self.btn_run.Bind(wx.EVT_BUTTON, self.OnRun)
        
        self.__doLayout()
        self.Layout()

    def __doLayout(self):
        dialogSizer = wx.BoxSizer(wx.VERTICAL)
        #
        # input
        #
        inputSizer = wx.StaticBoxSizer(self.inputBox, wx.HORIZONTAL)
        
        gridSizer = wx.FlexGridSizer(cols=2, vgap=5, hgap=5)
       
        gridSizer.Add(item=self.inputText,
                      flag=wx.ALIGN_CENTER_VERTICAL)
        gridSizer.AddGrowableCol(1)
        self.inputTypeSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.inputTypeSizer.Add(item=self.input[self.inputType][1], proportion = 1,
                                flag = wx.ALIGN_CENTER_VERTICAL)
        
        gridSizer.Add(item=self.inputTypeSizer,
                      flag=wx.EXPAND | wx.ALL)
        
        if self.importType != 'dxf':
            gridSizer.Add(item=self.formatText,
                          flag=wx.ALIGN_CENTER_VERTICAL)
            gridSizer.Add(item=self.format)
        
        inputSizer.Add(item=gridSizer, proportion=1,
                       flag=wx.EXPAND | wx.ALL)

        if self.importType != 'dxf':
            dialogSizer.Add(item=self.typeRadio, proportion=0,
                            flag=wx.ALL | wx.EXPAND, border=5)
        dialogSizer.Add(item=inputSizer, proportion=0,
                        flag=wx.ALL | wx.EXPAND, border=5)

        #
        # list of DXF layers
        #
        layerSizer = wx.StaticBoxSizer(self.layerBox, wx.HORIZONTAL)

        layerSizer.Add(item=self.list, proportion=1,
                      flag=wx.ALL | wx.EXPAND, border=5)
        
        dialogSizer.Add(item=layerSizer, proportion=1,
                        flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        if hasattr(self, "overrideCheck"):
            dialogSizer.Add(item=self.overrideCheck, proportion=0,
                            flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        dialogSizer.Add(item=self.overwrite, proportion=0,
                        flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)
        
        dialogSizer.Add(item=self.add, proportion=0,
                        flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=5)

        #
        # buttons
        #
        btnsizer = wx.BoxSizer(orient=wx.HORIZONTAL)
        
        btnsizer.Add(item=self.btn_cancel, proportion=0,
                     flag=wx.ALL | wx.ALIGN_CENTER,
                     border=10)
        
        btnsizer.Add(item=self.btn_run, proportion=0,
                     flag=wx.ALL | wx.ALIGN_CENTER,
                     border=10)
        
        dialogSizer.Add(item=btnsizer, proportion=0,
                        flag=wx.ALIGN_CENTER)
        
        # dialogSizer.SetSizeHints(self.panel)
        self.panel.SetAutoLayout(True)
        self.panel.SetSizer(dialogSizer)
        dialogSizer.Fit(self.panel)
        
        self.Layout()
        # auto-layout seems not work here - FIXME
        size = wx.Size(globalvar.DIALOG_GSELECT_SIZE[0] + 175, 400)
        self.SetMinSize(size)
        self.SetSize((size.width, size.height + 100))
        width = self.GetSize()[0]
        self.list.SetColumnWidth(col=1, width=width/2 - 50)
        
    def OnChangeType(self, event):
        """!Datasource type changed"""
        sel = event.GetSelection()
        win = self.input[self.inputType][1]
        self.inputTypeSizer.Remove(win)
        win.Hide()
        
        if sel == 0:   # file
            self.inputType = 'file'
            format = self.input[self.inputType][2][0]
            try:
                ext = self.formatToExt[format]
                if not ext:
                    raise KeyError
                format += ' (*.%s)|*.%s' % (ext, ext)
            except KeyError:
                format += ' (*.*)|*.*'
            
            win = filebrowse.FileBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                              size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                              dialogTitle=_('Choose input file'),
                                              buttonText=_('Browse'),
                                              startDirectory=os.getcwd(),
                                              changeCallback=self.OnSetInput,
                                              fileMask = format)
            self.input[self.inputType][1] = win
        elif sel == 1: # directory
            self.inputType = 'dir'
        elif sel == 2: # database
            self.inputType = 'db'
        elif sel == 3: # protocol
            self.inputType = 'pro'
        
        win = self.input[self.inputType][1]
        self.inputTypeSizer.Add(item = win, proportion = 1,
                                flag = wx.ALIGN_CENTER_VERTICAL)
        win.SetValue('')
        self.list.DeleteAllItems()
        win.Show()
        
        self.inputText.SetLabel(self.input[self.inputType][0])
        self.format.SetItems(self.input[self.inputType][2])
        self.format.SetSelection(0)
        
        self.inputTypeSizer.Layout()
        
    def OnCancel(self, event=None):
        """!Close dialog"""
        self.Close()

    def OnRun(self, event):
        """!Import/Link data (each layes as separate vector map)"""
        data = self.list.GetLayers()
        
        # hide dialog
        self.Hide()
        
        if self.importType == 'dxf':
            inputDxf = self.input[self.inputType][1].GetValue()
        else:
            if self.inputType == 'file':
                dsn = os.path.dirname(self.input[self.inputType][1].GetValue())
            else:
                if self.format.GetStringSelection() == 'PostgreSQL':
                    dsn = 'PG:dbname=%s' % self.input[self.inputType][1].GetStringSelection()
                else:
                    dsn = self.input[self.inputType][1].GetValue()
            try:
                ext = '.' + self.formatToExt[self.format.GetStringSelection()]
            except KeyError:
                ext = ''
        
        for layer, output in data:
            if self.importType == 'dxf':
                cmd = ['v.in.dxf',
                       'input=%s' % inputDxf,
                       'layers=%s' % layer,
                       'output=%s' % output]
            elif self.importType == 'ogr':
                if layer.rfind(ext) > -1:
                    layer = layer.replace(ext, '')
                if self.link:
                    cmd = ['v.external',
                           'dsn=%s' % dsn,
                           'output=%s' % output,
                           'layer=%s' % layer]
                else:
                    cmd = ['v.in.ogr',
                           'dsn=%s' % dsn,
                           'layer=%s' % layer,
                           'output=%s' % output]
            else: # gdal
                if self.link:
                    cmd = ['r.external',
                           'input=%s' % (os.path.join(dsn, layer)),
                           'output=%s' % output]
                else:
                    cmd = ['r.in.gdal',
                           'input=%s' % (os.path.join(dsn, layer)),
                           'output=%s' % output]
            
            if self.overwrite.IsChecked():
                cmd.append('--overwrite')
            
            if hasattr(self, "overrideCheck") and self.overrideCheck.IsChecked():
                cmd.append('-o')
            
            if UserSettings.Get(group='cmd', key='overwrite', subkey='enabled'):
                cmd.append('--overwrite')
            
            # run in Layer Manager
            self.parent.goutput.RunCmd(cmd, switchPage=True,
                                       onDone = self._addLayers)
        
        self.OnCancel()
        
    def _addLayers(self, returncode):
        """!Add imported/linked layers to layer tree"""
        self.commandId += 1
        
        if not self.add.IsChecked():
            return

        maptree = self.parent.curr_page.maptree
        
        layer, output = self.list.GetLayers()[self.commandId]
        
        if '@' not in output:
            name = output + '@' + grass.gisenv()['MAPSET']
        else:
            name = output
        # add imported layers into layer tree
        if self.importType == 'gdal':
            cmd = ['d.rast',
                   'map=%s' % name]
            if UserSettings.Get(group='cmd', key='rasterOverlay', subkey='enabled'):
                cmd.append('-o')
            
            item = maptree.AddLayer(ltype='raster',
                                    lname=name,
                                    lcmd=cmd)
        else:
            item = maptree.AddLayer(ltype='vector',
                                    lname=name,
                                    lcmd=['d.vect',
                                          'map=%s' % name])
        maptree.mapdisplay.MapWindow.ZoomToMap()
        
    def OnAbort(self, event):
        """!Abort running import

        @todo not yet implemented
        """
        pass
        
    def OnSetFormat(self, event):
        """!Format changed"""
        if self.inputType not in ['file', 'db']:
            return
        
        win = self.input[self.inputType][1]
        self.inputTypeSizer.Remove(win)
        
        if self.inputType == 'file':
            win.Destroy()
        else: # database
            win.Hide()
        
        format = event.GetString()
        
        if self.inputType == 'file':
            try:
                ext = self.formatToExt[format]
                if not ext:
                    raise KeyError
                format += ' (*.%s)|*.%s' % (ext, ext)
            except KeyError:
                format += ' (*.*)|*.*'
            
            win = filebrowse.FileBrowseButton(parent=self.panel, id=wx.ID_ANY, 
                                              size=globalvar.DIALOG_GSELECT_SIZE, labelText='',
                                              dialogTitle=_('Choose file'),
                                              buttonText=_('Browse'),
                                              startDirectory=os.getcwd(),
                                              changeCallback=self.OnSetInput,
                                              fileMask = format)
        else: # database
            if format == 'SQLite':
                win = self.input['db-win']['file']
            elif format == 'PostgreSQL':
                if grass.find_program('psql'):
                    win = self.input['db-win']['choice']
                    if not win.GetItems():
                        p = grass.Popen(['psql', '-ltA'], stdout = grass.PIPE)
                        ret = p.communicate()[0]
                        if ret:
                            db = list()
                            for line in ret.splitlines():
                                sline = line.split('|')
                                if len(sline) < 2:
                                    continue
                                dbname = sline[0]
                                if dbname:
                                    db.append(dbname)
                            win.SetItems(db)
                else:
                    win = self.input['db-win']['text']
            else:
                win = self.input['db-win']['text']
        
        self.input[self.inputType][1] = win
        if not win.IsShown():
            win.Show()
        self.inputTypeSizer.Add(item = win, proportion = 1,
                                flag = wx.ALIGN_CENTER_VERTICAL)
        self.inputTypeSizer.Layout()
        
    def OnSetInput(self, event):
        """!Input DXF file/OGR dsn defined, update list of layer widget"""
        path = event.GetString()
        if not path:
            return 

        data = list()        
        if self.importType == 'dxf':
            ret = gcmd.RunCommand('v.in.dxf',
                                  quiet = True,
                                  parent = self,
                                  read = True,
                                  flags = 'l',
                                  input = path)
            if not ret:
                self.list.LoadData()
                self.btn_run.Enable(False)
                return
            
            for line in ret.splitlines():
                layerId = line.split(':')[0].split(' ')[1]
                layerName = line.split(':')[1].strip()
                grassName = utils.GetValidLayerName(layerName)
                data.append((layerId, layerName.strip(), grassName.strip()))
        
        else: # gdal/ogr (for ogr maybe to use v.in.ogr -l)
            layerId = 1
            if self.format.GetStringSelection() == 'PostgreSQL':
                dsn = 'PG:dbname=%s' % self.input[self.inputType][1].GetStringSelection()
            else:
                dsn = self.input[self.inputType][1].GetValue()
            if self.inputType == 'file':
                baseName = os.path.basename(dsn)
                grassName = utils.GetValidLayerName(baseName.split('.', -1)[0])
                data.append((layerId, baseName, grassName))
            elif self.inputType == 'dir':
                try:
                    ext = self.formatToExt[self.format.GetStringSelection()]
                except KeyError:
                    ext = ''
                for file in glob.glob(os.path.join(dsn, "*.%s") % ext):
                    baseName = os.path.basename(file)
                    grassName = utils.GetValidLayerName(baseName.split('.', -1)[0])
                    data.append((layerId, baseName, grassName))
                    layerId += 1
            elif self.inputType == 'db':
                ret = gcmd.RunCommand('v.in.ogr',
                                      quiet = True,
                                      parent = self,
                                      read = True,
                                      flags = 'l',
                                      dsn = dsn)
                if not ret:
                    self.list.LoadData()
                    self.btn_run.Enable(False)
                    return
                layerId = 1
                for line in ret.split(','):
                    layerName = line.strip()
                    grassName = utils.GetValidLayerName(layerName)
                    data.append((layerId, layerName.strip(), grassName.strip()))
                    layerId += 1
        
        self.list.LoadData(data)
        if len(data) > 0:
            self.btn_run.Enable(True)
        else:
            self.btn_run.Enable(False)

class LayersList(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin,
                 listmix.CheckListCtrlMixin):
#                 listmix.CheckListCtrlMixin, listmix.TextEditMixin):
    """List of layers to be imported (dxf, shp...)"""
    def __init__(self, parent, pos=wx.DefaultPosition,
                 log=None):
        self.parent = parent
        
        wx.ListCtrl.__init__(self, parent, wx.ID_ANY,
                             style=wx.LC_REPORT)
        listmix.CheckListCtrlMixin.__init__(self)
        self.log = log

        # setup mixins
        listmix.ListCtrlAutoWidthMixin.__init__(self)
        # listmix.TextEditMixin.__init__(self)
        
        self.InsertColumn(0, _('Layer'))
        self.InsertColumn(1, _('Layer name'))
        self.InsertColumn(2, _('Name for GRASS map'))
        
        self.Bind(wx.EVT_COMMAND_RIGHT_CLICK, self.OnPopupMenu) #wxMSW
        self.Bind(wx.EVT_RIGHT_UP,            self.OnPopupMenu) #wxGTK

    def LoadData(self, data=None):
        """Load data into list"""
        if data is None:
            return

        self.DeleteAllItems()
        
        for id, name, grassName in data:
            index = self.InsertStringItem(sys.maxint, str(id))
            self.SetStringItem(index, 1, "%s" % str(name))
            self.SetStringItem(index, 2, "%s" % str(grassName))
            # check by default
            self.CheckItem(index, True)
        
        self.SetColumnWidth(col=0, width=wx.LIST_AUTOSIZE_USEHEADER)

    def OnPopupMenu(self, event):
        """Show popup menu"""
        if self.GetItemCount() < 1:
            return
        
        if not hasattr(self, "popupDataID1"):
            self.popupDataID1 = wx.NewId()
            self.popupDataID2 = wx.NewId()
            
            self.Bind(wx.EVT_MENU, self.OnSelectAll,  id=self.popupDataID1)
            self.Bind(wx.EVT_MENU, self.OnSelectNone, id=self.popupDataID2)
        
        # generate popup-menu
        menu = wx.Menu()
        menu.Append(self.popupDataID1, _("Select all"))
        menu.Append(self.popupDataID2, _("Deselect all"))

        self.PopupMenu(menu)
        menu.Destroy()

    def OnSelectAll(self, event):
        """Select all items"""
        item = -1
        
        while True:
            item = self.GetNextItem(item)
            if item == -1:
                break
            self.CheckItem(item, True)

        event.Skip()
        
    def OnSelectNone(self, event):
        """Deselect items"""
        item = -1
        
        while True:
            item = self.GetNextItem(item, wx.LIST_STATE_SELECTED)
            if item == -1:
                break
            self.CheckItem(item, False)

        event.Skip()
        
    def GetLayers(self):
        """Get list of layers (layer name, output name)"""
        data = []
        item = -1
        while True:
            item = self.GetNextItem(item)
            if item == -1:
                break
            if self.IsChecked(item):
                # layer / output name
                data.append((self.GetItem(item, 1).GetText(),
                             self.GetItem(item, 2).GetText()))

        return data

class SetOpacityDialog(wx.Dialog):
    """Set opacity of map layers"""
    def __init__(self, parent, id=wx.ID_ANY, title=_("Set Map Layer Opacity"),
                 size=wx.DefaultSize, pos=wx.DefaultPosition,
                 style=wx.DEFAULT_DIALOG_STYLE, opacity=100):

        self.parent = parent    # GMFrame
        self.opacity = opacity  # current opacity

        super(SetOpacityDialog, self).__init__(parent, id=id, pos=pos,
                                               size=size, style=style, title=title)

        panel = wx.Panel(parent=self, id=wx.ID_ANY)
        
        sizer = wx.BoxSizer(wx.VERTICAL)

        box = wx.GridBagSizer(vgap=5, hgap=5)
        self.value = wx.Slider(panel, id=wx.ID_ANY, value=self.opacity,
                               style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS | \
                                   wx.SL_TOP | wx.SL_LABELS,
                               minValue=0, maxValue=100,
                               size=(350, -1))

        box.Add(item=self.value,
                flag=wx.ALIGN_CENTRE, pos=(0, 0), span=(1, 2))
        box.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                   label=_("transparent")),
                pos=(1, 0))
        box.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                   label=_("opaque")),
                flag=wx.ALIGN_RIGHT,
                pos=(1, 1))

        sizer.Add(item=box, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTER_VERTICAL | wx.ALL, border=5)

        line = wx.StaticLine(parent=panel, id=wx.ID_ANY,
                             style=wx.LI_HORIZONTAL)
        sizer.Add(item=line, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTER_VERTICAL | wx.ALL, border=5)

        # buttons
        btnsizer = wx.StdDialogButtonSizer()

        btnOK = wx.Button(parent=panel, id=wx.ID_OK)
        btnOK.SetDefault()
        btnsizer.AddButton(btnOK)

        btnCancel = wx.Button(parent=panel, id=wx.ID_CANCEL)
        btnsizer.AddButton(btnCancel)
        btnsizer.Realize()

        sizer.Add(item=btnsizer, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTER_VERTICAL | wx.ALL, border=5)

        panel.SetSizer(sizer)
        sizer.Fit(panel)

        self.SetSize(self.GetBestSize())

        self.Layout()

    def GetOpacity(self):
        """Button 'OK' pressed"""
        # return opacity value
        opacity = float(self.value.GetValue()) / 100
        return opacity


