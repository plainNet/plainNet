/*
 * EasyFreeRtos.cpp
 *
 *  Created on: 27 янв. 2021 г.
 *      Author: kgn
 */

#include <EasyFreeRtos.h>
#include <stdlib.h>

namespace kvpr {
namespace freertos {

::std::vector<FreeRtosUserData*> EasyFreeRtos::freeRtosUsers_;
bool EasyFreeRtos::shedulerStarted_ = false;

EasyFreeRtos::EasyFreeRtos() {
	// TODO Auto-generated constructor stub

}

EasyFreeRtos::~EasyFreeRtos() {
	// TODO Auto-generated destructor stub
}

void EasyFreeRtos::allThreadsCallback(void* argument) {
	FreeRtosUserData* uData = (FreeRtosUserData*) argument;
	uData->userPointer->onThreadStart(uData->userPointer, uData->userParams);
	for(;;) {
		uData->userPointer->onThreadCall(uData->userPointer, uData->userParams);
	}
}
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
TaskHandle_t EasyFreeRtos::createThread(const char* name, uint32_t stackSize, osPriority priority, FreeRtosUser* user, void* params) {
	if(user == NULL) {
		return 0;
	}
	FreeRtosUserData* uData = (FreeRtosUserData*) malloc(sizeof(FreeRtosUserData));
	uData->userPointer = user;
	uData->userParams = params;
	EasyFreeRtos::freeRtosUsers_.push_back(uData);
	TaskHandle_t xHandle = NULL;
	xTaskCreate(EasyFreeRtos::allThreadsCallback, name, stackSize, uData, priority - osPriorityIdle, &xHandle);
	if(!EasyFreeRtos::shedulerStarted_) {
		EasyFreeRtos::shedulerStarted_ = true;
		vTaskStartScheduler();
	}
	return xHandle;
}
#else
TaskHandle_t EasyFreeRtos::createThread(const char* name, uint32_t stackSize, UBaseType_t priority, FreeRtosUser* user, void* params) {
	if(user == NULL) {
		return 0;
	}
	FreeRtosUserData* uData = (FreeRtosUserData*) malloc(sizeof(FreeRtosUserData));
	uData->userPointer = user;
	uData->userParams = params;
	EasyFreeRtos::freeRtosUsers_.push_back(uData);
	TaskHandle_t xHandle = NULL;
	xTaskCreate(EasyFreeRtos::allThreadsCallback, name, stackSize, uData, priority, &xHandle);
	if(!EasyFreeRtos::shedulerStarted_) {
		EasyFreeRtos::shedulerStarted_ = true;
		vTaskStartScheduler();
	}
	return xHandle;
}
#endif

}
}
