
#include "w7100.h"
#include "wizmemcpy.h"

void wizmemcpy(unsigned long fsrc, unsigned long fdst, unsigned int len)  small
{
	unsigned char  eaback = EA;

	DPX0BK = DPH0;
	DPX1BK = DPL0;

	DPX0 = (unsigned char)(fsrc>>16);
	DPH0 = (unsigned char)(fsrc>>8);
	DPL0 = (unsigned char)(fsrc);

	DPX1 = (unsigned char)(fdst>>16);
	DPH1 = (unsigned char)(fdst>>8);
	DPL1 = (unsigned char)(fdst);

	RAMEA16 = len;	 	 

	EA = 0;
	WCONF &= ~(0x40);     // Enable ISP Entry
	((void(code*)(void))ENTRY_FCOPY)();
	DPH = DPX0BK;
	DPL = DPX1BK;

	DPX0 = 00;
	DPX1 = 00;
	WCONF |= 0x40;        // Disable ISP Entry

	EA = eaback;
}
