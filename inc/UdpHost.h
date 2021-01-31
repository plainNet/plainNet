/*
 * UdpHost.h
 *
 *  Created on: 27 џэт. 2021 у.
 *      Author: kgn
 */

#ifndef UDPHOST_H_
#define UDPHOST_H_

#include <EasyFreeRtos.h>
#include <plainNet.h>
#include <lwip/sockets.h>
#include <vector>

namespace kvpr {
namespace network {

class UdpHost;
class UdpHostListener;

class UdpHostListener {

public:

	virtual void udpHost__clientDataReceived(struct sockaddr* from, uint8_t* data, uint32_t dataCount) {};

};

class UdpHost : public kvpr::freertos::FreeRtosUser {
private:
	static std::vector<struct pollfd> singletoneFD_;
	static std::vector<UdpHost*> singletoneInstances_;
	bool singletone_ = false;
	uint16_t port_;
	int descriptor_;
	struct pollfd fd_;
	struct sockaddr_in localHost_;
	std::vector<UdpHostListener*> listeners_;
	struct pollfd wrapSocket(int socket, uint16_t flags);
	uint8_t* rxBuf_ = nullptr;
	int poll_(struct pollfd* fd);

protected:
	virtual void onThreadCall(kvpr::freertos::FreeRtosUser* userInstance, void* params) override;
	virtual void onThreadStart(kvpr::freertos::FreeRtosUser* userInstance, void* params) override;

public:
	UdpHost(uint16_t port);
	virtual ~UdpHost();
	int start(HostStartParams* params, bool singletoneThread);
	void addListener(UdpHostListener* l);
};

} /* namespace network */
} /* namespace kvpr */

#endif /* UDPHOST_H_ */
