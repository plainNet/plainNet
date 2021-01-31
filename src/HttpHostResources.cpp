/*
 * HttpHostResources.cpp
 *
 *  Created on: 28 џэт. 2021 у.
 *      Author: kgn
 */

#include <HttpHostResources.h>
#include <string.h>
#include <stdlib.h>

namespace kvpr {
namespace network {

HttpHostResources::HttpHostResources() {

}

void HttpHostResources::add(const char* uri, HttpHostResponseType type, uint8_t* content, uint32_t contentSize) {
	HttpHostEvent* event_ = (HttpHostEvent*) malloc(sizeof(HttpHostEvent));
	//initialize event structure
	event_->content = content;
	event_->contentSize = contentSize;
	event_->type= type;
	//add resource to map
	this->resources_.insert({std::string(uri), event_});
}

bool HttpHostResources::get(const char* uri, HttpHostEvent* response) {
	return this->get(std::string(uri), response);
}

bool HttpHostResources::get(std::string uri, HttpHostEvent* response) {
	auto search = this->resources_.find(uri);
	if(search != this->resources_.end()) {
		HttpHostEvent* resource = search->second;
		response->type = resource->type;
		response->content = resource->content;
		response->contentSize = resource->contentSize;
		return true;
	}
	return false;
}


} /* namespace network */
} /* namespace kvpr */
