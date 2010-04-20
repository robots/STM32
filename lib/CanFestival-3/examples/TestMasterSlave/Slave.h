#include "TestSlave.h"

void TestSlave_heartbeatError(CO_Data* d, UNS8);

UNS8 TestSlave_canSend(Message *);

void TestSlave_initialisation(CO_Data* d);
void TestSlave_preOperational(CO_Data* d);
void TestSlave_operational(CO_Data* d);
void TestSlave_stopped(CO_Data* d);

void TestSlave_post_sync(CO_Data* d);
void TestSlave_post_TPDO(CO_Data* d);
void TestSlave_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
void TestSlave_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);
