#include "pch.h"
#include "picprog.h"

int _tmain(int argc, TCHAR** argv)
{
	picprog *pp = new picprog(argc, argv);
	pp->proc();
}
