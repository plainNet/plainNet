/*
 * UdpHost.cpp
 *
 *  Created on: 27 ���. 2021 �.
 *      Author: kgn
 */

#include <UdpHost.h>

namespace kvpr {
namespace network {

std::vector<struct pollfd> UdpHost::singletoneFD_;
std::vector<UdpHost*> UdpHost::singletoneInstances_;

UdpHost::UdpHost(uint16_t port) {
	this->port_ = port;
	this->rxBuf_ = (uint8_t*) malloc(UDP_HOST_RX_BUF_SIZE);
}

void UdpHost::freeRtosUser__onThreadStart(kvpr::freertos::FreeRtosUser* userInstance, void* params) {
}

void UdpHost::freeRtosUser__onThreadCall(kvpr::freertos::FreeRtosUser* userInstance, void* params) {
	int rc = this->singletone_ ?
		poll(&UdpHost::singletoneFD_[0], UdpHost::singletoneFD_.size(), UDP_HOST_POLL_TIMEOUT_IN_SECONDS * 1000)
		: poll(&this->fd_, 1, UDP_HOST_POLL_TIMEOUT_IN_SECONDS * 1000);
	if(rc > 0) {
		if(this->singletone_) {
			uint32_t size_ = UdpHost::singletoneInstances_.size();
			for(uint32_t i = 0; i < size_; i++) {
				UdpHost::singletoneInstances_[i]->poll_(&UdpHost::singletoneFD_[i]);
			}
		} else {
			this->poll_(&this->fd_);
		}
	}
}

UdpHost::~UdpHost() {
	// TODO Auto-generated destructor stub
}

int UdpHost::start(HostStartParams* params, bool singletoneThread) {
	this->singletone_ = singletoneThread;
	int on = 1;
	//create socket for incoming connections
	this->descriptor_ = socket(AF_INET, SOCK_DGRAM, 0);
	if(this->descriptor_ < 0) {
		return ERR_SOCKET_CREATE_FAILED;
	}
	//
	if(ioctl(this->descriptor_, FIONBIO, (char *)&on) < 0) {
		return ERR_SOCKET_NON_BLOCKING_MODE_SET_FAILED;
	}
	//
	memset(&this->localHost_, 0, sizeof(this->localHost_));
	this->localHost_.sin_family = AF_INET;
	this->localHost_.sin_addr.s_addr = INADDR_ANY;
	this->localHost_.sin_port = htons(this->port_);
	if(bind(this->descriptor_, (struct sockaddr *)&this->localHost_, sizeof(this->localHost_)) < 0) {
		close(this->descriptor_);
		return ERR_SOCKET_BIND_FAILED;
	}
	if(!this->singletone_ || UdpHost::singletoneFD_.size() == 0) {
		kvpr::freertos::EasyFreeRtos::createThread(
				(params == nullptr || params->taskName == nullptr) ? UDP_HOST_TASK_DEFAULT_NAME : params->taskName,
				(params == nullptr) ? UDP_HOST_TASK_DEFAULT_STACK_SIZE : params->taskStackSize,
				(params == nullptr) ? UDP_HOST_TASK_DEFAULT_PRIORITY : params->taskPriority,
				(kvpr::freertos::FreeRtosUser*) this,
				nullptr
		);
	}
	if(this->singletone_) {
		UdpHost::singletoneFD_.push_back(this->wrapSocket(this->descriptor_, POLLIN));
		UdpHost::singletoneInstances_.push_back(this);
	} else {
		this->fd_ = this->wrapSocket(this->descriptor_, POLLIN);
	}

	return 0;
}

struct pollfd UdpHost::wrapSocket(int socket, uint16_t flags) {
	struct pollfd fd;
	fd.fd = socket;
	fd.events = flags;
	return fd;
}

int UdpHost::poll_(struct pollfd* descriptor) {
	if((descriptor->revents & POLLIN)) {
		struct sockaddr from_;
		socklen_t addrlen = sizeof(from_);
		memset(&from_, 0, sizeof(from_));
		ssize_t size = recvfrom(descriptor->fd, this->rxBuf_, UDP_HOST_RX_BUF_SIZE, MSG_WAITALL, &from_, &addrlen);
		descriptor->revents = 0;
		if(size < 0) {
			close(descriptor->fd);
			if(this->singletone_) {
				return -1;
			}
		} else {
			struct sockaddr_in* sin = (struct sockaddr_in *) &from_;
			UdpRemoteAddr addr;
			addr.ip[0] = (sin->sin_addr.s_addr) & 255;
			addr.ip[1] = (sin->sin_addr.s_addr >> 8) & 255;
			addr.ip[2] = (sin->sin_addr.s_addr >> 16) & 255;
			addr.ip[3] = (sin->sin_addr.s_addr >> 24) & 255;
			addr.port = htons(sin->sin_port);
			for(uint32_t i = 0; i < this->listeners_.size(); i++) {
				if(this->listeners_[i]) {
					this->listeners_[i]->udpHost__clientDataReceived(&addr, this->rxBuf_, size);
				}
			}
		}
	}
	return 0;
}

void UdpHost::addListener(UdpHostListener* l) {
	if(l == nullptr) {
		return;
	}
	this->listeners_.push_back(l);
}

void UdpHost::tx(UdpRemoteAddr* to, uint8_t* data, uint32_t dataCount) {
	if(!to) {
		return;
	}
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = 0;
	for(uint32_t i = 4; i > 0; i--) {
		sin.sin_addr.s_addr <<= 8;
		sin.sin_addr.s_addr |= to->ip[i - 1];
	}
	sin.sin_port = htons(to->port);
	sin.sin_family = AF_INET;
	sendto(this->descriptor_, data, dataCount, 1, (struct sockaddr*) &sin, sizeof(sockaddr_in));
}

} /* namespace network */
} /* namespace kvpr */
