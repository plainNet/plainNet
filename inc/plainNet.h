/*
 * plainNet.h
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#ifndef PLAINNET_H_
#define PLAINNET_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Base library errors
 */
#define ERR_SOCKET_CREATE_FAILED 							-1
#define ERR_SOCKET_NON_BLOCKING_MODE_SET_FAILED 			-2
#define ERR_SOCKET_BIND_FAILED 								-3
#define ERR_SOCKET_LISTEN_FAILED 							-4


/**
 * Base library defines
 */
#ifdef PLAINNET_USER_CONFIG
#include PLAINNET_USER_CONFIG
#endif

#ifndef PLAINNET_USE_CMISIS_RTOS_PRIORITIES
#define PLAINNET_USE_CMISIS_RTOS_PRIORITIES					0
#endif

#ifndef PLAINNET_USE_INNER_MBED_TLS
#define PLAINNET_USE_INNER_MBED_TLS							1
#endif


/**
 * Base library params
 */
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
#include <cmsis_os.h>
#else
#include <task.h>
#endif

/**
 * Base library types
 */
#ifdef __cplusplus
typedef struct {
	char* taskName = NULL;
	uint32_t taskStackSize = 1024;
	#ifdef PLAINNET_USE_CMISIS_RTOS_PRIORITIES
	osPriority taskPriority = osPriority::osPriorityNormal;
	#else
	UBaseType_t taskPriority = 3;
	#endif/*USE_CMISIS_RTOS_PRIORITIES*/
} HostStartParams;
#else
typedef struct {
	char* taskName;
	uint32_t taskStackSize;
	#ifdef PLAINNET_USE_CMISIS_RTOS_PRIORITIES
	osPriority taskPriority;
	#else
	UBaseType_t taskPriority;
	#endif/*USE_CMISIS_RTOS_PRIORITIES*/
} HostStartParams;
#endif

/**
 * UdpHost params
 */
#ifndef UDP_HOST_TASK_DEFAULT_STACK_SIZE
#define UDP_HOST_TASK_DEFAULT_STACK_SIZE 					1024
#endif/*UDP_HOST_TASK_DEFAULT_STACK_SIZE*/

#ifndef UDP_HOST_TASK_DEFAULT_PRIORITY
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
#define UDP_HOST_TASK_DEFAULT_PRIORITY 						(osPriority::osPriorityNormal)
#else
#define UDP_HOST_TASK_DEFAULT_PRIORITY 						3
#endif/*USE_CMISIS_RTOS_PRIORITIES*/
#endif/*UDP_HOST_TASK_DEFAULT_PRIORITY*/

#ifndef UDP_HOST_POLL_TIMEOUT_IN_SECONDS
#define UDP_HOST_POLL_TIMEOUT_IN_SECONDS 					10
#endif/*UDP_HOST_POLL_TIMEOUT_IN_SECONDS*/

#ifndef UDP_HOST_RX_BUF_SIZE
#define UDP_HOST_RX_BUF_SIZE 								512
#endif/*UDP_HOST_RX_BUF_SIZE*/

#ifndef UDP_HOST_TASK_DEFAULT_NAME
#define UDP_HOST_TASK_DEFAULT_NAME 							"udp-host"
#endif/*UDP_HOST_DEFAULT_TASK_NAME*/

/**
 * TcpHost params
 */
#ifndef TCP_HOST_TASK_DEFAULT_STACK_SIZE
#define TCP_HOST_TASK_DEFAULT_STACK_SIZE 					1024
#endif/*TCP_HOST_TASK_DEFAULT_STACK_SIZE*/

#ifndef TCP_HOST_TASK_DEFAULT_PRIORITY
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
#define TCP_HOST_TASK_DEFAULT_PRIORITY 						(osPriority::osPriorityNormal)
#else
#define TCP_HOST_TASK_DEFAULT_PRIORITY 						3
#endif/*USE_CMISIS_RTOS_PRIORITIES*/
#endif/*TCP_HOST_TASK_DEFAULT_PRIORITY*/

#ifndef TCP_HOST_POLL_TIMEOUT_IN_SECONDS
#define TCP_HOST_POLL_TIMEOUT_IN_SECONDS 					10
#endif/*TCP_HOST_POLL_TIMEOUT_IN_SECONDS*/

#ifndef TCP_HOST_RX_BUF_SIZE
#define TCP_HOST_RX_BUF_SIZE 								512
#endif/*TCP_HOST_RX_BUF_SIZE*/

#ifndef TCP_HOST_MAX_TX_SEGMENT_SIZE
#define TCP_HOST_MAX_TX_SEGMENT_SIZE 						1024
#endif/*TCP_HOST_MAX_TX_SEGMENT_SIZE*/

#ifndef TCP_HOST_TASK_DEFAULT_NAME
#define TCP_HOST_TASK_DEFAULT_NAME 							"tcp-host"
#endif/*TCP_HOST_DEFAULT_TASK_NAME*/


/**
 * HttpHost params
 */
#ifndef PLAINNET_USE_DEFAULT_HTTP_RESOURCES
#define PLAINNET_USE_DEFAULT_HTTP_RESOURCES					0
#endif

#if PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1
#ifndef PLAINNET_DEFAULT_HTTP_RESOURCES_INCLUDE_FILE
#define PLAINNET_DEFAULT_HTTP_RESOURCES_INCLUDE_FILE		<HttpHostDefaultResources.h>
#endif/*PLAINNET_DEFAULT_HTTP_RESOURCES_INCLUDE_FILE*/
#endif/*PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1*/

#ifdef __cplusplus
}
#endif

#endif /* PLAINNET_H_ */
