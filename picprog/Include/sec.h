#pragma once

#ifndef SEC_H
#define SEC_H

#include "pch.h"

class sec
{
public:
	int ind;
	std::string name;
	uint32_t addr;
	uint32_t size;
	uint8_t* data;
	boolean le;
	bool operator<(sec s0) const;
	void print_attr();
	void print_sec(boolean le);
	sec(int ind, std::string name, uint32_t addr, uint32_t size, char* data,boolean le);
	~sec();
};

#endif // SEC_H
