#pragma once

#ifndef PICPROG_H
#define PICPROG_H

#include "pch.h"
#include "comport.h"
#include "elf.h"
#include "packet.h"
#include <queue>
#include <thread>

#define SEV_DEBUG	0
#define SEV_MESSAGE	2
#define SEV_FATAL	7

TCHAR* convertCharArrayToLPCWSTR(const char* charArray);

class picprog
{
public:
	picprog(int argc, TCHAR** argv);
	picprog(LPCTSTR comportname, LPCTSTR elffilename, HWND hwnd);
	~picprog();
	packet* send_wait_ack(packet* p, uint32_t t_us, uint16_t rep);
	class comport *port;
	class elf *e0;
	BOOL proc(void);
	packet* send_cmd(uint8_t cmd, uint16_t addr, uint32_t timeout, uint8_t rep);
	boolean send_sec(struct sec_merged* section, uint8_t delta, uint32_t timeout, uint8_t rep, uint8_t erase_flag);
	void error(int sev, LPCTSTR er);
	std::queue<packet*> qout,qin;
	BOOL w;
	boolean big_endian;
	HWND hwnd;
	BOOL good;
	short blocks;
	void FillUidSec(unsigned long* uid_l0);
	BOOL readUid(unsigned long* uid_l);
	sec_merged* uid_sec;
	uint8_t* uid_data;
private:
	void init();
	std::thread thread_send, thread_recv;
	static uint8_t seq;
	BOOL fatal(LPCTSTR er);
};

#endif // PICPROG_H