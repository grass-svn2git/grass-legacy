"""
MODULE:    global.py

PURPOSE:   Global variables

           This module provide the space for global variables
           used in the code.

AUTHOR(S): GRASS Development Team
           Martin Landa <landa.martin gmail.com>

COPYRIGHT: (C) 2007 by the GRASS Development Team

           This program is free software under the GNU General Public
           License (>=v2). Read the file COPYING that comes with GRASS
           for details.
"""

import os

### recursive import problem 
# import utils
# utils.CheckForWx()
import wx
import wx.lib.flatnotebook as FN

try:
    import subprocess
except:
    compatPath = os.path.join(globalvar.ETCWXDIR, "compat")
    sys.path.append(compatPath)
    import subprocess

"""
Query layer (generated for example by selecting item in the Attribute Table Manager)
Deleted automatically on re-render action
"""
# temporal query layer (removed on re-render action)
QUERYLAYER = 'qlayer'

# path to python scripts
ETCDIR = os.path.join(os.getenv("GISBASE"), "etc")
ETCWXDIR = os.path.join(ETCDIR, "wxpython")

"""Style definition for FlatNotebook pages"""
FNPageStyle = FN.FNB_VC8 | \
    FN.FNB_BACKGROUND_GRADIENT | \
    FN.FNB_NODRAG | \
    FN.FNB_TABS_BORDER_SIMPLE 
FNPageColor = wx.Colour(125,200,175)

if subprocess.mswindows:
    EXT_BIN = '.exe'
    EXT_SCT = '.bat'
else:
    EXT_BIN = ''
    EXT_SCT = ''
