/*
 * HttpHostDefaultResources.h
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#ifndef HTTPHOSTDEFAULTRESOURCES_H_
#define HTTPHOSTDEFAULTRESOURCES_H_

#include <plainNet.h>

#if PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1


#include <HttpHostResources.h>

kvpr::network::HttpHostResources* plainnet_getDefaultHttpResources();


#endif /* PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1 */

#endif /* HTTPHOSTDEFAULTRESOURCES_H_ */
