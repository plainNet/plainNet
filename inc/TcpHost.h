/*
 * TcpHost.h
 *
 *  Created on: 27 ���. 2021 �.
 *      Author: kgn
 */

#ifndef TCPHOST_H_
#define TCPHOST_H_

#include "EasyFreeRtos.h"
#include "semphr.h"
#include <lwip/sockets.h>
#include <plainNet.h>
#include <vector>

namespace kvpr {
namespace network {

class TcpHost;
class TcpHostListener;

class TcpHostListener {

public:

	virtual void tcpHost__clientConnected(int socket) {};

	virtual void tcpHost__clientDisconnected(int socket) {};

	virtual void tcpHost__clientDataReceived(int socket, uint8_t* data, uint32_t dataCount) {};

};

class TcpHost : public kvpr::freertos::FreeRtosUser {
private:
	uint16_t port_;
	int serverDescriptor_;
	struct sockaddr_in localHost_;
	std::vector<struct pollfd> descriptors_;
	std::vector<TcpHostListener*> listeners_;
	struct pollfd wrapSocket(int socket, uint16_t flags);
	uint8_t* rxBuf_ = nullptr;
	TcpHostListener* child_ = nullptr;
	SemaphoreHandle_t txSmphr_;

protected:
	virtual void freeRtosUser__onThreadCall(kvpr::freertos::FreeRtosUser* userInstance, void* params) override;
	virtual void freeRtosUser__onThreadStart(kvpr::freertos::FreeRtosUser* userInstance, void* params) override;
	void setChild(TcpHostListener* child);

public:
	TcpHost(uint16_t port);
	virtual ~TcpHost();
	int start(HostStartParams* params, bool singletoneThread);
	void addListener(TcpHostListener* l);
	bool transmit(int socket, uint8_t* data, uint32_t dataCount);
	void finish(int socket);
};

} /* namespace network */
} /* namespace kvpr */

#endif /* TCPHOST_H_ */
