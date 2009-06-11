"""
@package workspace.py

@brief Open/save workspace definition file

Classes:
 - ProcessWorkspaceFile
 - Nviz
 - WriteWorkspaceFile
 - ProcessGrcFile

(C) 2007-2009 by the GRASS Development Team
This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Martin Landa <landa.martin gmail.com>
"""

import os
import sys

import wx

### for gxw (workspace file) parsering
# xmlproc not available on Mac OS
# from xml.parsers.xmlproc import xmlproc
# from xml.parsers.xmlproc import xmlval
# from xml.parsers.xmlproc import xmldtd
import xml.sax
import xml.sax.handler
HandlerBase=xml.sax.handler.ContentHandler
from xml.sax import make_parser

import globalvar
from preferences import globalSettings as UserSettings

sys.path.append(os.path.join(globalvar.ETCWXDIR, "nviz"))
try:
    import grass6_wxnviz as wxnviz
except ImportError:
    wxnviz = None

class ProcessWorkspaceFile(HandlerBase):
    """
    A SAX handler for the GXW XML file, as
    defined in grass-gxw.dtd.
    """
    def __init__(self):
        self.inTag = {}
        for tag in ('gxw', 'layer', 'task', 'parameter',
                    'flag', 'value', 'group', 'display',
                    'layer_manager',
                    'nviz',
                    # surface
                    'surface', 'attribute', 'draw', 'resolution',
                    'wire_color', 'position', 'x', 'y', 'z',
                    # vector lines
                    'vlines', 'color', 'width', 'mode',
                    'map', 'height',
                    # vector points
                    'vpoints', 'size'):
            self.inTag[tag] = False

        #
        # layer manager properties
        #
        self.layerManager = {}
        self.layerManager['pos'] = None # window position
        self.layerManager['size'] = None # window size

        # list of mapdisplays
        self.displays = []
        # list of map layers
        self.layers = []

        self.cmd    = []
        self.displayIndex = -1 # first display has index '0'

        self.nvizDefault = Nviz()
        
    def __filterValue(self, value):
        """Translate value"""
        value = value.replace('&lt;', '<')
        value = value.replace('&gt;', '>')
        
        return value
    
    def startElement(self, name, attrs):
        if name == 'display':
            self.displayIndex += 1

            # window position and size
            posAttr = attrs.get('dim', '')
            if posAttr:
                posVal = map(int, posAttr.split(','))
                try:
                    pos = (posVal[0], posVal[1])
                    size = (posVal[2], posVal[3])
                except:
                    pos = None
                    size = None
            else:
                pos = None
                size = None

            extentAttr = attrs.get('extent', '')
            if extentAttr:
                # w, s, e, n
                extent = map(float, extentAttr.split(','))
            else:
                extent = None

            self.displays.append({
                "render"         : bool(int(attrs.get('render', "0"))),
                "mode"           : int(attrs.get('mode', 0)),
                "showCompExtent" : bool(int(attrs.get('showCompExtent', "0"))),
                "pos"            : pos,
                "size"           : size,
                "extent"         : extent,
                "constrainRes"   : bool(int(attrs.get('constrainRes', "0")))})
            
        elif name == 'group':
            self.groupName    = attrs.get('name', None)
            self.groupChecked = attrs.get('checked', None)
            self.layers.append({
                "type"    : 'group',
                "name"    : self.groupName,
                "checked" : int(self.groupChecked),
                "opacity" : None,
                "cmd"     : None,
                "group"   : self.inTag['group'],
                "display" : self.displayIndex,
                "nviz"    : None})

        elif name == 'layer':
            self.layerType     = attrs.get('type', None)
            self.layerName     = attrs.get('name', None)
            self.layerChecked  = attrs.get('checked', None)
            self.layerOpacity  = attrs.get('opacity', None)
            self.layerSelected = False
            self.layerNviz     = None
            self.cmd = []

        elif name == 'task':
            name = attrs.get('name', None)
            self.cmd.append(name)

        elif name == 'parameter':
            self.parameterName = attrs.get('name', None)

        elif name in ('value', 'x', 'y', 'z', 'map'):
            self.value = ''

        elif name == 'flag':
            name = attrs.get('name', None)
            self.cmd.append('-' + name)

        elif name == 'selected':
            if self.inTag['layer']:
                self.layerSelected = True;

        elif name == 'layer_manager':
            posAttr = attrs.get('dim', '')
            if posAttr:
                posVal = map(int, posAttr.split(','))
                try:
                    self.layerManager['pos'] = (posVal[0], posVal[1])
                    self.layerManager['size'] = (posVal[2], posVal[3])
                except:
                    pass
            else:
                pass

        #
        # Nviz section
        #
        elif name == 'nviz':
            # init nviz layer properties (use default values)
            self.layerNviz = {}
            if self.layerType == 'raster':
                self.layerNviz['surface'] = \
                    self.nvizDefault.SetSurfaceDefaultProp()
            elif self.layerType == 'vector':
                self.layerNviz['vector'] = \
                    self.nvizDefault.SetVectorDefaultProp()
            
        elif name == 'attribute':
            if self.inTag['nviz'] and self.inTag['surface']:
                tagName = str(name)
                attrbName = str(attrs.get('name', ''))
                self.layerNviz['surface'][tagName][attrbName] = {}
                if attrs.get('map', '0') == '0':
                    self.layerNviz['surface'][tagName][attrbName]['map'] = False
                else:
                    self.layerNviz['surface'][tagName][attrbName]['map'] = True

                self.refAttribute = self.layerNviz['surface'][tagName][attrbName]
        
        elif name == 'draw':
            if self.inTag['nviz'] and self.inTag['surface']:
                tagName = str(name)
                self.layerNviz['surface'][tagName]['all'] = False
                self.layerNviz['surface'][tagName]['mode'] = {}
                self.layerNviz['surface'][tagName]['mode']['value'] = -1 # to be calculated
                self.layerNviz['surface'][tagName]['mode']['desc'] = {}
                self.layerNviz['surface'][tagName]['mode']['desc']['shading'] = \
                    str(attrs.get('shading', ''))
                self.layerNviz['surface'][tagName]['mode']['desc']['style'] = \
                    str(attrs.get('style', ''))
                self.layerNviz['surface'][tagName]['mode']['desc']['mode'] = \
                    str(attrs.get('mode', ''))

        elif name == 'resolution':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.resolutionType = str(attrs.get('type', ''))
                if not self.layerNviz['surface']['draw'].has_key(str(name)):
                    self.layerNviz['surface']['draw'][str(name)] = {}

        elif name == 'position':
            if self.inTag['nviz'] and inTag['surface']:
                self.layerNviz['surface']['position'] = {}
        
        elif name == 'mode':
            if self.inTag['nviz']:
                if self.inTag['vlines']:
                    self.layerNviz['vector']['lines']['mode'] = {}
                    self.layerNviz['vector']['lines']['mode']['type'] = str(attrs.get('type', ''))
                    self.layerNviz['vector']['lines']['mode']['surface'] = ''
                elif self.inTag['vpoints']:
                    self.layerNviz['vector']['points']['mode'] = {}
                    self.layerNviz['vector']['points']['mode']['type'] = str(attrs.get('type', ''))
                    self.layerNviz['vector']['points']['mode']['surface'] = ''

        elif name == 'vpoints':
            if self.inTag['nviz']:
                marker = str(attrs.get('marker', ''))
                markerId = list(UserSettings.Get(group='nviz', key='vector',
                                                 subkey=['points', 'marker'], internal=True)).index(marker)
                self.layerNviz['vector']['points']['marker'] = markerId
        
        self.inTag[name] = True
        
    def endElement(self, name):
        if name == 'group':
            self.groupName = self.groupChecked = None

        elif name == 'layer':
            self.layers.append({
                    "type"     : self.layerType,
                    "name"     : self.layerName,
                    "checked"  : int(self.layerChecked),
                    "opacity"  : 1.0,
                    "cmd"      : None,
                    "group"    : self.inTag['group'],
                    "display"  : self.displayIndex,
                    "selected" : self.layerSelected,
                    "nviz"     : self.layerNviz})
            
            if self.layerOpacity:
                self.layers[-1]["opacity"] = float(self.layerOpacity)
            if self.cmd:
                self.layers[-1]["cmd"] = self.cmd
            
            self.layerType = self.layerName = self.Checked = \
                self.Opacity = self.cmd = None

        elif name == 'parameter':
            self.cmd.append('%s=%s' % (self.parameterName,
                                       self.__filterValue(self.value)))
            self.parameterName = self.value = None

        #
        # Nviz section
        #
        elif name == 'attribute':
            if self.inTag['nviz'] and self.inTag['surface']:
                try:
                    self.refAttribute['value'] = int(self.value)
                except ValueError:
                    try:
                        self.refAttribute['value'] = float(self.value)
                    except ValueError:
                        self.refAttribute['value'] = str(self.value)

        elif name == 'resolution':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.layerNviz['surface']['draw']['resolution'][self.resolutionType] = int(self.value)
                del self.resolutionType

        elif name == 'wire_color':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.layerNviz['surface']['draw']['wire-color'] = {}
                self.layerNviz['surface']['draw']['wire-color']['value'] = str(self.value)

        elif name == 'x':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.layerNviz['surface']['position']['x'] = int(self.value)

        elif name == 'y':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.layerNviz['surface']['position']['y'] = int(self.value)

        elif name == 'z':
            if self.inTag['nviz'] and self.inTag['surface']:
                self.layerNviz['surface']['position']['z'] = int(self.value)
        
        elif name == 'color':
            if self.inTag['nviz']:
                if self.inTag['vlines']:
                    self.layerNviz['vector']['lines']['color'] = dict()
                    self.layerNviz['vector']['lines']['color']['value'] = str(self.value)
                elif self.inTag['vpoints']:
                    self.layerNviz['vector']['points']['color'] = dict()
                    self.layerNviz['vector']['points']['color']['value'] = str(self.value)
                    
        elif name == 'width':
            if self.inTag['nviz']:
                if self.inTag['vlines']:
                    self.layerNviz['vector']['lines']['width'] = dict()
                    self.layerNviz['vector']['lines']['width']['value'] = int(self.value)
                elif self.inTag['vpoints']:
                    self.layerNviz['vector']['points']['width'] = dict()
                    self.layerNviz['vector']['points']['width']['value'] = int(self.value)

        elif name == 'height':
            if self.inTag['nviz']:
                if self.inTag['vlines']:
                    self.layerNviz['vector']['lines']['height'] = dict()
                    self.layerNviz['vector']['lines']['height']['value'] = int(self.value)
                elif self.inTag['vpoints']:
                    self.layerNviz['vector']['points']['height'] = dict()
                    self.layerNviz['vector']['points']['height']['value'] = int(self.value)
        
        elif name == 'size':
            if self.inTag['nviz'] and self.inTag['vpoints']:
                self.layerNviz['vector']['points']['size']['value'] = int(self.value)

        elif name == 'map':
            if self.inTag['nviz']:
                if self.inTag['vlines']:
                    self.layerNviz['vector']['lines']['mode']['surface'] = str(self.value)
                elif self.inTag['vpoints']:
                    self.layerNviz['vector']['points']['mode']['surface'] = str(self.value)

        self.inTag[name] = False

    def characters(self, ch):
        self.my_characters(ch)

    def my_characters(self, ch):
        if self.inTag['value'] or \
                self.inTag['x'] or \
                self.inTag['y'] or \
                self.inTag['z'] or \
                self.inTag['map']:
            self.value += ch

class Nviz:
    def __init__(self):
        """Default 3D settings"""
        pass
    
    def SetSurfaceDefaultProp(self):
        """Set default surface data properties"""
        data = dict()
        for sec in ('attribute', 'draw', 'mask', 'position'):
            data[sec] = {}
        
        #
        # attributes
        #
        for attrb in ('shine', ):
            data['attribute'][attrb] = {}
            for key, value in UserSettings.Get(group='nviz', key='volume',
                                               subkey=attrb).iteritems():
                data['attribute'][attrb][key] = value
            data['attribute'][attrb]['update'] = None
        
        #
        # draw
        #
        data['draw']['all'] = False # apply only for current surface
        for control, value in UserSettings.Get(group='nviz', key='surface', subkey='draw').iteritems():
            if control[:3] == 'res':
                if not data['draw'].has_key('resolution'):
                    data['draw']['resolution'] = {}
                if not data['draw']['resolution'].has_key('update'):
                    data['draw']['resolution']['update'] = None
                data['draw']['resolution'][control[4:]] = value
                continue
            
            if control == 'wire-color':
                value = str(value[0]) + ':' + str(value[1]) + ':' + str(value[2])
            elif control in ('mode', 'style', 'shading'):
                if not data['draw'].has_key('mode'):
                    data['draw']['mode'] = {}
                continue

            data['draw'][control] = { 'value' : value }
            data['draw'][control]['update'] = None
            
        value, desc = self.GetDrawMode(UserSettings.Get(group='nviz', key='surface', subkey=['draw', 'mode']),
                                       UserSettings.Get(group='nviz', key='surface', subkey=['draw', 'style']),
                                       UserSettings.Get(group='nviz', key='surface', subkey=['draw', 'shading']))

        data['draw']['mode'] = { 'value' : value,
                                 'desc' : desc, 
                                 'update': None }
        
        return data
    
    def SetVolumeDefaultProp(self):
        """Set default volume data properties"""
        data = dict()
        for sec in ('attribute', 'draw', 'position'):
            data[sec] = dict()
            for sec in ('isosurface', 'slice'):
                    data[sec] = list()
        
        #
        # draw
        #
        for control, value in UserSettings.Get(group='nviz', key='volume', subkey='draw').iteritems():
            if control == 'mode':
                continue
            if control == 'shading':
                sel = UserSettings.Get(group='nviz', key='surface', subkey=['draw', 'shading'])
                value, desc = self.GetDrawMode(shade=sel, string=False)

                data['draw']['shading'] = { 'value' : value,
                                            'desc' : desc['shading'] }
            elif control == 'mode':
                sel = UserSettings.Get(group='nviz', key='volume', subkey=['draw', 'mode'])
                if sel == 0:
                    desc = 'isosurface'
                else:
                    desc = 'slice'
                data['draw']['mode'] = { 'value' : sel,
                                         'desc' : desc, }
            else:
                data['draw'][control] = { 'value' : value }

            if not data['draw'][control].has_key('update'):
                data['draw'][control]['update'] = None
        
        #
        # isosurface attributes
        #
        for attrb in ('shine', ):
            data['attribute'][attrb] = {}
            for key, value in UserSettings.Get(group='nviz', key='volume',
                                               subkey=attrb).iteritems():
                data['attribute'][attrb][key] = value
        
        return data
    
    def SetVectorDefaultProp(self):
        """Set default vector data properties"""
        data = dict()
        for sec in ('lines', 'points'):
            data[sec] = {}
        
        self.SetVectorLinesDefaultProp(data['lines'])
        self.SetVectorPointsDefaultProp(data['points'])

        return data
    
    def SetVectorLinesDefaultProp(self, data):
        """Set default vector properties -- lines"""
        # width
        data['width'] = {'value' : UserSettings.Get(group='nviz', key='vector',
                                                    subkey=['lines', 'width']) }
        
        # color
        value = UserSettings.Get(group='nviz', key='vector',
                                 subkey=['lines', 'color'])
        color = str(value[0]) + ':' + str(value[1]) + ':' + str(value[2])
        data['color'] = { 'value' : color }

        # mode
        if UserSettings.Get(group='nviz', key='vector',
                            subkey=['lines', 'flat']):
            type = 'flat'
            map  = None
        else:
            type = 'flat'
            map = None

        data['mode'] = {}
        data['mode']['type'] = type
        data['mode']['update'] = None
        if map:
            data['mode']['surface'] = map

        # height
        data['height'] = { 'value' : UserSettings.Get(group='nviz', key='vector',
                                                      subkey=['lines', 'height']) }

        if data.has_key('object'):
            for attrb in ('color', 'width', 'mode', 'height'):
                data[attrb]['update'] = None
        
    def SetVectorPointsDefaultProp(self, data):
        """Set default vector properties -- points"""
        # size
        data['size'] = { 'value' : UserSettings.Get(group='nviz', key='vector',
                                                    subkey=['points', 'size']) }

        # width
        data['width'] = { 'value' : UserSettings.Get(group='nviz', key='vector',
                                                     subkey=['points', 'width']) }

        # marker
        data['marker'] = { 'value' : UserSettings.Get(group='nviz', key='vector',
                                                      subkey=['points', 'marker']) }

        # color
        value = UserSettings.Get(group='nviz', key='vector',
                                 subkey=['points', 'color'])
        color = str(value[0]) + ':' + str(value[1]) + ':' + str(value[2])
        data['color'] = { 'value' : color }

        # mode
        data['mode'] = { 'type' : 'surface',
                         'surface' : '', }
        
        # height
        data['height'] = { 'value' : UserSettings.Get(group='nviz', key='vector',
                                                      subkey=['points', 'height']) }

        if data.has_key('object'):
            for attrb in ('size', 'width', 'marker',
                          'color', 'surface', 'height'):
                data[attrb]['update'] = None
        
    def GetDrawMode(self, mode=None, style=None, shade=None, string=False):
        """Get surface draw mode (value) from description/selection

        @param mode,style,shade modes
        @param string if True input parameters are strings otherwise
        selections
        """
        if not wxnviz:
            return None
        
        value = 0
        desc = {}

        if string:
            if mode is not None:
                if mode == 'coarse':
                    value |= wxnviz.DM_WIRE
                elif mode == 'fine':
                    value |= wxnviz.DM_POLY
                else: # both
                    value |= wxnviz.DM_WIRE_POLY

            if style is not None:
                if style == 'wire':
                    value |= wxnviz.DM_GRID_WIRE
                else: # surface
                    value |= wxnviz.DM_GRID_SURF
                    
            if shade is not None:
                if shade == 'flat':
                    value |= wxnviz.DM_FLAT
                else: # surface
                    value |= wxnviz.DM_GOURAUD

            return value

        # -> string is False
        if mode is not None:
            if mode == 0: # coarse
                value |= wxnviz.DM_WIRE
                desc['mode'] = 'coarse'
            elif mode == 1: # fine
                value |= wxnviz.DM_POLY
                desc['mode'] = 'fine'
            else: # both
                value |= wxnviz.DM_WIRE_POLY
                desc['mode'] = 'both'

        if style is not None:
            if style == 0: # wire
                value |= wxnviz.DM_GRID_WIRE
                desc['style'] = 'wire'
            else: # surface
                value |= wxnviz.DM_GRID_SURF
                desc['style'] = 'surface'

        if shade is not None:
            if shade == 0:
                value |= wxnviz.DM_FLAT
                desc['shading'] = 'flat'
            else: # surface
                value |= wxnviz.DM_GOURAUD
                desc['shading'] = 'gouraud'
        
        return (value, desc)
    
class WriteWorkspaceFile(object):
    """Generic class for writing workspace file"""
    def __init__(self, lmgr, file):
        self.file =  file
        self.lmgr = lmgr
        self.indent = 0

        # write header
        self.file.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        self.file.write('<!DOCTYPE gxw SYSTEM "grass-gxw.dtd">\n')
        self.file.write('%s<gxw>\n' % (' ' * self.indent))
        
        self.indent =+ 4
        
        # layer manager
        windowPos = self.lmgr.GetPosition()
        windowSize = self.lmgr.GetSize()
        file.write('%s<layer_manager dim="%d,%d,%d,%d">\n' % (' ' * self.indent,
                                                              windowPos[0],
                                                              windowPos[1],
                                                              windowSize[0],
                                                              windowSize[1]
                                                              ))
        
        file.write('%s</layer_manager>\n' % (' ' * self.indent))
        
        # list of displays
        for page in range(0, self.lmgr.gm_cb.GetPageCount()):
            mapTree = self.lmgr.gm_cb.GetPage(page).maptree
            region = mapTree.Map.region
            
            displayPos = mapTree.mapdisplay.GetPosition()
            displaySize = mapTree.mapdisplay.GetSize()
            
            file.write('%s<display render="%d" '
                       'mode="%d" showCompExtent="%d" '
                       'constrainRes="%d" '
                       'dim="%d,%d,%d,%d" '
                       'extent="%f,%f,%f,%f">\n' % (' ' * self.indent,
                                                    int(mapTree.mapdisplay.autoRender.IsChecked()),
                                                    mapTree.mapdisplay.toggleStatus.GetSelection(),
                                                    int(mapTree.mapdisplay.showRegion.IsChecked()),
                                                    int(mapTree.mapdisplay.compResolution.IsChecked()),
                                                    displayPos[0],
                                                    displayPos[1],
                                                    displaySize[0],
                                                    displaySize[1],
                                                    region['w'],
                                                    region['s'],
                                                    region['e'],
                                                    region['n']
                                                    ))
            
            # list of layers
            item = mapTree.GetFirstChild(mapTree.root)[0]
            self.__writeLayer(mapTree, item)
            file.write('%s</display>\n' % (' ' * self.indent))

        self.indent =- 4
        file.write('%s</gxw>\n' % (' ' * self.indent))

    def __filterValue(self, value):
        """Make value XML-valid"""
        value = value.replace('<', '&lt;')
        value = value.replace('>', '&gt;')
        
        return value
    
    def __writeLayer(self, mapTree, item):
        """Write bunch of layers to GRASS Workspace XML file"""
        self.indent += 4
        itemSelected = mapTree.GetSelections()
        while item and item.IsOk():
            type = mapTree.GetPyData(item)[0]['type']
            if type != 'group':
                maplayer = mapTree.GetPyData(item)[0]['maplayer']
            else:
                maplayer = None

            checked = int(item.IsChecked())
            if type == 'command':
                cmd = mapTree.GetPyData(item)[0]['maplayer'].GetCmd(string=True)
                self.file.write('%s<layer type="%s" name="%s" checked="%d">\n' % \
                               (' ' * self.indent, type, cmd, checked));
                self.file.write('%s</layer>\n' % (' ' * self.indent));
            elif type == 'group':
                name = mapTree.GetItemText(item)
                self.file.write('%s<group name="%s" checked="%d">\n' % \
                               (' ' * self.indent, name, checked));
                self.indent += 4
                subItem = mapTree.GetFirstChild(item)[0]
                self.__writeLayer(mapTree, subItem)
                self.indent -= 4
                self.file.write('%s</group>\n' % (' ' * self.indent));
            else:
                cmd = mapTree.GetPyData(item)[0]['maplayer'].GetCmd(string=False)
                name = mapTree.GetItemText(item)
                # remove 'opacity' part
                if '(opacity' in name:
                    name = name.split('(', -1)[0].strip()
                opacity = maplayer.GetOpacity(float=True)
                self.file.write('%s<layer type="%s" name="%s" checked="%d" opacity="%f">\n' % \
                               (' ' * self.indent, type, name, checked, opacity));

                self.indent += 4
                # selected ?
                if item in itemSelected:
                    self.file.write('%s<selected />\n' % (' ' * self.indent))
                # layer properties
                self.file.write('%s<task name="%s">\n' % (' ' * self.indent, cmd[0]))
                self.indent += 4
                for option in cmd[1:]:
                    if option[0] == '-': # flag
                        self.file.write('%s<flag name="%s" />\n' %
                                   (' ' * self.indent, option[1]))
                    else: # parameter
                        key, value = option.split('=', 1)
                        self.file.write('%s<parameter name="%s">\n' %
                                   (' ' * self.indent, key))
                        self.indent += 4
                        self.file.write('%s<value>%s</value>\n' %
                                   (' ' * self.indent, self.__filterValue(value)))
                        self.indent -= 4
                        self.file.write('%s</parameter>\n' % (' ' * self.indent));
                self.indent -= 4
                self.file.write('%s</task>\n' % (' ' * self.indent));
                nviz = mapTree.GetPyData(item)[0]['nviz']
                if nviz:
                    self.file.write('%s<nviz>\n' % (' ' * self.indent));
                    if maplayer.type == 'raster':
                        self.__writeNvizSurface(nviz['surface'])
                    elif maplayer.type == 'vector':
                        self.__writeNvizVector(nviz['vector'])
                    self.file.write('%s</nviz>\n' % (' ' * self.indent));
                self.indent -= 4
                self.file.write('%s</layer>\n' % (' ' * self.indent));
            item = mapTree.GetNextSibling(item)
        self.indent -= 4

    def __writeNvizSurface(self, data):
        """Save Nviz raster layer properties to workspace

        @param data Nviz layer properties
        """
        if not data.has_key('object'): # skip disabled
            return

        self.indent += 4
        self.file.write('%s<surface>\n' % (' ' * self.indent))
        self.indent += 4
        for attrb in data.iterkeys():
            if len(data[attrb]) < 1: # skip empty attributes
                continue
            if attrb == 'object':
                continue
            
            for name in data[attrb].iterkeys():
                # surface attribute
                if attrb == 'attribute':
                    self.file.write('%s<%s name="%s" map="%d">\n' % \
                                   (' ' * self.indent, attrb, name, data[attrb][name]['map']))
                    self.indent += 4
                    self.file.write('%s<value>%s</value>\n' % (' ' * self.indent, data[attrb][name]['value']))
                    self.indent -= 4
                    # end tag
                    self.file.write('%s</%s>\n' % (' ' * self.indent, attrb))

            # draw mode
            if attrb == 'draw':
                self.file.write('%s<%s' %(' ' * self.indent, attrb))
                if data[attrb].has_key('mode'):
                    for tag, value in data[attrb]['mode']['desc'].iteritems():
                        self.file.write(' %s="%s"' % (tag, value))
                self.file.write('>\n') # <draw ...>

                if data[attrb].has_key('resolution'):
                    self.indent += 4
                    for type in ('coarse', 'fine'):
                        self.file.write('%s<resolution type="%s">\n' % (' ' * self.indent, type))
                        self.indent += 4
                        self.file.write('%s<value>%d</value>\n' % (' ' * self.indent,
                                                                   data[attrb]['resolution'][type]))
                        self.indent -= 4
                        self.file.write('%s</resolution>\n' % (' ' * self.indent))

                if data[attrb].has_key('wire-color'):
                    self.file.write('%s<wire_color>\n' % (' ' * self.indent))
                    self.indent += 4
                    self.file.write('%s<value>%s</value>\n' % (' ' * self.indent,
                                                               data[attrb]['wire-color']['value']))
                    self.indent -= 4
                    self.file.write('%s</wire_color>\n' % (' ' * self.indent))
                self.indent -= 4
            
            # position
            elif attrb == 'position':
                self.file.write('%s<%s>\n' %(' ' * self.indent, attrb))
                i = 0
                for tag in ('x', 'y', 'z'):
                    self.indent += 4
                    self.file.write('%s<%s>%d</%s>\n' % (' ' * self.indent, tag,
                                                        data[attrb][tag], tag))
                    i += 1
                    self.indent -= 4

            if attrb != 'attribute':
                # end tag
                self.file.write('%s</%s>\n' % (' ' * self.indent, attrb))

        self.indent -= 4
        self.file.write('%s</surface>\n' % (' ' * self.indent))
        self.indent -= 4

    def __writeNvizVector(self, data):
        """Save Nviz vector layer properties (lines/points) to workspace

        @param data Nviz layer properties
        """
        self.indent += 4
        for attrb in data.iterkeys():
            if len(data[attrb]) < 1: # skip empty attributes
                continue

            if not data[attrb].has_key('object'): # skip disabled
                continue
            if attrb == 'lines':
                self.file.write('%s<v%s>\n' % (' ' * self.indent, attrb))
            elif attrb == 'points':
                markerId = data[attrb]['marker']
                marker = UserSettings.Get(group='nviz', key='vector',
                                          subkey=['points', 'marker'], internal=True)[markerId]
                self.file.write('%s<v%s marker="%s">\n' % (' ' * self.indent,
                                                           attrb,
                                                           marker))
            self.indent += 4
            for name in data[attrb].iterkeys():
                if name in ('object', 'marker'):
                    continue
                if name == 'mode':
                    self.file.write('%s<%s type="%s">\n' % (' ' * self.indent, name,
                                                          data[attrb][name]['type']))
                    if data[attrb][name]['type'] == 'surface':
                        self.indent += 4
                        self.file.write('%s<map>%s</map>\n' % (' ' * self.indent,
                                                               data[attrb][name]['surface']))
                        self.indent -= 4
                    self.file.write('%s</%s>\n' % ((' ' * self.indent, name)))
                else:
                    self.file.write('%s<%s>\n' % (' ' * self.indent, name))
                    self.indent += 4
                    self.file.write('%s<value>%s</value>\n' % (' ' * self.indent, data[attrb][name]))
                    self.indent -= 4
                    self.file.write('%s</%s>\n' % (' ' * self.indent, name))
            self.indent -= 4
            self.file.write('%s</v%s>\n' % (' ' * self.indent, attrb))

        self.indent -= 4

class ProcessGrcFile(object):
    def __init__(self, filename):
        """Process GRC file"""
        self.filename = filename

        # elements
        self.inGroup = False
        self.inRaster = False
        self.inVector = False

        # list of layers
        self.layers = []

        # error message
        self.error = ''
        self.num_error = 0

    def read(self, parent):
        """Read GRC file

        @param parent parent window

        @return list of map layers
        """
        try:
            file = open(self.filename, "r")
        except IOError:
            wx.MessageBox(parent=parent,
                          message=_("Unable to open file <%s> for reading.") % self.filename,
                          caption=_("Error"), style=wx.OK | wx.ICON_ERROR)
            return []

        line_id = 1
        for line in file.readlines():
            self.process_line(line.rstrip('\n'), line_id)
            line_id +=1

        file.close()

        if self.num_error > 0:
            wx.MessageBox(parent=parent,
                          message=_("Some lines were skipped when reading settings "
                                    "from file <%(file)s>.\nSee 'Command output' window for details.\n\n"
                                    "Number of skipped lines: %(line)d") % \
                                        { 'file' : self.filename, 'line' : self.num_error },
                          caption=_("Warning"), style=wx.OK | wx.ICON_EXCLAMATION)
            parent.goutput.WriteLog('Map layers loaded from GRC file <%s>' % self.filename)
            parent.goutput.WriteLog('Skipped lines:\n%s' % self.error)

        return self.layers

    def process_line(self, line, line_id):
        """Process line definition"""
        element = self._get_element(line)
        if element == 'Group':
            self.groupName = self._get_value(line)
            self.layers.append({
                    "type"    : 'group',
                    "name"    : self.groupName,
                    "checked" : None,
                    "opacity" : None,
                    "cmd"     : None,
                    "group"   : self.inGroup,
                    "display" : 0 })
            self.inGroup = True

        elif element == '_check':
            if int(self._get_value(line)) ==  1:
                self.layers[-1]['checked'] = True
            else:
                self.layers[-1]['checked'] = False
            
        elif element == 'End':
            if self.inRaster:
                self.inRaster = False
            elif self.inVector:
                self.inVector = False
            elif self.inGroup:
                self.inGroup = False
            elif self.inGridline:
                self.inGridline = False
        
        elif element == 'opacity':
            self.layers[-1]['opacity'] = float(self._get_value(line))

        # raster
        elif element == 'Raster':
            self.inRaster = True
            self.layers.append({
                    "type"    : 'raster',
                    "name"    : self._get_value(line),
                    "checked" : None,
                    "opacity" : None,
                    "cmd"     : ['d.rast'],
                    "group"   : self.inGroup,
                    "display" : 0})

        elif element == 'map' and self.inRaster:
            self.layers[-1]['cmd'].append('map=%s' % self._get_value(line))
            
        elif element == 'overlay' and self.inRaster:
            if int(self._get_value(line)) == 1:
                self.layers[-1]['cmd'].append('-o')
            
        elif element == 'rastquery' and self.inRaster:
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('catlist=%s' % value)
            
        elif element == 'bkcolor' and self.inRaster:
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('bg=%s' % value)

        # vector
        elif element == 'Vector':
            self.inVector = True
            self.layers.append({
                    "type"    : 'vector',
                    "name"    : self._get_value(line),
                    "checked" : None,
                    "opacity" : None,
                    "cmd"     : ['d.vect'],
                    "group"   : self.inGroup,
                    "display" : 0})

        elif element == 'vect' and self.inVector:
            self.layers[-1]['cmd'].append('map=%s' % self._get_value(line))
                
        elif element in ('display_shape',
                         'display_cat',
                         'display_topo',
                         'display_dir',
                         'display_attr',
                         'type_point',
                         'type_line',
                         'type_boundary',
                         'type_centroid',
                         'type_area',
                         'type_face') and self.inVector:
            
            if int(self._get_value(line)) == 1:
                name = element.split('_')[0]
                type = element.split('_')[1]
                paramId = self._get_cmd_param_index(self.layers[-1]['cmd'], name)
                if paramId == -1:
                    self.layers[-1]['cmd'].append('%s=%s' % (name, type))
                else:
                    self.layers[-1]['cmd'][paramId] += ',%s' % type

        elif element in ('color',
                         'fcolor',
                         'lcolor') and self.inVector:
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('%s=%s' % (element,
                                                         self._color_name_to_rgb(value)))

        elif element == 'rdmcolor' and self.inVector:
            if int(self._get_value(line)) == 1:
                self.layers[-1]['cmd'].append('-c')

        elif element == 'sqlcolor' and self.inVector:
            if int(self._get_value(line)) == 1:
                self.layers[-1]['cmd'].append('-a')

        elif element in ('icon',
                         'size',
                         'layer',
                         'xref',
                         'yref',
                         'lsize',
                         'where',
                         'minreg',
                         'maxreg') and self.inVector:
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('%s=%s' % (element,
                                                         value))
        
        elif element == 'lwidth':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('width=%s' % value)

        elif element == 'lfield':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('llayer=%s' % value)
                                        
        elif element == 'attribute':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('attrcol=%s' % value)

        elif element == 'cat':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('cats=%s' % value)

        # gridline
        elif element == 'gridline':
            self.inGridline = True
            self.layers.append({
                    "type"    : 'grid',
                    "name"    : self._get_value(line),
                    "checked" : None,
                    "opacity" : None,
                    "cmd"     : ['d.grid'],
                    "group"   : self.inGroup,
                    "display" : 0})

        elif element == 'gridcolor':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('color=%s' % self._color_name_to_rgb(value))

        elif element == 'gridborder':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('bordercolor=%s' % self._color_name_to_rgb(value))

        elif element == 'textcolor':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('textcolor=%s' % self._color_name_to_rgb(value))

        elif element in ('gridsize',
                         'gridorigin'):
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('%s=%s' % (element[4:], value))

        elif element in 'fontsize':
            value = self._get_value(line)
            if value != '':
                self.layers[-1]['cmd'].append('%s=%s' % (element, value))
        
        elif element == 'griddraw':
            value = self._get_value(line)
            if value == '0':
                self.layers[-1]['cmd'].append('-n')
                
        elif element == 'gridgeo':
            value = self._get_value(line)
            if value == '1':
                self.layers[-1]['cmd'].append('-g')
        
        elif element == 'borderdraw':
            value = self._get_value(line)
            if value == '0':
                self.layers[-1]['cmd'].append('-b')

        elif element == 'textdraw':
            value = self._get_value(line)
            if value == '0':
                self.layers[-1]['cmd'].append('-t')
        
        else:
            self.error += _(' row %d:') % line_id + line + os.linesep
            self.num_error += 1

    def _get_value(self, line):
        """Get value of element"""
        try:
            return line.strip(' ').split(' ')[1].strip(' ')
        except:
            return ''

    def _get_element(self, line):
        """Get element tag"""
        return line.strip(' ').split(' ')[0].strip(' ')

    def _get_cmd_param_index(self, cmd, name):
        """Get index of parameter in cmd list

        @param cmd cmd list
        @param name parameter name

        @return index
        @return -1 if not found
        """
        i = 0
        for param in cmd:
            if '=' not in param:
                i += 1
                continue
            if param.split('=')[0] == name:
                return i

            i += 1

        return -1

    def _color_name_to_rgb(self, value):
        """Convert color name (#) to rgb values"""
        col = wx.NamedColour(value)
        return str(col.Red()) + ':' + \
            str(col.Green()) + ':' + \
            str(col.Blue())
