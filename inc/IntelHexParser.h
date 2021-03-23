/*
 * IntelHexParser.h
 *
 *  Created on: 5 мар. 2021 г.
 *      Author: kgn
 */

#ifndef INTELHEXPARSER_H_
#define INTELHEXPARSER_H_

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace kvpr {
namespace proto {

class IntelHexParser;
class IntelHexParserUser;

typedef enum {
	ihpe_wrongSymbolInsideRow = -1,
	ihpe_wrongFrameBytesCount = -2,
	ihpe_wrongCheckSum = -3,
	ihpe_wrongPayloadBytesCount = -4,
} IntelHexParserError;

typedef enum {
	DataRecord = 0,						//supported
	EndofFileRecord = 1,				//supported
	ExtendedSegmentAddressRecord = 2,
	StartSegmentAddressRecord = 3,
	ExtendedLinearAddressRecord = 4,	//supported
	StartLinearAddressRecord = 5,
} IntelHexType;

typedef struct {
	uint8_t count;
	uint16_t offset;
	uint8_t type;
	uint8_t* data;
	uint8_t checkSum;
} IHexRow;

class IntelHexParserUser {
public:
	virtual void intelHexParser__unsupportedRowReceived(IHexRow* parsedRow) {};
	virtual void intelHexParser__extendedLinearAddressParsed(uint32_t extendedLinearAddress) {};
	virtual void intelHexParser__writeRow(uint32_t baseAddress, IHexRow* parsedRow) {};
	virtual void intelHexParser__error(IntelHexParserError errorCode, int32_t errorData) {};
	virtual void intelHexParser__eof() {};
};

class IntelHexParser {
private:
	std::vector<uint8_t> ihexRow_;
	std::vector<uint8_t> data_;
	bool waitForPrefix_ = true;
	uint32_t extendedLinearAddress_ = 0;
	IntelHexParserUser* user_ = nullptr;
	uint8_t getByteFromHexChar(uint8_t hexChar);
	uint8_t getByteFromHex(uint8_t h, uint8_t l);
	void handleIHexRow();

public:
	IntelHexParser();
	virtual ~IntelHexParser();
	void parse(uint8_t* data, uint32_t dataCount);
	void setUser(IntelHexParserUser* user);
};

}
}

#endif /* INTELHEXPARSER_H_ */
