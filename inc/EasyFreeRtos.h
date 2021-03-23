/*
 * EasyFreeRtos.h
 *
 *  Created on: 27 ���. 2021 �.
 *      Author: kgn
 */

#ifndef EASYFREERTOS_H_
#define EASYFREERTOS_H_

#include <vector>
#include <cmsis_os.h>
#include <task.h>
#include <plainNet.h>

namespace kvpr {
namespace freertos {

class EasyFreeRtos;
class FreeRtosUser;

typedef struct {
	FreeRtosUser* userPointer = NULL;
	void* userParams = NULL;
} FreeRtosUserData;

class FreeRtosUser {

public:
	virtual void freeRtosUser__onThreadCall(FreeRtosUser* userInstance, void* params) {};

	virtual void freeRtosUser__onThreadStart(FreeRtosUser* userInstance, void* params) {};
};

class EasyFreeRtos {

private:
	EasyFreeRtos();
	virtual ~EasyFreeRtos();
	static ::std::vector<FreeRtosUserData*> freeRtosUsers_;
	static void allThreadsCallback(void * argument);
	static bool shedulerStarted_;

public:
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
	static TaskHandle_t createThread(const char* name, uint32_t stackSize, osPriority priority, FreeRtosUser* user, void* params);
#else
	static TaskHandle_t createThread(const char* name, uint32_t stackSize, UBaseType_t priority, FreeRtosUser* user, void* params);
#endif
};

}
}

#endif /* EASYFREERTOS_H_ */
