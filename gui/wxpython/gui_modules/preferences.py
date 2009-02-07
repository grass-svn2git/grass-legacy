"""
@package preferences

@brief User preferences dialog

Sets default display font, etc.

Classes:
 - PreferencesDialog
 - DefaultFontDialog
 - MapsetAccess

(C) 2007-2009 by the GRASS Development Team
This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Michael Barton (Arizona State University)
Martin Landa <landa.martin gmail.com>
"""

import os
import sys
import copy
import stat
if os.name in ('posix', 'mac'):
    import pwd

import wx
import wx.lib.filebrowsebutton as filebrowse
import wx.lib.colourselect as csel
import wx.lib.mixins.listctrl as listmix
from wx.lib.wordwrap import wordwrap

import gcmd
import grassenv
import utils
import globalvar
from debug import Debug as Debug

class Settings:
    """Generic class where to store settings"""
    def __init__(self):
        #
        # settings filename
        #
        self.fileName = ".grasswx6"
        self.filePath = None

        #
        # key/value separator
        #
        self.sep = ';'

        #
        # default settings
        #
        self.defaultSettings = {
            #
            # general
            #
            'general': {
                # use default window layout (layer manager, displays, ...)
                'defWindowPos' : {
                    'enabled' : False,
                    'dim' : ''
                    },
                # expand/collapse element list
                'elementListExpand' : {
                    'selection' : 0 
                    },
                },
            'manager' : {
                # show opacity level widget
                'changeOpacityLevel' : {
                    'enabled' : False
                    }, 
                # ask when removing layer from layer tree
                'askOnRemoveLayer' : {
                    'enabled' : True
                    },
                # ask when quiting wxGUI or closing display
                'askOnQuit' : {
                    'enabled' : True
                    },
                },
            #
            # display
            #
            'display': {
                'font' : {
                    'type' : '',
                    'encoding': 'ISO-8859-1',
                    },
                'driver': {
                    'type': 'default'
                    },
                'compResolution' : {
                    'enabled' : False
                    },
                'autoRendering': {
                    'enabled' : False
                    },
                'statusbarMode': {
                    'selection' : 0
                    },
                'bgcolor': {
                    'color' : (255, 255, 255, 255),
                    }
                },
            #
            # advanced
            #
            'advanced' : {
                'settingsFile' : {
                    'type' : 'home'
                    }, # home, gisdbase, location, mapset
                'iconTheme' : {
                    'type' : 'grass2'
                    }, # grass2, grass, silk
                },
            #
            # Attribute Table Manager
            #
            'atm' : {
                'highlight' : {
                    'color' : (255, 255, 0, 255),
                    'width' : 2
                    },
                'leftDbClick' : {
                    'selection' : 1 # draw selected
                    },
                'askOnDeleteRec' : {
                    'enabled' : True
                    },
                'keycolumn' : {
                    'value' : 'cat'
                    },
                'encoding' : {
                    'value' : '',
                    }
                },
            #
            # Command
            #
            'cmd': {
                'overwrite' : {
                    'enabled' : False
                    },
                'closeDlg' : {
                    'enabled' : False
                    },
                'verbosity' : {
                    'selection' : 'grassenv'
                    },
                # d.rast
                'rasterOverlay' : {
                    'enabled' : True
                    },
                # d.vect
                'showType': {
                    'point' : {
                        'enabled' : True
                        },
                    'line' : {
                        'enabled' : True
                        },
                    'centroid' : {
                        'enabled' : True
                        },
                    'boundary' : {
                        'enabled' : True
                        },
                    'area' : {
                        'enabled' : True
                        },
                    'face' : {
                        'enabled' : True
                        },
                    },
                'addNewLayer' : {
                    'enabled' : False
                    },
                },
            #
            # Workspace
            #
            'workspace' : {
                'posDisplay' : {
                    'enabled' : False
                    },
                'posManager' : {
                    'enabled' : False
                    },
                },
            #
            # vdigit
            #
            'vdigit' : {
                # symbology
                'symbol' : {
                    'highlight' : {
                        'enabled' : None,
                        'color' : (255, 255, 0, 255)
                        }, # yellow
                    'highlightDupl' : {
                        'enabled' : None,
                        'color' : (255, 72, 0, 255)
                        }, # red
                    'point' : {
                        'enabled' : True,
                        'color' : (0, 0, 0, 255)
                        }, # black
                    'line' : {
                        'enabled' : True,
                        'color' : (0, 0, 0, 255)
                        }, # black
                    'boundaryNo' : {
                        'enabled' : True,
                        'color' : (126, 126, 126, 255)
                        }, # grey
                    'boundaryOne' : {
                        'enabled' : True,
                        'color' : (0, 255, 0, 255)
                        }, # green
                    'boundaryTwo' : {
                        'enabled' : True,
                        'color' : (255, 135, 0, 255)
                        }, # orange
                    'centroidIn' : {
                        'enabled' : True,
                        'color' : (0, 0, 255, 255)
                        }, # blue
                    'centroidOut' : {
                        'enabled' : True,
                        'color' : (165, 42, 42, 255)
                        }, # brown
                    'centroidDup' : {
                        'enabled' : True,
                        'color' : (156, 62, 206, 255)
                        }, # violet
                    'nodeOne' : {
                        'enabled' : True,
                        'color' : (255, 0, 0, 255)
                        }, # red
                    'nodeTwo' : {
                        'enabled' : True,
                        'color' : (0, 86, 45, 255)
                        }, # dark green
                    'vertex' : {
                        'enabled' : False,
                        'color' : (255, 20, 147, 255)
                        }, # deep pink
                    'area' : {
                        'enabled' : False,
                        'color' : (217, 255, 217, 255)
                        }, # green
                    'direction' : {
                        'enabled' : False,
                        'color' : (255, 0, 0, 255)
                        }, # red
                    },
                # display
                'lineWidth' : {
                    'value' : 2,
                    'units' : 'screen pixels'
                    },
                # snapping
                'snapping' : {
                    'value' : 10,
                    'units' : 'screen pixels'
                    },
                'snapToVertex' : {
                    'enabled' : False
                    },
                # digitize new record
                'addRecord' : {
                    'enabled' : True
                    },
                'layer' :{
                    'value' : 1
                    },
                'category' : {
                    'value' : 1
                    },
                'categoryMode' : {
                    'selection' : 0
                    },
                # delete existing feature(s)
                'delRecord' : {
                    'enabled' : True
                    },
                # query tool
                'query' : {
                    'selection' : 0,
                    'box' : True
                    },
                'queryLength' : {
                    'than-selection' : 0,
                    'thresh' : 0
                    },
                'queryDangle' : {
                    'than-selection' : 0,
                    'thresh' : 0
                    },
                # select feature (point, line, centroid, boundary)
                'selectType': {
                    'point' : {
                        'enabled' : True
                        },
                    'line' : {
                        'enabled' : True
                        },
                    'centroid' : {
                        'enabled' : True
                        },
                    'boundary' : {
                        'enabled' : True
                        },
                    },
                'selectThresh' : {
                    'value' : 10,
                    'units' : 'screen pixels'
                    },
                'checkForDupl' : {
                    'enabled' : False
                    },
                'selectInside' : {
                    'enabled' : False
                    },
                # exit
                'saveOnExit' : {
                    'enabled' : False,
                    },
                # break lines on intersection
                'breakLines' : {
                    'enabled' : False,
                    },
                },
            'profile': {
                'raster0' : {
                    'pcolor' : (0, 0, 255, 255), # profile line color
                    'pwidth' : 1, # profile line width
                    'pstyle' : 'solid', # profile line pen style
                    },
                'raster1' : {
                    'pcolor' : (255, 0, 0, 255), 
                    'pwidth' : 1, 
                    'pstyle' : 'solid', 
                    },
                'raster2' : {
                    'pcolor' : (0, 255, 0, 255), 
                    'pwidth' : 1, 
                    'pstyle' : 'solid', 
                    },
                'font' : {
                    'titleSize' : 12,
                    'axisSize' : 11,
                    'legendSize' : 10,
                    },
                'marker' : {
                    'color' : (0, 0, 0, 255),
                    'fill' : 'transparent',
                    'size' : 2,
                    'type' : 'triangle',
                    'legend' : _('Segment break'),
                    },
                'grid' : {
                    'color' : (200, 200, 200, 255),
                    'enabled' : True,
                    },
                'x-axis' : {
                    'type' : 'auto', # axis format
                    'min' : 0, # axis min for custom axis range
                    'max': 0, # axis max for custom axis range
                    'log' : False,
                    },
                'y-axis' : {
                    'type' : 'auto', # axis format
                    'min' : 0, # axis min for custom axis range
                    'max': 0, # axis max for custom axis range
                    'log' : False,
                    },
                'legend' : {
                    'enabled' : True
                    },
                },
            'georect' : {
                'symbol' : {
                    'color' : (0, 0, 255, 255),
                    'width' : 2,
                    },
                },
            'nviz' : {
                'view' : {
                    'persp' : {
                        'value' : 40,
                        'step' : 5,
                        },
                    'pos' : {
                        'x' : 0.85,
                        'y' : 0.85,
                        },
                    'height' : {
                        'step' : 100,
                        },
                    'twist' : {
                        'value' : 0,
                        'step' : 5,
                        },
                    'z-exag' : {
                        'value': 1,
                        'step' : 1,
                        },
                    },
                'surface' : {
                    'shine': {
                        'map' : False,
                        'value' : 60.0,
                        },
                    'color' : {
                        'map' : True,
                        'value' : (0, 0, 0, 255), # constant: black
                        },
                    'draw' : {
                        'wire-color' : (136, 136, 136, 255),
                        'mode' : 1, # fine
                        'style' : 1, # surface
                        'shading' : 1, # gouraud
                        'res-fine' : 6,
                        'res-coarse' : 9,
                        },
                    'position' : {
                        'x' : 0,
                        'y' : 0,
                        'z' : 0,
                        },
                    },
                'vector' : {
                    'lines' : {
                        'show' : False,
                        'width' : 2,
                        'color' : (0, 0, 255, 255), # blue
                        'flat' : False,
                        'height' : 0,
                        },
                    'points' : {
                        'show' : False,
                        'size' : 100,
                        'width' : 2,
                        'marker' : 2,
                        'color' : (0, 0, 255, 255), # blue
                        'height' : 0,
                        }
                    },
                'volume' : {
                    'color' : {
                        'map' : True,
                        'value' : (0, 0, 0, 255), # constant: black
                        },
                    'draw' : {
                        'mode' : 0, # isosurfaces
                        'shading' : 1, # gouraud
                        'resolution' : 3, # polygon resolution
                        },
                    'shine': {
                        'map' : False,
                        'value' : 60.0,
                        },
                    },
                'settings': {
                    'general' : {
                        'bgcolor' : (255, 255, 255, 255), # white
                        },
                    },
                },
            }
        
        #
        # user settings
        #
        self.userSettings = copy.deepcopy(self.defaultSettings)
        try:
            self.ReadSettingsFile()
        except gcmd.SettingsError, e:
            print >> sys.stderr, e.message

        #
        # internal settings (based on user settings)
        #
        self.internalSettings = {}
        for group in self.userSettings.keys():
            self.internalSettings[group] = {}
            for key in self.userSettings[group].keys():
                self.internalSettings[group][key] = {}

        # self.internalSettings['general']["mapsetPath"]['value'] = self.GetMapsetPath()
        self.internalSettings['general']['elementListExpand']['choices'] = (_("Collapse all except PERMANENT and current"),
                                                                            _("Collapse all except PERMANENT"),
                                                                            _("Collapse all"),
                                                                            _("Expand all"))
        self.internalSettings['atm']['leftDbClick']['choices'] = (_('Edit selected record'),
                                                                  _('Display selected'))
        self.internalSettings['advanced']['settingsFile']['choices'] = ('home',
                                                                        'gisdbase',
                                                                        'location',
                                                                        'mapset')
        self.internalSettings['advanced']['iconTheme']['choices'] = ('grass',
                                                                     'grass2',
                                                                     'silk')
        self.internalSettings['cmd']['verbosity']['choices'] = ('grassenv',
                                                                'verbose',
                                                                'quiet')
        self.internalSettings['display']['driver']['choices'] = ['default']
        self.internalSettings['display']['statusbarMode']['choices'] = globalvar.MAP_DISPLAY_STATUSBAR_MODE

        self.internalSettings['nviz']['view'] = {}
        self.internalSettings['nviz']['view']['twist'] = {}
        self.internalSettings['nviz']['view']['twist']['min'] = -180
        self.internalSettings['nviz']['view']['twist']['max'] = 180
        self.internalSettings['nviz']['view']['persp'] = {}
        self.internalSettings['nviz']['view']['persp']['min'] = 1
        self.internalSettings['nviz']['view']['persp']['max'] = 100
        self.internalSettings['nviz']['view']['height'] = {}
        self.internalSettings['nviz']['view']['height']['value'] = -1
        self.internalSettings['nviz']['vector'] = {}
        self.internalSettings['nviz']['vector']['points'] = {}
        self.internalSettings['nviz']['vector']['points']['marker'] = ("x",
                                                                       _("box"),
                                                                       _("sphere"),
                                                                       _("cube"),
                                                                       _("diamond"),
                                                                       _("dtree"),
                                                                       _("ctree"),
                                                                       _("aster"),
                                                                       _("gyro"),
                                                                       _("histogram"))
        self.internalSettings['vdigit']['bgmap'] = {}
        self.internalSettings['vdigit']['bgmap']['value'] = ''
        
    def ReadSettingsFile(self, settings=None):
        """Reads settings file (mapset, location, gisdbase)"""
        if settings is None:
            settings = self.userSettings

        # look for settings file
        # -> mapser
        #  -> location
        #   -> gisdbase
        gisdbase = grassenv.GetGRASSVariable("GISDBASE")
        location_name = grassenv.GetGRASSVariable("LOCATION_NAME")
        mapset_name = grassenv.GetGRASSVariable("MAPSET")

        mapset_file = os.path.join(gisdbase, location_name, mapset_name, self.fileName)
        location_file = os.path.join(gisdbase, location_name, self.fileName)
        gisdbase_file = os.path.join(gisdbase, self.fileName)
        home_file = os.path.join(os.path.expanduser("~"), self.fileName) # MS Windows fix ?
        
        if os.path.isfile(mapset_file):
            self.filePath = mapset_file
        elif os.path.isfile(location_file):
            self.filePath = location_file
        elif os.path.isfile(gisdbase_file):
            self.filePath = gisdbase_file
        elif os.path.isfile(home_file):
            self.filePath = home_file
        
        if self.filePath:
            self.__ReadFile(self.filePath, settings)
        
        # set environment variables
        os.environ["GRASS_FONT"] = self.Get(group='display',
                                            key='font', subkey='type')
        os.environ["GRASS_ENCODING"] = self.Get(group='display',
                                                key='font', subkey='encoding')
        
    def __ReadFile(self, filename, settings=None):
        """Read settings from file to dict"""
        if settings is None:
            settings = self.userSettings

        try:
            file = open(filename, "r")
            line = ''
            for line in file.readlines():
                line = line.rstrip('%s' % os.linesep)
                group, key = line.split(self.sep)[0:2]
                kv = line.split(self.sep)[2:]
                subkeyMaster = None
                if len(kv) % 2 != 0: # multiple (e.g. nviz)
                    subkeyMaster = kv[0]
                    del kv[0]
                idx = 0
                while idx < len(kv):
                    if subkeyMaster:
                        subkey = [subkeyMaster, kv[idx]]
                    else:
                        subkey = kv[idx]
                    value = kv[idx+1]
                    value = self.__parseValue(value, read=True)
                    self.Append(settings, group, key, subkey, value)
                    idx += 2
        except ValueError, e:
            print >> sys.stderr, _("Error: Reading settings from file <%(file)s> failed.\n"
                                   "       Details: %(detail)s\n"
                                   "       Line: '%(line)s'") % { 'file' : filename,
                                                                  'detail' : e,
                                                                  'line' : line }
            file.close()

        file.close()

    def SaveToFile(self, settings=None):
        """Save settings to the file"""
        if settings is None:
            settings = self.userSettings
        
        loc = self.Get(group='advanced', key='settingsFile', subkey='type')
        home = os.path.expanduser("~") # MS Windows fix ?
        gisdbase = grassenv.GetGRASSVariable("GISDBASE")
        location_name = grassenv.GetGRASSVariable("LOCATION_NAME")
        mapset_name = grassenv.GetGRASSVariable("MAPSET")
        filePath = None
        if loc == 'home':
            filePath = os.path.join(home, self.fileName)
        elif loc == 'gisdbase':
            filePath = os.path.join(gisdbase, self.fileName)
        elif loc == 'location':
            filePath = os.path.join(gisdbase, location_name, self.fileName)
        elif loc == 'mapset':
            filePath = os.path.join(gisdbase, location_name, mapset_name, self.fileName)
        
        if filePath is None:
            raise gcmd.SettingsError(_('Uknown settings file location.'))

        try:
            file = open(filePath, "w")
            for group in settings.keys():
                for key in settings[group].keys():
                    file.write('%s%s%s%s' % (group, self.sep, key, self.sep))
                    subkeys = settings[group][key].keys()
                    for idx in range(len(subkeys)):
                        value = settings[group][key][subkeys[idx]]
                        if type(value) == type({}):
                            if idx > 0:
                                file.write('%s%s%s%s%s' % (os.linesep, group, self.sep, key, self.sep))
                            file.write('%s%s' % (subkeys[idx], self.sep))
                            kvalues = settings[group][key][subkeys[idx]].keys()
                            srange = range(len(kvalues))
                            for sidx in srange:
                                svalue = self.__parseValue(settings[group][key][subkeys[idx]][kvalues[sidx]])
                                file.write('%s%s%s' % (kvalues[sidx], self.sep,
                                                       svalue))
                                if sidx < len(kvalues) - 1:
                                    file.write('%s' % self.sep)
                        else:
                            value = self.__parseValue(settings[group][key][subkeys[idx]])
                            file.write('%s%s%s' % (subkeys[idx], self.sep, value))
                            if idx < len(subkeys) - 1:
                                file.write('%s' % self.sep)
                    file.write('%s' % os.linesep)
        except IOError, e:
            raise gcmd.SettingsError(message=e)
        except StandardError, e:
            raise gcmd.SettingsError(message=_('Writing settings to file <%(file)s> failed.'
                                               '\n\nDetails: %(detail)s') % { 'file' : filePath,
                                                                              'detail' : e })
        
        file.close()
        
        return filePath

    def __parseValue(self, value, read=False):
        """Parse value to be store in settings file"""
        if read: # -> read settings (cast values)
            if value == 'True':
                value = True
            elif value == 'False':
                value = False
            elif value == 'None':
                value = None
            elif ':' in value: # -> color
                value = tuple(map(int, value.split(':')))
            else:
                try:
                    value = int(value)
                except ValueError:
                    try:
                        value = float(value)
                    except ValueError:
                        pass
        else: # -> write settings
            if type(value) == type(()): # -> color
                value = str(value[0]) + ':' +\
                    str(value[1]) + ':' + \
                    str(value[2])
                
        return value

    def Get(self, group, key=None, subkey=None, internal=False):
        """Get value by key/subkey

        Raise KeyError if key is not found
        
        @param group settings group
        @param key (value, None)
        @param subkey (value, list or None)
        @param internal use internal settings instead

        @return value
        """
        if internal is True:
            settings = self.internalSettings
        else:
            settings = self.userSettings
            
        try:
            if subkey is None:
                if key is None:
                    return settings[group]
                else:
                    return settings[group][key]
            else:
                if type(subkey) == type([]) or \
                        type(subkey) == type(()):
                    return settings[group][key][subkey[0]][subkey[1]]
                else:
                    return settings[group][key][subkey]  

        except KeyError:
            #raise gcmd.SettingsError("%s %s:%s:%s." % (_("Unable to get value"),
            #                                           group, key, subkey))
            print >> sys.stderr, "Settings: unable to get value '%s:%s:%s'\n" % \
                (group, key, subkey)
        
    def Set(self, group, value, key=None, subkey=None, internal=False):
        """Set value of key/subkey

        Raise KeyError if group/key is not found
        
        @param group settings group
        @param key key (value, None)
        @param subkey subkey (value, list or None)
        @param value value
        @param internal use internal settings instead
        """
        if internal is True:
            settings = self.internalSettings
        else:
            settings = self.userSettings

        try:
            if subkey is None:
                if key is None:
                    settings[group] = value
                else:
                    settings[group][key] = value
            else:
                if type(subkey) == type([]):
                    settings[group][key][subkey[0]][subkey[1]] = value
                else:
                    settings[group][key][subkey] = value
        except KeyError:
            raise gcmd.SettingsError("%s '%s:%s:%s'" % (_("Unable to set "), group, key, subkey))

    def Append(self, dict, group, key, subkey, value):
        """Set value of key/subkey

        Create group/key/subkey if not exists
        
        @param dict settings dictionary to use
        @param group settings group
        @param key key
        @param subkey subkey (value or list)
        @param value value
        """
        if not dict.has_key(group):
            dict[group] = {}

        if not dict[group].has_key(key):
            dict[group][key] = {}

        if type(subkey) == type([]):
            # TODO: len(subkey) > 2
            if not dict[group][key].has_key(subkey[0]):
                dict[group][key][subkey[0]] = {}
            dict[group][key][subkey[0]][subkey[1]] = value
        else:
            dict[group][key][subkey] = value

    def GetDefaultSettings(self):
        """Get default user settings"""
        return self.defaultSettings

globalSettings = Settings()

class PreferencesDialog(wx.Dialog):
    """User preferences dialog"""
    def __init__(self, parent, title=_("User GUI settings"),
                 settings=globalSettings,
                 style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER):
        self.parent = parent # GMFrame
        self.title = title
        wx.Dialog.__init__(self, parent=parent, id=wx.ID_ANY, title=title,
                           style=style, size=(-1, -1))

        self.settings = settings
        # notebook
        notebook = wx.Notebook(parent=self, id=wx.ID_ANY, style=wx.BK_DEFAULT)

        # dict for window ids
        self.winId = {}

        # create notebook pages
        self.__CreateGeneralPage(notebook)
        self.__CreateDisplayPage(notebook)
        self.__CreateCmdPage(notebook)
        self.__CreateAttributeManagerPage(notebook)
        self.__CreateWorkspacePage(notebook)
        self.__CreateAdvancedPage(notebook)

        # buttons
        btnDefault = wx.Button(self, wx.ID_ANY, _("Set to default"))
        btnSave = wx.Button(self, wx.ID_SAVE)
        btnApply = wx.Button(self, wx.ID_APPLY)
        btnCancel = wx.Button(self, wx.ID_CANCEL)
        btnSave.SetDefault()

        # bindigs
        btnDefault.Bind(wx.EVT_BUTTON, self.OnDefault)
        btnDefault.SetToolTipString(_("Revert settings to default and apply changes"))
        btnApply.Bind(wx.EVT_BUTTON, self.OnApply)
        btnApply.SetToolTipString(_("Apply changes for the current session"))
        btnSave.Bind(wx.EVT_BUTTON, self.OnSave)
        btnSave.SetToolTipString(_("Apply and save changes to user settings file (default for next sessions)"))
        btnSave.SetDefault()
        btnCancel.Bind(wx.EVT_BUTTON, self.OnCancel)
        btnCancel.SetToolTipString(_("Close dialog and ignore changes"))

        # sizers
        btnSizer = wx.BoxSizer(wx.HORIZONTAL)
        btnSizer.Add(item=btnDefault, proportion=1,
                     flag=wx.ALL, border=5)
        btnStdSizer = wx.StdDialogButtonSizer()
        btnStdSizer.AddButton(btnCancel)
        btnStdSizer.AddButton(btnSave)
        btnStdSizer.AddButton(btnApply)
        btnStdSizer.Realize()
        
        mainSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer.Add(item=notebook, proportion=1, flag=wx.EXPAND | wx.ALL, border=5)
        mainSizer.Add(item=btnSizer, proportion=0,
                      flag=wx.EXPAND, border=0)
        mainSizer.Add(item=btnStdSizer, proportion=0,
                      flag=wx.EXPAND | wx.ALL | wx.ALIGN_RIGHT, border=5)

        self.SetSizer(mainSizer)
        mainSizer.Fit(self)

        self.SetMinSize(self.GetBestSize())
        self.SetSize((500, 375))

    def __CreateGeneralPage(self, notebook):
        """Create notebook page for general settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("General"))

        border = wx.BoxSizer(wx.VERTICAL)
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("General settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)

        #
        # expand element list
        #
        row = 0
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Element list:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        elementList = wx.Choice(parent=panel, id=wx.ID_ANY, 
                                choices=self.settings.Get(group='general', key='elementListExpand',
                                                          subkey='choices', internal=True),
                                name="GetSelection")
        elementList.SetSelection(self.settings.Get(group='general', key='elementListExpand',
                                                   subkey='selection'))
        self.winId['general:elementListExpand:selection'] = elementList.GetId()

        gridSizer.Add(item=elementList,
                      flag=wx.ALIGN_RIGHT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 1))

        #
        # default window layout
        #
        row += 1
        defaultPos = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                 label=_("Save current window layout as default"),
                                 name='IsChecked')
        defaultPos.SetValue(self.settings.Get(group='general', key='defWindowPos', subkey='enabled'))
        defaultPos.SetToolTip(wx.ToolTip (_("Save current position and size of Layer Manager window and opened "
                                            "Map Display window(s) and use as default for next sessions.")))
        self.winId['general:defWindowPos:enabled'] = defaultPos.GetId()

        gridSizer.Add(item=defaultPos,
                      pos=(row, 0), span=(1, 2))
        
        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=3)

        #
        # Layer Manager settings
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Layer Manager settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)


        #
        # ask when removing map layer from layer tree
        #
        row = 0
        askOnRemoveLayer = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                       label=_("Ask when removing map layer from layer tree"),
                                       name='IsChecked')
        askOnRemoveLayer.SetValue(self.settings.Get(group='manager', key='askOnRemoveLayer', subkey='enabled'))
        self.winId['manager:askOnRemoveLayer:enabled'] = askOnRemoveLayer.GetId()

        gridSizer.Add(item=askOnRemoveLayer,
                      pos=(row, 0), span=(1, 2))

        row += 1
        askOnQuit = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                label=_("Ask when quiting wxGUI or closing display"),
                                name='IsChecked')
        askOnQuit.SetValue(self.settings.Get(group='manager', key='askOnQuit', subkey='enabled'))
        self.winId['manager:askOnQuit:enabled'] = askOnQuit.GetId()

        gridSizer.Add(item=askOnQuit,
                      pos=(row, 0), span=(1, 2))
        
        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=3)

        panel.SetSizer(border)
        
        return panel

    def __CreateDisplayPage(self, notebook):
        """Create notebook page for display settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Display"))

        border = wx.BoxSizer(wx.VERTICAL)

        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Font settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)

        #
        # font settings
        #
        row = 0
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Default font for GRASS displays:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        fontButton = wx.Button(parent=panel, id=wx.ID_ANY,
                               label=_("Set font"), size=(100, -1))
        gridSizer.Add(item=fontButton,
                      flag=wx.ALIGN_RIGHT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 1))

        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=3)

        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Default display settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)

        #
        # display driver
        #
        row = 0
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Display driver:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        listOfDrivers = self.settings.Get(group='display', key='driver', subkey='choices', internal=True)
        # check if cairo is available
        if 'cairo' not in listOfDrivers:
            for line in gcmd.Command(['d.mon', '-l']).ReadStdOutput():
                if 'cairo' in line:
                    listOfDrivers.append('cairo')
                    break
        driver = wx.Choice(parent=panel, id=wx.ID_ANY, size=(150, -1),
                           choices=listOfDrivers,
                           name="GetStringSelection")
        driver.SetStringSelection(self.settings.Get(group='display', key='driver', subkey='type'))
        self.winId['display:driver:type'] = driver.GetId()

        gridSizer.Add(item=driver,
                      flag=wx.ALIGN_RIGHT,
                      pos=(row, 1))


        #
        # Statusbar mode
        #
        row += 1
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Statusbar mode:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        listOfModes = self.settings.Get(group='display', key='statusbarMode', subkey='choices', internal=True)
        statusbarMode = wx.Choice(parent=panel, id=wx.ID_ANY, size=(150, -1),
                                  choices=listOfModes,
                                  name="GetSelection")
        statusbarMode.SetSelection(self.settings.Get(group='display', key='statusbarMode', subkey='selection'))
        self.winId['display:statusbarMode:selection'] = statusbarMode.GetId()

        gridSizer.Add(item=statusbarMode,
                      flag=wx.ALIGN_RIGHT,
                      pos=(row, 1))

        #
        # Background color
        #
        row += 1
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Background color:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        bgColor = csel.ColourSelect(parent=panel, id=wx.ID_ANY,
                                    colour=self.settings.Get(group='display', key='bgcolor', subkey='color'),
                                    size=(35, 35))
        bgColor.SetName('GetColour')
        self.winId['display:bgcolor:color'] = bgColor.GetId()
        
        gridSizer.Add(item=bgColor,
                      flag=wx.ALIGN_RIGHT,
                      pos=(row, 1))
        
        #
        # Use computation resolution
        #
        row += 1
        compResolution = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                    label=_("Constrain display resolution to computational settings"),
                                    name="IsChecked")
        compResolution.SetValue(self.settings.Get(group='display', key='compResolution', subkey='enabled'))
        self.winId['display:compResolution:enabled'] = compResolution.GetId()

        gridSizer.Add(item=compResolution,
                      pos=(row, 0), span=(1, 2))

        #
        # auto-rendering
        #
        row += 1
        autoRendering = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                    label=_("Enable auto-rendering"),
                                    name="IsChecked")
        autoRendering.SetValue(self.settings.Get(group='display', key='autoRendering', subkey='enabled'))
        self.winId['display:autoRendering:enabled'] = autoRendering.GetId()

        gridSizer.Add(item=autoRendering,
                      pos=(row, 0), span=(1, 2))

        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=3)

        panel.SetSizer(border)
        
        # bindings
        fontButton.Bind(wx.EVT_BUTTON, self.OnSetFont)
        
        return panel

    def __CreateCmdPage(self, notebook):
        """Create notebook page for commad dialog settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Command"))
        
        border = wx.BoxSizer(wx.VERTICAL)
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Command dialog settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)
        
        #
        # command dialog settings
        #
        row = 0
        # overwrite
        overwrite = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                label=_("Allow output files to overwrite existing files"),
                                name="IsChecked")
        overwrite.SetValue(self.settings.Get(group='cmd', key='overwrite', subkey='enabled'))
        self.winId['cmd:overwrite:enabled'] = overwrite.GetId()
        
        gridSizer.Add(item=overwrite,
                      pos=(row, 0), span=(1, 2))
        row += 1
        # close
        close = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                            label=_("Close dialog on finish"),
                            name="IsChecked")
        close.SetValue(self.settings.Get(group='cmd', key='closeDlg', subkey='enabled'))
        self.winId['cmd:closeDlg:enabled'] = close.GetId()
        
        gridSizer.Add(item=close,
                      pos=(row, 0), span=(1, 2))
        row += 1
        # add layer
        add = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                          label=_("Add created map into layer tree"),
                          name="IsChecked")
        add.SetValue(self.settings.Get(group='cmd', key='addNewLayer', subkey='enabled'))
        self.winId['cmd:addNewLayer:enabled'] = add.GetId()
        
        gridSizer.Add(item=add,
                      pos=(row, 0), span=(1, 2))
        row += 1
        # verbosity
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Verbosity level:")),
                      flag=wx.ALIGN_LEFT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0))
        verbosity = wx.Choice(parent=panel, id=wx.ID_ANY, size=(200, -1),
                              choices=self.settings.Get(group='cmd', key='verbosity', subkey='choices', internal=True),
                              name="GetStringSelection")
        verbosity.SetStringSelection(self.settings.Get(group='cmd', key='verbosity', subkey='selection'))
        self.winId['cmd:verbosity:selection'] = verbosity.GetId()
        
        gridSizer.Add(item=verbosity,
                      pos=(row, 1))
        
        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=3)
        
        #
        # raster settings
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Raster settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)
        
        #
        # raster overlay
        #
        row = 0
        rasterOverlay = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                    label=_("Overlay raster maps"),
                                    name='IsChecked')
        rasterOverlay.SetValue(self.settings.Get(group='cmd', key='rasterOverlay', subkey='enabled'))
        self.winId['cmd:rasterOverlay:enabled'] = rasterOverlay.GetId()
        
        gridSizer.Add(item=rasterOverlay,
                      pos=(row, 0), span=(1, 2))
        
        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=3)
        
        #
        # vector settings
        #
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Vector settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)
        
        gridSizer = wx.FlexGridSizer (cols=7, hgap=3, vgap=3)
        
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Display:")),
                      flag=wx.ALIGN_CENTER_VERTICAL)
        
        for type in ('point', 'line', 'centroid', 'boundary',
                     'area', 'face'):
            chkbox = wx.CheckBox(parent=panel, label=type)
            checked = self.settings.Get(group='cmd', key='showType',
                                        subkey=[type, 'enabled'])
            chkbox.SetValue(checked)
            self.winId['cmd:showType:%s:enabled' % type] = chkbox.GetId()
            gridSizer.Add(item=chkbox)

        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND, border=3)
        
        panel.SetSizer(border)
        
        return panel

    def __CreateAttributeManagerPage(self, notebook):
        """Create notebook page for 'Attribute Table Manager' settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Attributes"))

        pageSizer = wx.BoxSizer(wx.VERTICAL)

        #
        # highlighting
        #
        highlightBox = wx.StaticBox(parent=panel, id=wx.ID_ANY,
                                    label=" %s " % _("Highlighting"))
        highlightSizer = wx.StaticBoxSizer(highlightBox, wx.VERTICAL)

        flexSizer = wx.FlexGridSizer (cols=2, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)
        
        label = wx.StaticText(parent=panel, id=wx.ID_ANY, label="Color")
        hlColor = csel.ColourSelect(parent=panel, id=wx.ID_ANY,
                                    colour=self.settings.Get(group='atm', key='highlight', subkey='color'),
                                    size=(35, 35))
        hlColor.SetName('GetColour')
        self.winId['atm:highlight:color'] = hlColor.GetId()

        flexSizer.Add(label, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(hlColor, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        label = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Line width (in pixels)"))
        hlWidth = wx.SpinCtrl(parent=panel, id=wx.ID_ANY, size=(50, -1),
                              initial=self.settings.Get(group='atm', key='highlight',subkey='width'),
                              min=1, max=1e6)
        self.winId['atm:highlight:width'] = hlWidth.GetId()

        flexSizer.Add(label, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(hlWidth, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        highlightSizer.Add(item=flexSizer,
                           proportion=0,
                           flag=wx.ALL | wx.EXPAND,
                           border=5)

        pageSizer.Add(item=highlightSizer,
                      proportion=0,
                      flag=wx.ALL | wx.EXPAND,
                      border=5)

        #
        # data browser related settings
        #
        dataBrowserBox = wx.StaticBox(parent=panel, id=wx.ID_ANY,
                                    label=" %s " % _("Data browser"))
        dataBrowserSizer = wx.StaticBoxSizer(dataBrowserBox, wx.VERTICAL)

        flexSizer = wx.FlexGridSizer (cols=2, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)
        label = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Left mouse double click"))
        leftDbClick = wx.Choice(parent=panel, id=wx.ID_ANY,
                                choices=self.settings.Get(group='atm', key='leftDbClick', subkey='choices', internal=True),
                                name="GetSelection")
        leftDbClick.SetSelection(self.settings.Get(group='atm', key='leftDbClick', subkey='selection'))
        self.winId['atm:leftDbClick:selection'] = leftDbClick.GetId()

        flexSizer.Add(label, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(leftDbClick, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        # encoding
        label = wx.StaticText(parent=panel, id=wx.ID_ANY, label=_("Encoding"))
        encoding = wx.TextCtrl(parent=panel, id=wx.ID_ANY,
                               value=self.settings.Get(group='atm', key='encoding', subkey='value'),
                               name="GetValue", size=(200, -1))
        self.winId['atm:encoding:value'] = encoding.GetId()

        flexSizer.Add(label, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(encoding, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        # ask on delete record
        askOnDeleteRec = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                     label=_("Ask when deleting data record(s) from table"),
                                     name='IsChecked')
        askOnDeleteRec.SetValue(self.settings.Get(group='atm', key='askOnDeleteRec', subkey='enabled'))
        self.winId['atm:askOnDeleteRec:enabled'] = askOnDeleteRec.GetId()

        flexSizer.Add(askOnDeleteRec, proportion=0)

        dataBrowserSizer.Add(item=flexSizer,
                           proportion=0,
                           flag=wx.ALL | wx.EXPAND,
                           border=5)

        pageSizer.Add(item=dataBrowserSizer,
                      proportion=0,
                      flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND,
                      border=3)

        #
        # create table
        #
        createTableBox = wx.StaticBox(parent=panel, id=wx.ID_ANY,
                                    label=" %s " % _("Create table"))
        createTableSizer = wx.StaticBoxSizer(createTableBox, wx.VERTICAL)

        flexSizer = wx.FlexGridSizer (cols=2, hgap=5, vgap=5)
        flexSizer.AddGrowableCol(0)

        label = wx.StaticText(parent=panel, id=wx.ID_ANY,
                              label=_("Key column"))
        keyColumn = wx.TextCtrl(parent=panel, id=wx.ID_ANY,
                                size=(250, -1))
        keyColumn.SetValue(self.settings.Get(group='atm', key='keycolumn', subkey='value'))
        self.winId['atm:keycolumn:value'] = keyColumn.GetId()
        
        flexSizer.Add(label, proportion=0, flag=wx.ALIGN_CENTER_VERTICAL)
        flexSizer.Add(keyColumn, proportion=0, flag=wx.ALIGN_RIGHT | wx.FIXED_MINSIZE)

        createTableSizer.Add(item=flexSizer,
                             proportion=0,
                             flag=wx.ALL | wx.EXPAND,
                             border=5)

        pageSizer.Add(item=createTableSizer,
                      proportion=0,
                      flag=wx.LEFT | wx.RIGHT | wx.BOTTOM | wx.EXPAND,
                      border=3)
        
        panel.SetSizer(pageSizer)

        return panel

    def __CreateWorkspacePage(self, notebook):
        """Create notebook page for workspace settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Workspace"))

        border = wx.BoxSizer(wx.VERTICAL)
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Loading workspace"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)

        row = 0

        #
        # positioning
        #
        posDisplay = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                 label=_("Suppress positioning Map Display Window(s)"),
                                 name='IsChecked')
        posDisplay.SetValue(self.settings.Get(group='workspace', key='posDisplay', subkey='enabled'))
        self.winId['workspace:posDisplay:enabled'] = posDisplay.GetId()

        gridSizer.Add(item=posDisplay,
                      pos=(row, 0), span=(1, 2))

        row +=1 

        posManager = wx.CheckBox(parent=panel, id=wx.ID_ANY,
                                 label=_("Suppress positioning Layer Manager window"),
                                 name='IsChecked')
        posManager.SetValue(self.settings.Get(group='workspace', key='posManager', subkey='enabled'))
        self.winId['workspace:posManager:enabled'] = posManager.GetId()

        gridSizer.Add(item=posManager,
                      pos=(row, 0), span=(1, 2))

        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=0, flag=wx.ALL | wx.EXPAND, border=3)

        panel.SetSizer(border)
        
        return panel

    def __CreateAdvancedPage(self, notebook):
        """Create notebook page for advanced settings"""
        panel = wx.Panel(parent=notebook, id=wx.ID_ANY)
        notebook.AddPage(page=panel, text=_("Advanced"))

        border = wx.BoxSizer(wx.VERTICAL)
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Advanced settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=3, vgap=3)
        gridSizer.AddGrowableCol(0)

        row = 0

        #
        # place where to store settings
        #
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Place where to store settings:")),
                       flag=wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL,
                       pos=(row, 0))
        settingsFile = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                                 choices=self.settings.Get(group='advanced', key='settingsFile',
                                                           subkey='choices', internal=True),
                                 name='GetStringSelection')
        settingsFile.SetStringSelection(self.settings.Get(group='advanced', key='settingsFile', subkey='type'))
        self.winId['advanced:settingsFile:type'] = settingsFile.GetId()

        gridSizer.Add(item=settingsFile,
                      flag=wx.ALIGN_RIGHT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 1))
        row += 1

        #
        # icon theme
        #
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Icon theme:")),
                       flag=wx.ALIGN_LEFT |
                       wx.ALIGN_CENTER_VERTICAL,
                       pos=(row, 0))
        iconTheme = wx.Choice(parent=panel, id=wx.ID_ANY, size=(125, -1),
                              choices=self.settings.Get(group='advanced', key='iconTheme',
                                                        subkey='choices', internal=True),
                              name="GetStringSelection")
        iconTheme.SetStringSelection(self.settings.Get(group='advanced', key='iconTheme', subkey='type'))
        self.winId['advanced:iconTheme:type'] = iconTheme.GetId()

        gridSizer.Add(item=iconTheme,
                      flag=wx.ALIGN_RIGHT |
                      wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 1))
        
        row += 1
        gridSizer.Add(item=wx.StaticText(parent=panel, id=wx.ID_ANY,
                                         label=_("Note: For changing the icon theme, "
                                                 "you must save the settings and restart this GUI.")),
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(row, 0), span=(1, 2))
                      
        sizer.Add(item=gridSizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=5)
        border.Add(item=sizer, proportion=1, flag=wx.ALL | wx.EXPAND, border=3)

        panel.SetSizer(border)
        
        return panel

    def OnSetFont(self, event):
        """'Set font' button pressed"""
        dlg = DefaultFontDialog(parent=self, id=wx.ID_ANY,
                                title=_('Select default display font'),
                                style=wx.DEFAULT_DIALOG_STYLE)
        
        if dlg.ShowModal() == wx.ID_OK:
            # set default font and encoding environmental variables
            if dlg.font:
                os.environ["GRASS_FONT"] = dlg.font
                self.settings.Set(group='display', value=dlg.font,
                                  key='font', subkey='type')

            if dlg.encoding and \
                    dlg.encoding != "ISO-8859-1":
                os.environ["GRASS_ENCODING"] = dlg.encoding
                self.settings.Set(group='display', value=dlg.encoding,
                                  key='font', subkey='encoding')
                
        dlg.Destroy()
        
        event.Skip()
        
    def OnSave(self, event):
        """Button 'Save' pressed"""
        if self.__UpdateSettings():
            file = self.settings.SaveToFile()
            self.parent.goutput.WriteLog(_('Settings saved to file \'%s\'.') % file)
            self.Close()

    def OnApply(self, event):
        """Button 'Apply' pressed"""
        if self.__UpdateSettings():
            self.Close()

    def OnCancel(self, event):
        """Button 'Cancel' pressed"""
        self.Close()

    def OnDefault(self, event):
        """Button 'Set to default' pressed"""
        self.settings.userSettings = copy.deepcopy(self.settings.defaultSettings)
        
        # update widgets
        for gks in self.winId.keys():
            try:
                group, key, subkey = gks.split(':')
                value = self.settings.Get(group, key, subkey)
            except ValueError:
                group, key, subkey, subkey1 = gks.split(':')
                value = self.settings.Get(group, key, [subkey, subkey1])
            win = self.FindWindowById(self.winId[gks])
            if win.GetName() in ('GetValue', 'IsChecked'):
                value = win.SetValue(value)
            elif win.GetName() == 'GetSelection':
                value = win.SetSelection(value)
            elif win.GetName() == 'GetStringSelection':
                value = win.SetStringSelection(value)
            else:
                value = win.SetValue(value)

    def __UpdateSettings(self):
        """Update user settings"""
        for item in self.winId.keys():
            try:
                group, key, subkey = item.split(':')
                subkey1 = None
            except ValueError:
                group, key, subkey, subkey1 = item.split(':')
            
            id = self.winId[item]
            win = self.FindWindowById(id)
            if win.GetName() == 'GetValue':
                value = win.GetValue()
            elif win.GetName() == 'GetSelection':
                value = win.GetSelection()
            elif win.GetName() == 'IsChecked':
                value = win.IsChecked()
            elif win.GetName() == 'GetStringSelection':
                value = win.GetStringSelection()
            elif win.GetName() == 'GetColour':
                value = tuple(win.GetValue())
            else:
                value = win.GetValue()

            if key == 'keycolumn' and value == '':
                wx.MessageBox(parent=self,
                              message=_("Key column cannot be empty string."),
                              caption=_("Error"), style=wx.OK | wx.ICON_ERROR)
                win.SetValue(self.settings.Get(group='atm', key='keycolumn', subkey='value'))
                return False

            if subkey1:
                self.settings.Set(group, value, key, [subkey, subkey1])
            else:
                self.settings.Set(group, value, key, subkey)
            
        #
        # update default window dimension
        #
        if self.settings.Get(group='general', key='defWindowPos', subkey='enabled') is True:
            dim = ''
            # layer manager
            pos = self.parent.GetPosition()
            size = self.parent.GetSize()
            dim = '%d,%d,%d,%d' % (pos[0], pos[1], size[0], size[1])
            # opened displays
            for page in range(0, self.parent.gm_cb.GetPageCount()):
                pos = self.parent.gm_cb.GetPage(page).maptree.mapdisplay.GetPosition()
                size = self.parent.gm_cb.GetPage(page).maptree.mapdisplay.GetSize()

                dim += ',%d,%d,%d,%d' % (pos[0], pos[1], size[0], size[1])

            self.settings.Set(group='general', key='defWindowPos', subkey='dim', value=dim)
        else:
            self.settings.Set(group='general', key='defWindowPos', subkey='dim', value='')

        return True

class DefaultFontDialog(wx.Dialog):
    """
    Opens a file selection dialog to select default font
    to use in all GRASS displays
    """
    def __init__(self, parent, id, title,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=wx.DEFAULT_DIALOG_STYLE,
                 settings=globalSettings):
        
        self.settings = settings
        
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        panel = wx.Panel(parent=self, id=wx.ID_ANY)
        
        if "GRASS_FONT" in os.environ:
            self.font = os.environ["GRASS_FONT"]
        else:
            self.font = self.settings.Get(group='display',
                                          key='font', subkey='type')

        self.fontlist = self.GetFonts()

        self.encoding = self.settings.Get(group='display',
                                          key='font', subkey='encoding')
        
        border = wx.BoxSizer(wx.VERTICAL)
        box   = wx.StaticBox (parent=panel, id=wx.ID_ANY, label=" %s " % _("Font settings"))
        sizer = wx.StaticBoxSizer(box, wx.VERTICAL)

        gridSizer = wx.GridBagSizer (hgap=5, vgap=5)
        gridSizer.AddGrowableCol(0)

        label = wx.StaticText(parent=panel, id=wx.ID_ANY,
                              label=_("Select font:"))
        gridSizer.Add(item=label,
                      flag=wx.ALIGN_TOP,
                      pos=(0,0))
        
        self.fontlb = wx.ListBox(parent=panel, id=wx.ID_ANY, pos=wx.DefaultPosition,
                                 choices=self.fontlist,
                                 style=wx.LB_SINGLE|wx.LB_SORT)
        self.Bind(wx.EVT_LISTBOX, self.EvtListBox, self.fontlb)
        self.Bind(wx.EVT_LISTBOX_DCLICK, self.EvtListBoxDClick, self.fontlb)
        if self.font:
            self.fontlb.SetStringSelection(self.font, True)

        gridSizer.Add(item=self.fontlb,
                flag=wx.EXPAND, pos=(0, 1))

        label = wx.StaticText(parent=panel, id=wx.ID_ANY,
                              label=("Character encoding:"))
        gridSizer.Add(item=label,
                      flag=wx.ALIGN_CENTER_VERTICAL,
                      pos=(1, 0))

        self.textentry = wx.TextCtrl(parent=panel, id=wx.ID_ANY,
                                     value=self.encoding)
        gridSizer.Add(item=self.textentry,
                flag=wx.EXPAND, pos=(1, 1))

        self.textentry.Bind(wx.EVT_TEXT, self.OnEncoding)

        sizer.Add(item=gridSizer, proportion=1,
                  flag=wx.EXPAND | wx.ALL,
                  border=5)

        border.Add(item=sizer, proportion=1,
                   flag=wx.ALL | wx.EXPAND, border=3)
        
        btnsizer = wx.StdDialogButtonSizer()

        btn = wx.Button(parent=panel, id=wx.ID_OK)
        btn.SetDefault()
        btnsizer.AddButton(btn)

        btn = wx.Button(parent=panel, id=wx.ID_CANCEL)
        btnsizer.AddButton(btn)
        btnsizer.Realize()

        border.Add(item=btnsizer, proportion=0,
                   flag=wx.EXPAND | wx.ALIGN_RIGHT | wx.ALL, border=5)
        
        panel.SetAutoLayout(True)
        panel.SetSizer(border)
        border.Fit(self)
        
        self.Layout()
        
    def EvtRadioBox(self, event):
        if event.GetInt() == 0:
            self.fonttype = 'grassfont'
        elif event.GetInt() == 1:
            self.fonttype = 'truetype'

        self.fontlist = self.GetFonts(self.fonttype)
        self.fontlb.SetItems(self.fontlist)

    def OnEncoding(self, event):
        self.encoding = event.GetString()

    def EvtListBox(self, event):
        self.font = event.GetString()
        event.Skip()

    def EvtListBoxDClick(self, event):
        self.font = event.GetString()
        event.Skip()

    def GetFonts(self):
        """
        parses fonts directory or fretypecap file to get a list of fonts for the listbox
        """
        fontlist = []

        cmd = ["d.font", "-l"]

        p = gcmd.Command(cmd, stderr=None)

        dfonts = p.ReadStdOutput()
        dfonts.sort(lambda x,y: cmp(x.lower(), y.lower()))
        for item in range(len(dfonts)):
           # ignore duplicate fonts and those starting with #
           if not dfonts[item].startswith('#') and \
                  dfonts[item] != dfonts[item-1]:
              fontlist.append(dfonts[item])

        return fontlist

class MapsetAccess(wx.Dialog):
    """
    Controls setting options and displaying/hiding map overlay decorations
    """
    def __init__(self, parent, id, title=_('Set/unset access to mapsets in current location'),
                 pos=wx.DefaultPosition, size=(350, 400),
                 style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER):
        
        wx.Dialog.__init__(self, parent, id, title, pos, size, style)

        self.all_mapsets = utils.ListOfMapsets(all=True)
        self.accessible_mapsets = utils.ListOfMapsets(all=False)
        self.curr_mapset = grassenv.GetGRASSVariable('MAPSET')

        # make a checklistbox from available mapsets and check those that are active
        sizer = wx.BoxSizer(wx.VERTICAL)

        label = wx.StaticText(parent=self, id=wx.ID_ANY,
                              label=_("Check mapset to make it accessible, uncheck it to hide it.%s"
                                      "Note: PERMANENT and current mapset are always accessible.") % os.linesep)
        sizer.Add(item=label, proportion=0,
                  flag=wx.ALL, border=5)

        self.mapsetlb = CheckListMapset(parent=self)
        self.mapsetlb.LoadData(self.all_mapsets)
        
        sizer.Add(item=self.mapsetlb, proportion=1,
                  flag=wx.ALL | wx.EXPAND, border=5)

        # check all accessible mapsets
        for mset in self.accessible_mapsets:
            self.mapsetlb.CheckItem(self.all_mapsets.index(mset), True)

        # dialog buttons
        line = wx.StaticLine(parent=self, id=wx.ID_ANY,
                             style=wx.LI_HORIZONTAL)
        sizer.Add(item=line, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_CENTRE | wx.ALL, border=5)

        btnsizer = wx.StdDialogButtonSizer()
        okbtn = wx.Button(self, wx.ID_OK)
        okbtn.SetDefault()
        btnsizer.AddButton(okbtn)

        cancelbtn = wx.Button(self, wx.ID_CANCEL)
        btnsizer.AddButton(cancelbtn)
        btnsizer.Realize()

        sizer.Add(item=btnsizer, proportion=0,
                  flag=wx.EXPAND | wx.ALIGN_RIGHT | wx.ALL, border=5)

        # do layout
        self.Layout()
        self.SetSizer(sizer)
        sizer.Fit(self)

        self.SetMinSize(size)
        
    def GetMapsets(self):
        """Get list of checked mapsets"""
        ms = []
        i = 0
        for mset in self.all_mapsets:
            if self.mapsetlb.IsChecked(i):
                ms.append(mset)
            i += 1

        return ms

class CheckListMapset(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin, listmix.CheckListCtrlMixin):
    """List of mapset/owner/group"""
    def __init__(self, parent, pos=wx.DefaultPosition,
                 log=None):
        self.parent = parent
        
        wx.ListCtrl.__init__(self, parent, wx.ID_ANY,
                             style=wx.LC_REPORT)
        listmix.CheckListCtrlMixin.__init__(self)
        self.log = log

        # setup mixins
        listmix.ListCtrlAutoWidthMixin.__init__(self)

    def LoadData(self, mapsets):
        """Load data into list"""
        self.InsertColumn(0, _('Mapset'))
        self.InsertColumn(1, _('Owner'))
        self.InsertColumn(2, _('Group'))
<<<<<<< .working
        locationPath = os.path.join(grassenv.GetGRASSVariable('GISDBASE'),
                                    grassenv.GetGRASSVariable('LOCATION_NAME'))
=======
        gisenv = grass.gisenv()
        locationPath = os.path.join(gisenv['GISDBASE'], gisenv['LOCATION_NAME'])

        ret = grass.read_command('g.mapsets',
                                 flags = 'l')
        ret = ret.strip(' \n')

        mapsets = []
        if ret:
            mapsets = ret.split()
>>>>>>> .merge-right.r35797
        
        for mapset in mapsets:
            index = self.InsertStringItem(sys.maxint, mapset)
            mapsetPath = os.path.join(locationPath,
                                      mapset)
            stat_info = os.stat(mapsetPath)
        if os.name in ('posix', 'mac'):
            self.SetStringItem(index, 1, "%s" % pwd.getpwuid(stat_info.st_uid)[0])
            # FIXME: get group name
            self.SetStringItem(index, 2, "%-8s" % stat_info.st_gid) 
        else:
            # FIXME: no pwd under MS Windows (owner: 0, group: 0)
            self.SetStringItem(index, 1, "%-8s" % stat_info.st_uid)
            self.SetStringItem(index, 2, "%-8s" % stat_info.st_gid)
                
        self.SetColumnWidth(col=0, width=wx.LIST_AUTOSIZE)
        self.SetColumnWidth(col=1, width=wx.LIST_AUTOSIZE)
        
    def OnCheckItem(self, index, flag):
        """Mapset checked/unchecked"""
        mapset = self.parent.all_mapsets[index]
        if mapset == 'PERMANENT' or mapset == self.parent.curr_mapset:
            self.CheckItem(index, True)
