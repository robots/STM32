#include "TestSlaveA.h"

void TestSlaveA_heartbeatError(CO_Data* d, UNS8);

UNS8 TestSlaveA_canSend(Message *);

void TestSlaveA_initialisation(CO_Data* d);
void TestSlaveA_preOperational(CO_Data* d);
void TestSlaveA_operational(CO_Data* d);
void TestSlaveA_stopped(CO_Data* d);

void TestSlaveA_post_sync(CO_Data* d);
void TestSlaveA_post_TPDO(CO_Data* d);
void TestSlaveA_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
void TestSlaveA_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
void TestSlaveA_NMT_Slave_Communications_Reset_Callback(CO_Data* d);
void TestSlaveA_StoreConfiguration(CO_Data* d, UNS8 *error, UNS8 *spec_error);
