#pragma once

#include "pch.h"
#include "picprog.h"
#include "packet.h"
#include <queue>
#include <atlstr.h>

#define RECV_BUFFER_SIZE 256

class comport
{
public:
	static comport* UseComport(LPCTSTR comportname, class picprog* pp);
	comport(LPCTSTR comportname, class picprog* pp);
	~comport();
	picprog* pp;
	HANDLE hComm;
	int open();
	int close();
	static int send(picprog* pp);
	static int recv(picprog* pp);
	static uint8_t HandleASuccessfulRead(uint8_t* buf, uint8_t len, DWORD NoBytesRead, picprog* pp);
	TCHAR comName[10];
	BOOL good;
};

