#pragma once

#ifndef ELF_H
#define ELF_H


#include "pch.h"
#include "sec.h"
#include "picprog.h"
#include <iostream> 
#include <set> 
#include <iterator> 

using namespace ELFIO;
using namespace std;

struct sec_merged
{
	uint8_t* merged_data;
	uint32_t addr;
	uint32_t size;
};

typedef sec_merged* sec_merged_t ;

class elf
{
public:
	int nsec;
	class picprog* pp;
	boolean le;
	set<sec> code;
	set<sec> conf;
	set<sec> data;
	elfio reader;
	sec_merged code_m;
	sec_merged conf_m;
	sec_merged data_m;
	void print_attr(set<sec> s, const char* comment);
	void print_sec(set<sec> s, const char* comment);
	void print_merged(sec_merged_t, const char* comment);
	uint32_t merge_sec(set<sec> s, sec_merged_t sec_m);
	elf(LPCTSTR filename,picprog* pp);
	BOOL good;
	~elf();
};

#endif  // ELF_H

