// picprog.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "picprog.h"
#include "elf.h"
#include "packet.h"
#include <thread>


uint8_t picprog::seq;
TCHAR z[8][10] = {_T("ACK"),_T("START"),_T("WRITE"),_T("ERASE_ALL"),_T("READ"),_T("NACK"),_T("DATA"),_T("STOP") };
CString u;
int pos;
uint16_t ids[16] = { 0x30CA,0x30CB,0x30CC,0x30CD,0x30D2,0x30D3,0x30CE,0x30CF,0x30D0,0x30D1,0x30D4,0x30D5,0x30D7,0x30D8,0x30D9,0x30DA };
TCHAR chips[16][16] = { {_T("PIC16F18424")},{_T("PIC16LF18424")},{_T("PIC16F18425")},{_T("PIC16LF18425")},
						{_T("PIC16F18426")},{_T("PIC16LF18426")},{_T("PIC16F18444")},{_T("PIC16LF18444")},
						{_T("PIC16F18445")},{_T("PIC16LF18445")},{_T("PIC16F18446")},{_T("PIC16LF18446")},
						{_T("PIC16F18455")},{_T("PIC16LF18455")},{_T("PIC16F18456")},{_T("PIC16LF18456")} };

// Char converter char -> wchar

TCHAR* convertCharArrayToLPCWSTR(const char* charArray)
{
	
#ifdef _UNICODE
	
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
#else
	return charArray;
#endif

}

// Constructor

picprog::picprog(int argc, TCHAR** argv)
{
	w = FALSE;
	good = FALSE;
	// print arguments
	for (int i = 0; i < argc; i++) _tprintf(_T("%s\n"), argv[i]);
	// initialisation of COM port
	port = new comport(argv[1], this);
	// Read ELF file
	e0 = new elf(argv[2], this);
	uid_data = (uint8_t*)malloc(8);
	uid_sec = (sec_merged*)malloc(sizeof(sec_merged));
	this->hwnd = NULL;
	init();
}

picprog::picprog(LPCTSTR comportname, LPCTSTR elffilename, HWND hwnd)
{
	w = TRUE;
	good = FALSE;
	port = new comport(comportname,this);
	e0 = new elf(elffilename, this);
	uid_data = (uint8_t*)malloc(8);
	uid_sec = (sec_merged*)malloc(sizeof(sec_merged));
	this->hwnd = hwnd;
	init();
}

void picprog::init()
{
// Initialisation of write (qout) and read (qin) queues
	qout = std::queue<packet*>();
	qin = std::queue<packet*>();
	int num = 1;
	picprog::seq = 0;
// define endianness
	if (*(char *)&num == 1)
	{
		big_endian=false;
	}
	else
	{
		big_endian = true;
	}
	if (w)
	{
		blocks = (e0->code_m.size + 1) / 64 + (e0->conf_m.size + 1) / 2 + (e0->data_m.size + 1) / 2;
		::SendMessage(hwnd, WM_PICRANGE, blocks, 0);
		pos = 0;
	}
}

/*
	send_wait_ack - send packet p and wait t_us microseconds
	Number of tries - rep
*/

packet* picprog::send_wait_ack(packet* p, uint32_t t_us, uint16_t rep)
{
	packet* p1;
	if (t_us < 100) t_us = 100;
	for (int i = 0; i < rep; i++)
	{
		qout.push(p);
		u = _T("i=");
		u.AppendFormat(_T("%d"), i);
		error(SEV_DEBUG, u);
		for (int j = 0; j < 100; j++)
		{
			if (!qin.empty())
			{
				p1 = qin.front();
				qin.pop();
				if ((p1->seq == (p->seq ==0xFF ? 0 : p->seq + 1)) && ((p->cmd == READ && p1->cmd == DATA) || (p->cmd != READ && p1->cmd == ACK)))
				{
					delete p;
					return p1;
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::microseconds((100 - i)*t_us / 100));
					break;
				}
			}
			std::this_thread::sleep_for(std::chrono::microseconds(t_us / 100));
			u = _T("i=");
			u.AppendFormat(_T("%d j=%d"), i,j);
			error(SEV_DEBUG, u);
		}
	}
	delete p;
	return NULL;
}


packet* picprog::send_cmd(uint8_t cmd, uint16_t addr, uint32_t timeout, uint8_t rep)
{
	packet* p1=NULL;
	packet* p = new packet(picprog::seq++, cmd, addr, 0, 0);
	if ((p1=send_wait_ack(p, timeout, rep))==NULL)
	{
		u = _T("No ack for ");
		u.AppendFormat(_T("%d ms."), timeout * rep / 1000);
		error(SEV_MESSAGE,u);
		return NULL;
	}
	else
	{
		u = _T("Answer from programmer: CMD ");
		u.AppendFormat(_T("%s executed"), z[cmd & 0x0f]);
		error(SEV_MESSAGE, u);
		return p1;
	}
}


boolean picprog::send_sec(struct sec_merged* section, uint8_t delta, uint32_t timeout, uint8_t rep,uint8_t erase_flag)
{
	uint16_t n = 0;
	uint16_t addr = section->addr;
	while (n < section->size)
	{
		uint8_t m = (n + delta < section->size) ? delta : section->size - n;
		packet* p = new packet(picprog::seq++, WRITE|erase_flag|FLAG_CHECK, addr + (n >> 1), &(section->merged_data[n]), m);
		if (!send_wait_ack(p, timeout, rep))
		{
			u = _T("No ack for ");
			u.AppendFormat(_T("%d ms."), timeout * rep / 1000);
			error(SEV_MESSAGE, u);
			return false;
		}
		else
		{
			u = _T("Written ");
			u.AppendFormat(_T("%d bytes at address 0x%04X"),m,addr + (n >> 1));
			error(SEV_MESSAGE, u);
			pos++;
			if(w && (pos%5==0 || pos==blocks)) ::SendMessage(hwnd, WM_PICPOINT, pos, 0);
		};
		n += m;
	}
	return true;
}

void picprog::FillUidSec(unsigned long* uid_l)
{
	for (int i = 0; i < 4; i++)
	{
		uid_data[2 * i] = (uid_l[i] >> 8) & 0xff;
		uid_data[2 * i + 1] = uid_l[i] & 0xff;
	}
	uid_sec->addr = 0x8000;
	uid_sec->merged_data = uid_data;
	uid_sec->size = 8;
}

BOOL picprog::proc(void)
{
	packet* p1=NULL;
	TCHAR x[200];
	if ( port->open() == 0 && e0->good) good = TRUE;
	thread_send = std::thread(&comport::send, this);
	thread_recv = std::thread(&comport::recv, this);
	if ((p1 = send_cmd(START, 0, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x01"));
	else delete p1;
	if ((p1 = send_cmd(ERASE_ALL, 0, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x04"));
	else delete p1;
	if (!send_sec(&e0->code_m, 64, 5 * 1000, 5, FLAG_ERASE)) return fatal(_T("There were critical errors writing code, code=0x05"));
	if (!send_sec(&e0->conf_m, 2, 5 * 1000, 5, FLAG_ERASE)) return fatal(_T("There were critical errors writing conf, code=0x06"));
	if (e0->data_m.size != 0) if (!send_sec(&e0->data_m, 2, 5 * 1000, 5, FLAG_ERASE)) return fatal(_T("There were critical errors writing data, code=0x07"));
	if (!send_sec(uid_sec, 2, 5 * 1000, 5, 0)) return fatal(_T("There were critical errors writing uid, code=0x09"));
	if ((p1 = send_cmd(STOP, 0, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x08"));
	else delete p1;
	good = FALSE;
	thread_send.join();
	thread_recv.join();
	port->close();
	return TRUE;
}


BOOL picprog::readUid(unsigned long* uid_l)
{
	packet* p1 = NULL;
	TCHAR x[200];
	if (port->open() == 0 && e0->good) good = TRUE;
	thread_send = std::thread(&comport::send, this);
	thread_recv = std::thread(&comport::recv, this);
	if ((p1 = send_cmd(START, 0, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x01"));
	else delete p1;
	if ((p1 = send_cmd(READ, 0x8006, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x02"));
	else
	{
		if (p1->data[0] == 0 && p1->data[1] == 0) return fatal(_T("No connection to PIC, code=0x03"));
		else
		{
			TCHAR chip[16] = { _T("Unknown") };
			uint16_t id = p1->data[0] * 256 + p1->data[1];
			for (int ii = 0; ii < 16; ii++) if (ids[ii] == id) _tcscpy_s(chip, chips[ii]);
			if (!w)
			{
				_stprintf_s(x, _T("Addr=0x%04X len=%d Device_ID=0x%04X Chip=%s"), ((p1->addr >> 8) | (p1->addr << 8)) & 0xFFFF, p1->len, id, chip);
				error(SEV_MESSAGE, x);
			}
			else
			{
				_stprintf_s(x, _T("Chip Type: Device_ID=0x%04X Chip=%s"), id, chip);
				::SendMessage(hwnd, WM_CHIPTYPE, 0, (LPARAM)x);
			}
		}
		delete p1;
	}
	for (int i = 0; i < 4; i++)
	{
		if ((p1 = send_cmd(READ, 0x8000+i, 10 * 1000, 5)) == NULL) return fatal(_T("There were critical errors, code=0x20"));
		else
		{
			uid_l[i] = (p1->data[0] << 8) | p1->data[1];
			delete p1;
		}
	}
	good = FALSE;
	thread_send.join();
	thread_recv.join();
	port->close();
	return TRUE;
}

BOOL picprog::fatal(LPCTSTR er)
{
	good = FALSE;
	thread_send.join();
	thread_recv.join();
	port->close();
	error(SEV_FATAL, er);
	return FALSE;
}

picprog::~picprog()
{
	delete port;
	delete e0;
}

void picprog::error(int sev, LPCTSTR er)
{
	if (w)
	{
		if (sev == SEV_MESSAGE)
		{
			::SendMessage(hwnd, WM_MessageMessage, 0, (LPARAM)er);
		}
		if (sev == SEV_FATAL)
		{
			good = FALSE;
			Sleep(1000);
			port->close();
			::MessageBox(NULL, er, _T("Error"), MB_OK | MB_ICONERROR);
		}
	}
	else
	{
		if(sev!=SEV_DEBUG) _tprintf_s(_T("%s\n"),er);
		if (sev == SEV_FATAL) exit(1);
	}
}


