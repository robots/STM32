Generator of Canopen SYNC message (cobid = 0x80)
---------------------------------------------
francis.dupin@inrets.fr
27 Janv 2005. 
Revised 13 Dec 2006


Status of the code :
====================
Tested with 
- CanFestival 3 rc2 (cvs version)
- gcc port for HC12 release 3.1
- Microcontroler MC9S12DP256 
on board  T-board (http://www.elektronikladen.de)



To build the example
--------------------
a) Compile CanFestival for hcs12 :
  Go to the root of CanFestival and enter
  ./configure --target=hcs12
  make clean all

b) Build the example

 cd examples/gene_SYNC_HCS12
 make clean all


What does the node ?
====================
Just reset it, it should send the SYNC (cobId 0x80) every 10 ms
Informations availables if you connect the serial port 0 to a terminal configured at 38400 8N1
				   




The node default values :
nodeId = 0x03
CAN rate = 250 kbps
Please read appli.c, these values can be modified by switch.


If you put the node in operational state, the CAN messages received are filtered : Only the NMT and Nodeguard can be received.
The parameters of the filter are mapped in the object dictionary, so that the filter can be configured by SDO before entering in operational state. See the object dictionary index 2015 to 2023. To have the values applied, always download at 0x2023 index 0 the value 1 before entering in operational.

Read the file objdict.c to see the capabilities of the node.



To change the period of SYNC,
-----------------------------
In pre-operational mode, send the SDO message cobid | .... (all in hexa):
0x603 | 23 06 10 00 40 42 0F 00 
to have a SYNC generated every 1 second. The change is instantaneous.
(Assume that the nodeId is 3).


To read the name of the node
----------------------------
(Assume that the nodeId is 3).
You must use the segmented SDO protocole to make an upload from index
0x1008 subindex 0x00

c : client
s : server (geneSync)

Here is the dialog you must obtain.
c: 0x603 | 40 08 10 00 00 00 00 00
s: 0x583 | 41 08 10 00 0A 00 00 00
c: 0x603 | 60 00 00 00 00 00 00 00
s: 0x583 | 00 47 45 4E 45 5F 53 59
c: 0x603 | 70 00 00 00 00 00 00 00
s: 0x583 | 19 4E 43 00 00 00 00 00

In the server's frames, you should read GENE_SYNC. (yes ...)

If in the exchange you are not responding too faster (less than 3
seconds. See in config.h #define SDO_TIMEOUT_MS),
the nodes send a SDO abort :
0x583 | 08 08 10 00 00 00 04 05












