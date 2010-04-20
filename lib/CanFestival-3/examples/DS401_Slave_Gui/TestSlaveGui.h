#ifndef TESTSLAVEGUI_H_
#define TESTSLAVEGUI_H_

extern "C"
{
#include "canfestival.h"
}

int main_can (s_BOARD SlaveBoard, char *LibraryPath);
void stop_slave ();

#endif /*TESTSLAVEGUI_H_ */
