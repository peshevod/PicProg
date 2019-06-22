#include "pch.h"
#include "sec.h"
#include <iostream>
#include <iomanip>

using namespace std;

sec::sec(int ind, std::string name, uint32_t addr, uint32_t size, char* data,boolean le)
{
	this->ind = ind;
	this->addr = addr;
	this->name = name;
	this->size = size;
	this->data = (uint8_t*)data;
	this->le = le;
}

void sec::print_attr()
{
	ios state(nullptr);
	cout << ind
		<< ' '
		<< name
		<< ' ';
	state.copyfmt(cout); // save current formatting
	cout << "0x" // now load up a bunch of formatting modifiers
		<< hex
		<< uppercase
		<< setw(4)
		<< setfill('0')
		<< addr
		<< " 0x"
		<< size // the actual value we wanted to print out
		<< endl;
	cout.copyfmt(state); // restore previous formatting
}

void sec::print_sec(boolean le)
{
	ios state(nullptr);
	int rows = size / 16;
	int rest = size % 16;
	if (rest != 0) rows++;
	state.copyfmt(cout); // save current formatting
	for (int i = 0; i < rows; i++)
	{
		cout << "0x" << hex << uppercase << setw(4) << setfill('0') << (addr + 8 * i) << " ";
		cout.copyfmt(state); // restore previous formatting

		for (int j = 0; j < ((i == (rows - 1)) && rest != 0 ? rest/2 : 8); j++)
		{
			uint16_t x;
				if(le) x= (data[i * 16 + 2 * j+1] << 8) | data[i * 16 + 2 * j];
				else x= (data[i * 16 + 2 * j] << 8) | data[i * 16 + 2 * j+1];
			cout << "  " << "0x" << hex << uppercase << setw(4) << setfill('0') << x;
		}
		cout << endl;
	}
	cout.copyfmt(state); // restore previous formatting
}

bool sec::operator<(sec s0) const
{
	return (this->addr < s0.addr);
}

sec::~sec()
{
}
