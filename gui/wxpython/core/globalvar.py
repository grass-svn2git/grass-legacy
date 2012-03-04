"""!
@package core.globalvar

@brief Global variables used by wxGUI

(C) 2007-2011 by the GRASS Development Team

This program is free software under the GNU General Public License
(>=v2). Read the file COPYING that comes with GRASS for details.

@author Martin Landa <landa.martin gmail.com>
"""

import os
import sys
import locale

if not os.getenv("GISBASE"):
    sys.exit("GRASS is not running. Exiting...")

# path to python scripts
ETCDIR = os.path.join(os.getenv("GISBASE"), "etc")
ETCICONDIR = os.path.join(os.getenv("GISBASE"), "etc", "gui", "icons")
ETCWXDIR = os.path.join(ETCDIR, "wxpython")
ETCIMGDIR = os.path.join(ETCDIR, "gui", "images")
ETCSYMBOLDIR = os.path.join(ETCDIR, "gui", "images", "symbols")

sys.path.append(os.path.join(ETCDIR, "python"))
import grass.script as grass

def CheckWxVersion(version = [2, 8, 11, 0]):
    """!Check wx version"""
    ver = wx.version().split(' ')[0]
    if map(int, ver.split('.')) < version:
        return False
    
    return True

def CheckForWx():
    """!Try to import wx module and check its version"""
    if 'wx' in sys.modules.keys():
        return
    
    minVersion = [2, 8, 1, 1]
    try:
        try:
            import wxversion
        except ImportError, e:
            raise ImportError(e)
        # wxversion.select(str(minVersion[0]) + '.' + str(minVersion[1]))
        wxversion.ensureMinimal(str(minVersion[0]) + '.' + str(minVersion[1]))
        import wx
        version = wx.version().split(' ')[0]
        
        if map(int, version.split('.')) < minVersion:
            raise ValueError('Your wxPython version is %s.%s.%s.%s' % tuple(version.split('.')))

    except ImportError, e:
        print >> sys.stderr, 'ERROR: wxGUI requires wxPython. %s' % str(e)
        sys.exit(1)
    except (ValueError, wxversion.VersionError), e:
        print >> sys.stderr, 'ERROR: wxGUI requires wxPython >= %d.%d.%d.%d. ' % tuple(minVersion) + \
            '%s.' % (str(e))
        sys.exit(1)
    except locale.Error, e:
        print >> sys.stderr, "Unable to set locale:", e
        os.environ['LC_ALL'] = ''
    
if not os.getenv("GRASS_WXBUNDLED"):
    CheckForWx()
import wx
import wx.lib.flatnotebook as FN

"""
Query layer (generated for example by selecting item in the Attribute Table Manager)
Deleted automatically on re-render action
"""
# temporal query layer (removed on re-render action)
QUERYLAYER = 'qlayer'

"""!Style definition for FlatNotebook pages"""
FNPageStyle = FN.FNB_VC8 | \
    FN.FNB_BACKGROUND_GRADIENT | \
    FN.FNB_NODRAG | \
    FN.FNB_TABS_BORDER_SIMPLE 

FNPageDStyle = FN.FNB_FANCY_TABS | \
    FN.FNB_BOTTOM | \
    FN.FNB_NO_NAV_BUTTONS | \
    FN.FNB_NO_X_BUTTON

FNPageColor = wx.Colour(125,200,175)

"""!Dialog widget dimension"""
DIALOG_SPIN_SIZE = (150, -1)
DIALOG_COMBOBOX_SIZE = (300, -1)
DIALOG_GSELECT_SIZE = (400, -1)
DIALOG_TEXTCTRL_SIZE = (400, -1)
DIALOG_LAYER_SIZE = (100, -1)
DIALOG_COLOR_SIZE = (30, 30)

MAP_WINDOW_SIZE = (800, 600)
GM_WINDOW_SIZE = (500, 600)

def GetGRASSCommands():
    """!Create list of available GRASS commands to use when parsing
    string from the command line

    @return list of commands (set) and directory of scripts (collected
    by extension - MS Windows only)
    """
    gisbase = os.environ['GISBASE']
    cmd = list()
    if sys.platform == 'win32':
        scripts = { '.bat' : list(),
                    '.py'  : list()
                    }
    else:
        scripts = {}
    
    # scan bin/
    if os.path.exists(os.path.join(gisbase, 'bin')):
        for fname in os.listdir(os.path.join(gisbase, 'bin')):
            if scripts: # win32
                name, ext = os.path.splitext(fname)
                if ext != '.manifest':
                    cmd.append(name)
                if ext in scripts.keys():
                    scripts[ext].append(name)
            else:
                cmd.append(fname)
    
    # scan scripts/ (not on MS Windows)
    if not scripts and os.path.exists(os.path.join(gisbase, 'scripts')):
        for fname in os.listdir(os.path.join(gisbase, 'scripts')):
            cmd.append(fname)
    
    # scan gui/scripts/
    if os.path.exists(os.path.join(gisbase, 'etc', 'gui', 'scripts')):
        os.environ["PATH"] = os.getenv("PATH") + os.pathsep + os.path.join(gisbase, 'etc', 'gui', 'scripts')
        os.environ["PATH"] = os.getenv("PATH") + os.pathsep + os.path.join(gisbase, 'etc', 'wxpython', 'scripts')
        
        pattern = "_wrapper"
        for script in os.listdir(os.path.join(gisbase, 'etc', 'gui', 'scripts')):
            if script[-len(pattern):] != pattern: # ignore wrappers
                cmd.append(script)
    
    # scan addons (path)
    if os.getenv('GRASS_ADDON_PATH'):
        for path in os.getenv('GRASS_ADDON_PATH').split(os.pathsep):
            if not os.path.exists(path) or not os.path.isdir(path):
                continue
            for fname in os.listdir(path):
                if scripts: # win32
                    name, ext = os.path.splitext(fname)
                    cmd.append(name)
                    if ext in scripts.keys():
                        scripts[ext].append(name)
                else:
                    cmd.append(fname)
    
    return set(cmd), scripts

"""@brief Collected GRASS-relared binaries/scripts"""
grassCmd, grassScripts = GetGRASSCommands()

"""@Toolbar icon size"""
toolbarSize = (24, 24)

"""@Is g.mlist available?"""
if 'g.mlist' in grassCmd:
    have_mlist = True
else:
    have_mlist = False

"""@Check version of wxPython, use agwStyle for 2.8.11+"""
hasAgw = CheckWxVersion()
