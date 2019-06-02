/*
*
@file		delay.c
@brief	delay functions for the W7100
*
*/

#include "delay.h"

void wait_1us(unsigned int cnt)
{
	unsigned int i;
	
	for(i = 0; i<cnt; i++) {
#pragma ASM
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
#pragma ENDASM
		}
}

// Warning > The wait function is optimized at 1us. If you use the 'wait_1ms' or 
//			'wait_10ms', there may be some errors.
//			The Timer can be used for accuate calculations.

void wait_1ms(unsigned int cnt)
{
	unsigned int i;

	for (i = 0; i < cnt; i++) wait_1us(1000);
}

void wait_10ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1ms(10);
}

