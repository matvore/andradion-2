#include "Fixed.h"

#pragma warning (disable : 4035)

FIXEDNUM FixedDiv(FIXEDNUM x,FIXEDNUM y)
{
	__asm
	{
		mov eax,x
		cdq
		shld edx,eax,16
		sal eax,16
		idiv y
		shld edx,eax,16
	}
}

FIXEDNUM FixedMul(FIXEDNUM x,FIXEDNUM y)
{
	__asm
	{
		mov  eax,x
		imul y
		shrd eax,edx,16
	}
}

#pragma warning (default : 4035)
