/*
 * PlainBootloader.cpp
 *
 *  Created on: 7 мар. 2021 г.
 *      Author: kgn
 */

#include <PlainBootloader.h>

namespace kvpr {
namespace network {

PlainBootloader::PlainBootloader(uint32_t minAddress, uint32_t maxAddress) {
	// TODO Auto-generated constructor stub
	this->minAddress_ = minAddress;
	this->maxAddress_ = maxAddress;
}

PlainBootloader::~PlainBootloader() {
	// TODO Auto-generated destructor stub
}

void PlainBootloader::setUser(PlainBootloaderUser* user) {
	this->user_ = user;
}

void PlainBootloader::launch() {
	kvpr::freertos::EasyFreeRtos::createThread(
		PlainBootloader::THREAD_NAME_,
		PlainBootloader::THREAD_STACK_SIZE,
#if PLAINNET_USE_CMISIS_RTOS_PRIORITIES == 1
		osPriority::osPriorityNormal,
#else
		3,
#endif
		(kvpr::freertos::FreeRtosUser*) this,
		nullptr
	);
}

void PlainBootloader::requestPortion() {
	if(this->finished_) {
		return;
	}
	uint8_t dataRequest[7];
	dataRequest[0] = 1;
	dataRequest[1] = this->requestOffset_ & 255;
	dataRequest[2] = (this->requestOffset_ >> 8) & 255;
	dataRequest[3] = (this->requestOffset_ >> 16) & 255;
	dataRequest[4] = (this->requestOffset_ >> 24) & 255;
	dataRequest[5] = PlainBootloader::bytesCountInPortionPerRequest_ & 255;
	dataRequest[6] = (PlainBootloader::bytesCountInPortionPerRequest_ >> 8) & 255;
	this->http_->wsSendBinary(nullptr, dataRequest, sizeof(dataRequest));
	this->portionsRequestedPerInteval_++;
}

void PlainBootloader::freeRtosUser__onThreadStart(FreeRtosUser* userInstance, void* params) {
	//create instances
	this->http_ = new kvpr::network::HttpHost();
	this->intelHex = new kvpr::proto::IntelHexParser();
	this->mutex_ = xSemaphoreCreateMutex();
	//
	this->http_->addListener((kvpr::network::HttpHostListener*) this);
	this->intelHex->setUser((kvpr::proto::IntelHexParserUser*) this);
	this->http_->start(nullptr, false);
	if(this->user_) {
		this->user_->plainBootloader_started();
	}
}

void PlainBootloader::freeRtosUser__onThreadCall(FreeRtosUser* userInstance, void* params) {
	vTaskDelay(100 / portTICK_PERIOD_MS);
	xSemaphoreTake(this->mutex_, portMAX_DELAY);
	if(this->portionsRequestedPerInteval_) {
		this->portionsRequestedPerInteval_ = 0;
	} else {
		this->requestPortion();
	}
	xSemaphoreGive(this->mutex_);
}

bool PlainBootloader::httpHost__get(const char* uri, kvpr::network::HttpHostEvent* response) {
	return false;
}

bool PlainBootloader::httpHost__wsAccept(kvpr::network::WsEndPoint* endpoint) {
	bool ok = memcmp((void*) endpoint->getUri(), (void*) PlainBootloader::WS_URI_, strlen(PlainBootloader::WS_URI_)) == 0;
	if(ok && this->user_) {
		this->user_->plainBootloader_connectionCreated();
	}
	return ok;
}

bool PlainBootloader::httpHost__wsData(kvpr::network::WsEndPoint* endpoint, uint8_t* data, uint32_t dataCount) {
	xSemaphoreTake(this->mutex_, portMAX_DELAY);
	if(dataCount < 7) {
		xSemaphoreGive(this->mutex_);
		return true;
	}
	uint8_t type = data[0];
	if(type == 1) {
		uint16_t size = data[1] | (static_cast<uint16_t>(data[2]) << 8);
		if((dataCount >= (size + 7UL))) {
			uint32_t offset = data[3]
			   | (static_cast<uint32_t>(data[4]) << 8)
			   | (static_cast<uint32_t>(data[5]) << 16)
			   | (static_cast<uint32_t>(data[6]) << 24);
			if(offset != requestOffset_) {
				xSemaphoreGive(this->mutex_);
				return true;
			}
			this->intelHex->parse(&data[7], size);
			requestOffset_ = offset + size;
		}
		this->requestPortion();
	}
	xSemaphoreGive(this->mutex_);
	return true;
}

bool PlainBootloader::httpHost__wsText(kvpr::network::WsEndPoint* endpoint, uint8_t* text, uint32_t textSize) {
	return true;
}

void PlainBootloader::intelHexParser__unsupportedRowReceived(kvpr::proto::IHexRow* parsedRow) {

}

void PlainBootloader::intelHexParser__extendedLinearAddressParsed(uint32_t extendedLinearAddress) {

}

void PlainBootloader::intelHexParser__writeRow(uint32_t baseAddress, kvpr::proto::IHexRow* parsedRow) {
	if(!this->user_) {
		return;
	}
	if((baseAddress + parsedRow->offset) >= this->minAddress_ && (baseAddress + parsedRow->offset) <= this->maxAddress_) {
		this->user_->plainBootloader_burn(baseAddress + parsedRow->offset, parsedRow->data, parsedRow->count);
	}
}

void PlainBootloader::intelHexParser__error(kvpr::proto::IntelHexParserError errorCode, int32_t errorData) {

}

void PlainBootloader::intelHexParser__eof() {
	this->finished_ = true;
	if(!this->user_) {
		return;
	}
	this->user_->plainBootloader_exit();

}

} /* namespace network */
} /* namespace kvpr */
