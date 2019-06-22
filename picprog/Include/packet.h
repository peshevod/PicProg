#pragma once

#ifndef WRITE_FLASH_H
#define WRITE_FLASH_H

#include "pch.h"

#define ID					0x44
#define FLAG_ANS			0x10	 
#define FLAG_CMD			0x20
#define FLAG_ERASE			0x40
#define FLAG_CHECK			0x80

#define ACK					(0x00|FLAG_ANS)
#define START				(0x01|FLAG_CMD)
#define WRITE				(0x02|FLAG_CMD)
#define ERASE_ALL			(0x03|FLAG_CMD)
#define READ				(0x04|FLAG_CMD)
#define NACK				(0x05|FLAG_ANS)
#define	DATA				(0x06|FLAG_ANS)
#define STOP				(0x07|FLAG_CMD)


#define MAX_DATA_LEN		0x40;

class packet
{
public:
	uint8_t id;
	uint8_t seq;
	uint8_t len;
	uint8_t cmd;
	uint16_t addr;
	uint16_t crc=0;
	uint8_t* data;
	uint8_t packetlen=0;
	packet(uint8_t seq, uint8_t cmd, uint16_t addr, uint8_t* data, uint8_t datalen);
	static packet* parse(uint8_t* buf, uint8_t len);
	static uint16_t crc16(uint8_t* ar, uint16_t len);
	uint8_t lpBuffer[128];
	void fill(void);
};

#endif WRITE_FLASH_H
