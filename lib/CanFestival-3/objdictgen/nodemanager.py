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

from gnosis.xml.pickle import *
from gnosis.xml.pickle.util import setParanoia
setParanoia(0)

from node import *
import eds_utils, gen_cfile

from types import *
import os, re

UndoBufferLength = 20

type_model = re.compile('([\_A-Z]*)([0-9]*)')
range_model = re.compile('([\_A-Z]*)([0-9]*)\[([\-0-9]*)-([\-0-9]*)\]')

# ID for the file viewed
CurrentID = 0

# Returns a new id
def GetNewId():
    global CurrentID
    CurrentID += 1
    return CurrentID

"""
Class implementing a buffer of changes made on the current editing Object Dictionary
"""

class UndoBuffer:

    """
    Constructor initialising buffer
    """
    def __init__(self, currentstate, issaved = False):
        self.Buffer = []
        self.CurrentIndex = -1
        self.MinIndex = -1
        self.MaxIndex = -1
        # if current state is defined
        if currentstate:
            self.CurrentIndex = 0
            self.MinIndex = 0
            self.MaxIndex = 0
        # Initialising buffer with currentstate at the first place
        for i in xrange(UndoBufferLength):
            if i == 0:
                self.Buffer.append(currentstate)
            else:
                self.Buffer.append(None)
        # Initialising index of state saved
        if issaved:
            self.LastSave = 0
        else:
            self.LastSave = -1
    
    """
    Add a new state in buffer
    """
    def Buffering(self, currentstate):
        self.CurrentIndex = (self.CurrentIndex + 1) % UndoBufferLength
        self.Buffer[self.CurrentIndex] = currentstate
        # Actualising buffer limits
        self.MaxIndex = self.CurrentIndex
        if self.MinIndex == self.CurrentIndex:
            # If the removed state was the state saved, there is no state saved in the buffer
            if self.LastSave == self.MinIndex:
                self.LastSave = -1
            self.MinIndex = (self.MinIndex + 1) % UndoBufferLength
        self.MinIndex = max(self.MinIndex, 0)
    
    """
    Return current state of buffer
    """
    def Current(self):
        return self.Buffer[self.CurrentIndex]
    
    """
    Change current state to previous in buffer and return new current state
    """
    def Previous(self):
        if self.CurrentIndex != self.MinIndex:
            self.CurrentIndex = (self.CurrentIndex - 1) % UndoBufferLength
            return self.Buffer[self.CurrentIndex]
        return None
    
    """
    Change current state to next in buffer and return new current state
    """
    def Next(self):
        if self.CurrentIndex != self.MaxIndex:
            self.CurrentIndex = (self.CurrentIndex + 1) % UndoBufferLength
            return self.Buffer[self.CurrentIndex]
        return None
    
    """
    Return True if current state is the first in buffer
    """
    def IsFirst(self):
        return self.CurrentIndex == self.MinIndex
    
    """
    Return True if current state is the last in buffer
    """
    def IsLast(self):
        return self.CurrentIndex == self.MaxIndex

    """
    Note that current state is saved
    """
    def CurrentSaved(self):
        self.LastSave = self.CurrentIndex
        
    """
    Return True if current state is saved
    """
    def IsCurrentSaved(self):
        return self.LastSave == self.CurrentIndex



"""
Class which control the operations made on the node and answer to view requests
"""

class NodeManager:

    """
    Constructor
    """
    def __init__(self):
        self.LastNewIndex = 0
        self.FilePaths = {}
        self.FileNames = {}
        self.NodeIndex = None
        self.CurrentNode = None
        self.UndoBuffers = {}

#-------------------------------------------------------------------------------
#                         Type and Map Variable Lists
#-------------------------------------------------------------------------------
    
    """
    Return the list of types defined for the current node
    """
    def GetCurrentTypeList(self):
        if self.CurrentNode:
            return self.CurrentNode.GetTypeList()
        else:
            return ""

    """
    Return the list of variables that can be mapped for the current node
    """
    def GetCurrentMapList(self):
        if self.CurrentNode:
            return self.CurrentNode.GetMapList()
        else:
            return ""

#-------------------------------------------------------------------------------
#                        Create Load and Save Functions
#-------------------------------------------------------------------------------

    """
    Create a new node and add a new buffer for storing it
    """
    def CreateNewNode(self, name, id, type, description, profile, filepath, NMT, options):
        # Create a new node
        node = Node()
        # Try to load profile given
        result = self.LoadProfile(profile, filepath, node)
        if not result:
            # if success, initialising node
            self.CurrentNode = node
            self.CurrentNode.SetNodeName(name)
            self.CurrentNode.SetNodeID(id)
            self.CurrentNode.SetNodeType(type)
            self.CurrentNode.SetNodeDescription(description)
            AddIndexList = self.GetMandatoryIndexes()
            AddSubIndexList = []
            if NMT == "NodeGuarding":
                AddIndexList.extend([0x100C, 0x100D])
            elif NMT == "Heartbeat":
                AddIndexList.append(0x1017)
            for option in options:
                if option == "DS302":
                    DS302Path = os.path.join(os.path.split(__file__)[0], "config/DS-302.prf")
                    # Charging DS-302 profile if choosen by user
                    if os.path.isfile(DS302Path):
                        try:
                            execfile(DS302Path)
                            self.CurrentNode.SetDS302Profile(Mapping)
                            self.CurrentNode.ExtendSpecificMenu(AddMenuEntries)
                        except:
                            return _("Problem with DS-302! Syntax Error.")
                    else:
                        return _("Couldn't find DS-302 in 'config' folder!")
                elif option == "GenSYNC":
                    AddIndexList.extend([0x1005, 0x1006])
                elif option == "Emergency":
                    AddIndexList.append(0x1014)
                elif option == "SaveConfig":
                    AddIndexList.extend([0x1010, 0x1011, 0x1020])
                elif option == "StoreEDS":
                    AddIndexList.extend([0x1021, 0x1022])
            if type == "slave":
                # add default SDO server
                AddIndexList.append(0x1200)
                # add default 4 receive and 4 transmit PDO
                for comm, mapping in [(0x1400, 0x1600),(0x1800, 0x1A00)]:
                    firstparamindex = self.GetLineFromIndex(comm)
                    firstmappingindex = self.GetLineFromIndex(mapping)
                    AddIndexList.extend(range(firstparamindex, firstparamindex + 4))
                    for idx in range(firstmappingindex, firstmappingindex + 4):
                        AddIndexList.append(idx)
                        AddSubIndexList.append((idx, 8))
            # Add a new buffer 
            index = self.AddNodeBuffer(self.CurrentNode.Copy(), False)
            self.SetCurrentFilePath("")
            # Add Mandatory indexes
            self.ManageEntriesOfCurrent(AddIndexList, [])
            for idx, num in AddSubIndexList:
                self.AddSubentriesToCurrent(idx, num)
            return index
        else:
            return result
    
    """
    Load a profile in node
    """
    def LoadProfile(self, profile, filepath, node):
        if profile != "None":
            # Try to charge the profile given
            try:
                execfile(filepath)
                node.SetProfileName(profile)
                node.SetProfile(Mapping)
                node.SetSpecificMenu(AddMenuEntries)
                return None
            except:
                return _("Syntax Error\nBad OD Profile file!")
        else:
            # Default profile
            node.SetProfileName("None")
            node.SetProfile({})
            node.SetSpecificMenu([])
            return None

    """
    Open a file and store it in a new buffer
    """
    def OpenFileInCurrent(self, filepath):
        try:
            # Open and load file
            file = open(filepath, "r")
            node = load(file)
            file.close()
            self.CurrentNode = node
            self.CurrentNode.SetNodeID(0)
            # Add a new buffer and defining current state
            index = self.AddNodeBuffer(self.CurrentNode.Copy(), True)
            self.SetCurrentFilePath(filepath)
            return index
        except:
            return _("Unable to load file \"%s\"!")%filepath

    """
    Save current node in  a file
    """
    def SaveCurrentInFile(self, filepath = None):
        # if no filepath given, verify if current node has a filepath defined
        if not filepath:
            filepath = self.GetCurrentFilePath()
            if filepath == "":
                return False
        # Save node in file
        file = open(filepath, "w")
        dump(self.CurrentNode, file)
        file.close()
        self.SetCurrentFilePath(filepath)
        # Update saved state in buffer
        self.UndoBuffers[self.NodeIndex].CurrentSaved()
        return True

    """
    Close current state
    """
    def CloseCurrent(self, ignore = False):
        # Verify if it's not forced that the current node is saved before closing it
        if self.NodeIndex in self.UndoBuffers and (self.UndoBuffers[self.NodeIndex].IsCurrentSaved() or ignore):
            self.RemoveNodeBuffer(self.NodeIndex)
            if len(self.UndoBuffers) > 0:
                previousindexes = [idx for idx in self.UndoBuffers.keys() if idx < self.NodeIndex]
                nextindexes = [idx for idx in self.UndoBuffers.keys() if idx > self.NodeIndex]
                if len(previousindexes) > 0:
                    previousindexes.sort()
                    self.NodeIndex = previousindexes[-1]
                elif len(nextindexes) > 0:
                    nextindexes.sort()
                    self.NodeIndex = nextindexes[0]
                else:
                    self.NodeIndex = None
            else:
                self.NodeIndex = None
            return True
        return False

    """
    Import an eds file and store it in a new buffer if no node edited
    """
    def ImportCurrentFromEDSFile(self, filepath):
        # Generate node from definition in a xml file
        result = eds_utils.GenerateNode(filepath)
        if isinstance(result, Node):
            self.CurrentNode = result
            index = self.AddNodeBuffer(self.CurrentNode.Copy(), False)
            self.SetCurrentFilePath("")
            return index
        else:
            return result
    
    """
    Export to an eds file and store it in a new buffer if no node edited
    """
    def ExportCurrentToEDSFile(self, filepath):
        return eds_utils.GenerateEDSFile(filepath, self.CurrentNode)
    
    """
    Build the C definition of Object Dictionary for current node 
    """
    def ExportCurrentToCFile(self, filepath):
        if self.CurrentNode:
            return gen_cfile.GenerateFile(filepath, self.CurrentNode)

#-------------------------------------------------------------------------------
#                        Add Entries to Current Functions
#-------------------------------------------------------------------------------

    """
    Add the specified number of subentry for the given entry. Verify that total
    number of subentry (except 0) doesn't exceed nbmax defined
    """
    def AddSubentriesToCurrent(self, index, number, node = None):
        disable_buffer = node != None
        if node == None:
            node = self.CurrentNode
        # Informations about entry
        length = node.GetEntry(index, 0)
        infos = node.GetEntryInfos(index)
        subentry_infos = node.GetSubentryInfos(index, 1)
        # Get default value for subindex
        if "default" in subentry_infos:
            default = subentry_infos["default"]
        else:
            default = self.GetTypeDefaultValue(subentry_infos["type"])   
        # First case entry is record
        if infos["struct"] & OD_IdenticalSubindexes: 
            for i in xrange(1, min(number,subentry_infos["nbmax"]-length) + 1):
                node.AddEntry(index, length + i, default)
            if not disable_buffer:
                self.BufferCurrentNode()
            return None
        # Second case entry is array, only possible for manufacturer specific
        elif infos["struct"] & OD_MultipleSubindexes and 0x2000 <= index <= 0x5FFF:
            values = {"name" : "Undefined", "type" : 5, "access" : "rw", "pdo" : True}
            for i in xrange(1, min(number,0xFE-length) + 1):
                node.AddMappingEntry(index, length + i, values = values.copy())
                node.AddEntry(index, length + i, 0)
            if not disable_buffer:
                self.BufferCurrentNode()
            return None
            

    """
    Remove the specified number of subentry for the given entry. Verify that total
    number of subentry (except 0) isn't less than 1
    """
    def RemoveSubentriesFromCurrent(self, index, number):
        # Informations about entry
        infos = self.GetEntryInfos(index)
        length = self.CurrentNode.GetEntry(index, 0)
        if "nbmin" in infos:
            nbmin = infos["nbmin"]
        else:
            nbmin = 1
        # Entry is a record, or is an array of manufacturer specific
        if infos["struct"] & OD_IdenticalSubindexes or 0x2000 <= index <= 0x5FFF and infos["struct"] & OD_IdenticalSubindexes:
            for i in xrange(min(number, length - nbmin)):
                self.RemoveCurrentVariable(index, length - i)
            self.BufferCurrentNode()

    """
    Add a SDO Server to current node
    """
    def AddSDOServerToCurrent(self):
        # An SDO Server is already defined at index 0x1200
        if self.CurrentNode.IsEntry(0x1200):
            indexlist = [self.GetLineFromIndex(0x1201)]
            if None not in indexlist:
                self.ManageEntriesOfCurrent(indexlist, [])
        # Add an SDO Server at index 0x1200
        else:
            self.ManageEntriesOfCurrent([0x1200], [])
        
    """
    Add a SDO Server to current node
    """
    def AddSDOClientToCurrent(self):
        indexlist = [self.GetLineFromIndex(0x1280)]
        if None not in indexlist:
            self.ManageEntriesOfCurrent(indexlist, [])

    """
    Add a Transmit PDO to current node
    """
    def AddPDOTransmitToCurrent(self):
        indexlist = [self.GetLineFromIndex(0x1800),self.GetLineFromIndex(0x1A00)]
        if None not in indexlist:
            self.ManageEntriesOfCurrent(indexlist, [])
        
    """
    Add a Receive PDO to current node
    """
    def AddPDOReceiveToCurrent(self):
        indexlist = [self.GetLineFromIndex(0x1400),self.GetLineFromIndex(0x1600)]
        if None not in indexlist:
            self.ManageEntriesOfCurrent(indexlist, [])

    """
    Add a list of entries defined in profile for menu item selected to current node
    """
    def AddSpecificEntryToCurrent(self, menuitem):
        indexlist = []
        for menu, indexes in self.CurrentNode.GetSpecificMenu():
            if menuitem == menu:
                for index in indexes:
                    indexlist.append(self.GetLineFromIndex(index))
        if None not in indexlist:
            self.ManageEntriesOfCurrent(indexlist, [])

    """
    Search the first index available for a pluri entry from base_index
    """
    def GetLineFromIndex(self, base_index):
        found = False
        index = base_index
        infos = self.GetEntryInfos(base_index)
        while index < base_index + infos["incr"]*infos["nbmax"] and not found:
            if not self.CurrentNode.IsEntry(index):
                found = True
            else:
                index += infos["incr"]
        if found:
            return index
        return None
    
    """
    Add entries specified in addinglist and remove entries specified in removinglist
    """
    def ManageEntriesOfCurrent(self, addinglist, removinglist, node = None):
        disable_buffer = node != None
        if node == None:
            node = self.CurrentNode
        # Add all the entries in addinglist
        for index in addinglist:
            infos = self.GetEntryInfos(index)
            if infos["struct"] & OD_MultipleSubindexes:
                # First case entry is a record
                if infos["struct"] & OD_IdenticalSubindexes:
                    subentry_infos = self.GetSubentryInfos(index, 1)
                    if "default" in subentry_infos:
                        default = subentry_infos["default"]
                    else:
                        default = self.GetTypeDefaultValue(subentry_infos["type"])
                    node.AddEntry(index, value = [])
                    if "nbmin" in subentry_infos:
                        for i in xrange(subentry_infos["nbmin"]):
                            node.AddEntry(index, i + 1, default)
                    else:
                        node.AddEntry(index, 1, default)
                # Second case entry is a record
                else:
                    i = 1
                    subentry_infos = self.GetSubentryInfos(index, i)
                    while subentry_infos:
                        if "default" in subentry_infos:
                            default = subentry_infos["default"]
                        else:
                            default = self.GetTypeDefaultValue(subentry_infos["type"])
                        node.AddEntry(index, i, default)
                        i += 1
                        subentry_infos = self.GetSubentryInfos(index, i)
            # Third case entry is a record
            else:
                subentry_infos = self.GetSubentryInfos(index, 0)
                if "default" in subentry_infos:
                    default = subentry_infos["default"]
                else:
                    default = self.GetTypeDefaultValue(subentry_infos["type"])
                node.AddEntry(index, 0, default)
        # Remove all the entries in removinglist
        for index in removinglist:
            self.RemoveCurrentVariable(index)
        if not disable_buffer:
            self.BufferCurrentNode()
        return None


    """
    Reset an subentry from current node to its default value
    """
    def SetCurrentEntryToDefault(self, index, subindex, node = None):
        disable_buffer = node != None
        if node == None:
            node = self.CurrentNode
        if node.IsEntry(index, subindex):
            subentry_infos = self.GetSubentryInfos(index, subindex)
            if "default" in subentry_infos:
                default = subentry_infos["default"]
            else:
                default = self.GetTypeDefaultValue(subentry_infos["type"])
            node.SetEntry(index, subindex, default)
            if not disable_buffer:
                self.BufferCurrentNode()

    """
    Remove an entry from current node. Analize the index to perform the correct
    method
    """
    def RemoveCurrentVariable(self, index, subIndex = None):
        Mappings = self.CurrentNode.GetMappings()
        if index < 0x1000 and subIndex == None:
            type = self.CurrentNode.GetEntry(index, 1)
            for i in Mappings[-1]:
                for value in Mappings[-1][i]["values"]:
                    if value["type"] == index:
                        value["type"] = type
            self.CurrentNode.RemoveMappingEntry(index)
            self.CurrentNode.RemoveEntry(index)
        elif index == 0x1200 and subIndex == None:
            self.CurrentNode.RemoveEntry(0x1200)
        elif 0x1201 <= index <= 0x127F and subIndex == None:
            self.CurrentNode.RemoveLine(index, 0x127F)
        elif 0x1280 <= index <= 0x12FF and subIndex == None:
            self.CurrentNode.RemoveLine(index, 0x12FF)
        elif 0x1400 <= index <= 0x15FF or 0x1600 <= index <= 0x17FF and subIndex == None:
            if 0x1600 <= index <= 0x17FF and subIndex == None:
                index -= 0x200
            self.CurrentNode.RemoveLine(index, 0x15FF)
            self.CurrentNode.RemoveLine(index + 0x200, 0x17FF)
        elif 0x1800 <= index <= 0x19FF or 0x1A00 <= index <= 0x1BFF and subIndex == None:
            if 0x1A00 <= index <= 0x1BFF:
                index -= 0x200
            self.CurrentNode.RemoveLine(index, 0x19FF)
            self.CurrentNode.RemoveLine(index + 0x200, 0x1BFF)
        else:
            found = False
            for menu,list in self.CurrentNode.GetSpecificMenu():
                for i in list:
                    iinfos = self.GetEntryInfos(i)
                    indexes = [i + incr * iinfos["incr"] for incr in xrange(iinfos["nbmax"])] 
                    if index in indexes:
                        found = True
                        diff = index - i
                        for j in list:
                            jinfos = self.GetEntryInfos(j)
                            self.CurrentNode.RemoveLine(j + diff, j + jinfos["incr"]*jinfos["nbmax"], jinfos["incr"])
            self.CurrentNode.RemoveMapVariable(index, subIndex)
            if not found:
                infos = self.GetEntryInfos(index)
                if not infos["need"]:
                    self.CurrentNode.RemoveEntry(index, subIndex)
            if index in Mappings[-1]:
                self.CurrentNode.RemoveMappingEntry(index, subIndex)

    def AddMapVariableToCurrent(self, index, name, struct, number, node = None):
        if 0x2000 <= index <= 0x5FFF:
            disable_buffer = node != None
            if node == None:
                node = self.CurrentNode
            if not node.IsEntry(index):
                node.AddMappingEntry(index, name = name, struct = struct)
                if struct == var:
                    values = {"name" : name, "type" : 0x05, "access" : "rw", "pdo" : True}
                    node.AddMappingEntry(index, 0, values = values)
                    node.AddEntry(index, 0, 0)
                else:
                    values = {"name" : "Number of Entries", "type" : 0x05, "access" : "ro", "pdo" : False}
                    node.AddMappingEntry(index, 0, values = values)
                    if struct == rec:
                        values = {"name" : name + " %d[(sub)]", "type" : 0x05, "access" : "rw", "pdo" : True, "nbmax" : 0xFE}
                        node.AddMappingEntry(index, 1, values = values)
                        for i in xrange(number):
                            node.AddEntry(index, i + 1, 0)
                    else:
                        for i in xrange(number):
                            values = {"name" : "Undefined", "type" : 0x05, "access" : "rw", "pdo" : True}
                            node.AddMappingEntry(index, i + 1, values = values)
                            node.AddEntry(index, i + 1, 0)
                if not disable_buffer:
                    self.BufferCurrentNode()
                return None
            else:
                return _("Index 0x%04X already defined!")%index
        else:
            return _("Index 0x%04X isn't a valid index for Map Variable!")%index

    def AddUserTypeToCurrent(self, type, min, max, length):
        index = 0xA0
        while index < 0x100 and self.CurrentNode.IsEntry(index):
            index += 1
        if index < 0x100:
            customisabletypes = self.GetCustomisableTypes()
            name, valuetype = customisabletypes[type]
            size = self.GetEntryInfos(type)["size"]
            default = self.GetTypeDefaultValue(type)
            if valuetype == 0:
                self.CurrentNode.AddMappingEntry(index, name = "%s[%d-%d]"%(name, min, max), struct = 3, size = size, default = default)
                self.CurrentNode.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x05, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 1, values = {"name" : "Type", "type" : 0x05, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 2, values = {"name" : "Minimum Value", "type" : type, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 3, values = {"name" : "Maximum Value", "type" : type, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddEntry(index, 1, type)
                self.CurrentNode.AddEntry(index, 2, min)
                self.CurrentNode.AddEntry(index, 3, max)
            elif valuetype == 1:
                self.CurrentNode.AddMappingEntry(index, name = "%s%d"%(name, length), struct = 3, size = length * size, default = default)
                self.CurrentNode.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x05, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 1, values = {"name" : "Type", "type" : 0x05, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 2, values = {"name" : "Length", "type" : 0x05, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddEntry(index, 1, type)
                self.CurrentNode.AddEntry(index, 2, length)
            self.BufferCurrentNode()
            return None
        else:
            return _("Too many User Types have already been defined!")

#-------------------------------------------------------------------------------
#                      Modify Entry and Mapping Functions
#-------------------------------------------------------------------------------

    def SetCurrentEntryCallbacks(self, index, value):
        if self.CurrentNode and self.CurrentNode.IsEntry(index):
            entry_infos = self.GetEntryInfos(index)
            if "callback" not in entry_infos:
                self.CurrentNode.SetParamsEntry(index, None, callback = value)
                self.BufferCurrentNode()

    def SetCurrentEntry(self, index, subIndex, value, name, editor, node = None):
        disable_buffer = node != None
        if node == None:
            node = self.CurrentNode
        if node and node.IsEntry(index):
            if name == "value":
                if editor == "map":
                    value = node.GetMapValue(value)
                    if value is not None:
                        node.SetEntry(index, subIndex, value)
                elif editor == "bool":
                    value = value == "True"
                    node.SetEntry(index, subIndex, value)
                elif editor == "time":
                    node.SetEntry(index, subIndex, value)
                elif editor == "number":
                    try:
                        node.SetEntry(index, subIndex, int(value))
                    except:
                        pass
                elif editor == "float":
                    try:
                        node.SetEntry(index, subIndex, float(value))
                    except:
                        pass
                elif editor == "domain":
                    try:
                        if len(value) % 2 != 0:
                            value = "0" + value
                        value = value.decode('hex_codec')
                        node.SetEntry(index, subIndex, value)
                    except:
                        pass
                elif editor == "dcf":
                    node.SetEntry(index, subIndex, value)
                else:
                    subentry_infos = self.GetSubentryInfos(index, subIndex)
                    type = subentry_infos["type"]
                    dic = {}
                    for typeindex, typevalue in CustomisableTypes:
                        dic[typeindex] = typevalue
                    if type not in dic:
                        type = node.GetEntry(type)[1]
                    if dic[type] == 0:
                        try:
                            if value.startswith("$NODEID"):
                                value = "\"%s\""%value
                            elif value.startswith("0x"):
                                value = int(value, 16)
                            else:
                                value = int(value)
                            node.SetEntry(index, subIndex, value)
                        except:
                            pass
                    else:
                        node.SetEntry(index, subIndex, value)
            elif name in ["comment", "save"]:
                if editor == "option":
                    value = value == "Yes"
                if name == "save":
                    node.SetParamsEntry(index, subIndex, save = value)
                elif name == "comment":
                    node.SetParamsEntry(index, subIndex, comment = value)
            else:
                if editor == "type":
                    value = self.GetTypeIndex(value)
                    size = self.GetEntryInfos(value)["size"]
                    node.UpdateMapVariable(index, subIndex, size)
                elif editor in ["access","raccess"]:
                    dic = {}
                    for abbrev,access in AccessType.iteritems():
                        dic[access] = abbrev
                    value = dic[value]
                    if editor == "raccess" and not node.IsMappingEntry(index):
                        entry_infos = self.GetEntryInfos(index)
                        subIndex0_infos = self.GetSubentryInfos(index, 0, False).copy()
                        subIndex1_infos = self.GetSubentryInfos(index, 1, False).copy()
                        node.AddMappingEntry(index, name = entry_infos["name"], struct = 7)
                        node.AddMappingEntry(index, 0, values = subIndex0_infos)
                        node.AddMappingEntry(index, 1, values = subIndex1_infos)
                node.SetMappingEntry(index, subIndex, values = {name : value})
            if not disable_buffer:
                self.BufferCurrentNode()
            return None

    def SetCurrentEntryName(self, index, name):
        self.CurrentNode.SetMappingEntry(index, name=name)
        self.BufferCurrentNode()

    def SetCurrentUserType(self, index, type, min, max, length):
        customisabletypes = self.GetCustomisableTypes()
        values, valuetype = self.GetCustomisedTypeValues(index)
        name, new_valuetype = customisabletypes[type]
        size = self.GetEntryInfos(type)["size"]
        default = self.GetTypeDefaultValue(type)
        if new_valuetype == 0:
            self.CurrentNode.SetMappingEntry(index, name = "%s[%d-%d]"%(name, min, max), struct = 3, size = size, default = default) 
            if valuetype == 1:
                self.CurrentNode.SetMappingEntry(index, 2, values = {"name" : "Minimum Value", "type" : type, "access" : "ro", "pdo" : False})
                self.CurrentNode.AddMappingEntry(index, 3, values = {"name" : "Maximum Value", "type" : type, "access" : "ro", "pdo" : False})
            self.CurrentNode.SetEntry(index, 1, type)
            self.CurrentNode.SetEntry(index, 2, min)
            if valuetype == 1:
                self.CurrentNode.AddEntry(index, 3, max)
            else:
                self.CurrentNode.SetEntry(index, 3, max)
        elif new_valuetype == 1:
            self.CurrentNode.SetMappingEntry(index, name = "%s%d"%(name, length), struct = 3, size = size, default = default)
            if valuetype == 0:
                self.CurrentNode.SetMappingEntry(index, 2, values = {"name" : "Length", "type" : 0x02, "access" : "ro", "pdo" : False})
                self.CurrentNode.RemoveMappingEntry(index, 3)
            self.CurrentNode.SetEntry(index, 1, type)
            self.CurrentNode.SetEntry(index, 2, length)
            if valuetype == 0:
                self.CurrentNode.RemoveEntry(index, 3)
        self.BufferCurrentNode()

#-------------------------------------------------------------------------------
#                      Current Buffering Management Functions
#-------------------------------------------------------------------------------

    def BufferCurrentNode(self):
        self.UndoBuffers[self.NodeIndex].Buffering(self.CurrentNode.Copy())

    def CurrentIsSaved(self):
        return self.UndoBuffers[self.NodeIndex].IsCurrentSaved()

    def OneFileHasChanged(self):
        result = False
        for buffer in self.UndoBuffers.values():
            result |= not buffer.IsCurrentSaved()
        return result

    def GetBufferNumber(self):
        return len(self.UndoBuffers)

    def GetBufferIndexes(self):
        return self.UndoBuffers.keys()

    def LoadCurrentPrevious(self):
        self.CurrentNode = self.UndoBuffers[self.NodeIndex].Previous().Copy()
    
    def LoadCurrentNext(self):
        self.CurrentNode = self.UndoBuffers[self.NodeIndex].Next().Copy()

    def AddNodeBuffer(self, currentstate = None, issaved = False):
        self.NodeIndex = GetNewId()
        self.UndoBuffers[self.NodeIndex] = UndoBuffer(currentstate, issaved)
        self.FilePaths[self.NodeIndex] = ""
        self.FileNames[self.NodeIndex] = ""
        return self.NodeIndex

    def ChangeCurrentNode(self, index):
        if index in self.UndoBuffers.keys():
            self.NodeIndex = index
            self.CurrentNode = self.UndoBuffers[self.NodeIndex].Current().Copy()
    
    def RemoveNodeBuffer(self, index):
        self.UndoBuffers.pop(index)
        self.FilePaths.pop(index)
        self.FileNames.pop(index)
    
    def GetCurrentNodeIndex(self):
        return self.NodeIndex
    
    def GetCurrentFilename(self):
        return self.GetFilename(self.NodeIndex)
    
    def GetAllFilenames(self):
        indexes = self.UndoBuffers.keys()
        indexes.sort()
        return [self.GetFilename(idx) for idx in indexes]
    
    def GetFilename(self, index):
        if self.UndoBuffers[index].IsCurrentSaved():
            return self.FileNames[index]
        else:
            return "~%s~"%self.FileNames[index]
    
    def SetCurrentFilePath(self, filepath):
        self.FilePaths[self.NodeIndex] = filepath
        if filepath == "":
            self.LastNewIndex += 1
            self.FileNames[self.NodeIndex] = _("Unnamed%d")%self.LastNewIndex
        else:
            self.FileNames[self.NodeIndex] = os.path.splitext(os.path.basename(filepath))[0]
                
    def GetCurrentFilePath(self):
        if len(self.FilePaths) > 0:
            return self.FilePaths[self.NodeIndex]
        else:
            return ""
    
    def GetCurrentBufferState(self):
        first = self.UndoBuffers[self.NodeIndex].IsFirst()
        last = self.UndoBuffers[self.NodeIndex].IsLast()
        return not first, not last

#-------------------------------------------------------------------------------
#                         Profiles Management Functions
#-------------------------------------------------------------------------------

    def GetCurrentCommunicationLists(self):
        list = []
        for index in MappingDictionary.iterkeys():
            if 0x1000 <= index < 0x1200:
                list.append(index)
        return self.GetProfileLists(MappingDictionary, list)
    
    def GetCurrentDS302Lists(self):
        return self.GetSpecificProfileLists(self.CurrentNode.GetDS302Profile())
    
    def GetCurrentProfileLists(self):
        return self.GetSpecificProfileLists(self.CurrentNode.GetProfile())
    
    def GetSpecificProfileLists(self, mappingdictionary):
        validlist = []
        exclusionlist = []
        for name, list in self.CurrentNode.GetSpecificMenu():
            exclusionlist.extend(list)
        for index in mappingdictionary.iterkeys():
            if index not in exclusionlist:
                validlist.append(index)
        return self.GetProfileLists(mappingdictionary, validlist)
    
    def GetProfileLists(self, mappingdictionary, list):
        dictionary = {}
        current = []
        for index in list:
            dictionary[index] = (mappingdictionary[index]["name"], mappingdictionary[index]["need"])
            if self.CurrentNode.IsEntry(index):
                current.append(index)
        return dictionary, current

    def GetCurrentNextMapIndex(self):
        if self.CurrentNode:
            index = 0x2000
            while self.CurrentNode.IsEntry(index) and index < 0x5FFF:
                index += 1
            if index < 0x6000:
                return index
            else:
                return None

    def CurrentDS302Defined(self):
        if self.CurrentNode:
            return len(self.CurrentNode.GetDS302Profile()) > 0
        return False

#-------------------------------------------------------------------------------
#                         Node State and Values Functions
#-------------------------------------------------------------------------------
    
    def GetCurrentNodeName(self):
        if self.CurrentNode:
            return self.CurrentNode.GetNodeName()
        else:
            return ""

    def GetCurrentNodeCopy(self):
        if self.CurrentNode:
            return self.CurrentNode.Copy()
        else:
            return None
    
    def GetCurrentNodeID(self, node = None):
        if self.CurrentNode:
            return self.CurrentNode.GetNodeID()
        else:
            return None

    def GetCurrentNodeInfos(self):
        name = self.CurrentNode.GetNodeName()
        id = self.CurrentNode.GetNodeID()
        type = self.CurrentNode.GetNodeType()
        description = self.CurrentNode.GetNodeDescription()
        return name, id, type, description
    
    def SetCurrentNodeInfos(self, name, id, type, description):
        self.CurrentNode.SetNodeName(name)
        self.CurrentNode.SetNodeID(id)
        self.CurrentNode.SetNodeType(type)
        self.CurrentNode.SetNodeDescription(description)
        self.BufferCurrentNode()

    def GetCurrentNodeDefaultStringSize(self):
        if self.CurrentNode:
            return self.CurrentNode.GetDefaultStringSize()
        else:
            return Node.DefaultStringSize
    
    def SetCurrentNodeDefaultStringSize(self, size):
        if self.CurrentNode:
            self.CurrentNode.SetDefaultStringSize(size)
        else:
            Node.DefaultStringSize = size

    def GetCurrentProfileName(self):
        if self.CurrentNode:
            return self.CurrentNode.GetProfileName()
        return ""

    def IsCurrentEntry(self, index):
        if self.CurrentNode:
            return self.CurrentNode.IsEntry(index)
        return False
    
    def GetCurrentEntry(self, index, subIndex = None, compute = True):
        if self.CurrentNode:
            return self.CurrentNode.GetEntry(index, subIndex, compute)
        return None
    
    def GetCurrentParamsEntry(self, index, subIndex = None):
        if self.CurrentNode:
            return self.CurrentNode.GetParamsEntry(index, subIndex)
        return None
    
    def GetCurrentValidIndexes(self, min, max):
        validindexes = []
        for index in self.CurrentNode.GetIndexes():
            if min <= index <= max:
                validindexes.append((self.GetEntryName(index), index))
        return validindexes
        
    def GetCurrentValidChoices(self, min, max):
        validchoices = []
        exclusionlist = []
        for menu, indexes in self.CurrentNode.GetSpecificMenu():
            exclusionlist.extend(indexes)
            good = True
            for index in indexes:
                good &= min <= index <= max
            if good:
                validchoices.append((menu, None))
        list = [index for index in MappingDictionary.keys() if index >= 0x1000]
        profiles = self.CurrentNode.GetMappings(False)
        for profile in profiles:
            list.extend(profile.keys())
        list.sort()
        for index in list:
            if min <= index <= max and not self.CurrentNode.IsEntry(index) and index not in exclusionlist:
                validchoices.append((self.GetEntryName(index), index))
        return validchoices
    
    def HasCurrentEntryCallbacks(self, index):
        if self.CurrentNode:
            return self.CurrentNode.HasEntryCallbacks(index)
        return False
    
    def GetCurrentEntryValues(self, index):
        if self.CurrentNode:
            return self.GetNodeEntryValues(self.CurrentNode, index)
    
    def GetNodeEntryValues(self, node, index):
        if node and node.IsEntry(index):
            entry_infos = node.GetEntryInfos(index)
            data = []
            editors = []
            values = node.GetEntry(index, compute = False)
            params = node.GetParamsEntry(index)
            if isinstance(values, ListType):
                for i, value in enumerate(values):
                    data.append({"value" : value})
                    data[-1].update(params[i])      
            else:
                data.append({"value" : values})
                data[-1].update(params)
            for i, dic in enumerate(data):
                infos = node.GetSubentryInfos(index, i)
                dic["subindex"] = "0x%02X"%i
                dic["name"] = infos["name"]
                dic["type"] = node.GetTypeName(infos["type"])
                if dic["type"] is None:
                    dic["type"] = "Unknown"
                dic["access"] = AccessType[infos["access"]]
                dic["save"] = OptionType[dic["save"]]
                editor = {"subindex" : None, "name" : None, 
                          "type" : None, "value" : None,
                          "access" : None, "save" : "option", 
                          "callback" : "option", "comment" : "string"}
                if isinstance(values, ListType) and i == 0:
                    if 0x1600 <= index <= 0x17FF or 0x1A00 <= index <= 0x1C00:
                        editor["access"] = "raccess"
                else:
                    if infos["user_defined"]:
                        if entry_infos["struct"] & OD_IdenticalSubindexes:
                            if i == 0:
                                editor["type"] = "type"
                                editor["access"] = "access"
                        else:
                            if entry_infos["struct"] & OD_MultipleSubindexes:
                                editor["name"] = "string"
                            editor["type"] = "type"
                            editor["access"] = "access"
                    if index < 0x260:
                        if i == 1:
                            dic["value"] = node.GetTypeName(dic["value"])
                    elif 0x1600 <= index <= 0x17FF or 0x1A00 <= index <= 0x1C00:
                        editor["value"] = "map"
                        dic["value"] = node.GetMapName(dic["value"])
                    else:
                        if dic["type"].startswith("VISIBLE_STRING") or dic["type"].startswith("OCTET_STRING"):
                            editor["value"] = "string"
                        elif dic["type"] in ["TIME_OF_DAY","TIME_DIFFERENCE"]:
                            editor["value"] = "time"
                        elif dic["type"] == "DOMAIN":
                            if index == 0x1F22:
                                editor["value"] = "dcf"
                            else:
                                editor["value"] = "domain"
                            dic["value"] = dic["value"].encode('hex_codec')
                        elif dic["type"] == "BOOLEAN":
                            editor["value"] = "bool"
                            dic["value"] = BoolType[dic["value"]]
                        result = type_model.match(dic["type"])
                        if result:
                            values = result.groups()
                            if values[0] == "UNSIGNED":
                                try:
                                    format = "0x%0" + str(int(values[1])/4) + "X"
                                    dic["value"] = format%dic["value"]
                                except:
                                    pass
                                editor["value"] = "string"
                            if values[0] == "INTEGER":
                                editor["value"] = "number"
                            elif values[0] == "REAL":
                                editor["value"] = "float"
                            elif values[0] in ["VISIBLE_STRING", "OCTET_STRING"]:
                                editor["length"] = values[0]
                        result = range_model.match(dic["type"])
                        if result:
                            values = result.groups()
                            if values[0] in ["UNSIGNED", "INTEGER", "REAL"]:
                                editor["min"] = values[2]
                                editor["max"] = values[3]
                editors.append(editor)
            return data, editors
        else:
            return None

    def AddToDCF(self, node_id, index, subindex, size, value):
        if self.CurrentNode.IsEntry(0x1F22, node_id):
            dcf_value = self.CurrentNode.GetEntry(0x1F22, node_id)
            if dcf_value != "":
                nbparams = BE_to_LE(dcf_value[:4])
            else:
                nbparams = 0
            new_value = LE_to_BE(nbparams + 1, 4) + dcf_value[4:]
            new_value += LE_to_BE(index, 2) + LE_to_BE(subindex, 1) + LE_to_BE(size, 4) + LE_to_BE(value, size)
            self.CurrentNode.SetEntry(0x1F22, node_id, new_value)

#-------------------------------------------------------------------------------
#                         Node Informations Functions
#-------------------------------------------------------------------------------

    def GetCustomisedTypeValues(self, index):
        if self.CurrentNode:
            values = self.CurrentNode.GetEntry(index)
            customisabletypes = self.GetCustomisableTypes()
            return values, customisabletypes[values[1]][1]
        else:
            return None, None

    def GetEntryName(self, index, compute=True):
        if self.CurrentNode:
            return self.CurrentNode.GetEntryName(index, compute)
        else:
            return FindEntryName(index, MappingDictionary, compute)
    
    def GetEntryInfos(self, index, compute=True):
        if self.CurrentNode:
            return self.CurrentNode.GetEntryInfos(index, compute)
        else:
            return FindEntryInfos(index, MappingDictionary, compute)
    
    def GetSubentryInfos(self, index, subindex, compute=True):
        if self.CurrentNode:
            return self.CurrentNode.GetSubentryInfos(index, subindex, compute)
        else:
            result = FindSubentryInfos(index, subindex, MappingDictionary, compute)
            if result:
                result["user_defined"] = False
            return result
    
    def GetTypeIndex(self, typename):
        if self.CurrentNode:
            return self.CurrentNode.GetTypeIndex(typename)
        else:
            return FindTypeIndex(typename, MappingDictionary)
    
    def GetTypeName(self, typeindex):
        if self.CurrentNode:
            return self.CurrentNode.GetTypeName(typeindex)
        else:
            return FindTypeName(typeindex, MappingDictionary)
    
    def GetTypeDefaultValue(self, typeindex):
        if self.CurrentNode:
            return self.CurrentNode.GetTypeDefaultValue(typeindex)
        else:
            return FindTypeDefaultValue(typeindex, MappingDictionary)
    
    def GetMapVariableList(self, compute=True):
        if self.CurrentNode:
            return self.CurrentNode.GetMapVariableList(compute)
        else:
            return []

    def GetMandatoryIndexes(self):
        if self.CurrentNode:
            return self.CurrentNode.GetMandatoryIndexes()
        else:
            return FindMandatoryIndexes(MappingDictionary)
    
    def GetCustomisableTypes(self):
        dic = {}
        for index, valuetype in CustomisableTypes:
            name = self.GetTypeName(index)
            dic[index] = [name, valuetype]
        return dic
    
    def GetCurrentSpecificMenu(self):
        if self.CurrentNode:
            return self.CurrentNode.GetSpecificMenu()
        return []

