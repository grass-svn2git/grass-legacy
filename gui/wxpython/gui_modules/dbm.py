"""
Database browser for GRASS GIS >= 7

This program is based on FileHunter, publicated in "The wxPython Linux
Tutorial" on wxPython WIKI pages.

It also uses some functions from http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/426407

Usage:
    dbm.py table_name

"""
############################################################################
#
# MODULE:       dbm.py
# AUTHOR(S):    Jachym Cepicky <jachym les-ejk cz>
# PURPOSE:      GRASS attribute table manager
# COPYRIGHT:    (C) 2007 by the GRASS Development Team
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################
import wx
import wx.lib.mixins.listctrl  as  listmix

import sys,os
        
#----------------------------------------------------------------------
class Log:
    r"""\brief Needed by the wxdemos.
    The log output is redirected to the status bar of the containing frame.
    """
    def __init__(self,parent):
        self.parent = parent

    def write(self,text_string):
        self.parent.SetStatusText(text_string.strip())

#----------------------------------------------------------------------
# The panel you want to test (TestVirtualList)
#----------------------------------------------------------------------

class TestVirtualList(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin, listmix.ColumnSorterMixin):
    def __init__(self, parent,log,tablename):
        wx.ListCtrl.__init__( self, parent, -1, style=wx.LC_REPORT|wx.LC_VIRTUAL|wx.LC_HRULES|wx.LC_VRULES)

        self.log=log
        self.tablename = tablename
        self.columns = []
        self.columnNumber = 0
        self.parent = parent

        #adding some attributes (colourful background for each item rows)
        self.attr1 = wx.ListItemAttr()
        self.attr1.SetBackgroundColour("light blue")
        self.attr2 = wx.ListItemAttr()
        self.attr2.SetBackgroundColour("white")

        #building the columns
        i = 0
        # FIXME: subprocess.Popen should be used
        # FIXME: Maximal number of columns, when the GUI is still usable
        for line in os.popen("db.describe -c table=%s" %
                (self.tablename)).readlines()[1:]:

            x,column,type = line.strip().split(":")
            # FIXME: here will be more types
            if type.lower().find("integer") > -1:
                self.columns.append({"name":column,"type":int})
            elif type.lower().find("double") > -1:
                self.columns.append({"name":column,"type":float})
            elif type.lower().find("float") > -1:
                self.columns.append({"name":column,"type":float})
            else:
                self.columns.append({"name":column,"type":str})

            self.InsertColumn(i, column)
            self.SetColumnWidth(i, 50)
            i += 1
            if i >= 256:
                self.log.write("Can display only 256 columns")
                break

        #These two should probably be passed to init more cleanly
        #setting the numbers of items = number of elements in the dictionary
        self.itemDataMap = {}
        self.itemIndexMap = []
        # FIXME: subprocess.Popen should be used
        # FIXME: Max. number of rows, while the GUI is still usable
        i = 0
        for line in os.popen("""db.select -c sql="SELECT * FROM %s" """ % self.tablename):
            attributes = line.strip().split("|")
            self.itemDataMap[i] = []
            for attribute in attributes:
                self.itemDataMap[i].append(attribute)
                self.itemIndexMap.append(i)
            i += 1
            if i >= 32000:
                self.log.write("Can display only 32000 lines")
                break

        self.SetItemCount(len(self.itemDataMap))
        
        #mixins
        listmix.ListCtrlAutoWidthMixin.__init__(self)
        listmix.ColumnSorterMixin.__init__(self, 3)

        #sort by genre (column 2), A->Z ascending order (1)
        self.SortListItems(0, 1)

        #events
        self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnItemSelected)
        self.Bind(wx.EVT_LIST_ITEM_ACTIVATED, self.OnItemActivated)
        self.Bind(wx.EVT_LIST_ITEM_DESELECTED, self.OnItemDeselected)
        self.Bind(wx.EVT_LIST_COL_CLICK, self.OnColClick)

    def OnColClick(self,event):
        self.columnNumber = event.GetColumn()
        event.Skip()

    def OnItemSelected(self, event):
        self.currentItem = event.m_itemIndex
        self.log.write('OnItemSelected: "%s", "%s"\n' %
                           (self.currentItem,
                            self.GetItemText(self.currentItem)))

        print self.parent.gismanager
        if self.parent.gismanager:
            gism = self.parent.gismanager
            curr_pg = gism.gm_cb.GetCurrentPage()
            disp_idx = gism.track.Track().GetDisp_idx(curr_pg)

            print self.parent.gismanager.mapdisplays#[self.parent.gismanager.disp_idx]
            print self.parent.gismanager.disp_idx

    def OnItemActivated(self, event):
        self.currentItem = event.m_itemIndex
        self.log.write("OnItemActivated: %s\nTopItem: %s\n" %
                           (self.GetItemText(self.currentItem), self.GetTopItem()))

    def getColumnText(self, index, col):
        item = self.GetItem(index, col)
        return item.GetText()

    def OnItemDeselected(self, evt):
        self.log.write("OnItemDeselected: %s" % evt.m_itemIndex)


    #---------------------------------------------------
    # These methods are callbacks for implementing the
    # "virtualness" of the list...

    def OnGetItemText(self, item, col):
        index=self.itemIndexMap[item]
        s = self.itemDataMap[index][col]
        return s

    # def OnGetItemImage(self, item):
    #     index=self.itemIndexMap[item]
    #     if ( index % 2) == 0:

    def OnGetItemAttr(self, item):
        index=self.itemIndexMap[item]

        return self.attr2
        #if ( index % 2) == 0:
        #    return self.attr2
        #else:
        #    return self.attr1

    #---------------------------------------------------
    # Matt C, 2006/02/22
    # Here's a better SortItems() method --
    # the ColumnSorterMixin.__ColumnSorter() method already handles the ascending/descending,
    # and it knows to sort on another column if the chosen columns have the same value.

    def SortItems(self,sorter=cmp):
        items = list(self.itemDataMap.keys())
        # for i in range(len(items)):
        #     items[i] =  self.columns[self.columnNumber]["type"](items[i])
        items.sort(self.Sorter)
        #items.sort(sorter)
        # for i in range(len(items)):
        #     items[i] =  str(items[i])
        self.itemIndexMap = items
        
        # redraw the list
        self.Refresh()

    # Used by the ColumnSorterMixin, see wx/lib/mixins/listctrl.py
    def GetListCtrl(self):
        return self

    # stolen from python2.4/site-packages/wx-2.8-gtk2-unicode/wx/lib/mixins/listctrl.py
    def Sorter(self, key1,key2):
         col = self._col
         ascending = self._colSortFlag[col]
         # convert, because the it is allways string
         item1 = self.columns[col]["type"](self.itemDataMap[key1][col])
         item2 = self.columns[col]["type"](self.itemDataMap[key2][col])

         #--- Internationalization of string sorting with locale module
         if type(item1) == type('') or type(item2) == type(''):
             cmpVal = locale.strcoll(str(item1), str(item2))
         else:
             cmpVal = cmp(item1, item2)
         #---

         # If the items are equal then pick something else to make the sort v    ->alue unique
         if cmpVal == 0:
             cmpVal = apply(cmp, self.GetSecondarySortValues(col, key1, key2))

         if ascending:
             return cmpVal
         else:
             return -cmpVal

        #return cmp(self.columns[self.columnNumber]["type"](a),
        #           self.columns[self.columnNumber]["type"](b))

    # Used by the ColumnSorterMixin, see wx/lib/mixins/listctrl.py
    #def GetSortImages(self):
    #    return (self.sm_dn, self.sm_up)

    #XXX Looks okay to remove this one (was present in the original demo)
    #def getColumnText(self, index, col):
    #    item = self.GetItem(index, col)
    #    return item.GetText()

#----------------------------------------------------------------------
# The main window
#----------------------------------------------------------------------
# This is where you populate the frame with a panel from the demo.
#  original line in runTest (in the demo source):
#    win = TestPanel(nb, log)
#  this is changed to:
#    self.win=TestPanel(self,log)
#----------------------------------------------------------------------

class AttributeManager(wx.Frame):

    def __init__(self, parent, id, title, size, style = wx.DEFAULT_FRAME_STYLE, table=None ):

        wx.Frame.__init__(self, parent, id, title, size=size, style=style)

        self.CreateStatusBar(1)

        log=Log(self)
            
        # probably
        self.gismanager = parent
        print self.gismanager

        self.win = TestVirtualList(self, log,tablename=table)
        self.Show()

def main(argv=None):
    if argv is None:
        argv = sys.argv

    if len(argv) != 2:
        print >>sys.stderr, __doc__
        sys.exit()

    # Command line arguments of the script to be run are preserved by the
    # hotswap.py wrapper but hotswap.py and its options are removed that
    # sys.argv looks as if no wrapper was present.
    #print "argv:", `argv`

    #some applications might require image handlers
    #wx.InitAllImageHandlers()

    app = wx.PySimpleApp()
    f = AttributeManager(None, -1, "GRASS Attribute Table Manager",wx.Size(500,300),table=argv[1])
    app.MainLoop()



if __name__ == '__main__':
    main()
