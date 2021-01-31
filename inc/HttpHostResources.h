/*
 * HttpHostResources.h
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#ifndef HTTPHOSTRESOURCES_H_
#define HTTPHOSTRESOURCES_H_

#include <map>
#include <string>

namespace kvpr {
namespace network {


typedef enum {
	_html_,
	_javascript_,
	_css_,
	_json_,
	_img_png_,
	_img_jpeg_,
	_img_svg_,
	_img_tiff_,
	_img_xicon_,
	_octet_stream_,
	_none_,
} HttpHostResponseType;

typedef struct {
	int socket;
	int code;
	HttpHostResponseType type;
	uint8_t* content;
	uint32_t contentSize;
} HttpHostEvent;

class HttpHostResources {

private:
	std::map<std::string, HttpHostEvent*> resources_;

public:

	HttpHostResources();

	void add(const char* uri, HttpHostResponseType type, uint8_t* content, uint32_t contentSize);

	bool get(const char* uri, HttpHostEvent* response);

	bool get(std::string uri, HttpHostEvent* response);

};

} /* namespace network */
} /* namespace kvpr */

#endif /* HTTPHOSTRESOURCES_H_ */
