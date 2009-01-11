"""
Original GRASS icon set from old TCL/TK GUI
"""

import os

import wx

import globalvar

iconpath = os.path.join(globalvar.ETCDIR, "gui", "icons", "grass")
iconpath_vdigit  = os.path.join(globalvar.ETCDIR, "gui", "icons", "grass", "vdigit")

IconsGrass = {
    # map display
    "displaymap" : 'gui-display.gif',
    "rendermap"  : 'gui-redraw.gif',
    "erase"      : 'gui-erase.gif',
    "pointer"    : 'gui-pointer.gif',
    "zoom_in"    : 'gui-zoom_in.gif',
    "zoom_out"   : 'gui-zoom_out.gif',
    "pan"        : 'gui-pan.gif',
    "query"      : 'gui-query.gif',
    "zoom_back"  : 'gui-zoom_back.gif',
    "zoommenu"   : 'gui-mapzoom.gif',
    "savefile"   : 'file-save.gif',
    "printmap"   : 'file-print.gif',
    "overlay"    : 'gui-overlay.gif',
    # digit
    ## add feature
    "digAddPoint": 'new.point.gif',
    "digAddLine" : 'new.line.gif',
    "digAddBoundary": 'new.boundary.gif',
    "digAddCentroid": 'new.centroid.gif',
    ## vertex
    "digAddVertex" : 'add.vertex.gif',
    "digMoveVertex" : 'move.vertex.gif',
    "digRemoveVertex" : 'rm.vertex.gif',
    "digSplitLine" : 'split.line.gif',
    ## edit feature
    "digEditLine" : 'edit.line.gif',
    "digMoveLine" : 'move.line.gif',
    "digDeleteLine" : 'delete.line.gif',
    ## cats
    "digCopyCats" : 'copy.cats.gif',
    "digDispCats" : 'display.cats.gif',
    ## attributes
    "digDispAttr" : 'display.attributes.gif',
    ## general
    "digUndo" : wx.ART_ERROR, # FIXME
    "digSettings" : 'settings.gif',
    "digAdditionalTools" : wx.ART_ERROR, # FIXME
    # layer manager
    "newdisplay" : 'gui-startmon.gif',
    "workspaceNew" : 'file-new.gif',
    "workspaceLoad" : 'file-new.gif', # change the icon if possible
    "workspaceOpen" : 'file-open.gif',
    "workspaceSave" : 'file-save.gif',
    "addrast"    : 'element-cell.gif',
    "addrast3d"  : 'element-grid3.gif',
    "addvect"    : 'element-vector.gif',
    "addcmd"     : 'gui-cmd.gif',
    "addgrp"     : 'gui-group.gif',
    "addovl"     : 'module-d.grid.gif',
    "delcmd"     : 'edit-cut.gif',
    "attrtable"  : 'db-values.gif',
    "addrgb"     : 'module-d.rgb.gif',
    "addhis"     : 'channel-his.gif',
    "addshaded"  : 'module-d.shadedmap.gif',
    "addrarrow"  : 'module-d.rast.arrow.gif',
    "addrnum"    : 'module-d.rast.num.gif',
    "addthematic": 'module-d.vect.thematic.gif',
    "addchart"   : 'module-d.vect.chart.gif',
    "addgrid"    : 'module-d.grid.gif',
    "addgeodesic": 'module-d.geodesic.gif',
    "addrhumb"   : 'module-d.rhumbline.gif',
    "addlabels"  : 'module-d.labels.gif',
    "addtext"    : 'module-d.text.gif',
    "addbarscale": 'module-d.barscale.gif',
    "addlegend"  : 'module-d.legend.gif',
    "quit"       : 'gui-exit.gif',
    # analyze raster
    "analyze"    : 'gui-rastanalyze.gif',
    "measure"    : 'gui-measure.gif',
    "font"       : 'gui-font.gif',
    "histogram"  : 'module-d.histogram.gif',
    "color"      : 'edit-color.gif',
    "layeropts"  : 'gui-layeroptions.gif',
    # profile 
    "profile"    : 'gui-profile.gif',
    "transect"   : 'gui-profiledefine.gif',
    # "profiledraw": 'gui-profiledraw.gif',
    "profiledraw" : 'gui-display.gif',
    "profileopt" : 'gui-profileopt.gif',
    # georectify
    'grGcpClear'   : 'gui-gcperase.gif',
    'grGcpSet'     : 'gui-gcpset.gif',
    'grGeorect'    : 'gui-georect.gif',
    'grGcpRms'     : 'gui-rms.gif',
    'grGcpRefresh' : 'gui-display.gif',
    "grGcpSave"    : 'file-save.gif', 
    "grGcpAdd"     : wx.ART_NEW, # FIXME
    "grGcpDelete"  : wx.ART_DELETE, # FIXME
    "grGcpReload"  : 'gui-redraw.gif',
    "grSettings"   : 'edit-color.gif', 
    # nviz 
    "nvizSettings" : 'edit-color.gif',   
    }
