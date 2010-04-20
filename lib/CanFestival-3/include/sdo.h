/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/** @defgroup comobj Communication Objects
 *  @ingroup userapi
 */
 
/** @defgroup sdo Service Data Object (SDO)
 *  SDOs provide the access to entries in the CANopen Object Dictionary.
 * 	An SDO is made up of at least two CAN messages with different identifiers.
 * 	SDO s are always confirmed point-to-point communications services. 
 *  @ingroup comobj
 */
 
#ifndef __sdo_h__
#define __sdo_h__

struct struct_s_transfer;

#include "timer.h"

typedef void (*SDOCallback_t)(CO_Data* d, UNS8 nodeId);

/* The Transfer structure
Used to store the different segments of
 - a SDO received before writing in the dictionary
 - the reading of the dictionary to put on a SDO to transmit
*/

struct struct_s_transfer {
  UNS8           nodeId;     /**<own ID if server, or node ID of the server if client */

  UNS8           whoami;     /**< Takes the values SDO_CLIENT or SDO_SERVER */
  UNS8           state;      /**< state of the transmission : Takes the values SDO_... */
  UNS8           toggle;	
  UNS32          abortCode;  /**< Sent or received */
  /**< index and subindex of the dictionary where to store */
  /**< (for a received SDO) or to read (for a transmit SDO) */
  UNS16          index;
  UNS8           subIndex;
  UNS32          count;      /**< Number of data received or to be sent. */
  UNS32          offset;     /**< stack pointer of data[]
                              * Used only to tranfer part of a line to or from a SDO.
                              * offset is always pointing on the next free cell of data[].
                              * WARNING s_transfer.data is subject to ENDIANISATION
                              * (with respect to CANOPEN_BIG_ENDIAN)
                              */
  UNS8           data [SDO_MAX_LENGTH_TRANSFERT];
  UNS8           dataType;   /**< Defined in objdictdef.h Value is visible_string
                              * if it is a string, any other value if it is not a string,
                              * like 0. In fact, it is used only if client.
                              */
  TIMER_HANDLE   timer;      /**< Time counter to implement a timeout in milliseconds.
                              * It is automatically incremented whenever
                              * the line state is in SDO_DOWNLOAD_IN_PROGRESS or
                              * SDO_UPLOAD_IN_PROGRESS, and reseted to 0
                              * when the response SDO have been received.
                              */
  SDOCallback_t Callback;   /**< The user callback func to be called at SDO transaction end */
};
typedef struct struct_s_transfer s_transfer;


#include "data.h"


struct BODY{
    UNS8 data[8]; /**< The 8 bytes data of the SDO */
};

/* The SDO structure ...*/
struct struct_s_SDO {
  UNS8 nodeId;		/**< In any case, Node ID of the server (case sender or receiver).*/
  struct BODY body;
};


typedef struct struct_s_SDO s_SDO;

/** 
 * @brief Reset of a SDO exchange on timeout.
 * Send a SDO abort.
 * @param *d Pointer on a CAN object data structure
 * @param id
 */
void SDOTimeoutAlarm(CO_Data* d, UNS32 id);

/** 
 * @brief Reset all SDO buffers.
 * @param *d Pointer on a CAN object data structure
 */
void resetSDO (CO_Data* d);


/** 
 * @brief Copy the data received from the SDO line transfert to the object dictionary.
 * @param *d Pointer on a CAN object data structure
 * @param line SDO line
 * @return SDO error code if error. Else, returns 0.
 */
UNS32 SDOlineToObjdict (CO_Data* d, UNS8 line);

/** 
 * @brief Copy the data from the object dictionary to the SDO line for a network transfert.
 * @param *d Pointer on a CAN object data structure
 * @param line SDO line
 * @return SDO error code if error. Else, returns 0.
 */
UNS32 objdictToSDOline (CO_Data* d, UNS8 line);

/** 
 * @brief Copy data from an existant line in the argument "* data"
 * @param d Pointer on a CAN object data structure
 * @param line SDO line
 * @param nbBytes
 * @param *data Pointer on the data
 * @return 0xFF if error. Else, returns 0.
 */
UNS8 lineToSDO (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8 * data);

/** 
 * @brief Add data to an existant line
 * @param d Pointer on a CAN object data structure
 * @param line SDO line
 * @param nbBytes
 * @param *data Pointer on the data
 * @return 0xFF if error. Else, returns 0.
 */
UNS8 SDOtoLine (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8 * data);

/** 
 * @brief Called when an internal SDO abort occurs.
 * Release the line * Only if server *
 * If client, the line must be released manually in the core application.
 * The reason of that is to permit the program to read the transfers structure before its reset,
 * because many informations are stored on it : index, subindex, data received or trasmited, ...
 * In all cases, sends a SDO abort.
 * @param *d Pointer on a CAN object data structure
 * @param nodeId
 * @param whoami
 * @param index
 * @param subIndex
 * @param abortCode
 * @return 0
 */
UNS8 failedSDO (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS16 index, UNS8 subIndex, UNS32 abortCode);

/** 
 * @brief Reset an unused line.
 * @param *d Pointer on a CAN object data structure
 * @param line SDO line
 */
void resetSDOline (CO_Data* d, UNS8 line);

/** 
 * @brief Initialize some fields of the structure.
 * @param *d Pointer on a CAN object data structure
 * @param line
 * @param nodeId
 * @param index
 * @param subIndex
 * @param state
 * @return 0
 */
UNS8 initSDOline (CO_Data* d, UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state);

/** 
 * @brief Search for an unused line in the transfers array
 * to store a new SDO.
 * ie a line which value of the field "state" is "SDO_RESET"
 * An unused line have the field "state" at the value SDO_RESET
 * @param *d Pointer on a CAN object data structure
 * @param whoami Create the line for a SDO_SERVER or SDO_CLIENT.
 * @param *line Pointer on a SDO line 
 * @return 0xFF if all the lines are on use. Else, return 0.
 */
UNS8 getSDOfreeLine (CO_Data* d, UNS8 whoami, UNS8 *line);

/** 
 * @brief Search for the line, in the transfers array, which contains the
 * beginning of the reception of a fragmented SDO
 * @param *d Pointer on a CAN object data structure
 * @param nodeId correspond to the message node-id
 * @param whoami takes 2 values : look for a line opened as SDO_CLIENT or SDO_SERVER
 * @param *line Pointer on a SDO line 
 * @return 0xFF if error.  Else, return 0
 */
UNS8 getSDOlineOnUse (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS8 *line);

/** 
 * @brief Close a transmission.
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Node id of the server if both server or client
 * @param whoami Line opened as SDO_CLIENT or SDO_SERVER
 */
UNS8 closeSDOtransfer (CO_Data* d, UNS8 nodeId, UNS8 whoami);

/** 
 * @brief Bytes in the line structure which must be transmited (or received)
 * @param *d Pointer on a CAN object data structure
 * @param line SDO line 
 * @param *nbBytes Pointer on nbBytes
 * @return 0.
 */
UNS8 getSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 * nbBytes);

/** 
 * @brief Store in the line structure the nb of bytes which must be transmited (or received)
 * @param *d Pointer on a CAN object data structure
 * @param line SDO line 
 * @param nbBytes
 * @return 0 if success, 0xFF if error.
 */
UNS8 setSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 nbBytes);

/**
 * @brief Transmit a SDO frame on the bus bus_id
 * @param *d Pointer on a CAN object data structure
 * @param whoami Takes 2 values : SDO_CLIENT or SDO_SERVER
 * @param sdo SDO Structure which contains the sdo to transmit
 * @return canSend(bus_id,&m) or 0xFF if error.
 */
UNS8 sendSDO (CO_Data* d, UNS8 whoami, s_SDO sdo);

/** 
 * @brief Transmit a SDO error to the client. The reasons may be :
 * Read/Write to a undefined object
 * Read/Write to a undefined subindex
 * Read/write a not valid length object
 * Write a read only object
 * @param *d Pointer on a CAN object data structure
 * @param whoami takes 2 values : SDO_CLIENT or SDO_SERVER
 * @param nodeId
 * @param index
 * @param subIndex
 * @param abortCode
 * @return 0
 */
UNS8 sendSDOabort (CO_Data* d, UNS8 whoami, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS32 abortCode);

/** 
 * @brief Treat a SDO frame reception
 * call the function sendSDO
 * @param *d Pointer on a CAN object data structure
 * @param *m Pointer on a CAN message structure 
 * @return code : 
 * 		   - 0xFF if error
 *         - 0x80 if transfert aborted by the server
 *         - 0x0  ok
 */
UNS8 proceedSDO (CO_Data* d, Message *m);

/** 
 * @ingroup sdo
 * @brief Used to send a SDO request frame to write the data at the index and subIndex indicated
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param count number of bytes to write in the dictionnary.
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @param *data Pointer to data
 * @return 
 * - 0 is returned upon success.
 * - 0xFE is returned when no sdo client to communicate with node.
 * - 0xFF is returned when error occurs.
 */
UNS8 writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data);

/** 
 * @ingroup sdo
 * @brief Used to send a SDO request frame to write in a distant node dictionnary.
 * @details The function Callback which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort).
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param count number of bytes to write in the dictionnary.
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @param *data Pointer to data
 * @param Callback Callback function
 * @return 
 * - 0 is returned upon success.
 * - 0xFE is returned when no sdo client to communicate with node.
 * - 0xFF is returned when error occurs.
 */
UNS8 writeNetworkDictCallBack (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback);

/**
 * @ingroup sdo 
 * @brief Used to send a SDO request frame to write in a distant node dictionnary.
 * @details The function Callback which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort). First free SDO client parameter is
 * automatically initialized for specific node if not already defined.
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param count number of bytes to write in the dictionnary.
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @param *data Pointer to data
 * @param Callback Callback function
 * @param endianize When not 0, data is endianized into network byte order
 *                  when 0, data is not endianized and copied in machine native
 *                  endianness
 * @return 
 * - 0 is returned upon success.
 * - 0xFF is returned when error occurs.
 */
UNS8 writeNetworkDictCallBackAI (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize);

/**
 * @ingroup sdo 
 * @brief Used to send a SDO request frame to read.
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @return 
 * - 0 is returned upon success.
 * - 0xFE is returned when no sdo client to communicate with node.
 * - 0xFF is returned when error occurs.
 */
UNS8 readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType);

/** 
 * @ingroup sdo
 * @brief Used to send a SDO request frame to read in a distant node dictionnary.
 * @details The function Callback which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort).
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @param Callback Callback function
 * @return 
 * - 0 is returned upon success.
 * - 0xFE is returned when no sdo client to communicate with node.
 * - 0xFF is returned when error occurs.
 */
UNS8 readNetworkDictCallback (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);

/** 
 * @ingroup sdo
 * @brief Used to send a SDO request frame to read in a distant node dictionnary.
 * @details The function Callback which must be defined in the user code is called at the
 * end of the exchange. (on succes or abort). First free SDO client parameter is
 * automatically initialized for specific node if not already defined.
 * @param *d Pointer on a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param index At index indicated
 * @param subIndex At subIndex indicated
 * @param dataType (defined in objdictdef.h) : put "visible_string" for strings, 0 for integers or reals or other value.
 * @param Callback Callback function
 * @return 
 * - 0 is returned upon success.
 * - 0xFF is returned when error occurs.
 */
UNS8 readNetworkDictCallbackAI (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback);

/** 
 * @ingroup sdo
 * @brief Use this function after calling readNetworkDict to get the result.
 * 
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param *data Pointer to the datas
 * @param *size Pointer to the size
 * @param *abortCode Pointer to the abortcode. (0 = not available. Else : SDO abort code. (received if return SDO_ABORTED_RCV)
 * 
 * 
 * @return
 *           - SDO_FINISHED             // datas are available
 *           - SDO_ABORTED_RCV          // Transfert failed (abort SDO received)
 *           - SDO_ABORTED_INTERNAL     // Transfert failed (internal abort)
 *           - SDO_UPLOAD_IN_PROGRESS   // Datas are not yet available
 *           - SDO_DOWNLOAD_IN_PROGRESS // Download is in progress
 * \n\n
 * example :
 * @code
 * UNS32 data;
 * UNS8 size;
 * readNetworkDict(0, 0x05, 0x1016, 1, 0) // get the data index 1016 subindex 1 of node 5
 * while (getReadResultNetworkDict (0, 0x05, &data, &size) == SDO_UPLOAD_IN_PROGRESS);
 * @endcode
*/
UNS8 getReadResultNetworkDict (CO_Data* d, UNS8 nodeId, void* data, UNS32 *size, UNS32 * abortCode);

/**
 * @ingroup sdo
 * @brief Use this function after calling writeNetworkDict function to get the result of the write.
 * @details It is mandatory to call this function because it is releasing the line used for the transfer.
 * @param *d Pointer to a CAN object data structure
 * @param nodeId Node Id of the slave
 * @param *abortCode Pointer to the abortcode
 * - 0 = not available. 
 * - SDO abort code (received if return SDO_ABORTED_RCV)
 * 
 * @return : 
 *           - SDO_FINISHED             // datas are available
 *           - SDO_ABORTED_RCV          // Transfert failed (abort SDO received)
 *           - SDO_ABORTED_INTERNAL     // Transfert failed (Internal abort)
 *           - SDO_DOWNLOAD_IN_PROGRESS // Datas are not yet available
 *           - SDO_UPLOAD_IN_PROGRESS   // Upload in progress
 * \n\n
 * example :
 * @code
 * UNS32 data = 0x50;
 * UNS8 size;
 * UNS32 abortCode;
 * writeNetworkDict(0, 0x05, 0x1016, 1, size, &data) // write the data index 1016 subindex 1 of node 5
 * while (getWriteResultNetworkDict (0, 0x05, &abortCode) == SDO_DOWNLOAD_IN_PROGRESS);
 * @endcode
*/
UNS8 getWriteResultNetworkDict (CO_Data* d, UNS8 nodeId, UNS32 * abortCode);

#endif
