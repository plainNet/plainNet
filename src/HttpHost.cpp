/*
 * HttpHost.cpp
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#include <HttpHost.h>
#include <Platform.h>
#include <mbedtls/base64.h>
#include <mbedtls/sha1.h>
#include <string.h>
#include <string>
#include <stdlib.h>

namespace kvpr {
namespace network {

#define HTTP_HOST_CLOSE_CONNECTION								0
#define HTTP_HOST_CONTINUE										1

/**
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 */

/**
 * HTTP/1.1 501 NOT IMPLEMENTED
 * Server: lwIP/plainNetHttpServer
 *
 */
const uint8_t HttpHost::_HTTP_NOT_IMPLEMENTED_HEADER[] = {
	0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x20, 0x35,
	0x30, 0x31, 0x20, 0x4E, 0x4F, 0x54, 0x20, 0x49, 0x4D, 0x50,
	0x4C, 0x45, 0x4D, 0x45, 0x4E, 0x54, 0x45, 0x44, 0x0D, 0x0A,
	0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x3A, 0x20, 0x6C, 0x77,
	0x49, 0x50, 0x2F, 0x70, 0x6C, 0x61, 0x69, 0x6E, 0x4E, 0x65,
	0x74, 0x48, 0x74, 0x74, 0x70, 0x53, 0x65, 0x72, 0x76, 0x65,
	0x72, 0x0D, 0x0A, 0x0D, 0x0A
};

/**
 * HTTP/1.1 200 OK
 * Server: lwIP/plainNetHttpServer
 * Cache-Control: no-cache
 */
const uint8_t HttpHost::_HTTP_OK[] = {
	0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x20, 0x32,
	0x30, 0x30, 0x20, 0x4F, 0x4B, 0x0D, 0x0A, 0x53, 0x65, 0x72,
	0x76, 0x65, 0x72, 0x3A, 0x20, 0x6C, 0x77, 0x49, 0x50, 0x2F,
	0x70, 0x6C, 0x61, 0x69, 0x6E, 0x4E, 0x65, 0x74, 0x48, 0x74,
	0x74, 0x70, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x0D, 0x0A,
	0x43, 0x61, 0x63, 0x68, 0x65, 0x2D, 0x43, 0x6F, 0x6E, 0x74,
	0x72, 0x6F, 0x6C, 0x3A, 0x20, 0x6E, 0x6F, 0x2D, 0x63, 0x61,
	0x63, 0x68, 0x65, 0x0D, 0x0A
};

/**
 * HTTP/1.1 404 NOT FOUND
 * Server: lwIP/plainNetHttpServer
 *
 */
const uint8_t HttpHost::_HTTP_404[] = {
	0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x20, 0x34,
	0x30, 0x34, 0x20, 0x4E, 0x4F, 0x54, 0x20, 0x46, 0x4F, 0x55,
	0x4E, 0x44, 0x0D, 0x0A, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
	0x3A, 0x20, 0x6C, 0x77, 0x49, 0x50, 0x2F, 0x70, 0x6C, 0x61,
	0x69, 0x6E, 0x4E, 0x65, 0x74, 0x48, 0x74, 0x74, 0x70, 0x53,
	0x65, 0x72, 0x76, 0x65, 0x72, 0x0D, 0x0A, 0x0D, 0x0A
};

/**
 * Content-Type: ........
 */
#define CONTENT_TYPE_OFFSET 14
const uint8_t HttpHost::_CONTENT_TYPE[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20
};

/**
 * Content-Type: text/html
 */
const uint8_t HttpHost::_CONTENT_TYPE_TEXT_HTML[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x68,
	0x74, 0x6D, 0x6C, 0x0D, 0x0A
};

/**
 * Content-Type: text/css
 */
const uint8_t HttpHost::_CONTENT_TYPE_TEXT_CSS[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x63,
	0x73, 0x73, 0x0D, 0x0A
};

/**
 * Content-Type: text/javascript
 */
const uint8_t HttpHost::_CONTENT_TYPE_TEXT_JAVASCRIPT[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x6A,
	0x61, 0x76, 0x61, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x0D,
	0x0A
};

/**
 * Content-Type: image/png
 */
const uint8_t HttpHost::_CONTENT_TYPE_IMAGE_PNG[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F,
	0x70, 0x6E, 0x67, 0x0D, 0x0A
};

/**
 * Content-Type: image/jpeg
 */
const uint8_t HttpHost::_CONTENT_TYPE_IMAGE_JPEG[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F,
	0x6A, 0x70, 0x65, 0x67, 0x0D, 0x0A
};

/**
 * Content-Type: image/svg+xml
 */
const uint8_t HttpHost::_CONTENT_TYPE_IMAGE_SVG[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F,
	0x73, 0x76, 0x67, 0x2B, 0x78, 0x6D, 0x6C, 0x0D, 0x0A,
};

/**
 * Content-Type: image/tiff
 */
const uint8_t HttpHost::_CONTENT_TYPE_IMAGE_TIFF[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F,
	0x74, 0x69, 0x66, 0x66, 0x0D, 0x0A,
};

/**
 * Content-Type: image/x-icon
 */
const uint8_t HttpHost::_CONTENT_TYPE_IMAGE_XICON[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F,
	0x78, 0x2D, 0x69, 0x63, 0x6F, 0x6E, 0x0D, 0x0A,
};

/**
 * Content-Type: application/json
 */
const uint8_t HttpHost::_CONTENT_TYPE_APPLICATION_JSON[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63,
	0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x6A, 0x73, 0x6F, 0x6E,
	0x0D, 0x0A,
};

/**
 * Content-Type: application/octet-stream
 */
const uint8_t HttpHost::_CONTENT_TYPE_APPLICATION_OCTET_STREAM[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x54, 0x79,
	0x70, 0x65, 0x3A, 0x20, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63,
	0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x6F, 0x63, 0x74, 0x65,
	0x74, 0x2D, 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D, 0x0D, 0x0A,
};

/**
 * Content-Length:
 */
#define CONTENT_LENGTH_OFFSET 16
const uint8_t HttpHost::_CONTENT_LENGTH[] = {
	0x43, 0x6F, 0x6E, 0x74, 0x65, 0x6E, 0x74, 0x2D, 0x4C, 0x65,
	0x6E, 0x67, 0x74, 0x68, 0x3A, 0x20
};

/**
 * WEB SOCKET GUID
 */
const char* HttpHost::WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
/**
 * HTTP/1.1 101 Switching Protocols
 * Upgrade: websocket
 * Connection: Upgrade
 * Sec-WebSocket-Accept:
 */
const uint8_t HttpHost::WS_ACCEPT[] = {
	0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x20, 0x31,
	0x30, 0x31, 0x20, 0x53, 0x77, 0x69, 0x74, 0x63, 0x68, 0x69,
	0x6E, 0x67, 0x20, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x63, 0x6F,
	0x6C, 0x73, 0x0D, 0x0A, 0x55, 0x70, 0x67, 0x72, 0x61, 0x64,
	0x65, 0x3A, 0x20, 0x77, 0x65, 0x62, 0x73, 0x6F, 0x63, 0x6B,
	0x65, 0x74, 0x0D, 0x0A, 0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63,
	0x74, 0x69, 0x6F, 0x6E, 0x3A, 0x20, 0x55, 0x70, 0x67, 0x72,
	0x61, 0x64, 0x65, 0x0D, 0x0A, 0x53, 0x65, 0x63, 0x2D, 0x57,
	0x65, 0x62, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x2D, 0x41,
	0x63, 0x63, 0x65, 0x70, 0x74, 0x3A, 0x20,
};
/**
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 */
HttpHost::HttpHost() : HttpHost(80) {
}


HttpHost::HttpHost(uint16_t port) : TcpHost(port), TcpHostListener() {
	setChild((TcpHostListener*) this);
}

HttpHost::~HttpHost() {
	// TODO Auto-generated destructor stub
}

void HttpHost::tcpHost__clientConnected(int socket) {
	this->httpConnections_.push_back(new HttpHostConnection(socket, this));
}

void HttpHost::tcpHost__clientDisconnected(int socket) {
	for(uint32_t i = 0; i < this->httpConnections_.size(); i++) {
		if(this->httpConnections_[i]->getDescriptor() == socket) {
			delete this->httpConnections_[i];
			this->httpConnections_.erase(this->httpConnections_.begin() + i);
			break;
		}
	}
}

void HttpHost::tcpHost__clientDataReceived(int socket, uint8_t* data, uint32_t dataCount) {
	for(uint32_t i = 0; i < this->httpConnections_.size(); i++) {
		if(this->httpConnections_[i]->getDescriptor() == socket) {
			if(this->httpConnections_[i]->handle(data, dataCount) == HTTP_HOST_CLOSE_CONNECTION) {
				delete this->httpConnections_[i];
				this->httpConnections_.erase(this->httpConnections_.begin() + i);
				this->finish(socket);
			}
			break;
		}
	}
}


void HttpHost::addListener(HttpHostListener* listener) {
	this->listeners_.push_back(listener);
}

/**
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------------------------
 */
HttpHostConnection::HttpHostConnection(int descriptor, HttpHost* source) : WsEndPoint() {
	this->descriptor_ = descriptor;
	this->source_ = source;
	this->headerBuf_ = (uint8_t*) malloc(HTTP_HOST_MAX_INPUT_HTTP_HEADER_SIZE);
}

HttpHostConnection::~HttpHostConnection() {
	if(this->headerBuf_) {
		free(this->headerBuf_);
		this->headerBuf_ = nullptr;
	}
	if(this->contentBuf_) {
		free(this->contentBuf_);
		this->contentBuf_ = nullptr;
	}
	this->tmp_.clear();
	this->method_.clear();
	this->host_.clear();
	this->connection_.clear();
	this->uri_.clear();
	this->version_.clear();
	this->upgrade_.clear();
	this->secWebsocketKey_.clear();
	this->webSocketFrame_.clear();
	this->webSocketPayload_.clear();
}

int HttpHostConnection::getDescriptor() {
	return descriptor_;
}

bool HttpHostConnection::isWebSocket() {
	return webSocket_;
}

void HttpHostConnection::setWebSocket(bool webSocket) {
	webSocket_ = webSocket;
	if(webSocket_) {

	}
}

int HttpHostConnection::handle(uint8_t* data, uint32_t dataCount) {
	if(webSocket_) {
		return this->handleWebSocket(data, dataCount);
	} else {
		return this->handleHttp(data, dataCount);
	}
}

void HttpHostConnection::resetInputBuffers() {
	this->contentSeparator_ = 0;
	this->headerBw_ = 0;
	this->headerBr_ = 0;
}

void HttpHostConnection::intoVector(std::vector<uint8_t>& dst, uint8_t* source, uint32_t start, uint32_t count) {
	for(uint32_t i = start; i < count; i++) {
		dst.push_back(source[i]);
	}
}

HttpMethod HttpHostConnection::parseHttpHeaders() {
	uint32_t offset = 0;
	this->tmp_.clear();
	this->method_.clear();
	this->host_.clear();
	this->connection_.clear();
	this->upgrade_.clear();
	this->secWebsocketKey_.clear();
	HttpMethod methodType = HttpMethod::_UNSUPPORTED_;
	while(offset < this->headerBw_) {
		if(this->headerBuf_[offset] != 13 && this->headerBuf_[offset] != 10) {
			this->tmp_.push_back(this->headerBuf_[offset]);
		} else {
			uint32_t tmpArrSize = this->tmp_.size();
			if(tmpArrSize) {
				uint8_t* tmpArr = this->tmp_.data();
				if(!this->method_.size()) {
					if(tmpArrSize >= 4  && tmpArr[3] == 0x20 && tmpArr[0] == 'G' && tmpArr[1] == 'E' && tmpArr[2] == 'T') {
						this->method_.insert(this->method_.begin(), this->tmp_.begin() + 4, this->tmp_.end());
						methodType = HttpMethod::_GET_;
					} else if(tmpArrSize >= 5  && tmpArr[4] == 0x20 && tmpArr[0] == 'H' && tmpArr[1] == 'E' && tmpArr[2] == 'A' && tmpArr[3] == 'D') {
						this->method_.insert(this->method_.begin(), this->tmp_.begin() + 5, this->tmp_.end());
						methodType = HttpMethod::_HEAD_;
					} else {
						offset = this->headerBw_;
					}
				} else {
					if(tmpArrSize >= 6 && !this->host_.size() && memcmp(tmpArr, "Host: ", 6) == 0) {
						this->host_.insert(this->host_.begin(), this->tmp_.begin() + 6, this->tmp_.end());
					} else if(tmpArrSize >= 12 && !this->connection_.size()  && memcmp(tmpArr, "Connection: ", 12) == 0) {
						this->connection_.insert(this->connection_.begin(), this->tmp_.begin() + 12, this->tmp_.end());
					} else if(tmpArrSize >= 9 && !this->upgrade_.size()  && memcmp(tmpArr, "Upgrade: ", 9) == 0) {
						this->upgrade_.insert(this->upgrade_.begin(), this->tmp_.begin() + 9, this->tmp_.end());
					} else if(tmpArrSize >= 19 && !this->secWebsocketKey_.size()  && memcmp(tmpArr, "Sec-WebSocket-Key: : ", 19) == 0) {
						this->secWebsocketKey_.insert(this->secWebsocketKey_.begin(), this->tmp_.begin() + 19, this->tmp_.end());
					}
				}
			}
			this->tmp_.clear();
		}
		offset++;
	}
	return methodType;
}

void HttpHostConnection::parseUriAndVersion() {
	uri_.clear();
	version_.clear();
	uint32_t offset = 0;
	bool ver = false;
	while(offset < method_.size()) {
		if(method_[offset] == 0x20) {
			ver = true;
		} else {
			(ver ? version_ : uri_) += (char) method_[offset];
		}
		offset++;
	}
	if(uri_.find("?") != std::string::npos) {
		uri_ = uri_.substr(0, uri_.find("?"));
	}
}

uint8_t* HttpHostConnection::getResponseTypeArray(HttpHostResponseType type) {
	switch(type) {
		case HttpHostResponseType::_html_: return (uint8_t*) HttpHost::_CONTENT_TYPE_TEXT_HTML;
		case HttpHostResponseType::_css_: return (uint8_t*) HttpHost::_CONTENT_TYPE_TEXT_CSS;
		case HttpHostResponseType::_javascript_: return (uint8_t*) HttpHost::_CONTENT_TYPE_TEXT_JAVASCRIPT;
		case HttpHostResponseType::_img_png_: return (uint8_t*) HttpHost::_CONTENT_TYPE_IMAGE_PNG;
		case HttpHostResponseType::_img_jpeg_:return (uint8_t*) HttpHost::_CONTENT_TYPE_IMAGE_JPEG;
		case HttpHostResponseType::_img_svg_: return (uint8_t*) HttpHost::_CONTENT_TYPE_IMAGE_SVG;
		case HttpHostResponseType::_img_tiff_: return (uint8_t*) HttpHost::_CONTENT_TYPE_IMAGE_TIFF;
		case HttpHostResponseType::_img_xicon_: return (uint8_t*) HttpHost::_CONTENT_TYPE_IMAGE_XICON;
		case HttpHostResponseType::_json_: return (uint8_t*) HttpHost::_CONTENT_TYPE_APPLICATION_JSON;
		default: return (uint8_t*) HttpHost::_CONTENT_TYPE_APPLICATION_OCTET_STREAM;
	}
}

uint32_t HttpHostConnection::getResponseTypeArraySize(HttpHostResponseType type) {
	switch(type) {
		case HttpHostResponseType::_html_: return sizeof(HttpHost::_CONTENT_TYPE_TEXT_HTML);
		case HttpHostResponseType::_css_: return sizeof(HttpHost::_CONTENT_TYPE_TEXT_CSS);
		case HttpHostResponseType::_javascript_: return sizeof(HttpHost::_CONTENT_TYPE_TEXT_JAVASCRIPT);
		case HttpHostResponseType::_img_png_: return sizeof(HttpHost::_CONTENT_TYPE_IMAGE_PNG);
		case HttpHostResponseType::_img_jpeg_: return sizeof(HttpHost::_CONTENT_TYPE_IMAGE_JPEG);
		case HttpHostResponseType::_img_svg_: return sizeof(HttpHost::_CONTENT_TYPE_IMAGE_SVG);
		case HttpHostResponseType::_img_tiff_: return sizeof(HttpHost::_CONTENT_TYPE_IMAGE_TIFF);
		case HttpHostResponseType::_img_xicon_: return sizeof(HttpHost::_CONTENT_TYPE_IMAGE_XICON);
		case HttpHostResponseType::_json_: return sizeof(HttpHost::_CONTENT_TYPE_APPLICATION_JSON);
		default: return sizeof(HttpHost::_CONTENT_TYPE_APPLICATION_OCTET_STREAM);
	}
}

int HttpHostConnection::handleResponse(HttpHostEvent* response, HttpMethod requestType) {
	//get response type array
	uint8_t* responseTypeArray = this->getResponseTypeArray(response->type);
	uint32_t responseTypeArraySize = this->getResponseTypeArraySize(response->type);
	//if threre is no type array for resource - continue
	if(responseTypeArray == nullptr) {
		return HTTP_HOST_CONTINUE;
	}
	//send header first segment
	this->source_->transmit(this->descriptor_, (uint8_t*) HttpHost::_HTTP_OK, sizeof(HttpHost::_HTTP_OK));
	//send Response-Type
	this->source_->transmit(this->descriptor_, responseTypeArray, responseTypeArraySize);
	//create and send Content-Length
	std::string cl("Content-Length: ");
	char toStr_[10];
	itoa(response->contentSize, toStr_, 10);
	cl += (char*) toStr_;
	cl += "\r\n\r\n";
	this->source_->transmit(this->descriptor_, (uint8_t*) cl.c_str(), cl.size());
	//send response content if GET method in request
	if(requestType == HttpMethod::_GET_) {
		this->source_->transmit(this->descriptor_, response->content, response->contentSize);
	}
	return HTTP_HOST_CLOSE_CONNECTION;
}

bool HttpHostConnection::checkForWebsocketSwitch() {
	uint32_t cs_ = this->connection_.size();
	uint32_t us_ = this->upgrade_.size();
	if(cs_ == 7 && us_ == 9 && this->secWebsocketKey_.size()
			&& memcmp(this->connection_.data(), "Upgrade", cs_) == 0
			&& memcmp(this->upgrade_.data(), "websocket", us_) == 0) {
		return true;
	}
	return false;
}

void HttpHostConnection::acceptWs() {
	size_t olen;
	memcpy((void*) this->source_->wsHandshake_, (void*) this->secWebsocketKey_.data(), this->secWebsocketKey_.size());
	memcpy((void*) &this->source_->wsHandshake_[this->secWebsocketKey_.size()], (void*) HttpHost::WS_GUID, strlen(HttpHost::WS_GUID));
	mbedtls_sha1(this->source_->wsHandshake_, this->secWebsocketKey_.size() + strlen(HttpHost::WS_GUID), this->source_->wsHandshakeSHA1_);
	mbedtls_base64_encode(this->source_->wsHandshakeBase64_, 100, &olen, this->source_->wsHandshakeSHA1_, 20);
	this->source_->wsHandshakeBase64_[olen++] = 13;
	this->source_->wsHandshakeBase64_[olen++] = 10;
	this->source_->wsHandshakeBase64_[olen++] = 13;
	this->source_->wsHandshakeBase64_[olen++] = 10;
	this->source_->transmit(this->descriptor_, (uint8_t*) HttpHost::WS_ACCEPT, sizeof(HttpHost::WS_ACCEPT));
	this->source_->transmit(this->descriptor_, (uint8_t*) this->source_->wsHandshakeBase64_, olen);
}

int HttpHostConnection::handleHttp(uint8_t* data, uint32_t dataCount) {
	if(headerIsWaiting_) {
		//move input data bytes into internal buffer for header bytes
		for(uint32_t i = 0; i < dataCount; i++) {
			this->contentSeparator_ <<= 8;
			this->contentSeparator_ |= data[i];
			this->headerBuf_[this->headerBw_++] = data[i];
			//check if maximum header size is reached or \r\n\r\n delimeter is founded
			if(this->headerBw_ == HTTP_HOST_MAX_INPUT_HTTP_HEADER_SIZE || this->contentSeparator_ == HTTP_HOST_CONTENT_SEPARATOR) {
				break;
			}
		}
		//if delimeter not found
		if(this->contentSeparator_ != HTTP_HOST_CONTENT_SEPARATOR) {
			//this is an error - maximum header size is reached and delimeter not founded
			if(this->headerBw_ == HTTP_HOST_MAX_INPUT_HTTP_HEADER_SIZE) {
				this->resetInputBuffers();
				return HTTP_HOST_CLOSE_CONNECTION;
			}
			return HTTP_HOST_CONTINUE;
		}
		//call parse HTTP headers method
		HttpMethod methodType = this->parseHttpHeaders();
		//clear header buf
		this->resetInputBuffers();
		//in case of unsupported METHOD send HTTP/1.1 501 NOT IMPLEMENTED
		if(methodType == HttpMethod::_UNSUPPORTED_) {
			this->source_->transmit(this->descriptor_, (uint8_t*) HttpHost::_HTTP_NOT_IMPLEMENTED_HEADER, sizeof(HttpHost::_HTTP_NOT_IMPLEMENTED_HEADER));
			return HTTP_HOST_CLOSE_CONNECTION;
		}
		//parse URI and HTTP version from header
		this->parseUriAndVersion();
		//check for HTTP version
		if(version_.find("HTTP/1.0") == std::string::npos && version_.find("HTTP/1.1") == std::string::npos) {
			//unsupported HTTP version
			return HTTP_HOST_CLOSE_CONNECTION;
		}
		//check for web socket connection switch request
		if(methodType == HttpMethod::_GET_) {
			if(this->checkForWebsocketSwitch()) {
				if(this->source_->listeners_.size()) {
					for(uint32_t i = 0; i < this->source_->listeners_.size(); i++) {
						if(this->source_->listeners_[i]->httpHost__wsAccept((WsEndPoint*) this)) {
							this->acceptWs();
							return HTTP_HOST_CONTINUE;
						}
						this->source_->transmit(this->descriptor_, (uint8_t*) HttpHost::_HTTP_404, sizeof(HttpHost::_HTTP_404));
						return HTTP_HOST_CLOSE_CONNECTION;
					}
				} else {
					this->acceptWs();
					this->webSocket_ = true;
					return HTTP_HOST_CONTINUE;
				}
			}
		}
		//create response
		HttpHostEvent response;
#if PLAINNET_USE_DEFAULT_HTTP_RESOURCES == 1
		//default resources use is set - check request in default resources
		if(this->source_ && this->source_->defaultResources_ && this->source_->defaultResources_->get(uri_, &response)) {
			if(this->handleResponse(&response, methodType) == HTTP_HOST_CLOSE_CONNECTION) {
				return HTTP_HOST_CLOSE_CONNECTION;
			}
		}
#endif
		//default resources unit is not used or there is no default resource for this URI - make callback on user level
		if(this->source_) {
			for(uint32_t i = 0; i < this->source_->listeners_.size(); i++) {
				if(this->source_->listeners_[i]->httpHost__get(uri_.c_str(), &response)) {
					if(this->handleResponse(&response, methodType) == HTTP_HOST_CLOSE_CONNECTION) {
						return HTTP_HOST_CLOSE_CONNECTION;
					}
				}
			}
		}
		//there is a point of 404 response
		this->source_->transmit(this->descriptor_, (uint8_t*) HttpHost::_HTTP_404, sizeof(HttpHost::_HTTP_404));
		return HTTP_HOST_CLOSE_CONNECTION;
	}
	return HTTP_HOST_CONTINUE;
}

bool HttpHostConnection::parseWsFrameHeader(WsFrameHeader* header, uint8_t* data, uint32_t dataCount) {
	memset((void*) header, 0, sizeof(WsFrameHeader));
	uint32_t offset = 0;
	header->FIN = (data[offset] & 128) != 0;
	header->RSV1 = (data[offset] & 64) != 0;
	header->RSV2 = (data[offset] & 32) != 0;
	header->RSV3 = (data[offset] & 16) != 0;
	header->opcode = (data[offset++] & 15);
	if(offset >= dataCount) {
		return false;
	}
	header->MASK = (data[offset] & 128) != 0;
	header->payloadLength = (data[offset++] & 127);
	if(offset >= dataCount) {
		return false;
	}
	if(header->payloadLength == 126 || header->payloadLength == 127) {
		uint8_t cnt = (header->payloadLength == 126 ? 2 : 8);
		header->payloadLength = 0;
		for(uint32_t i = 0; i < cnt; i++) {
			header->payloadLength <<= 8;
			header->payloadLength |= data[offset++];
			if(offset >= dataCount) {
				return false;
			}
		}
	}
	if(header->MASK) {
		for(uint32_t i = 0; i < 4; i++) {
			header->maskingKey[i] = data[offset++];
			if(offset >= dataCount) {
				return false;
			}
		}
	}
	header->headerSize = offset;
	return true;
}

void HttpHostConnection::tryParseWsFrame() {
	WsFrameHeader header;
	uint8_t* wsFrame = this->webSocketFrame_.data();
	if(this->parseWsFrameHeader(&header, wsFrame, this->webSocketFrame_.size())) {
		if(this->webSocketFrame_.size() >= (header.payloadLength + header.headerSize)) {
			if(header.MASK) {
				for(uint32_t i = 0; i < header.payloadLength; i++) {
					wsFrame[header.headerSize + i] ^= header.maskingKey[i % 4];
				}
			}
			if(this->source_->listeners_.size()) {
				for(uint32_t i = 0; i < this->source_->listeners_.size(); i++) {
					if(header.opcode == 1) {//text data
						this->source_->listeners_[i]->httpHost__wsText((WsEndPoint*) this, &wsFrame[header.headerSize], (uint32_t) header.payloadLength);
					} else if(header.opcode == 2) {//binary data
						this->source_->listeners_[i]->httpHost__wsData((WsEndPoint*) this, &wsFrame[header.headerSize], (uint32_t) header.payloadLength);
					}
				}
			} else {
				this->wsSend(header.opcode, &wsFrame[header.headerSize], (uint32_t) header.payloadLength);
			}
			this->webSocketFrame_.erase(this->webSocketFrame_.begin(), this->webSocketFrame_.begin() + (header.headerSize + header.payloadLength));
		} else {
			this->webSocketFrameToReceive_ = (header.payloadLength + header.headerSize) - this->webSocketFrame_.size();
		}
	}
}

int HttpHostConnection::handleWebSocket(uint8_t* data, uint32_t dataCount) {
	while(this->webSocketFrameToReceive_ && dataCount) {
		this->webSocketFrame_.push_back(*data++);
		this->webSocketFrameToReceive_--;
		dataCount--;
	}
	if(this->webSocketFrameToReceive_ == 0) {
		if(this->webSocketFrame_.size()) {
			this->tryParseWsFrame();
		}
		this->webSocketFrame_.clear();
		while(dataCount) {
			this->webSocketFrame_.push_back(*data++);
			dataCount--;
		}
		this->tryParseWsFrame();
	}
	return HTTP_HOST_CONTINUE;
}

const char* HttpHostConnection::getUri() {
	return this->uri_.c_str();
}

void HttpHostConnection::wsSend(uint8_t opCode, uint8_t* data, uint32_t dataSize) {
	uint8_t header[dataSize <= 125 ? 2 : 4];
	header[0] = 128 | opCode;
	header[1] = dataSize <= 125 ? dataSize : 126;
	if(dataSize >= 126) {
		header[2] = (dataSize >> 8) & 255;
		header[3] = dataSize & 255;
	}
	this->source_->transmit(this->descriptor_, (uint8_t*) header, dataSize <= 125 ? 2 : 4);
	this->source_->transmit(this->descriptor_, (uint8_t*) data, dataSize);
}

void HttpHostConnection::sendBinaryData(uint8_t* data, uint32_t dataSize) {
	this->wsSend(2, data, dataSize);
}

void HttpHostConnection::sendTextData(uint8_t* text, uint32_t textSize) {
	this->wsSend(1, text, textSize);
}


} /* namespace network */
} /* namespace kvpr */

