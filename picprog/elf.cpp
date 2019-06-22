#include "pch.h"
#include "elf.h"
#include "picprog.h"
#include "sec.h"
#include <iostream>
#include <iomanip>

extern CString u;

elf::elf(LPCTSTR filename,picprog* pp)
{
	this->pp = pp;
	good = FALSE;
	std::ifstream stream;
	stream.open(filename, std::ios::in | std::ios::binary);
	if(!stream || !reader.load(stream))
	{
		u = _T("Error reading file ");
		u.AppendFormat(_T("%s"), filename);
		pp->error(SEV_FATAL,u);
		return;
	}
	nsec=reader.sections.size();
	if (reader.get_encoding() == ELFDATA2LSB)
		le=true;
	else
		le=false;
	data_m.size = 0;
	for(int i = 0; i < nsec; i++)
	{
		if (reader.sections[i]->get_type() != SHT_PROGBITS) continue;
		uint32_t addr = reader.sections[i]->get_address();
		sec* s = new sec(i, reader.sections[i]->get_name(), addr, (uint32_t)reader.sections[i]->get_size(), (char*)reader.sections[i]->get_data(),le);
		if (addr < 0x8000) code.insert(*s);
		else if (addr >= 0x8007 && addr <= 0x800b) conf.insert(*s);
		else if (addr >= 0xF000) data.insert(*s);
	}
//	print_sec(code, "code");
//	print_sec(data, "data");
	merge_sec(code, &code_m);
	merge_sec(conf, &conf_m);
	if(data.size()!=0) merge_sec(data, &data_m);
//	print_merged(&code_m, "merged code");
//	print_merged(&conf_m, "merged conf");
//

//	print_merged(&data_m, "merged data");
	stream.close();
	good = TRUE;

}

void elf::print_attr(set<sec> s, const char* comment)
{
	set<sec>::iterator it;
	printf("%s\n", comment);
	for (it = s.begin(); it != s.end(); ++it)
	{
		sec s1 = *it;
		s1.print_attr();
	}

}

void elf::print_sec(set<sec> s, const char* comment)
{
	set<sec>::iterator it;
	cout << endl << comment << endl;
	for (it = s.begin(); it != s.end(); ++it)
	{
		sec s1 = *it;
		s1.print_attr();
		cout << endl;
		s1.print_sec(le);
		cout << endl;
	}

}

void elf::print_merged(sec_merged_t sec_m, const char* comment)
{
	cout << endl << comment << endl;
	ios state(nullptr);
	int rows = sec_m->size / 16;
	int rest = sec_m->size % 16;
	if (rest != 0) rows++;
	state.copyfmt(cout); // save current formatting
	for (int i = 0; i < rows; i++)
	{
		cout << "0x" << hex << uppercase << setw(4) << setfill('0') << (sec_m->addr + 8 * i) << " ";
		cout.copyfmt(state); // restore previous formatting

		for (int j = 0; j < ((i == (rows - 1)) && rest != 0 ? rest / 2 : 8); j++)
		{
			uint16_t x = (sec_m->merged_data[i * 16 + 2 * j] << 8) | sec_m->merged_data[i * 16 + 2 * j+1];
			cout << "  " << "0x" << hex << uppercase << setw(4) << setfill('0') << x;
		}
		cout << endl;
	}
	cout.copyfmt(state); // restore previous formatting
}

uint32_t elf::merge_sec(set<sec> s, sec_merged_t sec_m)
{
	set<sec>::iterator it, it_first;
	set<sec>::reverse_iterator it_last;
	it_first= s.begin();
	sec first_sec = *it_first;
	it_last = s.rbegin();
	sec last_sec = *it_last;
	sec_m->addr = first_sec.addr;
	sec_m->size = (last_sec.addr - first_sec.addr) * 2 + last_sec.size;
	sec_m->merged_data = (uint8_t*)malloc(sec_m->size);
	uint32_t last_byte = 0;
	for (it = s.begin(); it != s.end(); ++it)
	{
		sec s1 = *it;
		if (((s1.addr - first_sec.addr) * 2) > last_byte)
		{
			for (uint32_t i = last_byte; i < ((s1.addr - first_sec.addr) * 2); i++) sec_m->merged_data[i] = 0;
			last_byte = (s1.addr - first_sec.addr) * 2;
		}
		if(!le) memcpy(&(sec_m->merged_data[(s1.addr-first_sec.addr)*2]), s1.data, s1.size);
		else
		{
			for (uint32_t i =0; i < s1.size; i += 2)
			{
				sec_m->merged_data[i + (s1.addr - first_sec.addr) * 2] = s1.data[i + 1];
				sec_m->merged_data[i + (s1.addr - first_sec.addr) * 2+1] = s1.data[i];
			}
		}
		last_byte = (s1.addr - first_sec.addr) * 2 + s1.size;
	}
	return sec_m->size;
}

elf::~elf()
{
	if (good)
	{
		if (code_m.size != 0)
		{
			free(code_m.merged_data);
			code_m.size = 0;
		};
		if (conf_m.size != 0)
		{
			free(conf_m.merged_data);
			conf_m.size = 0;
		};
		if (data_m.size != 0)
		{
			free(data_m.merged_data);
			data_m.size = 0;
		};
	}
}

