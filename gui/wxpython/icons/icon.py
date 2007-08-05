"""
MODULE: icon

CLASSES:
 * MetaIcon

PURPOSE: Icon themes

         from icons import Icons as Icons

AUTHORS: The GRASS Development Team
         Martin Landa <landa.martin gmail.com>

COPYRIGHT: (C) 2007 by the GRASS Development Team
           This program is free software under the GNU General Public
           License (>=v2). Read the file COPYING that comes with GRASS
           for details.
"""

import os
import wx

iconpath_default = os.path.join(os.getenv("GISBASE"), "etc", "gui", "icons")
iconpath_vdigit  = os.path.join(os.getenv("GISBASE"), "etc", "v.digit")
iconpath = os.getenv("GRASS_ICONPATH")

icons_default = {
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
    "digAddPoint": 'new.point.gif',
    "digAddLine" : 'new.line.gif',
    "digAddBoundary": 'new.boundary.gif',
    "digAddCentroid": 'new.centroid.gif',
    "digAddVertex" : 'add.vertex.gif',
    "digCopyCats" : 'copy.cats.gif',
    "digDeleteLine" : 'delete.line.gif',
    "digDispAttr" : 'display.attributes.gif',
    "digDispCats" : 'display.cats.gif',
    "digEditLine" : 'edit.line.gif',
    "digMoveLine" : 'move.line.gif',
    "digMoveVertex" : 'move.vertex.gif',
    "digRemoveVertex" : 'rm.vertex.gif',
    "digSettings" : 'settings.gif',
    "digSplitLine" : 'split.line.gif',
    "digExit"    : 'exit.gif',
    # gis manager
    "newdisplay" : 'gui-startmon.gif',
    "addrast"    : 'element-cell.gif',
    "addvect"    : 'element-vector.gif',
    "addcmd"     : 'gui-cmd.gif',
    "addgrp"     : wx.ART_ERROR,
    "addovl"     : 'module-d.grid.gif',
    "delcmd"     : wx.ART_ERROR,
    "attrtable"  : 'db-values.gif',
    "addrgb"     : 'module-d.rgb.gif',
    "addhis"     : 'channel-his.gif',
    "addshaded"  : 'module-d.shadedmap.gif',
    "addrarrow"  : 'module-d.rast.arrow.gif',
    "addrnum"    : 'module-d.rast.num.gif',
    "elvect"     : 'element-vector.gif',
    "addthematic": 'module-d.vect.thematic.gif',
    "addchart"   : 'module-d.vect.chart.gif',
    "addgrid"    : 'module-d.grid.gif',
    "addgeodesic": 'module-d.geodesic.gif',
    "addrhumb"   : 'module-d.rhumbline.gif',
    "addlabels"  : 'module-d.labels.gif',
    "addtext"    : 'module-d.text.gif',
    "addbarscale": 'module-d.barscale.gif',
    "addlegend"  : 'module-d.legend.gif',
    "quit"       : wx.ART_ERROR,
    # analyze raster
    "analyze"    : 'gui-rastanalyze.gif',
    "measure"    : 'gui-measure.gif',
    "font"       : 'gui-font.gif',
    "histogram"  : 'module-d.histogram.gif',
    "color"      : 'edit-color.gif',
    "options"    : wx.ART_ERROR,
    "profile"    : 'gui-profile.gif',
    "transect"   : 'gui-profiledefine.gif',
    "profiledraw": 'gui-profiledraw.gif',
    "profileopt" : 'gui-profileopt.gif',
    "analyze"    : 'gui-rastanalyze.gif', # TODO: fix?
    }

# merge icons dictionaries, join paths
if iconpath and iconpath.find('silk') > -1: # silk icon theme
    from silk import IconsSilk as icons_img
    # use default icons if needed
    for key, img in icons_default.iteritems():
        if not icons_img.has_key(key): # add key
            icons_img[key] = img
            if key[0:3] == 'dig':
                iconpath_tmp = iconpath_vdigit
            else:
                iconpath_tmp = iconpath_default
        else:
            iconpath_tmp = iconpath

        if icons_img[key]: # join paths
            if type (icons_img[key]) == type(''):
                icons_img[key] = os.path.join(iconpath_tmp, icons_img[key])
else: # default icons
    icons_img = icons_default
    for key, img in icons_img.iteritems():
        if img and type (icons_img[key]) == type(''):
            if key[0:3] == 'dig':
                icons_img[key] = os.path.join(iconpath_vdigit, img)
            else:
                icons_img[key] = os.path.join(iconpath_default, img)

class MetaIcon:
    """
    Handle icon metadata (image path, tooltip, ...)
    """
    def __init__(self, img, label, desc=None):
        self.imagepath = img
        if not self.imagepath:
            self.type = 'unknown'
        else:
            if self.imagepath.find ('wxART_') > -1:
                self.type = 'wx'
            else:
                self.type = 'img'

        self.label = label

        if desc:
            self.description = desc
        else:
            self.description = ''

    def __str__(self):
        """Debugging"""
        return "label=%s, img=%s, type=%s" % (self.label, self.imagepath, self.type)

    def GetBitmap (self, size=None):
        """Get bitmap"""
        bmp = None

        if self.type == 'wx':
            bmp = wx.ArtProvider.GetBitmap(id=self.imagepath, client=wx.ART_TOOLBAR, size=size)
        elif self.type == 'img':
            if os.path.isfile(self.imagepath) and os.path.getsize(self.imagepath):
                if size and len(size) == 2:
                    image = wx.Image (name=self.imagepath)
                    image.Rescale (size[0], size[1])
                    bmp = image.ConvertToBitmap()
                elif self.imagepath:
                    bmp = wx.Bitmap (name=self.imagepath)

        return bmp

    def GetLabel (self):
        return self.label

    def GetDesc (self):
        return self.description

#
# create list of icon instances
#
Icons = {
    # map display
    "displaymap" : MetaIcon (img=icons_img["displaymap"], label="Display map"),
    "rendermap"  : MetaIcon (img=icons_img["rendermap"], label="Re-render map",
                             desc="Force re-rendering of all layers"),
    "erase"      : MetaIcon (img=icons_img["erase"], label="Erase display"),
    "pointer"    : MetaIcon (img=icons_img["pointer"], label="Pointer"),
    "zoom_in"    : MetaIcon (img=icons_img["zoom_in"], label="Zoom in",
                             desc="Drag or click mouse to zoom"),
    "zoom_out"   : MetaIcon (img=icons_img["zoom_out"], label="Zoom out",
                             desc="Drag or click mouse to unzoom"),
    "pan"        : MetaIcon (img=icons_img["pan"], label="Pan",
                             desc="Drag with mouse to pan"),
    "query"      : MetaIcon (img=icons_img["query"], label="Query",
                             desc="Query selected map"),
    "zoom_back"  : MetaIcon (img=icons_img["zoom_back"], label="Return to previous zoom"),
    "zoommenu"   : MetaIcon (img=icons_img["zoommenu"], label="Zoom options",
                             desc="Display zoom management"),
    "overlay"    : MetaIcon (img=icons_img["overlay"], label="Add overlay",
                             desc="Add graphic overlays to map"),
    "addbarscale": MetaIcon (img=icons_img["addbarscale"], label="Add scalebar and north arrow"),
    "addlegend"  : MetaIcon (img=icons_img["addlegend"], label="Add legend"),
    "savefile"   : MetaIcon (img=icons_img["savefile"], label="Save display to PNG file"),
    "printmap"   : MetaIcon (img=icons_img["printmap"], label="Print display"),
    # gis manager
    "newdisplay" : MetaIcon (img=icons_img["newdisplay"], label="Start new display"),
    "addrast"    : MetaIcon (img=icons_img["addrast"], label="Add raster map layer"),
    "addvect"    : MetaIcon (img=icons_img["addvect"], label="Add vector map layer"),
    "addcmd"     : MetaIcon (img=icons_img["addcmd"], label="Add command layer"),
    "addgrp"     : MetaIcon (img=icons_img["addgrp"], label="Add layer group"),
    "addovl"     : MetaIcon (img=icons_img["addovl"], label="Add grid or vector labels overlay"),
    "delcmd"     : MetaIcon (img=icons_img["delcmd"], label="Delete selected layer"),
    "quit"       : MetaIcon (img=icons_img["quit"], label="Quit"),
    "attrtable"  : MetaIcon (img=icons_img["attrtable"], label="Show attribute table"),
    "addrgb"     : MetaIcon (img=icons_img["addrgb"], label="Add RGB layer"),
    "addhis"     : MetaIcon (img=icons_img["addhis"], label="Add HIS layer"),
    "addshaded"  : MetaIcon (img=icons_img["addshaded"], label="Add shaded relief map layer"),
    "addrarrow"  : MetaIcon (img=icons_img["addrarrow"], label="Add raster flow arrows"),
    "addrnum"    : MetaIcon (img=icons_img["addrnum"], label="Add raster cell numbers"),
    "elvect"     : MetaIcon (img=icons_img["elvect"], label=""),
    "addthematic": MetaIcon (img=icons_img["addthematic"], label="Add thematic layer"),
    "addchart"   : MetaIcon (img=icons_img["addchart"], label="Add thematic chart layer"),
    "addgrid"    : MetaIcon (img=icons_img["addgrid"], label="Add grid layer"),
    "addgeodesic": MetaIcon (img=icons_img["addgeodesic"], label="Add geodesic line layer"),
    "addrhumb"   : MetaIcon (img=icons_img["addrhumb"], label="Add rhumbline layer"),
    "addlabels"  : MetaIcon (img=icons_img["addlabels"], label="Add labels"),
    "addtext"    : MetaIcon (img=icons_img["addtext"], label="Add text layer"),
    # digit
    "digAddPoint": MetaIcon (img=icons_img["digAddPoint"], label="Digitize new point",
                             desc="Left: new point"),
    "digAddLine" : MetaIcon (img=icons_img["digAddLine"], label="Digitize new line",
                             desc="Left: new point; Middle: undo last point; Right: close line"),
    "digAddBoundary": MetaIcon (img=icons_img["digAddBoundary"], label="Digitize new boundary",
                             desc="Left: new point; Middle: undo last point; Right: close line"),
    "digAddCentroid": MetaIcon (img=icons_img["digAddCentroid"], label="Digitize new centroid",
                             desc="Left: new point"),
    "digAddVertex": MetaIcon (img=icons_img["digAddVertex"], label="Add new vertex",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digCopyCats": MetaIcon (img=icons_img["digCopyCats"], label="Copy categories",
                              desc="Not implemented yet"),
    "digDeleteLine": MetaIcon (img=icons_img["digDeleteLine"], label="Delete line",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digDispAttr": MetaIcon (img=icons_img["digDispAttr"], label="Display attributes",
                              desc="Display attributes of given feature"),
    "digDispCats": MetaIcon (img=icons_img["digDispCats"], label="Display categories",
                              desc="Not implemented yet"),
    "digEditLine": MetaIcon (img=icons_img["digEditLine"], label="Edit line",
                              desc="Not implemented yet"),
    "digMoveLine": MetaIcon (img=icons_img["digMoveLine"], label="Move line",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digMoveVertex": MetaIcon (img=icons_img["digMoveVertex"], label="Move vertex",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digRemoveVertex": MetaIcon (img=icons_img["digRemoveVertex"], label="Remove vertex",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digSettings": MetaIcon (img=icons_img["digSettings"], label="Settings",
                              desc="Settings dialog for digitization tool"),
    "digSplitLine": MetaIcon (img=icons_img["digSplitLine"], label="Split line",
                              desc="Left: Select; Middle: Unselect; Right: Confirm"),
    "digExit"    : MetaIcon (img=icons_img["digExit"], label="Quit digitization tool"),
    # analyze raster
    "analyze"    : MetaIcon (img=icons_img["analyze"], label="Analyze map"),
    "measure"    : MetaIcon (img=icons_img["measure"], label="Measure distance"),
    "transect"   : MetaIcon (img=icons_img["transect"], label="Draw transect to profile"),
    "profile"    : MetaIcon (img=icons_img["profile"], label="Profile surface map"),
    "profiledraw": MetaIcon (img=icons_img["profiledraw"], label="Create profile"),
    "profileopt" : MetaIcon (img=icons_img["profileopt"], label="Profile options"),
    "histogram"  : MetaIcon (img=icons_img["histogram"], label="Create histogram of image or raster file"),
    "font"       : MetaIcon (img=icons_img["font"], label="Select font"),
    "color"      : MetaIcon (img=icons_img["color"], label="Select color"),
    "options"    : MetaIcon (img=icons_img["options"], label="Set histogram options"),
    "analyze"    : MetaIcon (img=icons_img["analyze"], label="Analyze")}

# testing ...
if __name__ == "__main__":
    for k,v in Icons.iteritems():
        print k, "/", v

