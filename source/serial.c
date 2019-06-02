
#include "W7100.h"
#include "serial.h"
/*
int Serial_int4()	 interrupt 4	   
{
	//(RI) RI =0; //clear
	//(TI) TI =0;
}
*/

void InitSerial(void)
{
	ET1 = 0;		/* TIMER1 INT DISABLE */
	TMOD = 0x20;  	/* TIMER MODE 2 */
	PCON |= 0x80;		/* SMOD = 1 */


	TH1 = 0xFC;		/* X2 115200(SMOD=1) at 88.4736 MHz */
	TR1 = 1;		/* START THE TIMER1 */	
	SCON = 0x52;		/* SERIAL MODE 1, REN=1, TI=1, RI=0 */

	/* Interrupt */
	RI   = 0; 		
	TI   = 0; 		
	ES   = 0; 	/* Serial interrupt disable */
}

/*
char _getkey ()  //reentrant
{
 	unsigned char byData;
	// Wait till data is received.
	while(!RI);		
	RI = 0;
	// Read data.
	byData = SBUF;		
	return byData;
}
 */
char putchar (char c)  
{
 	// Write data into serial-buffer.
	SBUF = c; 
	// Wait till data recording is finished.
	while(!TI);
	TI = 0;
	return c;
}
