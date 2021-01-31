/*
 * HttpHost.h
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#ifndef HTTPHOST_H_
#define HTTPHOST_H_

#include <TcpHost.h>
#include <HttpHostResources.h>
#include <plainNet.h>
#include <vector>
#include <vector>
#if PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1
#include PLAINNET_DEFAULT_HTTP_RESOURCES_INCLUDE_FILE
#endif

namespace kvpr {
namespace network {

#define HTTP_HOST_MAX_INPUT_HTTP_HEADER_SIZE 				1024
#define HTTP_HOST_MAX_INPUT_HTTP_CONTENT_SIZE 				1024
#define HTTP_HOST_MAX_PARALLEL_CONNECTIONS 					8
#define HTTP_HOST_CONTENT_SEPARATOR 						0x0D0A0D0AL

class HttpHost;
class HttpHostListener;
class HttpHostConnection;

typedef enum {
	_GET_,
	_HEAD_,
	_UNSUPPORTED_,
} HttpMethod;

class HttpHostListener {

public:

	HttpHostListener() {}

	virtual bool httpHost__get(const char* uri, HttpHostEvent* response) {
		return false;
	}

	virtual bool httpHost__wsData(const char* uri, HttpHostEvent* request, HttpHostEvent* response) {
		return false;
	}

	virtual bool httpHost__wsText(const char* uri, HttpHostEvent* request, HttpHostEvent* response) {
		return false;
	}
};

class HttpHostConnection {

private:
	int descriptor_;
	uint8_t *headerBuf_ = nullptr;
	uint32_t headerBw_ = 0;
	uint32_t headerBr_ = 0;
	uint8_t *contentBuf_ = nullptr;
	uint32_t contentBw_ = 0;
	uint32_t contentBr_ = 0;
	uint32_t contentSeparator_ = 0;
	std::string uri_;
	std::string version_;
	bool webSocket_ = false;
	bool headerIsWaiting_ = true;
	bool contentIsWaiting_ = false;
	std::vector<uint8_t> tmp_;
	std::vector<uint8_t> method_;
	std::vector<uint8_t> host_;
	std::vector<uint8_t> connection_;
	std::vector<uint8_t> upgrade_;
	std::vector<uint8_t> secWebsocketKey_;
	HttpHost* source_ = nullptr;
	int handleHttp(uint8_t* data, uint32_t dataCount);
	int handleWebSocket(uint8_t* data, uint32_t dataCount);
	void resetInputBuffers();
	HttpMethod parseHttpHeaders();
	void parseUriAndVersion();
	uint8_t* getResponseTypeArray(HttpHostResponseType type);
	uint32_t getResponseTypeArraySize(HttpHostResponseType type);
	int handleResponse(HttpHostEvent* response, HttpMethod requestType);
	bool checkForWebsocketSwitch();
	void intoVector(std::vector<uint8_t>& dst, uint8_t* source, uint32_t start, uint32_t count);
	void acceptWs();

public:
	HttpHostConnection(int descriptor, HttpHost* source);
	virtual ~HttpHostConnection();
	int getDescriptor();
	bool isWebSocket();
	void setWebSocket(bool webSocket);
	int handle(uint8_t* data, uint32_t dataCount);
};

class HttpHost : public TcpHost, TcpHostListener {

	friend class HttpHostConnection;

private:
	std::vector<HttpHostConnection*> httpConnections_;
	std::vector<HttpHostListener*> listeners_;
#if PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1
	HttpHostResources* defaultResources_ = plainnet_getDefaultHttpResources();
#endif
	static const uint8_t _HTTP_NOT_IMPLEMENTED_HEADER[];
	static const uint8_t _HTTP_OK[];
	static const uint8_t _HTTP_404[];
	static const uint8_t _CONTENT_TYPE[];
	static const uint8_t _CONTENT_TYPE_TEXT_HTML[];
	static const uint8_t _CONTENT_TYPE_TEXT_CSS[];
	static const uint8_t _CONTENT_TYPE_TEXT_JAVASCRIPT[];
	static const uint8_t _CONTENT_TYPE_IMAGE_PNG[];
	static const uint8_t _CONTENT_TYPE_IMAGE_JPEG[];
	static const uint8_t _CONTENT_TYPE_IMAGE_SVG[];
	static const uint8_t _CONTENT_TYPE_IMAGE_TIFF[];
	static const uint8_t _CONTENT_TYPE_IMAGE_XICON[];
	static const uint8_t _CONTENT_TYPE_APPLICATION_JSON[];
	static const uint8_t _CONTENT_TYPE_APPLICATION_OCTET_STREAM[];
	static const uint8_t _CONTENT_LENGTH[];
	static const char* WS_GUID;
	static const uint8_t WS_ACCEPT[];
	uint8_t wsHandshake_[128];

protected:
	virtual void tcpHost__clientConnected(int socket) override;

	virtual void tcpHost__clientDisconnected(int socket) override;

	virtual void tcpHost__clientDataReceived(int socket, uint8_t* data, uint32_t dataCount) override;

public:
	HttpHost();
	HttpHost(uint16_t port);
	virtual ~HttpHost();
	void addListener(HttpHostListener* listener);
};

} /* namespace network */
} /* namespace kvpr */

#endif /* HTTPHOST_H_ */
