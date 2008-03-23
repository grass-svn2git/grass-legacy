"""
MODULE:     wxgui.py

CLASSES:
    * GMFrame
    * GMApp

PURPOSE:    Main Python app for GRASS wxPython GUI. Main menu, layer management
            toolbar, notebook control for display management and access to
            command console.

AUTHORS:    The GRASS Development Team
            Michael Barton (Arizona State University)
            Jachym Cepicky (Mendel University of Agriculture)
            Martin Landa <landa.martin gmail.com>

COPYRIGHT:  (C) 2006-2007 by the GRASS Development Team
            This program is free software under the GNU General Public
            License (>=v2). Read the file COPYING that comes with GRASS
            for details.

"""

import sys
import os
import time
import traceback
import types
import re
import string
import getopt

### XML 
import xml.sax
import xml.sax.handler
HandlerBase=xml.sax.handler.ContentHandler
from xml.sax import make_parser

### i18N
import gettext
gettext.install('grasswxpy', os.path.join(os.getenv("GISBASE"), 'locale'), unicode=True)

import gui_modules
gmpath = gui_modules.__path__[0]
sys.path.append(gmpath)

import images
imagepath = images.__path__[0]
sys.path.append(imagepath)

import icons
gmpath = icons.__path__[0]
sys.path.append(gmpath)

import gui_modules.globalvar as globalvar
globalvar.CheckForWx()

import wx
import wx.aui
import wx.combo
import wx.html
import wx.stc
import wx.lib.customtreectrl as CT
import wx.lib.flatnotebook as FN
from wx.lib.wordwrap import wordwrap
try:
    import subprocess
except:
    import compat.subprocess as subprocess

import gui_modules.utils as utils
import gui_modules.preferences as preferences
import gui_modules.wxgui_utils as wxgui_utils
import gui_modules.mapdisp as mapdisp
import gui_modules.menudata as menudata
import gui_modules.menuform as menuform
import gui_modules.grassenv as grassenv
import gui_modules.histogram as histogram
import gui_modules.profile as profile
import gui_modules.rules as rules
import gui_modules.mcalc_builder as mapcalculator
import gui_modules.gcmd as gcmd
import gui_modules.georect as georect
import gui_modules.dbm as dbm
import gui_modules.workspace as workspace
import gui_modules.goutput as goutput
import gui_modules.gdialogs as gdialogs
from   gui_modules.debug import Debug as Debug
from   icons.icon import Icons as Icons

UserSettings = preferences.globalSettings

class GMFrame(wx.Frame):
    """
    GIS Manager frame with notebook widget for controlling
    GRASS GIS. Includes command console page for typing GRASS
    (and other) commands, tree widget page for managing GIS map layers.
    """
    def __init__(self, parent, id=wx.ID_ANY, title=_("GRASS GIS Layer Manager (Experimental Prototype)"),
                 workspace=None):
        self.parent    = parent
        self.baseTitle = title
        self.iconsize  = (16, 16)

        wx.Frame.__init__(self, parent=parent, id=id, size=(500, 400),
                          style=wx.DEFAULT_FRAME_STYLE)
        self.SetTitle(self.baseTitle)

        self.SetIcon(wx.Icon(os.path.join(globalvar.ETCDIR, 'grass.ico'), wx.BITMAP_TYPE_ICO))

        self._auimgr = wx.aui.AuiManager(self)

        # initialize variables
        self.disp_idx      = 1            # index value for map displays and layer trees
        self.curr_page     = ''           # currently selected page for layer tree notebook
        self.curr_pagenum  = ''           # currently selected page number for layer tree notebook
        self.encoding      = 'ISO-8859-1' # default encoding for display fonts
        self.workspaceFile = workspace    # workspace file
        self.menucmd       = {}           # menuId / cmd
        self.georectifying = False        # says whether we're running the georectifier
        self.gr            = ""           # ID of georectify instance

        # creating widgets
        # -> self.notebook, self.goutput, self.outpage
        self.notebook  = self.__createNoteBook()
        self.cmdprompt = self.__createCommandPrompt()
        self.menubar   = self.__createMenuBar()
        self.toolbar   = self.__createToolBar()
        self.statusbar = self.CreateStatusBar(number=1)

        # bindings
        self.Bind(wx.EVT_CLOSE,     self.OnCloseWindow)
        self.Bind(wx.EVT_LEFT_DOWN, self.AddRaster, self.notebook)

        # minimal frame size
        self.SetMinSize((500, 400))

        # AUI stuff
        #       self._auimgr.AddPane(self.toolbar, wx.aui.AuiPaneInfo().ToolbarPane().
        #                              Top().Dockable(False).CloseButton(False).
        #                              DestroyOnClose(True).Row(0).Layer(0))
        self._auimgr.AddPane(self.notebook, wx.aui.AuiPaneInfo().
                             Left().CentrePane().BestSize((-1,-1)).Dockable(False).
                             CloseButton(False).DestroyOnClose(True).Row(1).Layer(0))
        self._auimgr.AddPane(self.cmdprompt, wx.aui.AuiPaneInfo().
                             Bottom().BestSize((-1,25)).Dockable(False).
                             CloseButton(False).DestroyOnClose(True).
                             PaneBorder(False).Row(1).Layer(0).Position(0).
                             CaptionVisible(False))

        self._auimgr.Update()

        wx.CallAfter(self.notebook.SetSelection, 0)

        # start default initial display
        self.NewDisplay(show=False)
        self.Show()

        # load workspace file if requested
        if (self.workspaceFile):
            # load given workspace file
            if self.LoadWorkspaceFile(self.workspaceFile):
                self.SetTitle(self.baseTitle + " - " +  os.path.basename(self.workspaceFile))
            else:
                self.workspaceFile = None

        # show map display widnow
        # -> OnSize() -> UpdateMap()
        self.curr_page.maptree.mapdisplay.Show()

    def __doLayout(self):
        """Do Layout (unused bacause of aui manager...)"""
        # self.panel = wx.Panel(self,-1, style= wx.EXPAND)
        # sizer= wx.BoxSizer(wx.VERTICAL)
        # self.cmdsizer = wx.BoxSizer(wx.HORIZONTAL)

        # item, proportion, flag, border, userData
        # self.sizer.Add(self.notebook, proportion=1, flag=wx.EXPAND, border=1)
        # self.sizer.Add(self.cmdinput, proportion=0, flag=wx.EXPAND, border=1)
        # self.SetSizer(self.sizer)

        # self.sizer.Fit(self)
        # self.Layout()

    def __createCommandPrompt(self):
        """Creates command-line input area"""
        self.cmdprompt = wx.Panel(self)

        label = wx.StaticText(parent=self.cmdprompt, id=wx.ID_ANY, label="Cmd >")
	# label.SetFont(wx.Font(pointSize=11, family=wx.FONTFAMILY_DEFAULT,
        #                      style=wx.NORMAL, weight=wx.BOLD))
        input = wx.TextCtrl(parent=self.cmdprompt, id=wx.ID_ANY,
                            value="",
                            style=wx.TE_LINEWRAP | wx.TE_PROCESS_ENTER,
                            size=(-1, 25))

        input.SetFont(wx.Font(10, wx.FONTFAMILY_MODERN, wx.NORMAL, wx.NORMAL, 0, ''))

        wx.CallAfter(input.SetInsertionPoint, 0)

        self.Bind(wx.EVT_TEXT_ENTER, self.OnRunCmd,        input)
        self.Bind(wx.EVT_TEXT,       self.OnUpdateStatusBar, input)

        # layout
        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(item=label, proportion=0,
                  flag=wx.EXPAND | wx.ALL | wx.ALIGN_CENTER_VERTICAL | wx.ALIGN_CENTER,
                  border=4)
        sizer.Add(item=input, proportion=1,
                  flag=wx.EXPAND | wx.ALL,
                  border=1)

        self.cmdprompt.SetSizer(sizer)
        sizer.Fit(self.cmdprompt)
        self.cmdprompt.Layout()

        return self.cmdprompt

    def __createMenuBar(self):
        """Creates menubar"""

        self.menubar = wx.MenuBar()
        menud = menudata.Data()
        for eachMenuData in menud.GetMenu():
            for eachHeading in eachMenuData:
                menuLabel = eachHeading[0]
                menuItems = eachHeading[1]
                self.menubar.Append(self.__createMenu(menuItems), menuLabel)
        self.SetMenuBar(self.menubar)

        return self.menubar

    def __createMenu(self, menuData):
        """Creates menu"""

        menu = wx.Menu()
        for eachItem in menuData:
            if len(eachItem) == 2:
                label = eachItem[0]
                subMenu = self.__createMenu(eachItem[1])
                menu.AppendMenu(wx.ID_ANY, label, subMenu)
            else:
                self.__createMenuItem(menu, *eachItem)
        self.Bind(wx.EVT_MENU_HIGHLIGHT_ALL, self.OnMenuHighlight)
        return menu

    def __createMenuItem(self, menu, label, help, handler, gcmd, kind=wx.ITEM_NORMAL):
        """Creates menu items"""

        if not label:
            menu.AppendSeparator()
            return

        if len(gcmd) > 0:
            helpString = gcmd + ' -- ' + help
        else:
            helpString = help

        menuItem = menu.Append(wx.ID_ANY, label, helpString, kind)
        
        self.menucmd[menuItem.GetId()] = gcmd

        rhandler = eval(handler)

        self.Bind(wx.EVT_MENU, rhandler, menuItem)

    def __createNoteBook(self):
        """Creates notebook widgets"""

        #create main notebook widget
        nbStyle = FN.FNB_FANCY_TABS | \
                    FN.FNB_BOTTOM | \
                    FN.FNB_NO_NAV_BUTTONS
        self.notebook = FN.FlatNotebook(parent=self, id=wx.ID_ANY, style=nbStyle)

        #self.notebook = wx.aui.AuiNotebook(parent=self, id=wx.ID_ANY, style=wx.aui.AUI_NB_BOTTOM)
        #self.notebook.SetFont(wx.Font(pointSize=11, family=wx.FONTFAMILY_DEFAULT, style=wx.NORMAL, weight=0))

        # create displays notebook widget and add it to main notebook page
        cbStyle = globalvar.FNPageStyle
        self.gm_cb = FN.FlatNotebook(self, id=wx.ID_ANY, style=cbStyle)
        self.gm_cb.SetTabAreaColour(globalvar.FNPageColor)
        self.notebook.AddPage(self.gm_cb, text=_("Map layers for each display"))

        # create command output text area and add it to main notebook page
        self.goutput = goutput.GMConsole(self)
        self.outpage = self.notebook.AddPage(self.goutput, text=_("Command output"))

        # bingings
        self.Bind(FN.EVT_FLATNOTEBOOK_PAGE_CHANGED, self.onCBPageChanged, self.gm_cb)
        self.Bind(FN.EVT_FLATNOTEBOOK_PAGE_CLOSING, self.onCBPageClosed,  self.gm_cb)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(item=self.goutput, proportion=1,
                  flag=wx.EXPAND | wx.ALL, border=1)
        self.SetSizer(sizer)
#        self.out_sizer.Fit(self.outpage)
        self.Layout()

        self.Centre()
        return self.notebook

    def __createToolBar(self):
        """Creates toolbar"""

        self.toolbar = self.CreateToolBar()
        self.toolbar.SetToolBitmapSize(globalvar.toolbarSize)

        for each in self.ToolbarData():
            self.AddToolbarButton(self.toolbar, *each)
        self.toolbar.Realize()

        return self.toolbar

    def OnMenuHighlight(self, event):
        """
        Default menu help handler
        """
         # Show how to get menu item info from this event handler
        id = event.GetMenuId()
        item = self.GetMenuBar().FindItemById(id)
        if item:
            text = item.GetText()
            help = item.GetHelp()

        # but in this case just call Skip so the default is done
        event.Skip()

    def OnGeorectify(self, event):
        """
        Launch georectifier module
        """
        self.gr = georect.GeorectWizard(self)
        self.georectifying = True
        
    def OnMapsets(self, event):
        """
        Launch mapset access dialog
        """
        dlg = preferences.MapsetAccess(parent=self, id=wx.ID_ANY)
        dlg.CenterOnScreen()

        # if OK is pressed...
        if dlg.ShowModal() == wx.ID_OK:
            ms = dlg.GetMapsets()
            # run g.mapsets with string of accessible mapsets
            cmdlist = ['g.mapsets', 'mapset=%s' % ','.join(ms)]
            gcmd.Command(cmdlist)
            UserSettings.Set(group='general', key='mapsetPath', subkey='value', value=ms, internal=True)
            
    def OnRDigit(self, event):
        """
        Launch raster digitizing module
        """
        pass

    # choicebook methods
    def onCBPageChanged(self, event):
        """Page in notebook changed"""

        old_pgnum = event.GetOldSelection()
        new_pgnum = event.GetSelection()
        self.curr_page   = self.gm_cb.GetCurrentPage()
        self.curr_pagenum = self.gm_cb.GetSelection()
        try:
            self.curr_page.maptree.mapdisplay.SetFocus()
            self.curr_page.maptree.mapdisplay.Raise()
        except:
            pass

        event.Skip()

    def onCBPageClosed(self, event):
        """
        Page of notebook closed
        Also close associated map display
        """

        self.gm_cb.GetPage(event.GetSelection()).maptree.Map.Clean()
        self.gm_cb.GetPage(event.GetSelection()).maptree.Close(True)
        event.Skip()

    def OnRunCmd(self, event):
        """Run command"""

        cmd = event.GetString()

        self.goutput.RunCmd(cmd)

        self.OnUpdateStatusBar(None)

    def GetLogWindow(self):
        """Get widget for command output"""
        return self.goutput

    def OnUpdateStatusBar(self, event):
        if event is None:
            self.statusbar.SetStatusText("")
        else:
            self.statusbar.SetStatusText(_("Type GRASS command and run by pressing ENTER"))

    def GetMenuCmd(self, event):
        """Get GRASS command from menu item

        Return command as a list"""
        cmd = self.menucmd[event.GetId()]
        try:
            cmdlist = cmd.split(' ')
        except: # already list?
            cmdlist = cmd

        try:
            layer = self.curr_page.maptree.layer_selected
            name = self.curr_page.maptree.GetPyData(layer)[0]['maplayer'].name
            type = self.curr_page.maptree.GetPyData(layer)[0]['type']
        except:
            layer = None
        if layer and len(cmdlist) == 1: # only if no paramaters given
            if type == 'raster' and cmdlist[0][0] == 'r' and cmdlist[0][1] != '3':
                cmdlist.append(name) # TODO map/input=
            elif type == 'vector' and cmdlist[0][0] == 'v':
                cmdlist.append(name) # TODO map/input=

        return cmdlist

    def RunMenuCmd(self, event):
        """Run command selected from menu"""
        cmd = self.GetMenuCmd(event)
        self.goutput.RunCmd(cmd)

    def OnMenuCmd(self, event):
        """Parse command selected from menu"""
        cmd = self.GetMenuCmd(event)
        menuform.GUI().ParseCommand(cmd, parentframe=self)

    def OnNewVector(self, event):
        """Create new vector map layer"""
        name = gdialogs.CreateNewVector(self)
        if name:
            self.goutput.WriteCmdLog('New vector map <%s> created' % name)
            
    def OnAboutGRASS(self, event):
        """Display 'About GRASS' dialog"""
        info = wx.AboutDialogInfo()
        # name
        info.SetName("GRASS GIS")
        # version
        versionCmd = gcmd.Command(['g.version'])
        info.SetVersion(versionCmd.ReadStdOutput()[0].replace('GRASS', '').strip())
        # description
        copyrightFile = open(os.path.join(os.getenv("GISBASE"), "COPYING"), 'r')
        copyrightOut = []
        copyright = copyrightFile.readlines()
        info.SetCopyright(wordwrap(''.join(copyright[:11] + copyright[26:-3]),
                                   550, wx.ClientDC(self)))
        copyrightFile.close()
        # website
        info.SetWebSite(("http://grass.osgeo.org", "The official GRASS site"))
        # licence
        licenceFile = open(os.path.join(os.getenv("GISBASE"), "GPL.TXT"), 'r')
        info.SetLicence(''.join(licenceFile.readlines()))
        licenceFile.close()
        # credits
        authorsFile = open(os.path.join(os.getenv("GISBASE"), "AUTHORS"), 'r')
        info.SetDevelopers([''.join(authorsFile.readlines())])
        authorsFile.close()

        wx.AboutBox(info)

    def OnWorkspace(self, event):
        """Workspace menu (new, load)"""
        point = wx.GetMousePosition()
        menu = wx.Menu()

        # Add items to the menu
        new = wx.MenuItem(menu, wx.ID_ANY, Icons["workspaceNew"].GetLabel())
        new.SetBitmap(Icons["workspaceNew"].GetBitmap(self.iconsize))
        menu.AppendItem(new)
        self.Bind(wx.EVT_MENU, self.OnWorkspaceNew, new)

        load = wx.MenuItem(menu, wx.ID_ANY, Icons["workspaceLoad"].GetLabel())
        load.SetBitmap(Icons["workspaceLoad"].GetBitmap(self.iconsize))
        menu.AppendItem(load)
        self.Bind(wx.EVT_MENU, self.OnWorkspaceLoad, load)

        # create menu
        self.PopupMenu(menu)
        menu.Destroy()

    def OnWorkspaceNew(self, event=None):
        """Create new workspace file

        Erase current workspace settings first"""

        Debug.msg(4, "GMFrame.OnWorkspaceNew():")

        maptree = self.curr_page.maptree

        # ask user to save current settings
        if maptree.GetCount() > 0:
             dlg = wx.MessageDialog(self, message=_("Workspace is not empty. "
                                                    "Do you want to store current settings "
                                                    "to workspace file?"),
                                    caption=_("Save current settings?"),
                                    style=wx.OK | wx.CANCEL | wx.ICON_QUESTION)
             if dlg.ShowModal() == wx.ID_OK:
                 self.OnWorkspaceSaveAs()
             dlg.Destroy()

        # delete all items
        maptree.DeleteAllItems()

        # add new root element
        maptree.root = maptree.AddRoot("Map Layers")
        self.curr_page.maptree.SetPyData(maptree.root, (None,None))

    def OnWorkspaceOpen(self, event=None):
        """Open file with workspace definition"""
        dlg = wx.FileDialog(parent=self, message=_("Choose workspace file"),
                            defaultDir=os.getcwd(), wildcard="*.gxw")

        filename = ''
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetPath()

        if filename == '':
            return

        Debug.msg(4, "GMFrame.OnWorkspaceOpen(): filename=%s" % filename)

        self.LoadWorkspaceFile(filename)

        self.workspaceFile = filename
        self.SetTitle(self.baseTitle + " - " +  os.path.basename(self.workspaceFile))

    def LoadWorkspaceFile(self, filename):
        """Load layer tree definition stored in GRASS Workspace XML file (gxw)

        Return True on success
        Return False on error"""

        # dtd
        dtdFilename = os.path.join(globalvar.ETCWXDIR, "gui_modules", "grass-gxw.dtd")

        # validate xml agains dtd
        #         dtd = xmldtd.load_dtd(dtdFilename)
        #         parser = xmlproc.XMLProcessor()
        #         parser.set_application(xmlval.ValidatingApp(dtd, parser))
        #         parser.dtd = dtd
        #         parser.ent = dtd
        #         try:
        #             # TODO: set_error_handler(self,err)
        #             parser.parse_resource(filename)
        #         except:
        #             dlg = wx.MessageDialog(self, _("Unable to open workspace file <%s>. "
        #                                            "It is not valid GRASS Workspace File.") % filename,
        #                                    _("Error"), wx.OK | wx.ICON_ERROR)
        #             dlg.ShowModal()
        #             dlg.Destroy()
        #             return False

        # delete current layer tree content
        self.OnWorkspaceNew()

        # read file and fix patch to dtd
        try:
            file = open(filename, "r")

            fileStream = ''.join(file.readlines())
            p = re.compile( '(grass-gxw.dtd)')
            p.search(fileStream)
            fileStream = p.sub(dtdFilename, fileStream)

            # sax
            gxwXml = workspace.ProcessWorkspaceFile()
            
            try:
                xml.sax.parseString(fileStream, gxwXml)
            except xml.sax.SAXParseException, err:
                raise gcmd.GStdError(_("Reading workspace file <%s> failed. "
                                       "Invalid file, unable to parse XML document.") % filename + \
                                         "\n\n%s" % err,
                                     parent=self)
            except ValueError, err:
                raise gcmd.GStdError(_("Reading workspace file <%s> failed. "
                                       "Invalid file, unable to parse XML document.") % filename + \
                                         "\n\n%s" % err,
                                     parent=self)

            busy = wx.BusyInfo(message=_("Please wait, loading map layers into layer tree..."),
                               parent=self)
            wx.Yield()

            for layer in gxwXml.layers:
                if layer['display'] >= self.disp_idx:
                    # create new map display window if needed
                    self.NewDisplay()
                maptree = self.gm_cb.GetPage(layer['display']).maptree
                newItem = maptree.AddLayer(ltype=layer['type'],
                                           lname=layer['name'],
                                           lchecked=layer['checked'],
                                           lopacity=layer['opacity'],
                                           lcmd=layer['cmd'],
                                           lgroup=layer['group'])
            #   maptree.PropertiesDialog(newItem, show=False)

            busy.Destroy()
            
            # reverse list of map layers
            maptree.Map.ReverseListOfLayers()

            file.close()
        except IOError, err:
            wx.MessageBox(parent=self,
                          message="%s <%s>. %s%s" % (_("Unable to read workspace file"),
                                                     filename, os.linesep, err),
                          caption=_("Error"), style=wx.OK | wx.ICON_ERROR | wx.CENTRE)
            return False
        except gcmd.GStdError, e:
            print e
            return False
                               
        return True

    def OnWorkspaceLoad(self, event=None):
        """Load given map layers into layer tree"""
        dialog = wxgui_utils.LoadMapLayersDialog(parent=self, title=_("Load map layers into layer tree"))

        if dialog.ShowModal() == wx.ID_OK:
            maptree = self.curr_page.maptree
            busy = wx.BusyInfo(message=_("Please wait, loading map layers into layer tree..."),
                               parent=self)
            wx.Yield()

            for layerName in dialog.GetMapLayers():
                if dialog.GetLayerType() == 'raster':
                    cmd = ['d.rast', 'map=%s' % layerName]
                elif dialog.GetLayerType() == 'vector':
                    cmd = ['d.vect', 'map=%s' % layerName]
                newItem = maptree.AddLayer(ltype=dialog.GetLayerType(),
                                           lname=layerName,
                                           lchecked=True,
                                           lopacity=1.0,
                                           lcmd=cmd,
                                           lgroup=None)

            busy.Destroy()

    def OnWorkspaceSaveAs(self, event=None):
        """Save workspace definition to selected file"""

        dlg = wx.FileDialog(parent=self, message=_("Choose file to save current workspace"),
                            defaultDir=os.getcwd(), wildcard="*.gxw", style=wx.FD_SAVE)

        filename = ''
        if dlg.ShowModal() == wx.ID_OK:
            filename = dlg.GetPath()

        if filename == '':
            return False

        # check for extension
        if filename[-4:] != ".gxw":
            filename += ".gxw"

        if os.path.exists(filename):
            dlg = wx.MessageDialog(self, message=_("Workspace file <%s> already exists. "
                                                   "Do you want to overwrite this file?") % filename,
                                   caption=_("Warning"), style=wx.OK | wx.CANCEL | wx.ICON_QUESTION)
            if dlg.ShowModal() != wx.ID_OK:
                dlg.Destroy()
                return False

        Debug.msg(4, "GMFrame.OnWorkspaceSaveAs(): filename=%s" % filename)

        self.SaveToWorkspaceFile(filename)
        self.workspaceFile = filename
        self.SetTitle(self.baseTitle + " - " + os.path.basename(self.workspaceFile))

    def OnWorkspaceSave(self, event=None):
        """Save file with workspace definition"""

        if self.workspaceFile:
            dlg = wx.MessageDialog(self, message=_("Workspace file <%s> already exists. "
                                                   "Do you want to overwrite this file?") % \
                                       self.workspaceFile,
                                   caption=_("Warning"), style=wx.OK | wx.CANCEL | wx.ICON_QUESTION)
            if dlg.ShowModal() != wx.ID_OK:
                dlg.Destroy()
            else:
                Debug.msg(4, "GMFrame.OnWorkspaceSave(): filename=%s" % self.workspaceFile)
                self.SaveToWorkspaceFile(self.workspaceFile)
        else:
            self.OnWorkspaceSaveAs()

    def WriteLayersToWorkspaceFile(self, file, mapTree, item):
        """Write bunch of layers to GRASS Workspace XML file"""
        self.indent += 4
        while item and item.IsOk():
            type = mapTree.GetPyData(item)[0]['type']
            if type != 'group':
                maplayer = mapTree.GetPyData(item)[0]['maplayer']
            else:
                maplayer = None

            checked = int(item.IsChecked())
            cmd = mapTree.GetPyData(item)[0]['cmd']
            if type == 'command':
                file.write('%s<layer type="%s" name="%s" checked="%d">\n' % \
                               (' ' * self.indent, type, ' '.join(cmd), checked));
                file.write('%s</layer>\n' % (' ' * self.indent));
            elif type == 'group':
                name = mapTree.GetItemText(item)
                file.write('%s<group name="%s" checked="%d">\n' % \
                               (' ' * self.indent, name, checked));
                self.indent += 4
                subItem = mapTree.GetFirstChild(item)[0]
                self.WriteLayersToWorkspaceFile(file, mapTree, subItem)
                self.indent -= 4
                file.write('%s</group>\n' % (' ' * self.indent));
            else:
                name = mapTree.GetItemText(item)
                opacity = maplayer.GetOpacity(float=True)
                file.write('%s<layer type="%s" name="%s" checked="%d" opacity="%f">\n' % \
                               (' ' * self.indent, type, name, checked, opacity));
                # layer properties
                self.indent += 4
                file.write('%s<task name="%s">\n' % (' ' * self.indent, cmd[0]))
                self.indent += 4
                for option in cmd[1:]:
                    if option[0] == '-': # flag
                        file.write('%s<flag name="%s" />\n' %
                                   (' ' * self.indent, option[1]))
                    else: # parameter
                        key, value = option.split('=')
                        file.write('%s<parameter name="%s">\n' %
                                   (' ' * self.indent, key))
                        self.indent += 4
                        file.write('%s<value>%s</value>\n' %
                                   (' ' * self.indent, value))
                        self.indent -= 4
                        file.write('%s</parameter>\n' % (' ' * self.indent));
                self.indent -= 4
                file.write('%s</task>\n' % (' ' * self.indent));
                self.indent -= 4
                file.write('%s</layer>\n' % (' ' * self.indent));
            item = mapTree.GetNextSibling(item)
        self.indent -= 4

    def SaveToWorkspaceFile(self, filename):
        """Save layer tree layout to workspace file

        Return True on success, False on error
        """

        try:
            file = open(filename, "w")
        except IOError:
            wx.MessageBox(parent=self,
                          message=_("Unable to open workspace file <%s> for writing.") % filename,
                          caption=_("Error"), style=wx.OK | wx.ICON_ERROR | wx.CENTRE)
            return False

        try:
            self.indent = 0 # number of spaces
            # write header
            file.write('<?xml version="1.0" encoding="UTF-8"?>\n')
            file.write('<!DOCTYPE gxw SYSTEM "grass-gxw.dtd">\n')
            file.write('%s<gxw>\n' % (' ' * self.indent))
            # list of displays
            for page in range(0, self.gm_cb.GetPageCount()):
                self.indent =+ 4
                file.write('%s<display>\n' % (' ' * self.indent))
                mapTree = self.gm_cb.GetPage(page).maptree
                # list of layers
                item = mapTree.GetFirstChild(mapTree.root)[0]
                self.WriteLayersToWorkspaceFile(file, mapTree, item)
                file.write('%s</display>\n' % (' ' * self.indent))
            self.indent =- 4
            file.write('%s</gxw>\n' % (' ' * self.indent))
            del self.indent
        except StandardError, e:
            file.close()
            wx.MessageBox(parent=self, message=_("Writing current settings to workspace file failed (%s)." % e),
                          caption=_("Error"), style=wx.OK | wx.ICON_ERROR | wx.CENTRE)
            return False

        file.close()
        
        return True
    
    def OnWorkspaceClose(self, event=None):
        """Close file with workspace definition

        If workspace has been modified ask user to save the changes.
        """

        Debug.msg(4, "GMFrame.OnWorkspaceClose(): file=%s" % self.workspaceFile)
        self.workspaceFile = None
        self.SetTitle(self.baseTitle)

    def RulesCmd(self, event):
        """
        Launches dialog for commands that need rules
        input and processes rules
        """
        command = self.GetMenuCmd(event)

        dlg = rules.RulesText(self, cmd=command)
        if dlg.ShowModal() == wx.ID_OK:
            gtemp = utils.GetTempfile()
            output = open(gtemp, "w")
            try:
                output.write(dlg.rules)
            finally:
                output.close()

            if command[0] == 'r.colors':
                cmdlist = [command[0],
                           'map=%s' % dlg.inmap,
                           'rules=%s' % gtemp]
            else:
                cmdlist = [command[0],
                           'input=%s' % dlg.inmap,
                           'output=%s' % dlg.outmap,
                           'rules=%s' % gtemp]

            if dlg.overwrite == True:
                cmdlist.append('--o')

            dlg.Destroy()

            self.goutput.RunCmd(cmdlist)

    def OnXTerm(self, event):
        """
        Run commands that need interactive xmon
        """
        command = self.GetMenuCmd(event)

        # unset display mode
        del os.environ['GRASS_RENDER_IMMEDIATE']

        # open next available xmon
        xmonlist = []
        gisbase = os.environ['GISBASE']

        # make list of xmons that are not running
        cmdlist = ["d.mon", "-L"]
        p = gcmd.Command(cmdlist)

        for line in p.ReadStdOutput():                
            line = line.strip()
            if line.startswith('x') and 'not running' in line:
                xmonlist.append(line[0:2])

        # open available xmon
        xmon = xmonlist[0]
        cmdlist = ["d.mon","start=%s" % xmon]
        p = gcmd.Command(cmdlist)

        # run the command        
        runbat = os.path.join(gisbase,'etc','grass-run.bat')
        xtermwrapper = os.path.join(gisbase,'etc','grass-xterm-wrapper')
        grassrun = os.path.join(gisbase,'etc','grass-run.sh')
        command = ' '.join(command)
        
        if 'OS' in os.environ and os.environ['OS'] == "Windows_NT":
            cmdlist = ["cmd.exe", "/c", 'start "%s"' % runbat, command]
        else:
            cmdlist = [xtermwrapper, '-e "%s"' % grassrun, command]
        p = gcmd.Command(cmdlist)

        # reset display mode
        os.environ['GRASS_RENDER_IMMEDIATE'] = 'TRUE'

    def OnPreferences(self, event):
        """General GUI preferences/settings"""
        preferences.PreferencesDialog(parent=self).ShowModal()

    def DispHistogram(self, event):
        """
        Init histogram display canvas and tools
        """
        self.histogram = histogram.HistFrame(self,
                                           id=wx.ID_ANY, pos=wx.DefaultPosition, size=(400,300),
                                           style=wx.DEFAULT_FRAME_STYLE)

        #show new display
        self.histogram.Show()
        self.histogram.Refresh()
        self.histogram.Update()

    def DispProfile(self, event):
        """
        Init profile canvas and tools
        """
        self.profile = profile.ProfileFrame(self,
                                           id=wx.ID_ANY, pos=wx.DefaultPosition, size=(400,300),
                                           style=wx.DEFAULT_FRAME_STYLE)
        self.profile.Show()
        self.profile.Refresh()
        self.profile.Update()
        
    def DispMapCalculator(self, event):
        """
        Init map calculator for interactive creation of mapcalc statements
        """
        
        self.mapcalculator = mapcalculator.MapCalcFrame(self, wx.ID_ANY, title='',
                                                        dimension=2)

    def Disp3DMapCalculator(self, event):
        """
        Init map calculator for interactive creation of mapcalc statements
        """
        
        self.mapcalculator = mapcalculator.MapCalcFrame(self, wx.ID_ANY, title='',
                                                        dimension=3)

    def AddToolbarButton(self, toolbar, label, icon, help, handler):
        """Adds button to the given toolbar"""

        if not label:
            toolbar.AddSeparator()
            return
        tool = toolbar.AddLabelTool(id=wx.ID_ANY, label=label, bitmap=icon, shortHelp=help)
        self.Bind(wx.EVT_TOOL, handler, tool)

    def ToolbarData(self):

        return   (
                 ('newdisplay', Icons["newdisplay"].GetBitmap(),
                  Icons["newdisplay"].GetLabel(), self.OnNewDisplay),
                 ('', '', '', ''),
                 ('workspaceLoad', Icons["workspaceLoad"].GetBitmap(),
                  Icons["workspaceLoad"].GetLabel(), self.OnWorkspace),
                 ('workspaceOpen', Icons["workspaceOpen"].GetBitmap(),
                  Icons["workspaceOpen"].GetLabel(), self.OnWorkspaceOpen),
                 ('workspaceSave', Icons["workspaceSave"].GetBitmap(),
                  Icons["workspaceSave"].GetLabel(), self.OnWorkspaceSave),
                 ('', '', '', ''),
                 ('addrast', Icons["addrast"].GetBitmap(),
                  Icons["addrast"].GetLabel(), self.OnRaster),
                 ('addvect', Icons["addvect"].GetBitmap(),
                  Icons["addvect"].GetLabel(), self.OnVector),
                 ('addcmd',  Icons["addcmd"].GetBitmap(),
                  Icons["addcmd"].GetLabel(),  self.AddCommand),
                 ('addgrp',  Icons["addgrp"].GetBitmap(),
                  Icons["addgrp"].GetLabel(), self.AddGroup),
                 ('addovl',  Icons["addovl"].GetBitmap(),
                  Icons["addovl"].GetLabel(), self.OnOverlay),
                 ('addlabels',  Icons["addlabels"].GetBitmap(),
                  Icons["addlabels"].GetLabel(), self.AddLabels),
                 ('delcmd',  Icons["delcmd"].GetBitmap(),
                  Icons["delcmd"].GetLabel(), self.DeleteLayer),
                 ('', '', '', ''),
                 ('attrtable', Icons["attrtable"].GetBitmap(),
                  Icons["attrtable"].GetLabel(), self.ShowAttributeTable)
                  )

    def ShowAttributeTable(self, event):
        """
        Show attribute table of the given vector map layer
        """
        layer = self.curr_page.maptree.layer_selected
        # no map layer selected
        if not layer:
            self.MsgNoLayerSelected()
            return

        # available only for vector map layers
        try:
            maptype = self.curr_page.maptree.GetPyData(layer)[0]['maplayer'].type
        except:
            maptype = None

        if not maptype or maptype != 'vector':
            wx.MessageBox(parent=self,
                          message=_("Attribute management is available only "
                                    "for vector maps."),
                          caption=_("Message"),
                          style=wx.OK | wx.ICON_INFORMATION | wx.CENTRE)
            return

        if not self.curr_page.maptree.GetPyData(layer)[0]:
            return
        dcmd = self.curr_page.maptree.GetPyData(layer)[0]['cmd']
        if not dcmd:
            return

        size = icon = None
        mapname = utils.GetLayerNameFromCmd(dcmd)

        for option in dcmd:
            if option.find('size') > -1:
                size = option.split('=')[1]
            elif option.find('icon') > -1:
                icon = option.split('=')[1]

        pointdata = (icon, size)

        self.dbmanager = dbm.AttributeManager(parent=self, id=wx.ID_ANY,
                                              title="%s - <%s>" % (_("GRASS GIS Attribute Table Manager"),
                                                                   mapname),
                                              size=wx.Size(500,300), vectmap=mapname,
                                              pointdata=pointdata)
        self.dbmanager.Show()

    def OnNewDisplay(self, event=None):
        """Create new layer tree and map display instance"""
        self.NewDisplay()

    def NewDisplay(self, show=True):
        """Create new layer tree, which will
        create an associated map display frame
        """
        Debug.msg(1, "GMFrame.NewDisplay(): idx=%d" % self.disp_idx)

        # make a new page in the bookcontrol for the layer tree (on page 0 of the notebook)
        self.pg_panel = wx.Panel(self.gm_cb, id=wx.ID_ANY, style= wx.EXPAND)
        self.gm_cb.AddPage(self.pg_panel, text="Display "+ str(self.disp_idx), select = True)
        self.curr_page = self.gm_cb.GetCurrentPage()

        # create layer tree (tree control for managing GIS layers)  and put on new notebook page
        self.curr_page.maptree = wxgui_utils.LayerTree(self.curr_page, id=wx.ID_ANY, pos=wx.DefaultPosition,
                                                       size=wx.DefaultSize, style=wx.TR_HAS_BUTTONS
                                                       |wx.TR_LINES_AT_ROOT|wx.TR_EDIT_LABELS|wx.TR_HIDE_ROOT
                                                       |wx.TR_DEFAULT_STYLE|wx.NO_BORDER|wx.FULL_REPAINT_ON_RESIZE,
                                                       idx=self.disp_idx, gismgr=self, notebook=self.gm_cb,
                                                       auimgr=self._auimgr, showMapDisplay=show)

        # layout for controls
        cb_boxsizer = wx.BoxSizer(wx.VERTICAL)
        cb_boxsizer.Add(self.curr_page.maptree, proportion=1, flag=wx.EXPAND, border=1)
        self.curr_page.SetSizer(cb_boxsizer)
        cb_boxsizer.Fit(self.curr_page.maptree)
        self.curr_page.Layout()
        self.curr_page.maptree.Layout()

        self.disp_idx += 1

        #        self._auimgr.SetManagedWindow(self.curr_page.maptree.testframe)
        #
        #        self._auimgr.AddPane(self.curr_page.maptree.testframe,
        #                             wx.aui.AuiPaneInfo().Right().
        #                             BestSize((-1,-1)).
        #                             CloseButton(True).MinimizeButton(True).
        #                             DestroyOnClose(True).Layer(2))
        #
        #        self._auimgr.Update()

    # toolBar button handlers
    def OnRaster(self, event):
        """Add raster menu"""
        point = wx.GetMousePosition()
        rastmenu = wx.Menu()
        # Add items to the menu
        addrast = wx.MenuItem(rastmenu, -1, Icons["addrast"].GetLabel())
        addrast.SetBitmap(Icons["addrast"].GetBitmap(self.iconsize))
        rastmenu.AppendItem(addrast)
        self.Bind(wx.EVT_MENU, self.AddRaster, addrast)

        addshaded = wx.MenuItem(rastmenu, -1, Icons ["addshaded"].GetLabel())
        addshaded.SetBitmap(Icons["addshaded"].GetBitmap (self.iconsize))
        rastmenu.AppendItem(addshaded)
        self.Bind(wx.EVT_MENU, self.AddShaded, addshaded)

        addrgb = wx.MenuItem(rastmenu, -1, Icons["addrgb"].GetLabel())
        addrgb.SetBitmap(Icons["addrgb"].GetBitmap(self.iconsize))
        rastmenu.AppendItem(addrgb)
        self.Bind(wx.EVT_MENU, self.AddRGB, addrgb)

        addhis = wx.MenuItem(rastmenu, -1, Icons ["addhis"].GetLabel())
        addhis.SetBitmap(Icons["addhis"].GetBitmap (self.iconsize))
        rastmenu.AppendItem(addhis)
        self.Bind(wx.EVT_MENU, self.AddHIS, addhis)

        addrastarrow = wx.MenuItem(rastmenu, -1, Icons ["addrarrow"].GetLabel())
        addrastarrow.SetBitmap(Icons["addrarrow"].GetBitmap (self.iconsize))
        rastmenu.AppendItem(addrastarrow)
        self.Bind(wx.EVT_MENU, self.AddRastarrow, addrastarrow)

        #        addrastnums = wx.MenuItem(rastmenu, -1, Icons ["addrnum"].GetLabel())
        #        addrastnums.SetBitmap(Icons["addrnum"].GetBitmap (self.iconsize))
        #        rastmenu.AppendItem(addrastnums)
        #        self.Bind(wx.EVT_MENU, self.AddRastnum, addrastnums)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(rastmenu)
        rastmenu.Destroy()

    def OnVector(self, event):
        """Add vector menu"""
        point = wx.GetMousePosition()
        vectmenu = wx.Menu()

        addvect = wx.MenuItem(vectmenu, -1, Icons["addvect"].GetLabel())
        addvect.SetBitmap(Icons["addvect"].GetBitmap(self.iconsize))
        vectmenu.AppendItem(addvect)
        self.Bind(wx.EVT_MENU, self.addVector, addvect)

        addtheme = wx.MenuItem(vectmenu, -1, Icons["addthematic"].GetLabel())
        addtheme.SetBitmap(Icons["addthematic"].GetBitmap(self.iconsize))
        vectmenu.AppendItem(addtheme)
        self.Bind(wx.EVT_MENU, self.addThemeMap, addtheme)

        addchart = wx.MenuItem(vectmenu, -1, Icons["addchart"].GetLabel())
        addchart.SetBitmap(Icons["addchart"].GetBitmap(self.iconsize))
        vectmenu.AppendItem(addchart)
        self.Bind(wx.EVT_MENU, self.addThemeChart, addchart)
        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(vectmenu)
        vectmenu.Destroy()

    def OnOverlay(self, event):
        """Add overlay menu"""
        point = wx.GetMousePosition()
        ovlmenu = wx.Menu()

        addgrid = wx.MenuItem(ovlmenu, -1, Icons["addgrid"].GetLabel())
        addgrid.SetBitmap(Icons["addgrid"].GetBitmap(self.iconsize))
        ovlmenu.AppendItem(addgrid)
        self.Bind(wx.EVT_MENU, self.AddGrid, addgrid)

        addgeodesic = wx.MenuItem(ovlmenu, -1, Icons["addgeodesic"].GetLabel())
        addgeodesic.SetBitmap(Icons["addgeodesic"].GetBitmap(self.iconsize))
        ovlmenu.AppendItem(addgeodesic)
        self.Bind(wx.EVT_MENU, self.AddGeodesic, addgeodesic)

        addrhumb = wx.MenuItem(ovlmenu, -1, Icons["addrhumb"].GetLabel())
        addrhumb.SetBitmap(Icons["addrhumb"].GetBitmap(self.iconsize))
        ovlmenu.AppendItem(addrhumb)
        self.Bind(wx.EVT_MENU, self.AddRhumb, addrhumb)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.PopupMenu(ovlmenu)
        ovlmenu.Destroy()

    def AddRaster(self, event):
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('raster')

    def AddRGB(self, event):
        """Add RGB layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('rgb')

    def AddHIS(self, event):
        """Add HIS layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('his')

    def AddShaded(self, event):
        """Add shaded relief map layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('shaded')

    def AddRastarrow(self, event):
        """Add raster flow arrows map"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('rastarrow')

    def AddRastnum(self, event):
        """Add raster map with cell numbers"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('rastnum')

    def addVector(self, event):
        """Add vector layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('vector')

    def addThemeMap(self, event):
        """Add thematic map layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('thememap')

    def addThemeChart(self, event):
        """Add thematic chart layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('themechart')

    def AddCommand(self, event):
        """Add command line layer"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('command')

    def AddGroup(self, event):
        """Add layer group"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('group')

    def AddGrid(self, event):
        """Add layer grid"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('grid')

    def AddGeodesic(self, event):
        """Add layer geodesic"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('geodesic')

    def AddRhumb(self, event):
        """Add layer rhumb"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('rhumb')

    def AddLabels(self, event):
        """Add layer vector labels"""
        self.notebook.SetSelection(0)
        self.curr_page.maptree.AddLayer('labels')

    def GetSelectedDisplay(self):
        return self.notebook.GetSelection()

    def DeleteLayer(self, event):
        """
        Delete selected map display layer in GIS Manager tree widget
        """
        if not self.curr_page.maptree.layer_selected:
            self.MsgNoLayerSelected()
            return

        layerName = str(self.curr_page.maptree.GetItemText(self.curr_page.maptree.layer_selected))
        if layerName:
            message = _("Do you want to remove map layer <%s> "
                        "from layer tree?") % layerName
        else:
            message = _("Do you want to remove selected map layer "
                        "from layer tree?")

        dlg = wx.MessageDialog (parent=self, message=message,
                                caption=_("Remove map layer"),
                                style=wx.YES_NO | wx.YES_DEFAULT | wx.CANCEL | wx.ICON_QUESTION)

        if dlg.ShowModal() in [wx.ID_NO, wx.ID_CANCEL]:
            dlg.Destroy()
            return

        dlg.Destroy()

        for layer in self.curr_page.maptree.GetSelections():
            if self.curr_page.maptree.GetPyData(layer)[0]['type'] == 'group':
                self.curr_page.maptree.DeleteChildren(layer)
            self.curr_page.maptree.Delete(layer)

    #Misc methods
    def OnCloseWindow(self, event):
        """Cleanup when wxgui.py is quit"""
        try:
            for page in range(self.gm_cb.GetPageCount()):
                self.gm_cb.GetPage(page).maptree.Map.Clean()
            self.DeleteAllPages()
        except:
            pass
        # self.DestroyChildren()
        self._auimgr.UnInit()
        self.Destroy()

    def MsgNoLayerSelected(self):
        """Show dialog message 'No layer selected'"""
        wx.MessageBox(parent=self,
                      message=_("No map layer selected. Operation cancelled."),
                      caption=_("Message"),
                      style=wx.OK | wx.ICON_INFORMATION | wx.CENTRE)

class GMApp(wx.App):
    """
    GMApp class
    """
    def __init__(self, workspace=None):
        self.workspaceFile = workspace
        
        # call parent class initializer
        wx.App.__init__(self, False)
        
    def OnInit(self):
        # initialize all available image handlers
        wx.InitAllImageHandlers()

        # create splash screen
        introImagePath = os.path.join(globalvar.ETCWXDIR, "images", "intro.png")
        introImage     = wx.Image(introImagePath, wx.BITMAP_TYPE_PNG)
        introBmp       = introImage.ConvertToBitmap()
        wx.SplashScreen (bitmap=introBmp, splashStyle=wx.SPLASH_CENTRE_ON_SCREEN | wx.SPLASH_TIMEOUT,
                         milliseconds=1500, parent=None, id=wx.ID_ANY)
        wx.Yield()

        # create and show main frame
        mainframe = GMFrame(parent=None, id=wx.ID_ANY,
                            workspace = self.workspaceFile)

        mainframe.Show()
        self.SetTopWindow(mainframe)

        return True

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def printHelp():
    """Print program help"""
    print >> sys.stderr, "Usage:"
    print >> sys.stderr, " python wxgui.py [options]"
    print >> sys.stderr, "%sOptions:" % os.linesep
    print >> sys.stderr, " -w\t--workspace file\tWorkspace file to load"
    sys.exit(0)

def process_opt(opts, args):
    """Process command-line arguments"""
    workspaceFile = None
    for o, a in opts:
        if o in ("-h", "--help"):
            printHelp()
            
        if o in ("-w", "--workspace"):
            if a != '':
                workspaceFile = str(a)
            else:
                workspaceFile = args.pop(0)

    return (workspaceFile,)

def main(argv=None):
    #
    # reexec for MacOS
    #
    utils.reexec_with_pythonw()

    #
    # process command-line arguments
    #
    if argv is None:
        argv = sys.argv
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "hw:",
                                       ["help", "workspace"])
        except getopt.error, msg:
            raise Usage(msg)

    except Usage, err:
        print >> sys.stderr, err.msg
        print >> sys.stderr, "for help use --help"
        printHelp()

    workspaceFile = process_opt(opts, args)[0]

    #
    # run application
    #
    app = GMApp(workspaceFile)
    # suppress wxPython logs
    q = wx.LogNull()

    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())
