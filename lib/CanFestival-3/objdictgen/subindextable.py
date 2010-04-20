#!/usr/bin/env python
# -*- coding: utf-8 -*-

#This file is part of CanFestival, a library implementing CanOpen Stack. 
#
#Copyright (C): Edouard TISSERANT, Francis DUPIN and Laurent BESSARD
#
#See COPYING file for copyrights details.
#
#This library is free software; you can redistribute it and/or
#modify it under the terms of the GNU Lesser General Public
#License as published by the Free Software Foundation; either
#version 2.1 of the License, or (at your option) any later version.
#
#This library is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#Lesser General Public License for more details.
#
#You should have received a copy of the GNU Lesser General Public
#License along with this library; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import wx
import wx.grid

from types import *
from commondialogs import *

from node import OD_Subindex, OD_MultipleSubindexes, OD_IdenticalSubindexes, OD_IdenticalIndexes

ColSizes = [75, 250, 150, 125, 100, 60, 250]
ColAlignements = [wx.ALIGN_CENTER, wx.ALIGN_LEFT, wx.ALIGN_CENTER, wx.ALIGN_RIGHT, wx.ALIGN_CENTER, wx.ALIGN_CENTER, wx.ALIGN_LEFT]

def GetAccessList(write=True):
    _ = lambda x : x
    if write:
        return [_("Read Only"), _("Write Only"), _("Read/Write")]
    return [_("Read Only"), _("Read/Write")]
AccessList = ",".join(map(_, GetAccessList()))
RAccessList = ",".join(map(_, GetAccessList(False)))
ACCESS_LIST_DICT = dict([(_(access), access) for access in GetAccessList()])

def GetBoolList():
    _ = lambda x : x
    return [_("True"), _("False")]
BoolList = ",".join(map(_, GetBoolList()))
BOOL_LIST_DICT = dict([(_(bool), bool) for bool in GetBoolList()])

def GetOptionList():
    _ = lambda x : x
    return [_("Yes"), _("No")]
OptionList = ",".join(map(_, GetOptionList()))
OPTION_LIST_DICT = dict([(_(option), option) for option in GetOptionList()])

[USER_TYPE, SDO_SERVER, SDO_CLIENT, 
 PDO_TRANSMIT, PDO_RECEIVE, MAP_VARIABLE] = range(6)

INDEXCHOICE_OPTIONS = {
    USER_TYPE: (_("User Type"), 0, "AddUserType"), 
    SDO_SERVER: (_("SDO Server"), 1, "AddSDOServerToCurrent"),
    SDO_CLIENT: (_("SDO Client"), 1, "AddSDOClientToCurrent"),
    PDO_RECEIVE: (_("PDO Receive"), 1, "AddPDOReceiveToCurrent"),
    PDO_TRANSMIT: (_("PDO Transmit"), 1, "AddPDOTransmitToCurrent"),
    MAP_VARIABLE: (_("Map Variable"), 0, "AddMapVariable")
}

INDEXCHOICE_OPTIONS_DICT = dict([(translation, option) for option, (translation, object, function) in INDEXCHOICE_OPTIONS.iteritems()])

INDEXCHOICE_SECTIONS = {
    0 : [USER_TYPE],
    2 : [SDO_SERVER, SDO_CLIENT],
    3 : [PDO_RECEIVE],
    4 : [PDO_RECEIVE],
    5 : [PDO_TRANSMIT],
    6 : [PDO_TRANSMIT],
    8 : [MAP_VARIABLE],
}

def GetSubindexTableColnames():
    _ = lambda x : x
    return [_("subindex"), _("name"), _("type"), _("value"), _("access"), _("save"), _("comment")]

DictionaryOrganisation = [
    {"minIndex" : 0x0001, "maxIndex" : 0x0FFF, "name" : "Data Type Definitions"},
    {"minIndex" : 0x1000, "maxIndex" : 0x1029, "name" : "Communication Parameters"},
    {"minIndex" : 0x1200, "maxIndex" : 0x12FF, "name" : "SDO Parameters"},
    {"minIndex" : 0x1400, "maxIndex" : 0x15FF, "name" : "Receive PDO Parameters"},
    {"minIndex" : 0x1600, "maxIndex" : 0x17FF, "name" : "Receive PDO Mapping"},
    {"minIndex" : 0x1800, "maxIndex" : 0x19FF, "name" : "Transmit PDO Parameters"},
    {"minIndex" : 0x1A00, "maxIndex" : 0x1BFF, "name" : "Transmit PDO Mapping"},
    {"minIndex" : 0x1C00, "maxIndex" : 0x1FFF, "name" : "Other Communication Parameters"},
    {"minIndex" : 0x2000, "maxIndex" : 0x5FFF, "name" : "Manufacturer Specific"},
    {"minIndex" : 0x6000, "maxIndex" : 0x9FFF, "name" : "Standardized Device Profile"},
    {"minIndex" : 0xA000, "maxIndex" : 0xBFFF, "name" : "Standardized Interface Profile"}]

SizeConversion = {1 : "X", 8 : "B", 16 : "W", 24 : "D", 32 : "D", 40 : "L", 48 : "L", 56 : "L", 64 : "L"}

class SubindexTable(wx.grid.PyGridTableBase):
    
    """
    A custom wxGrid Table using user supplied data
    """
    def __init__(self, parent, data, editors, colnames):
        # The base class must be initialized *first*
        wx.grid.PyGridTableBase.__init__(self)
        self.data = data
        self.editors = editors
        self.CurrentIndex = 0
        self.colnames = colnames
        self.Parent = parent
        self.Editable = True
        # XXX
        # we need to store the row length and collength to
        # see if the table has changed size
        self._rows = self.GetNumberRows()
        self._cols = self.GetNumberCols()
    
    def Disable(self):
        self.Editable = False
        
    def Enable(self):
        self.Editable = True
    
    def GetNumberCols(self):
        return len(self.colnames)
        
    def GetNumberRows(self):
        return len(self.data)

    def GetColLabelValue(self, col, translate=True):
        if col < len(self.colnames):
            if translate:
                return _(self.colnames[col])
            return self.colnames[col]

    def GetRowLabelValues(self, row, translate=True):
        return row

    def GetValue(self, row, col, translate=True):
        if row < self.GetNumberRows():
            colname = self.GetColLabelValue(col, False)
            value = unicode(self.data[row].get(colname, ""))
            if translate and (colname == "access" or 
                              self.editors[row][colname] in ["bool", "option"] or
                              self.editors[row][colname] == "map" and value == "None"):
                value = _(value)
            return value
            
    def GetEditor(self, row, col):
        if row < self.GetNumberRows():
            return self.editors[row].get(self.GetColLabelValue(col, False), "")
    
    def GetValueByName(self, row, colname):
        return self.data[row].get(colname)

    def SetValue(self, row, col, value):
        if col < len(self.colnames):
            colname = self.GetColLabelValue(col, False)
            if colname == "access":
                value = ACCESS_LIST_DICT[value]
            elif self.editors[row][colname] == "bool":
                value = BOOL_LIST_DICT[value]
            elif self.editors[row][colname] == "option":
                value = OPTION_LIST_DICT[value]
            elif self.editors[row][colname] == "map" and value == _("None"):
                value = "None"
            self.data[row][colname] = value
        
    def ResetView(self, grid):
        """
        (wx.grid.Grid) -> Reset the grid view.   Call this to
        update the grid if rows and columns have been added or deleted
        """
        grid.BeginBatch()
        for current, new, delmsg, addmsg in [
            (self._rows, self.GetNumberRows(), wx.grid.GRIDTABLE_NOTIFY_ROWS_DELETED, wx.grid.GRIDTABLE_NOTIFY_ROWS_APPENDED),
            (self._cols, self.GetNumberCols(), wx.grid.GRIDTABLE_NOTIFY_COLS_DELETED, wx.grid.GRIDTABLE_NOTIFY_COLS_APPENDED),
        ]:
            if new < current:
                msg = wx.grid.GridTableMessage(self,delmsg,new,current-new)
                grid.ProcessTableMessage(msg)
            elif new > current:
                msg = wx.grid.GridTableMessage(self,addmsg,new-current)
                grid.ProcessTableMessage(msg)
                self.UpdateValues(grid)
        grid.EndBatch()

        self._rows = self.GetNumberRows()
        self._cols = self.GetNumberCols()
        # update the column rendering scheme
        self._updateColAttrs(grid)

        # update the scrollbars and the displayed part of the grid
        grid.AdjustScrollbars()
        grid.ForceRefresh()


    def UpdateValues(self, grid):
        """Update all displayed values"""
        # This sends an event to the grid table to update all of the values
        msg = wx.grid.GridTableMessage(self, wx.grid.GRIDTABLE_REQUEST_VIEW_GET_VALUES)
        grid.ProcessTableMessage(msg)

    def _updateColAttrs(self, grid):
        """
        wx.grid.Grid -> update the column attributes to add the
        appropriate renderer given the column name.

        Otherwise default to the default renderer.
        """
        
        for col in range(self.GetNumberCols()):
            attr = wx.grid.GridCellAttr()
            attr.SetAlignment(ColAlignements[col], wx.ALIGN_CENTRE)
            grid.SetColAttr(col, attr)
            grid.SetColMinimalWidth(col, ColSizes[col])
            grid.AutoSizeColumn(col, False)
        
        typelist = None
        maplist = None
        for row in range(self.GetNumberRows()):
            editors = self.editors[row]
            grid.SetRowMinimalHeight(row, 28)
            grid.AutoSizeRow(row, False)
            for col in range(self.GetNumberCols()):
                editor = None
                renderer = None
                
                colname = self.GetColLabelValue(col, False)
                editortype = editors[colname]
                if editortype and self.Editable:
                    grid.SetReadOnly(row, col, False)
                    if editortype == "string":
                        editor = wx.grid.GridCellTextEditor()
                        renderer = wx.grid.GridCellStringRenderer()
                        if colname == "value" and "length" in editors:
                            editor.SetParameters(editors["length"]) 
                    elif editortype == "number":
                        editor = wx.grid.GridCellNumberEditor()
                        renderer = wx.grid.GridCellNumberRenderer()
                        if colname == "value" and "min" in editors and "max" in editors:
                            editor.SetParameters("%s,%s"%(editors["min"],editors["max"]))
                    elif editortype == "float":
                        editor = wx.grid.GridCellTextEditor()
                        renderer = wx.grid.GridCellStringRenderer()
                    elif editortype == "bool":
                        editor = wx.grid.GridCellChoiceEditor()
                        editor.SetParameters(BoolList)
                    elif editortype == "access":
                        editor = wx.grid.GridCellChoiceEditor()
                        editor.SetParameters(AccessList)
                    elif editortype == "raccess":
                        editor = wx.grid.GridCellChoiceEditor()
                        editor.SetParameters(RAccessList)
                    elif editortype == "option":
                        editor = wx.grid.GridCellChoiceEditor()
                        editor.SetParameters(OptionList)
                    elif editortype == "type":
                        editor = wx.grid.GridCellChoiceEditor()
                        if typelist == None:
                            typelist = self.Parent.Manager.GetCurrentTypeList()
                        editor.SetParameters(typelist)
                    elif editortype == "map":
                        editor = wx.grid.GridCellChoiceEditor()
                        if maplist == None:
                            maplist = self.Parent.Manager.GetCurrentMapList()
                        editor.SetParameters(maplist)
                    elif editortype == "time":
                        editor = wx.grid.GridCellTextEditor()
                        renderer = wx.grid.GridCellStringRenderer()
                    elif editortype == "domain":
                        editor = wx.grid.GridCellTextEditor()
                        renderer = wx.grid.GridCellStringRenderer()
                    elif editortype == "dcf":
                        editor = wx.grid.GridCellTextEditor()
                        renderer = wx.grid.GridCellStringRenderer()
                else:
                    grid.SetReadOnly(row, col, True)
                    
                grid.SetCellEditor(row, col, editor)
                grid.SetCellRenderer(row, col, renderer)
                
                grid.SetCellBackgroundColour(row, col, wx.WHITE)
    
    def SetData(self, data):
        self.data = data
        
    def SetEditors(self, editors):
        self.editors = editors
    
    def GetCurrentIndex(self):
        return self.CurrentIndex
    
    def SetCurrentIndex(self, index):
        self.CurrentIndex = index
    
    def AppendRow(self, row_content):
        self.data.append(row_content)

    def Empty(self):
        self.data = []
        self.editors = []

[ID_EDITINGPANEL, ID_EDITINGPANELADDBUTTON, ID_EDITINGPANELINDEXCHOICE, 
 ID_EDITINGPANELINDEXLIST, ID_EDITINGPANELINDEXLISTPANEL, ID_EDITINGPANELPARTLIST, 
 ID_EDITINGPANELSECONDSPLITTER, ID_EDITINGPANELSUBINDEXGRID,
 ID_EDITINGPANELSUBINDEXGRIDPANEL, ID_EDITINGPANELCALLBACKCHECK,
] = [wx.NewId() for _init_ctrls in range(10)]

[ID_EDITINGPANELINDEXLISTMENUITEMS0, ID_EDITINGPANELINDEXLISTMENUITEMS1, 
 ID_EDITINGPANELINDEXLISTMENUITEMS2, 
] = [wx.NewId() for _init_coll_IndexListMenu_Items in range(3)]

[ID_EDITINGPANELMENU1ITEMS0, ID_EDITINGPANELMENU1ITEMS1, 
 ID_EDITINGPANELMENU1ITEMS3, ID_EDITINGPANELMENU1ITEMS4, 
] = [wx.NewId() for _init_coll_SubindexGridMenu_Items in range(4)]

class EditingPanel(wx.SplitterWindow):
    def _init_coll_AddToListSizer_Items(self, parent):
        parent.AddWindow(self.AddButton, 0, border=0, flag=0)
        parent.AddWindow(self.IndexChoice, 0, border=0, flag=wx.GROW)

    def _init_coll_SubindexGridSizer_Items(self, parent):
        parent.AddWindow(self.CallbackCheck, 0, border=0, flag=0)
        parent.AddWindow(self.SubindexGrid, 0, border=0, flag=wx.GROW)

    def _init_coll_IndexListSizer_Items(self, parent):
        parent.AddWindow(self.IndexList, 0, border=0, flag=wx.GROW)
        parent.AddSizer(self.AddToListSizer, 0, border=0, flag=wx.GROW)

    def _init_coll_AddToListSizer_Growables(self, parent):
        parent.AddGrowableCol(1)

    def _init_coll_SubindexGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(1)

    def _init_coll_IndexListSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_SubindexGridMenu_Items(self, parent):
        parent.Append(help='', id=ID_EDITINGPANELMENU1ITEMS0,
              kind=wx.ITEM_NORMAL, text=_('Add subindexes'))
        parent.Append(help='', id=ID_EDITINGPANELMENU1ITEMS1,
              kind=wx.ITEM_NORMAL, text=_('Delete subindexes'))
        parent.AppendSeparator()
        parent.Append(help='', id=ID_EDITINGPANELMENU1ITEMS3,
              kind=wx.ITEM_NORMAL, text=_('Default value'))
        if not self.Editable:
            parent.Append(help='', id=ID_EDITINGPANELMENU1ITEMS4,
                  kind=wx.ITEM_NORMAL, text=_('Add to DCF'))
        self.Bind(wx.EVT_MENU, self.OnAddSubindexMenu,
              id=ID_EDITINGPANELMENU1ITEMS0)
        self.Bind(wx.EVT_MENU, self.OnDeleteSubindexMenu,
              id=ID_EDITINGPANELMENU1ITEMS1)
        self.Bind(wx.EVT_MENU, self.OnDefaultValueSubindexMenu,
              id=ID_EDITINGPANELMENU1ITEMS3)
        if not self.Editable:
            self.Bind(wx.EVT_MENU, self.OnAddToDCFSubindexMenu,
                  id=ID_EDITINGPANELMENU1ITEMS4)

    def _init_coll_IndexListMenu_Items(self, parent):
        parent.Append(help='', id=ID_EDITINGPANELINDEXLISTMENUITEMS0,
              kind=wx.ITEM_NORMAL, text=_('Rename'))
        parent.Append(help='', id=ID_EDITINGPANELINDEXLISTMENUITEMS2,
              kind=wx.ITEM_NORMAL, text=_('Modify'))
        parent.Append(help='', id=ID_EDITINGPANELINDEXLISTMENUITEMS1,
              kind=wx.ITEM_NORMAL, text=_('Delete'))
        self.Bind(wx.EVT_MENU, self.OnRenameIndexMenu,
              id=ID_EDITINGPANELINDEXLISTMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnDeleteIndexMenu,
              id=ID_EDITINGPANELINDEXLISTMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnModifyIndexMenu,
              id=ID_EDITINGPANELINDEXLISTMENUITEMS2)

    def _init_utils(self):
        self.IndexListMenu = wx.Menu(title='')
        self.SubindexGridMenu = wx.Menu(title='')

        self._init_coll_IndexListMenu_Items(self.IndexListMenu)
        self._init_coll_SubindexGridMenu_Items(self.SubindexGridMenu)

    def _init_sizers(self):
        self.IndexListSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)
        self.SubindexGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)
        self.AddToListSizer = wx.FlexGridSizer(cols=2, hgap=0, rows=1, vgap=0)

        self._init_coll_IndexListSizer_Growables(self.IndexListSizer)
        self._init_coll_IndexListSizer_Items(self.IndexListSizer)
        self._init_coll_SubindexGridSizer_Growables(self.SubindexGridSizer)
        self._init_coll_SubindexGridSizer_Items(self.SubindexGridSizer)
        self._init_coll_AddToListSizer_Growables(self.AddToListSizer)
        self._init_coll_AddToListSizer_Items(self.AddToListSizer)

        self.SubindexGridPanel.SetSizer(self.SubindexGridSizer)
        self.IndexListPanel.SetSizer(self.IndexListSizer)
        
    def _init_ctrls(self, prnt):
        wx.SplitterWindow.__init__(self, id=ID_EDITINGPANEL,
              name='MainSplitter', parent=prnt, point=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=wx.SP_3D)
        self._init_utils()
        self.SetNeedUpdating(True)
        self.SetMinimumPaneSize(1)

        self.PartList = wx.ListBox(choices=[], id=ID_EDITINGPANELPARTLIST,
              name='PartList', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=0)
        self.PartList.Bind(wx.EVT_LISTBOX, self.OnPartListBoxClick,
              id=ID_EDITINGPANELPARTLIST)

        self.SecondSplitter = wx.SplitterWindow(id=ID_EDITINGPANELSECONDSPLITTER,
              name='SecondSplitter', parent=self, point=wx.Point(0, 0), 
              size=wx.Size(-1, -1), style=wx.SP_3D)
        self.SecondSplitter.SetMinimumPaneSize(1)
        self.SplitHorizontally(self.PartList, self.SecondSplitter,
              110)

        self.SubindexGridPanel = wx.Panel(id=ID_EDITINGPANELSUBINDEXGRIDPANEL,
              name='SubindexGridPanel', parent=self.SecondSplitter, 
              pos=wx.Point(0, 0), size=wx.Size(-1, -1), style=wx.TAB_TRAVERSAL)

        self.IndexListPanel = wx.Panel(id=ID_EDITINGPANELINDEXLISTPANEL,
              name='IndexListPanel', parent=self.SecondSplitter, 
              pos=wx.Point(0, 0), size=wx.Size(-1, -1), style=wx.TAB_TRAVERSAL)
        self.SecondSplitter.SplitVertically(self.IndexListPanel,
              self.SubindexGridPanel, 280)

        self.SubindexGrid = wx.grid.Grid(id=ID_EDITINGPANELSUBINDEXGRID,
              name='SubindexGrid', parent=self.SubindexGridPanel, pos=wx.Point(0,
              0), size=wx.Size(-1, -1), style=0)
        self.SubindexGrid.SetFont(wx.Font(12, 77, wx.NORMAL, wx.NORMAL, False,
              'Sans'))
        self.SubindexGrid.SetLabelFont(wx.Font(10, 77, wx.NORMAL, wx.NORMAL,
              False, 'Sans'))
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_CELL_CHANGE,
              self.OnSubindexGridCellChange)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_CELL_RIGHT_CLICK,
              self.OnSubindexGridRightClick)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_SELECT_CELL,
              self.OnSubindexGridSelectCell)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_CELL_LEFT_CLICK, 
              self.OnSubindexGridCellLeftClick)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_EDITOR_SHOWN, 
              self.OnSubindexGridEditorShown)

        self.CallbackCheck = wx.CheckBox(id=ID_EDITINGPANELCALLBACKCHECK,
              label=_('Have Callbacks'), name='CallbackCheck',
              parent=self.SubindexGridPanel, pos=wx.Point(0, 0), size=wx.Size(152,
              24), style=0)
        self.CallbackCheck.Bind(wx.EVT_CHECKBOX, self.OnCallbackCheck,
              id=ID_EDITINGPANELCALLBACKCHECK)

        self.IndexList = wx.ListBox(choices=[], id=ID_EDITINGPANELINDEXLIST,
              name='IndexList', parent=self.IndexListPanel, pos=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=0)
        self.IndexList.Bind(wx.EVT_LISTBOX, self.OnIndexListClick,
              id=ID_EDITINGPANELINDEXLIST)
        self.IndexList.Bind(wx.EVT_RIGHT_UP, self.OnIndexListRightUp)

        self.AddButton = wx.Button(id=ID_EDITINGPANELADDBUTTON, label=_('Add'),
              name='AddButton', parent=self.IndexListPanel, pos=wx.Point(0, 0),
              size=wx.DefaultSize, style=0)
        self.AddButton.Bind(wx.EVT_BUTTON, self.OnAddButtonClick,
              id=ID_EDITINGPANELADDBUTTON)

        self.IndexChoice = wx.ComboBox(choices=[], id=ID_EDITINGPANELINDEXCHOICE,
              name='IndexChoice', parent=self.IndexListPanel, pos=wx.Point(50,
              0), size=wx.Size(-1, 30), style=wx.CB_READONLY)

        self._init_sizers()

    def __init__(self, parent, window, manager, editable = True):
        self.Editable = editable
        self._init_ctrls(parent)
        self.ParentWindow = window
        self.Manager = manager
        self.ListIndex = []
        self.ChoiceIndex = []
        self.FirstCall = False
        self.Index = None
        
        for values in DictionaryOrganisation:
            text = "   0x%04X-0x%04X      %s"%(values["minIndex"], values["maxIndex"], values["name"])
            self.PartList.Append(text)
        self.Table = SubindexTable(self, [], [], GetSubindexTableColnames())
        self.SubindexGrid.SetTable(self.Table)
        self.SubindexGrid.SetRowLabelSize(0)
        self.CallbackCheck.Disable()
        self.Table.ResetView(self.SubindexGrid)

        if not self.Editable:
            self.AddButton.Disable()
            self.IndexChoice.Disable()
            self.CallbackCheck.Disable()
            self.Table.Disable()

    def GetIndex(self):
        return self.Index
    
    def SetIndex(self, index):
        self.Index = index

    def GetSelection(self):
        selected = self.IndexList.GetSelection()
        if selected != wx.NOT_FOUND:
            index = self.ListIndex[selected]
            subIndex = self.SubindexGrid.GetGridCursorRow()
            return index, subIndex
        return None

    def OnSubindexGridCellLeftClick(self, event):
        if not self.ParentWindow.ModeSolo:
            col = event.GetCol()
            if self.Editable and col == 0:
                selected = self.IndexList.GetSelection()
                if selected != wx.NOT_FOUND:
                    index = self.ListIndex[selected]
                    subindex = event.GetRow()
                    entry_infos = self.Manager.GetEntryInfos(index)
                    if not entry_infos["struct"] & OD_MultipleSubindexes or subindex != 0:
                        subentry_infos = self.Manager.GetSubentryInfos(index, subindex)
                        typeinfos = self.Manager.GetEntryInfos(subentry_infos["type"])
                        if typeinfos:
                            bus_id = '.'.join(map(str, self.ParentWindow.GetBusId()))
                            size = typeinfos["size"]
                            data = wx.TextDataObject(str(("%s%s.%d.%d"%(SizeConversion[size], bus_id, index, subindex), "location")))
                            dragSource = wx.DropSource(self.SubindexGrid)
                            dragSource.SetData(data)
                            dragSource.DoDragDrop()
            elif col == 0:
                selected = self.IndexList.GetSelection()
                if selected != wx.NOT_FOUND:
                    index = self.ListIndex[selected]
                    subindex = event.GetRow()
                    entry_infos = self.Manager.GetEntryInfos(index)
                    if not entry_infos["struct"] & OD_MultipleSubindexes or subindex != 0:
                        subentry_infos = self.Manager.GetSubentryInfos(index, subindex)
                        typeinfos = self.Manager.GetEntryInfos(subentry_infos["type"])
                        if subentry_infos["pdo"] and typeinfos:
                            bus_id = '.'.join(map(str, self.ParentWindow.GetBusId()))
                            node_id = self.ParentWindow.GetCurrentNodeId()
                            size = typeinfos["size"]
                            data = wx.TextDataObject(str(("%s%s.%d.%d.%d"%(SizeConversion[size], bus_id, node_id, index, subindex), "location")))
                            dragSource = wx.DropSource(self.SubindexGrid)
                            dragSource.SetData(data)
                            dragSource.DoDragDrop()
        event.Skip()

    def OnAddButtonClick(self, event):
        if self.Editable:
            self.SubindexGrid.SetGridCursor(0, 0)
            selected = self.IndexChoice.GetStringSelection()
            if selected != "":
                choice = INDEXCHOICE_OPTIONS_DICT.get(selected, None)
                if choice is not None:
                    if INDEXCHOICE_OPTIONS[choice][1] == 0:
                        getattr(self.ParentWindow, INDEXCHOICE_OPTIONS[choice][2])()
                    elif INDEXCHOICE_OPTIONS[choice][1] == 1:
                        getattr(self.Manager, INDEXCHOICE_OPTIONS[choice][2])()
                elif selected in [menu for menu, indexes in self.Manager.GetCurrentSpecificMenu()]:
                    self.Manager.AddSpecificEntryToCurrent(selected)
                else:
                    index = self.ChoiceIndex[self.IndexChoice.GetSelection()]
                    self.Manager.ManageEntriesOfCurrent([index], [])
                self.ParentWindow.RefreshBufferState()
                self.RefreshIndexList()
        event.Skip()

    def OnPartListBoxClick(self, event):
        if not self.ParentWindow.IsClosing():
            self.SubindexGrid.SetGridCursor(0, 0)
            self.RefreshIndexList()
        event.Skip()

    def OnIndexListClick(self, event):
        if not self.ParentWindow.IsClosing():
            self.SubindexGrid.SetGridCursor(0, 0)
            self.RefreshTable()
        event.Skip()

    def OnSubindexGridSelectCell(self, event):
        if not self.ParentWindow.IsClosing():
            wx.CallAfter(self.ParentWindow.RefreshStatusBar)
        event.Skip()

#-------------------------------------------------------------------------------
#                             Refresh Functions
#-------------------------------------------------------------------------------

    def RefreshIndexList(self):
        selected = self.IndexList.GetSelection()
        choice = self.IndexChoice.GetStringSelection()
        choiceindex = self.IndexChoice.GetSelection()
        if selected != wx.NOT_FOUND:
            selectedindex = self.ListIndex[selected]
        self.IndexList.Clear()
        self.IndexChoice.Clear()
        i = self.PartList.GetSelection()
        if i < len(DictionaryOrganisation):
            values = DictionaryOrganisation[i]
            self.ListIndex = []
            for name, index in self.Manager.GetCurrentValidIndexes(values["minIndex"], values["maxIndex"]):
                self.IndexList.Append("0x%04X   %s"%(index, name))
                self.ListIndex.append(index)
            if self.Editable:
                self.ChoiceIndex = []
                choices = INDEXCHOICE_SECTIONS.get(i, None)
                if choices is not None:
                    for c in choices:
                        self.IndexChoice.Append(INDEXCHOICE_OPTIONS[c][0])
                    if len(choices) > 1:
                        if choiceindex != wx.NOT_FOUND and choice == self.IndexChoice.GetString(choiceindex):
                            self.IndexChoice.SetStringSelection(choice)
                    else:
                        self.IndexChoice.SetSelection(0)
                else:
                    for name, index in self.Manager.GetCurrentValidChoices(values["minIndex"], values["maxIndex"]):
                        if index:
                            self.IndexChoice.Append("0x%04X   %s"%(index, name))
                        else:
                            self.IndexChoice.Append(name)
                        self.ChoiceIndex.append(index)
                if choiceindex != wx.NOT_FOUND and choiceindex < self.IndexChoice.GetCount() and choice == self.IndexChoice.GetString(choiceindex):
                    self.IndexChoice.SetStringSelection(choice)
        if self.Editable:
            self.IndexChoice.Enable(self.IndexChoice.GetCount() != 0)
            self.AddButton.Enable(self.IndexChoice.GetCount() != 0)
        if selected == wx.NOT_FOUND or selected >= len(self.ListIndex) or selectedindex != self.ListIndex[selected]:
            self.Table.Empty()
            self.CallbackCheck.SetValue(False)
            self.CallbackCheck.Disable()
            self.Table.ResetView(self.SubindexGrid)
            self.ParentWindow.RefreshStatusBar()
        else:
            self.IndexList.SetSelection(selected)
            self.RefreshTable()

    def RefreshTable(self):
        selected = self.IndexList.GetSelection()
        if selected != wx.NOT_FOUND:
            index = self.ListIndex[selected]
            if index > 0x260 and self.Editable:
                self.CallbackCheck.Enable()
                self.CallbackCheck.SetValue(self.Manager.HasCurrentEntryCallbacks(index))
            result = self.Manager.GetCurrentEntryValues(index)
            if result != None:
                self.Table.SetCurrentIndex(index)
                data, editors = result
                self.Table.SetData(data)
                self.Table.SetEditors(editors)
                self.Table.ResetView(self.SubindexGrid)
        self.ParentWindow.RefreshStatusBar()

#-------------------------------------------------------------------------------
#                        Editing Table value function
#-------------------------------------------------------------------------------

    def OnSubindexGridEditorShown(self, event):
        row, col = event.GetRow(), event.GetCol() 
        if self.Table.GetEditor(row, col) == "dcf":
            wx.CallAfter(self.ShowDCFEntryDialog, row, col)
            event.Veto()
        else:
            event.Skip()

    def ShowDCFEntryDialog(self, row, col):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    dialog = DCFEntryValuesDialog(self)
                    dialog.SetValues(self.Table.GetValue(row, col).decode("hex_codec"))
                    if dialog.ShowModal() == wx.ID_OK:
                        value = dialog.GetValues()
                        self.Manager.SetCurrentEntry(index, row, value, "value", "dcf")
                        self.ParentWindow.RefreshBufferState()
                        wx.CallAfter(self.RefreshTable)

    def OnSubindexGridCellChange(self, event):
        if self.Editable:
            index = self.Table.GetCurrentIndex()
            subIndex = event.GetRow()
            col = event.GetCol()
            name = self.Table.GetColLabelValue(col, False)
            value = self.Table.GetValue(subIndex, col, False)
            editor = self.Table.GetEditor(subIndex, col)
            self.Manager.SetCurrentEntry(index, subIndex, value, name, editor)
            self.ParentWindow.RefreshBufferState()
            wx.CallAfter(self.RefreshTable)
        event.Skip()

    def OnCallbackCheck(self, event):
        if self.Editable:
            index = self.Table.GetCurrentIndex()
            self.Manager.SetCurrentEntryCallbacks(index, self.CallbackCheck.GetValue())
            self.ParentWindow.RefreshBufferState()
            wx.CallAfter(self.RefreshTable)
        event.Skip()

#-------------------------------------------------------------------------------
#                          Contextual Menu functions
#-------------------------------------------------------------------------------

    def OnIndexListRightUp(self, event):
        if self.Editable:
            if not self.FirstCall:
                self.FirstCall = True
                selected = self.IndexList.GetSelection()
                if selected != wx.NOT_FOUND:
                    index = self.ListIndex[selected]
                    if index < 0x260:
                        self.IndexListMenu.FindItemByPosition(0).Enable(False)
                        self.IndexListMenu.FindItemByPosition(1).Enable(True)
                        self.PopupMenu(self.IndexListMenu)
                    elif 0x1000 <= index <= 0x1FFF:
                        self.IndexListMenu.FindItemByPosition(0).Enable(False)
                        self.IndexListMenu.FindItemByPosition(1).Enable(False)
                        self.PopupMenu(self.IndexListMenu)
                    elif 0x2000 <= index <= 0x5FFF:
                        self.IndexListMenu.FindItemByPosition(0).Enable(True)
                        self.IndexListMenu.FindItemByPosition(1).Enable(False)
                        self.PopupMenu(self.IndexListMenu)
                    elif index >= 0x6000:
                        self.IndexListMenu.FindItemByPosition(0).Enable(False)
                        self.IndexListMenu.FindItemByPosition(1).Enable(False)
                        self.PopupMenu(self.IndexListMenu)
            else:
                self.FirstCall = False
        event.Skip()

    def OnSubindexGridRightClick(self, event):
        self.SubindexGrid.SetGridCursor(event.GetRow(), event.GetCol())
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    showpopup = False
                    infos = self.Manager.GetEntryInfos(index)
                    if 0x2000 <= index <= 0x5FFF and infos["struct"] & OD_MultipleSubindexes or infos["struct"] & OD_IdenticalSubindexes:
                        showpopup = True
                        self.SubindexGridMenu.FindItemByPosition(0).Enable(True)
                        self.SubindexGridMenu.FindItemByPosition(1).Enable(True)
                    else:
                        self.SubindexGridMenu.FindItemByPosition(0).Enable(False)
                        self.SubindexGridMenu.FindItemByPosition(1).Enable(False)
                    if self.Table.GetColLabelValue(event.GetCol()) == "value":
                        showpopup = True
                        self.SubindexGridMenu.FindItemByPosition(3).Enable(True)
                    else:
                        self.SubindexGridMenu.FindItemByPosition(3).Enable(False)
                    if showpopup:
                        self.PopupMenu(self.SubindexGridMenu)
        elif self.Table.GetColLabelValue(event.GetCol()) == "value":
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    infos = self.Manager.GetEntryInfos(index)
                    if not infos["struct"] & OD_MultipleSubindexes or event.GetRow() > 0:
                        self.SubindexGridMenu.FindItemByPosition(0).Enable(False)
                        self.SubindexGridMenu.FindItemByPosition(1).Enable(False)
                        self.SubindexGridMenu.FindItemByPosition(3).Enable(False)
                        self.SubindexGridMenu.FindItemByPosition(4).Enable(True)
                        self.PopupMenu(self.SubindexGridMenu)
        event.Skip()

    def OnAddToDCFSubindexMenu(self, event):
        if not self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                subindex = self.SubindexGrid.GetGridCursorRow()
                entry_infos = self.Manager.GetEntryInfos(index)
                if not entry_infos["struct"] & OD_MultipleSubindexes or subindex != 0:
                    subentry_infos = self.Manager.GetSubentryInfos(index, subindex)
                    typeinfos = self.Manager.GetEntryInfos(subentry_infos["type"])
                    if typeinfos:
                        node_id = self.ParentWindow.GetCurrentNodeId()
                        value = self.Table.GetValueByName(subindex, "value")
                        if value == "True":
                            value = 1
                        elif value == "False":
                            value = 0
                        elif value.isdigit():
                            value = int(value)
                        elif value.startswith("0x"):
                            value = int(value, 16)
                        else:
                            value = int(value.encode("hex_codec"), 16)
                        self.Manager.AddToMasterDCF(node_id, index, subindex, max(1, typeinfos["size"] / 8), value)    
                        self.ParentWindow.OpenMasterDCFDialog(node_id)

    def OpenDCFDialog(self, node_id):
        self.PartList.SetSelection(7)
        self.RefreshIndexList()
        self.IndexList.SetSelection(self.ListIndex.index(0x1F22))
        self.RefreshTable()
        self.SubindexGrid.SetGridCursor(node_id, 3)
        self.ShowDCFEntryDialog(node_id, 3)

    def OnRenameIndexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    infos = self.Manager.GetEntryInfos(index)
                    dialog = wx.TextEntryDialog(self, _("Give a new name for index 0x%04X")%index,
                                 _("Rename an index"), infos["name"], wx.OK|wx.CANCEL)
                    if dialog.ShowModal() == wx.ID_OK:
                        self.Manager.SetCurrentEntryName(index, dialog.GetValue())
                        self.ParentWindow.RefreshBufferState()
                        self.RefreshIndexList()
                    dialog.Destroy()
        
    def OnModifyIndexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index) and index < 0x260:
                    values, valuetype = self.Manager.GetCustomisedTypeValues(index)
                    dialog = UserTypeDialog(self)
                    dialog.SetTypeList(self.Manager.GetCustomisableTypes(), values[1])
                    if valuetype == 0:
                        dialog.SetValues(min = values[2], max = values[3])
                    elif valuetype == 1:
                        dialog.SetValues(length = values[2])
                    if dialog.ShowModal() == wx.ID_OK:
                        type, min, max, length = dialog.GetValues()
                        self.Manager.SetCurrentUserType(index, type, min, max, length)
                        self.ParentWindow.RefreshBufferState()
                        self.RefreshIndexList()
        
    def OnDeleteIndexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    self.Manager.ManageEntriesOfCurrent([],[index])
                    self.ParentWindow.RefreshBufferState()
                    self.RefreshIndexList()
        
    def OnAddSubindexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    dialog = wx.TextEntryDialog(self, _("Number of subindexes to add:"),
                                 _("Add subindexes"), "1", wx.OK|wx.CANCEL)
                    if dialog.ShowModal() == wx.ID_OK:
                        try:
                            number = int(dialog.GetValue())
                            self.Manager.AddSubentriesToCurrent(index, number)
                            self.ParentWindow.RefreshBufferState()
                            self.RefreshIndexList()
                        except:
                            message = wx.MessageDialog(self, _("An integer is required!"), _("ERROR"), wx.OK|wx.ICON_ERROR)
                            message.ShowModal()
                            message.Destroy()
                    dialog.Destroy()
        
    def OnDeleteSubindexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    dialog = wx.TextEntryDialog(self, _("Number of subindexes to delete:"),
                                 _("Delete subindexes"), "1", wx.OK|wx.CANCEL)
                    if dialog.ShowModal() == wx.ID_OK:
                        try:
                            number = int(dialog.GetValue())
                            self.Manager.RemoveSubentriesFromCurrent(index, number)
                            self.ParentWindow.RefreshBufferState()
                            self.RefreshIndexList()
                        except:
                            message = wx.MessageDialog(self, _("An integer is required!"), _("ERROR"), wx.OK|wx.ICON_ERROR)
                            message.ShowModal()
                            message.Destroy()
                    dialog.Destroy()
        
    def OnDefaultValueSubindexMenu(self, event):
        if self.Editable:
            selected = self.IndexList.GetSelection()
            if selected != wx.NOT_FOUND:
                index = self.ListIndex[selected]
                if self.Manager.IsCurrentEntry(index):
                    row = self.SubindexGrid.GetGridCursorRow()
                    self.Manager.SetCurrentEntryToDefault(index, row)
                    self.ParentWindow.RefreshBufferState()
                    self.RefreshIndexList()
        