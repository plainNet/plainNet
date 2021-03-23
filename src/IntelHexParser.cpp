/*
 * IntelHexParser.cpp
 *
 *  Created on: 5 мар. 2021 г.
 *      Author: kgn
 */

#include <IntelHexParser.h>

namespace kvpr {
namespace proto {

IntelHexParser::IntelHexParser() {
	// TODO Auto-generated constructor stub

}

IntelHexParser::~IntelHexParser() {
	// TODO Auto-generated destructor stub
}

void IntelHexParser::handleIHexRow() {
	if(!this->ihexRow_.size()) {
		return;
	}
	if(ihexRow_.size() % 2) {
		if(this->user_) {
			this->user_->intelHexParser__error(
				IntelHexParserError::ihpe_wrongFrameBytesCount,
				ihexRow_.size()
			);
		}
		return;
	}
	uint8_t* row = this->ihexRow_.data();
	uint32_t offset = 0;
	IHexRow ihex;
	ihex.count = getByteFromHex(row[offset], row[offset + 1]);
	offset += 2;
	uint32_t pSize = ihex.count;
	this->data_.clear();
	pSize = (pSize * 2) + 10;
	if(pSize != ihexRow_.size()) {
		if(this->user_) {
			this->user_->intelHexParser__error(
				IntelHexParserError::ihpe_wrongPayloadBytesCount,
				pSize - ihexRow_.size()
			);
		}
		return;
	}
	ihex.offset = getByteFromHex(row[offset], row[offset + 1]);
	offset += 2;
	ihex.offset <<= 8;
	ihex.offset |= getByteFromHex(row[offset], row[offset + 1]);
	offset += 2;
	ihex.type = getByteFromHex(row[offset], row[offset + 1]);
	offset += 2;
	uint8_t checkSum = ihex.count + ((ihex.offset >> 8) & 255) + (ihex.offset & 255) + ihex.type;
	for(uint32_t i = 0; i < ihex.count; i++) {
		uint8_t b = getByteFromHex(row[offset], row[offset + 1]);
		offset += 2;
		this->data_.push_back(b);
		checkSum += b;
	}
	checkSum = 1 + (checkSum ^ 255);
	ihex.checkSum = getByteFromHex(row[offset], row[offset + 1]);
	ihex.data = ihex.count ? this->data_.data() : nullptr;
	if(ihex.checkSum != checkSum) {
		if(this->user_) {
			this->user_->intelHexParser__error(
				IntelHexParserError::ihpe_wrongCheckSum,
				pSize - ihexRow_.size()
			);
		}
		return;
	}
	if(this->user_) {
		if(ihex.type == IntelHexType::DataRecord) {//write
			this->user_->intelHexParser__writeRow(extendedLinearAddress_, &ihex);
		} else if(ihex.type == IntelHexType::ExtendedLinearAddressRecord) {
			extendedLinearAddress_ = this->data_.data()[0];
			extendedLinearAddress_ <<= 8;
			extendedLinearAddress_ |= this->data_.data()[1];
			extendedLinearAddress_ <<= 16;
			this->user_->intelHexParser__extendedLinearAddressParsed(extendedLinearAddress_);
		} else if(ihex.type == IntelHexType::EndofFileRecord) {
			this->user_->intelHexParser__eof();
		} else {
			this->user_->intelHexParser__unsupportedRowReceived(&ihex);
		}
	}
}

void IntelHexParser::parse(uint8_t* data, uint32_t dataCount) {
	while(dataCount-- > 0) {
		uint8_t byte = *data++;
		if(this->waitForPrefix_) {
			if(byte == ':') {
				this->ihexRow_.clear();
				this->waitForPrefix_ = false;
			}
		} else {
			if(byte == 13 || byte == 10) {
				if(this->ihexRow_.size()) {
					this->handleIHexRow();
				}
				this->ihexRow_.clear();
				this->waitForPrefix_ = true;
			} else if((byte >= 0x30 && byte <= 0x39) || (byte >= 0x41 && byte <= 0x46) || (byte >= 0x61 && byte <= 0x66)) {
				this->ihexRow_.push_back(byte);
			} else {
				if(this->user_) {
					this->user_->intelHexParser__error(
						IntelHexParserError::ihpe_wrongSymbolInsideRow,
						static_cast<uint32_t>(byte)
					);
				}
			}
		}
	}
}

void IntelHexParser::setUser(IntelHexParserUser* user) {
	this->user_ = user;
}

uint8_t IntelHexParser::getByteFromHex(uint8_t h, uint8_t l) {
	return (this->getByteFromHexChar(h) << 4) | this->getByteFromHexChar(l);
}

uint8_t IntelHexParser::getByteFromHexChar(uint8_t hexChar) {
	if(hexChar >= 0x30 && hexChar <= 0x39) {
		return hexChar - 0x30;
	} else if(hexChar >= 0x41 && hexChar <= 0x46) {
		return (hexChar - 0x41) + 10;
	} else if(hexChar >= 0x61 && hexChar <= 0x66) {
		return (hexChar - 0x61) + 10;
	}
	return 0;
}

}
}
