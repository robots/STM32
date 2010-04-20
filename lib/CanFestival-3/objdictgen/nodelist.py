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

from node import *
import eds_utils
import os, shutil, types

#-------------------------------------------------------------------------------
#                          Definition of NodeList Object
#-------------------------------------------------------------------------------

"""
Class recording a node list for a CANOpen network.
"""

class NodeList:
    
    def __init__(self, manager, netname = ""):
        self.Root = ""
        self.EDSFolder = ""
        self.Manager = manager
        self.NetworkName = netname
        self.SlaveNodes = {}
        self.EDSNodes = {}
        self.CurrentSelected = None
        self.Changed = False
    
    def HasChanged(self):
        return self.Changed or not self.Manager.CurrentIsSaved()
    
    def ForceChanged(self, changed):
        self.Changed = changed
    
    def GetNetworkName(self):
        return self.NetworkName
    
    def SetNetworkName(self, name):
        self.NetworkName = name
    
    def GetManager(self):
        return self.Manager
    
    def GetRoot(self):
        return self.Root

    def SetRoot(self, newrootpath):
        """
        Define a new path for the CanOpen network project
        !!! Does not check if new path is valid !!!
        """
        self.Root = newrootpath
        self.Manager.SetCurrentFilePath(os.path.join(self.Root, "master.od"))
    
    def GetSlaveNumber(self):
        return len(self.SlaveNodes)
    
    def GetSlaveNames(self):
        nodes = self.SlaveNodes.keys()
        nodes.sort()
        return ["0x%2.2X %s"%(idx, self.SlaveNodes[idx]["Name"]) for idx in nodes]
    
    def GetSlaveIDs(self):
        nodes = self.SlaveNodes.keys()
        nodes.sort()
        return nodes
        
    def SetCurrentSelected(self, selected):
        self.CurrentSelected = selected
        
    def GetCurrentSelected(self):
        return self.CurrentSelected
            
    def LoadProject(self, root, netname = None):
        self.SlaveNodes = {}
        self.EDSNodes = {}
        
        self.Root = root
        if not os.path.exists(self.Root):
            return _("\"%s\" folder doesn't exist")%self.Root
        
        self.EDSFolder = os.path.join(self.Root, "eds")
        if not os.path.exists(self.EDSFolder):
            os.mkdir(self.EDSFolder)
            #return "\"%s\" folder doesn't contain a \"eds\" folder"%self.Root
        
        files = os.listdir(self.EDSFolder)
        for file in files:
            filepath = os.path.join(self.EDSFolder, file)
            if os.path.isfile(filepath) and os.path.splitext(filepath)[-1] == ".eds":
                result = self.LoadEDS(file)
                if result != None:
                    return result
                
        result = self.LoadMasterNode(netname)
        if result != None:
            return result
            
        result = self.LoadSlaveNodes(netname)
        if result != None:
            return result
        
        self.NetworkName = netname
    
    def SaveProject(self, netname = None):
        result = self.SaveMasterNode(netname)
        if result != None:
            return result
            
        result = self.SaveNodeList(netname)
        if result != None:
            return result
    
    def ImportEDSFile(self, edspath, force = False):
        dir, file = os.path.split(edspath)
        eds = os.path.join(self.EDSFolder, file)
        if not force and os.path.isfile(eds):
            return _("EDS file already imported")
        else:
            shutil.copy(edspath, self.EDSFolder)
            return self.LoadEDS(file)
    
    def LoadEDS(self, eds):
        edspath = os.path.join(self.EDSFolder, eds)
        node = eds_utils.GenerateNode(edspath)
        if isinstance(node, Node):
            self.EDSNodes[eds] = node
            return None
        else:
            return node
    
    def AddSlaveNode(self, nodeName, nodeID, eds):
        if eds in self.EDSNodes.keys():
            slave = {"Name" : nodeName, "EDS" : eds, "Node" : self.EDSNodes[eds]}
            self.SlaveNodes[nodeID] = slave
            self.Changed = True
            return None
        else:
            return _("\"%s\" EDS file is not available")%eds
    
    def RemoveSlaveNode(self, index):
        if index in self.SlaveNodes.keys():
            self.SlaveNodes.pop(index)
            return None
        else:
            return _("Node with \"0x%2.2X\" ID doesn't exist")
    
    def LoadMasterNode(self, netname = None):
        if netname:
            masterpath = os.path.join(self.Root, "%s_master.od"%netname)
        else:
            masterpath = os.path.join(self.Root, "master.od")
        if os.path.isfile(masterpath):
            result = self.Manager.OpenFileInCurrent(masterpath)
        else:
            result = self.Manager.CreateNewNode("MasterNode", 0x00, "master", "", "None", "", "heartbeat", ["DS302"])
        if not isinstance(result, types.IntType):
            return result
        return None
    
    def SaveMasterNode(self, netname = None):
        if netname:
            masterpath = os.path.join(self.Root, "%s_master.od"%netname)
        else:
            masterpath = os.path.join(self.Root, "master.od")
        if self.Manager.SaveCurrentInFile(masterpath):
            return None
        else:
            return _("Fail to save Master Node")
    
    def LoadSlaveNodes(self, netname = None):
        cpjpath = os.path.join(self.Root, "nodelist.cpj")
        if os.path.isfile(cpjpath):
            try:
                networks = eds_utils.ParseCPJFile(cpjpath)
                network = None
                if netname:
                    for net in networks:
                        if net["Name"] == netname:
                            network = net
                    self.NetworkName = netname
                elif len(networks) > 0:
                    network = networks[0]
                    self.NetworkName = network["Name"]
                if network:
                    for nodeid, node in network["Nodes"].items():
                        if node["Present"] == 1:
                            result = self.AddSlaveNode(node["Name"], nodeid, node["DCFName"])
                            if result != None:
                                return result        
                self.Changed = False
            except SyntaxError, message:
                return _("Unable to load CPJ file\n%s")%message
        return None
    
    def SaveNodeList(self, netname = None):
        try:
            cpjpath = os.path.join(self.Root, "nodelist.cpj")
            content = eds_utils.GenerateCPJContent(self)
            if netname:
                file = open(cpjpath, "a")
            else:
                file = open(cpjpath, "w")
            file.write(content)
            file.close()
            self.Changed = False
            return None
        except:
            return _("Fail to save node list")
    
    def GetSlaveNodeEntry(self, nodeid, index, subindex = None):
        if nodeid in self.SlaveNodes.keys():
            self.SlaveNodes[nodeid]["Node"].SetNodeID(nodeid)
            return self.SlaveNodes[nodeid]["Node"].GetEntry(index, subindex)
        else:
            return _("Node 0x%2.2X doesn't exist")%nodeid

    def GetMasterNodeEntry(self, index, subindex = None):
        return self.Manager.GetCurrentEntry(index, subindex)
        
    def SetMasterNodeEntry(self, index, subindex = None, value = None):
        self.Manager.SetCurrentEntry(index, subindex, value)
    
    def GetOrderNumber(self, nodeid):
        nodeindexes = self.SlaveNodes.keys()
        nodeindexes.sort()
        return nodeindexes.index(nodeid) + 1
    
    def GetNodeByOrder(self, order):
        if order > 0:
            nodeindexes = self.SlaveNodes.keys()
            nodeindexes.sort()
            if order <= len(nodeindexes):
                return self.SlaveNodes[nodeindexes[order - 1]]["Node"]
        return None
    
    def IsCurrentEntry(self, index):
        if self.CurrentSelected != None:
            if self.CurrentSelected == 0:
                return self.Manager.IsCurrentEntry(index)
            else:
                node = self.SlaveNodes[self.CurrentSelected]["Node"]
                if node:
                    node.SetNodeID(self.CurrentSelected)
                    return node.IsEntry(index)
        return False
    
    def GetEntryInfos(self, index):
        if self.CurrentSelected != None:
            if self.CurrentSelected == 0:
                return self.Manager.GetEntryInfos(index)
            else:
                node = self.SlaveNodes[self.CurrentSelected]["Node"]
                if node:
                    node.SetNodeID(self.CurrentSelected)
                    return node.GetEntryInfos(index)
        return None

    def GetSubentryInfos(self, index, subindex):
        if self.CurrentSelected != None:
            if self.CurrentSelected == 0:
                return self.Manager.GetSubentryInfos(index, subindex)
            else:
                node = self.SlaveNodes[self.CurrentSelected]["Node"]
                if node:
                    node.SetNodeID(self.CurrentSelected)
                    return node.GetSubentryInfos(index, subindex)
        return None

    def GetCurrentValidIndexes(self, min, max):
        if self.CurrentSelected != None:
            if self.CurrentSelected == 0:
                return self.Manager.GetCurrentValidIndexes(min, max)
            else:
                node = self.SlaveNodes[self.CurrentSelected]["Node"]
                if node:
                    node.SetNodeID(self.CurrentSelected)
                    validindexes = []
                    for index in node.GetIndexes():
                        if min <= index <= max:
                            validindexes.append((node.GetEntryName(index), index))
                    return validindexes
                else:
                    print _("Can't find node")
        return []
    
    def GetCurrentEntryValues(self, index):
        if self.CurrentSelected != None:
            node = self.SlaveNodes[self.CurrentSelected]["Node"]
            if node:
                node.SetNodeID(self.CurrentSelected)
                return self.Manager.GetNodeEntryValues(node, index)
            else:
                print _("Can't find node")
        return [], []
    
    def AddToMasterDCF(self, node_id, index, subindex, size, value):
        # Adding DCF entry into Master node
        if not self.Manager.IsCurrentEntry(0x1F22):
            self.Manager.ManageEntriesOfCurrent([0x1F22], [])
        self.Manager.AddSubentriesToCurrent(0x1F22, 127)

        self.Manager.AddToDCF(node_id, index, subindex, size, value)
    
if __name__ == "__main__":
    from nodemanager import *
    import os, sys, shutil
    
    manager = NodeManager(sys.path[0])
    
    nodelist = NodeList(manager)
    
    result = nodelist.LoadProject("/home/laurent/test_nodelist")
    if result != None:
        print result
    else:
        print "MasterNode :"
        manager.CurrentNode.Print()
        print 
        for nodeid, node in nodelist.SlaveNodes.items():
            print "SlaveNode name=%s id=0x%2.2X :"%(node["Name"], nodeid)
            node["Node"].Print()
            print

