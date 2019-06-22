#include "pch.h"
#include "comport.h"
#include "picprog.h"
#include "crc16.h"
#include "packet.h"
#include <iostream>
#include <cstring>
#include <regex>
#include <thread>
#include <chrono>
#include <atlstr.h>


comport* comport::UseComport(LPCTSTR comportname, picprog* pp)
{
#ifdef _UNICODE
	if (!std::regex_match(comportname, std::wregex(L"COM\\d\\d?")))
#else
	if (!std::regex_match(comportname, std::regex("COM\\d\\d?")))
#endif
	{
		CString u = _T("Wrong serial port name ");
		u.AppendFormat(_T("%s"), comportname);
		pp->error(SEV_FATAL,u);
		return NULL;
	}
	return (new comport(comportname,pp));
}

comport::comport(LPCTSTR comportname, picprog* pp)
{
	this->pp = pp;
	_tcscpy_s(comName,_T("\\\\.\\"));
	_tcscat_s(comName,comportname);
	good = FALSE;
}


int comport::open()
{
	hComm = CreateFile(comName,      //port name
		GENERIC_READ | GENERIC_WRITE, //Read/Write
		0,                            // No Sharing
		NULL,                         // No Security
		OPEN_EXISTING,// Open existing port only
		FILE_FLAG_OVERLAPPED,            // Non Overlapped I/O
		NULL);        // Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE)
	{
		CString u = _T("Error in openning serial port ");
		u.AppendFormat(_T("%s"), comName);
		pp->error(SEV_FATAL,u);
		return -1;
	}
	else
	{
		CString u = _T("Opening serial port ");
		u.AppendFormat(_T("%s%s"), comName, _T(" successful"));
		pp->error(SEV_MESSAGE, u);
		good = TRUE;
	}

	DCB dcbSerialParams = { 0 }; // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	int Status = GetCommState(hComm, &dcbSerialParams);
	dcbSerialParams.BaudRate = CBR_115200;  // Setting BaudRate = 115200
	dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;  // Setting Parity = None
	SetCommState(hComm, &dcbSerialParams);
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 2; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 10; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 1; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 2; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 1; // in milliseconds

	SetCommTimeouts(hComm, &timeouts);
	/*
	ReadIntervalTimeout Specifies the maximum time interval between arrival of two bytes. If the arrival time exceeds these limits the ReadFile() function returns.

	ReadTotalTimeoutConstant is used to calculate the total time-out period for read operations. For each read operation, this value is added to the product of the ReadTotalTimeoutMultiplier member and the requested number of bytes.

	ReadTotalTimeoutMultiplier is used to calculate the total time-out period for read operations. For each read operation, this value is multiplied by the requested number of bytes to be read.

	WriteTotalTimeoutConstant similar to ReadTotalTimeoutConstant but for write operation.

	WriteTotalTimeoutMultiplier similar to ReadTotalTimeoutMultiplier but for write operation.

	*/
	PurgeComm(hComm, 0x0000000F);
	return 0;

}


int comport::close()
{
	if (good) CloseHandle(hComm);
	good = FALSE;
	return 0;
}

#define WRITE_TIMEOUT      500      // milliseconds


int comport::send(picprog* pp)
{
	DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
	DWORD dwRes;
	OVERLAPPED osWrite = { 0 };
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL)
	{
		pp->error(SEV_FATAL, _T("Error creating overlapped event in comport::send"));
	}
	while (1)
	{
		if (!pp->good) break;
		if (!pp->qout.empty())
		{
//			pp->error(SEV_MESSAGE, "Are byte to send");
			packet* pout = pp->qout.front();
			pp->qout.pop();
			pout->fill();
			if (!WriteFile(pp->port->hComm, pout->lpBuffer, pout->packetlen + 2, &dNoOfBytesWritten, &osWrite))
			{
				if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				{
					pp->error(SEV_MESSAGE, _T("Error in communication 1 comport::send"));
					continue;
				}
				else dwRes = WaitForSingleObject(osWrite.hEvent, WRITE_TIMEOUT);
				switch (dwRes)
				{
				case WAIT_OBJECT_0:
					if (!GetOverlappedResult(pp->port->hComm, &osWrite, &dNoOfBytesWritten, FALSE)) pp->error(SEV_MESSAGE, _T("Error in communication 2 comport::send"));
//					else pp->error(SEV_MESSAGE, "Sent " + std::to_string(dNoOfBytesWritten));
					break;
				case WAIT_TIMEOUT:
					pp->error(SEV_MESSAGE, _T("Error in communication 4 comport::send"));
					break;
				default:
					pp->error(SEV_MESSAGE, _T("Error in communication 3 comport::send"));
					break;
				}
			}
			else
			{
				CString u = _T("Sent ");
				u.AppendFormat(_T("%d"), dNoOfBytesWritten);
				pp->error(SEV_MESSAGE, u);
			}
		}
		if (!pp->good) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}

// https://docs.microsoft.com/en-us/previous-versions/ff802693(v=msdn.10)

#define READ_TIMEOUT      500      // milliseconds
uint8_t comport::HandleASuccessfulRead(uint8_t* buf, uint8_t len, DWORD NoBytesRead, picprog* pp)
{
	uint8_t plen = 0;
	packet* pin;
	len += (uint8_t)NoBytesRead;
	while (len != 0 && buf[0] == ID)
	{
		plen = ((buf[2] + 9) & 0xFE);
		if (plen <= len)
		{
			if ((pin = packet::parse(buf, plen)) != NULL)
			{
				pp->qin.push(pin);
			}
			if (buf[plen] == ID)
			{
				len -= plen;
				for (int i = 0; i < len; i++) buf[i] = buf[i + plen];
			}
			else len = 0;
		}
		else return len;
	}
	return 0;
}


int comport::recv(picprog* pp)
{
	uint8_t buf[RECV_BUFFER_SIZE];
	DWORD NoBytesRead;
	DWORD dwRes;
	uint8_t len = 0;
	BOOL fWaitingOnRead = FALSE;
	OVERLAPPED osReader = { 0 };
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osReader.hEvent == NULL) pp->error(SEV_FATAL, _T("Error creating overlapped event in comport::recv"));
	while (1)
	{
		if (!pp->good) return 0;
		if (!fWaitingOnRead)
		{
			if (!ReadFile(pp->port->hComm, &buf[len], RECV_BUFFER_SIZE - len, &NoBytesRead, &osReader))
			{
				if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				{
					pp->error(SEV_MESSAGE, _T("Error in communication 1 comport::recv"));
				}
				else fWaitingOnRead = TRUE;
			}
			else
			{
				len = HandleASuccessfulRead(buf, len, NoBytesRead, pp);
			}
		}
		if (fWaitingOnRead)
		{
			dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
			if (!pp->good) return 0;
			switch (dwRes)
			{
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(pp->port->hComm, &osReader, &NoBytesRead, FALSE))
				{
					pp->error(SEV_MESSAGE, _T("Error in communication 2 comport::recv"));
					len = 0;
				}
				else
				{
					len = HandleASuccessfulRead(buf, len, NoBytesRead, pp);
				}
			case WAIT_TIMEOUT :
				fWaitingOnRead = FALSE;
				continue;
			default:
				pp->error(SEV_MESSAGE, _T("Error in communication 3 comport::recv"));
				len = 0;
				fWaitingOnRead = FALSE;
				break;
			}
		}
	}
	return 0;
}



comport::~comport()
{
	if (good) CloseHandle(hComm);
	good = FALSE;
}
