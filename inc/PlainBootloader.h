/*
 * PlainBootloader.h
 *
 *  Created on: 7 мар. 2021 г.
 *      Author: kgn
 */

#ifndef PLAINBOOTLOADER_H_
#define PLAINBOOTLOADER_H_

#include "HttpHost.h"
#include "IntelHexParser.h"
#include "semphr.h"

namespace kvpr {
namespace network {

class PlainBootloaderUser {
public:
	virtual void plainBootloader_started() {};
	virtual void plainBootloader_burn(uint32_t address, uint8_t* data, uint32_t dataCount) {};
	virtual void plainBootloader_exit() {};
};

class PlainBootloader : public kvpr::freertos::FreeRtosUser, public kvpr::network::HttpHostListener, public kvpr::proto::IntelHexParserUser  {

private:
	static constexpr const char* WS_URI_ = "/ws";
	static constexpr const char* THREAD_NAME_ = "plain-boot";
	static constexpr const uint32_t THREAD_STACK_SIZE = 2048;
	static constexpr const uint16_t bytesCountInPortionPerRequest_ = 1024;
	kvpr::network::HttpHost* http_ = nullptr;
	kvpr::proto::IntelHexParser* intelHex = nullptr;
	PlainBootloaderUser* user_ = nullptr;
	SemaphoreHandle_t mutex_;
	uint32_t requestOffset_ = 0;
	uint32_t portionsRequestedPerInteval_ = 0;
	uint32_t minAddress_ = 0;
	uint32_t maxAddress_ = 0;
	void requestPortion();
	bool finished_ = false;

protected:
	virtual bool httpHost__get(const char* uri, kvpr::network::HttpHostEvent* response) override;
	virtual bool httpHost__wsAccept(kvpr::network::WsEndPoint* endpoint) override;
	virtual bool httpHost__wsData(kvpr::network::WsEndPoint* endpoint, uint8_t* data, uint32_t dataCount) override;
	virtual bool httpHost__wsText(kvpr::network::WsEndPoint* endpoint, uint8_t* text, uint32_t textSize) override;

	virtual void intelHexParser__unsupportedRowReceived(kvpr::proto::IHexRow* parsedRow) override;
	virtual void intelHexParser__extendedLinearAddressParsed(uint32_t extendedLinearAddress) override;
	virtual void intelHexParser__writeRow(uint32_t baseAddress, kvpr::proto::IHexRow* parsedRow) override;
	virtual void intelHexParser__error(kvpr::proto::IntelHexParserError errorCode, int32_t errorData) override;
	virtual void intelHexParser__eof() override;

	virtual void freeRtosUser__onThreadCall(FreeRtosUser* userInstance, void* params) override;
	virtual void freeRtosUser__onThreadStart(FreeRtosUser* userInstance, void* params) override;

public:
	PlainBootloader(uint32_t minAddress, uint32_t maxAddress);
	virtual ~PlainBootloader();
	void setUser(PlainBootloaderUser* user);
	void launch();
};

} /* namespace network */
} /* namespace kvpr */

#endif /* PLAINBOOTLOADER_H_ */
