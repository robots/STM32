#ifndef CANFESTIVAL_H_
#define CANFESTIVAL_H_

#include "timerscfg.h"
#include "can_driver.h"
#include "data.h"
#include "timers_driver.h"

typedef void* LIB_HANDLE;

/** @defgroup userapi User API */

/** @defgroup can CAN management
 *  @ingroup userapi
 */
/**
 * @ingroup can
 * @brief Load CAN driver interface.
 * @param *driver_name The location of the library to load
 * @return
 *       - handle of the CAN driver interface is returned upon success.
 *       - NULL is returned if the CAN driver interface can't be loaded.
 */
#define LoadCanDriver(driver_name) 1

/**
 * @brief Send a CAN message
 * @param port CanFestival file descriptor
 * @param *m The CAN message to send
 * @return 0 if succes
 */
UNS8 canSend(CAN_PORT port, Message *m);

/**
 * @ingroup can
 * @brief Open a CANOpen device
 * @param *board Pointer to the board structure that contains busname and baudrate 
 * @param *d Pointer to the CAN object data structure
 * @return
 *       - CanFestival file descriptor is returned upon success.
 *       - NULL is returned if the CANOpen board can't be opened.
 */
CAN_PORT canOpen(s_BOARD *board, CO_Data * d);

/**
 * @ingroup can
 * @brief Close a CANOpen device
 * @param *d Pointer to the CAN object data structure
 * @return
 *       - 0 is returned upon success.
 *       - errorcode if error. (if implemented)  
 */
int canClose(CO_Data * d);

/**
 * @ingroup can
 * @brief Change the CANOpen device baudrate 
 * @param port CanFestival file descriptor 
 * @param *baud The new baudrate to assign
 * @return
 *       - 0 is returned upon success or if not supported by the CAN driver.
 *       - errorcode from the CAN driver is returned if an error occurs. (if implemented in the CAN driver)
 */
UNS8 canChangeBaudRate(CAN_PORT port, char* baud);

#endif
