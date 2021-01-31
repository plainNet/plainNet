/*
 * TcpHost.cpp
 *
 *  Created on: 27 џэт. 2021 у.
 *      Author: kgn
 */

#include "TcpHost.h"

namespace kvpr {
namespace network {

TcpHost::TcpHost(uint16_t port) {
	// TODO Auto-generated constructor stub
	this->port_ = port;
	this->rxBuf_ = (uint8_t*) malloc(TCP_HOST_RX_BUF_SIZE);
}

TcpHost::~TcpHost() {
	// TODO Auto-generated destructor stub
}

void TcpHost::onThreadStart(kvpr::freertos::FreeRtosUser* userInstance, void* params) {
	this->descriptors_.push_back(this->wrapSocket(this->serverDescriptor_, POLLIN));
}

void TcpHost::onThreadCall(kvpr::freertos::FreeRtosUser* userInstance, void* params) {
	int rc = poll(&this->descriptors_[0], this->descriptors_.size(), TCP_HOST_POLL_TIMEOUT_IN_SECONDS * 1000);
	if(rc > 0) {
		uint32_t size_ = this->descriptors_.size();
		int descriptorForDelete = -1;
		for(uint32_t i = 0; i < size_; i++) {
			if(this->descriptors_[i].revents == 0)  {
				continue;
			} else if(this->descriptors_[i].fd == this->serverDescriptor_) {
				if((this->descriptors_[i].revents & POLLIN)) {
					int inputConnectionSocket = accept(this->serverDescriptor_, NULL, NULL);
					if(inputConnectionSocket >= 0) {
						this->descriptors_.push_back(this->wrapSocket(inputConnectionSocket, POLLIN));
						if(this->child_) {
							this->child_->tcpHost__clientConnected(inputConnectionSocket);
						} else {
							for(uint32_t i = 0; i < this->listeners_.size(); i++) {
								if(this->listeners_[i]) {
									this->listeners_[i]->tcpHost__clientConnected(inputConnectionSocket);
								}
							}
						}
					}
				}
			} else {
				if((this->descriptors_[i].revents & POLLIN)) {
					ssize_t size = recv(this->descriptors_[i].fd, this->rxBuf_, TCP_HOST_RX_BUF_SIZE, MSG_WAITALL);
					if(size < 0) {
						close(this->descriptors_[i].fd);
						descriptorForDelete = i;
						if(this->child_) {
							this->child_->tcpHost__clientDisconnected(this->descriptors_[i].fd);
						} else {
							for(uint32_t i = 0; i < this->listeners_.size(); i++) {
								if(this->listeners_[i]) {
									this->listeners_[i]->tcpHost__clientDisconnected(this->descriptors_[i].fd);
								}
							}
						}
					} else {
						if(this->child_) {
							this->child_->tcpHost__clientDataReceived(this->descriptors_[i].fd, this->rxBuf_, size);
						} else {
							for(uint32_t i = 0; i < this->listeners_.size(); i++) {
								if(this->listeners_[i]) {
									this->listeners_[i]->tcpHost__clientDataReceived(this->descriptors_[i].fd, this->rxBuf_, size);
								}
							}
						}
					}
				}
			}
			this->descriptors_[i].revents = 0;
			if(descriptorForDelete >= 0) {
				break;
			}
		}
		if(descriptorForDelete >= 0) {
			close(this->descriptors_[descriptorForDelete].fd);
			this->descriptors_.erase(this->descriptors_.begin() + descriptorForDelete);
		}
	}
}

struct pollfd TcpHost::wrapSocket(int socket, uint16_t flags) {
	struct pollfd fd;
	fd.fd = socket;
	fd.events = flags;
	return fd;
}

int TcpHost::start(HostStartParams* params, bool singletoneThread) {
	int on = 1;
	//create socket for incoming connections
	this->serverDescriptor_ = socket(AF_INET, SOCK_STREAM, 0);
	if(this->serverDescriptor_ < 0) {
		return ERR_SOCKET_CREATE_FAILED;
	}
	//
	if(ioctl(this->serverDescriptor_, FIONBIO, (char *)&on) < 0) {
		return ERR_SOCKET_NON_BLOCKING_MODE_SET_FAILED;
	}
	//
	memset(&this->localHost_, 0, sizeof(this->localHost_));
	this->localHost_.sin_family = AF_INET;
	this->localHost_.sin_addr.s_addr = INADDR_ANY;
	this->localHost_.sin_port = htons(this->port_);
	if(bind(this->serverDescriptor_, (struct sockaddr *)&this->localHost_, sizeof(this->localHost_)) < 0) {
		close(this->serverDescriptor_);
		return ERR_SOCKET_BIND_FAILED;
	}
	if(listen(this->serverDescriptor_, 5) < 0) {
		close(this->serverDescriptor_);
		return ERR_SOCKET_LISTEN_FAILED;
	}
	kvpr::freertos::EasyFreeRtos::createThread(
			(params == nullptr || params->taskName == nullptr) ? TCP_HOST_TASK_DEFAULT_NAME : params->taskName,
			(params == nullptr) ? TCP_HOST_TASK_DEFAULT_STACK_SIZE : params->taskStackSize,
			(params == nullptr) ? TCP_HOST_TASK_DEFAULT_PRIORITY : params->taskPriority,
			(kvpr::freertos::FreeRtosUser*) this,
			nullptr
	);
	return 0;
}

void TcpHost::setChild(TcpHostListener* child) {
	this->child_ = child;
}

void TcpHost::addListener(TcpHostListener* l) {
	if(l == nullptr) {
		return;
	}
	this->listeners_.push_back(l);
}

bool TcpHost::transmit(int socket, uint8_t* data, uint32_t dataCount) {
	uint32_t toSend = dataCount > TCP_HOST_MAX_TX_SEGMENT_SIZE ? TCP_HOST_MAX_TX_SEGMENT_SIZE : dataCount;
	uint32_t offset = 0;
	while(dataCount) {
		ssize_t sended = send(socket, &data[offset], toSend, 0);
		if(sended >= 0) {
			offset += sended;
			dataCount -= sended;
		} else {
			return false;
		}
		toSend = dataCount > TCP_HOST_MAX_TX_SEGMENT_SIZE ? TCP_HOST_MAX_TX_SEGMENT_SIZE : dataCount;
	}
	return true;
}

void TcpHost::finish(int socket) {
	uint32_t size_ = this->descriptors_.size();
	for(uint32_t i = 0; i < size_; i++) {
		if(this->descriptors_[i].fd == socket)  {
			this->descriptors_.erase(this->descriptors_.begin() + i);
			close(socket);
			break;
		}
	}
}

} /* namespace network */
} /* namespace kvpr */
