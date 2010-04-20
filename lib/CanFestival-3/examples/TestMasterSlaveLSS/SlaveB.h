#include "TestSlaveB.h"

void TestSlaveB_heartbeatError(CO_Data* d, UNS8);

UNS8 TestSlaveB_canSend(Message *);

void TestSlaveB_initialisation(CO_Data* d);
void TestSlaveB_preOperational(CO_Data* d);
void TestSlaveB_operational(CO_Data* d);
void TestSlaveB_stopped(CO_Data* d);

void TestSlaveB_post_sync(CO_Data* d);
void TestSlaveB_post_TPDO(CO_Data* d);
void TestSlaveB_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
void TestSlaveB_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
void TestSlaveB_NMT_Slave_Communications_Reset_Callback(CO_Data* d);
void TestSlaveB_StoreConfiguration(CO_Data* d, UNS8 *error, UNS8 *spec_error);
