
/*

Circuit Cellar WizNet Contest 2010
Project 3035
Budget Network Attached Storage Unit

*/


#include <stdio.h>
#include "lcd.h"
#include "serial.h" 		// serial related functions
#include "socket.h" 		// W7100A driver file
#include "types.h"
#include "w7100.h"
#include "Wizmemcpy.h"
#include "TCPIPcore.h"
#include "delay.h"
#include "ataoe.h"

void Init_iMCU(); 		
void Init_Network();	

uint8 xdata ip[4] = {10,1,1,2};                   // for setting SIP register
uint8 xdata gw[4] = {10,1,1,1};                     // for setting GAR register
uint8 xdata sn[4] = {255,255,255,0};                     // for setting SUBR register
uint8 xdata mac[6] = {0x00,0x08,0xDC,0x00,0x00,0x00};      // for setting SHAR register

void main()
{
	init_ata_hardware();

	Init_iMCU();		// Initialize   iMCUW7100
	lcd_init();  
	evb_set_lcd_text(0,(uint8 *) "NAS INIT");     
	Init_Network(); 	// Initialize   Network Configuration
 			
	//start message
	printf("\r\n-------ATAOE DRIVER START--------\r\n");
	
	ataoe(0, &mac);

	printf("\r\n-------ATAOE DRIVER END--------\r\n");
	
	while(1);

}

void Init_iMCU(void)
{ 
	uint8 xdata i;

	EA = 0; 			// Disable all interrupts 
	CKCON = 0x02;		// External Memory Access Time
	WTST = 0x03;
#ifdef __DEF_IINCHIP_INT__	
	IINCHIP_WRITE(IMR,0xff);
	for(i=0;i<MAX_SOCK_NUM;i++) IINCHIP_WRITE(Sn_IMR(i),0x1f);	
	IINCHIP_ISR_ENABLE();
#endif	
	InitSerial(); 	// Initialize serial port (Refer to serial.c)
	EA = 1;
}

void Init_Network(void)
{

	uint8 xdata str[17];

	uint8 xdata txsize[MAX_SOCK_NUM] = {8,1,1,1,1,1,1,1};
	uint8 xdata rxsize[MAX_SOCK_NUM] = {8,1,1,1,1,1,1,1};
	// uint8 xdata txsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
	// uint8 xdata rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};

	/* Write MAC address of W7100 to SHAR register */
	IINCHIP_WRITE (SIPR0+0,ip[0]);
	IINCHIP_WRITE (SIPR0+1,ip[1]);
	IINCHIP_WRITE (SIPR0+2,ip[2]);
	IINCHIP_WRITE (SIPR0+3,ip[3]);
	
	/* Write GATEWAY of W7100 to GAR register */
	IINCHIP_WRITE (GAR0+0, gw[0]);
	IINCHIP_WRITE (GAR0+1, gw[1]);
	IINCHIP_WRITE (GAR0+2, gw[2]);
	IINCHIP_WRITE (GAR0+3, gw[3]);
	/* Write SUBNETMASK of W7100 to SURR register */
	IINCHIP_WRITE (SUBR0+0, sn[0]);
	IINCHIP_WRITE (SUBR0+1, sn[1]);
	IINCHIP_WRITE (SUBR0+2, sn[2]);
	IINCHIP_WRITE (SUBR0+3, sn[3]);
	/* Write MAC address of W7100 to SHAR register */
	IINCHIP_WRITE (SHAR0+0, mac[0]);
	IINCHIP_WRITE (SHAR0+1, mac[1]);
	IINCHIP_WRITE (SHAR0+2, mac[2]);
	IINCHIP_WRITE (SHAR0+3, mac[3]);
	IINCHIP_WRITE (SHAR0+4, mac[4]);
	IINCHIP_WRITE (SHAR0+5, mac[5]);
	
	set_MEMsize(txsize,rxsize);

    while(!(EIF & 0x02));
	EIF &= ~0x02;

	printf( "==============================================\r\n");
	printf( "       W7100   Net Config Information         \r\n");
	printf( "==============================================\r\n");

	printf( "MAC ADDRESS IP : %.2x.%.2x.%.2x.%.2x.%.2x.%.2x\r\n", 
		(int)IINCHIP_READ (SHAR0+0), (int)IINCHIP_READ (SHAR0+1), 
		(int)IINCHIP_READ (SHAR0+2), (int)IINCHIP_READ (SHAR0+3),
		(int)IINCHIP_READ (SHAR0+4), (int)IINCHIP_READ (SHAR0+5)
		) ;
 	sprintf(str,"%.3d.%.3d.%.3d.%.3d",
			(int)IINCHIP_READ (SUBR0+0), (int)IINCHIP_READ (SUBR0+1), 
			(int)IINCHIP_READ (SUBR0+2), (int)IINCHIP_READ (SUBR0+3));
	printf( "SUBNET MASK  : %s\r\n",str); 

 	sprintf(str,"%.3d.%.3d.%.3d.%.3d",
			(int)IINCHIP_READ (GAR0+0), (int)IINCHIP_READ (GAR0+1), 
			(int)IINCHIP_READ (GAR0+2), (int)IINCHIP_READ (GAR0+3));

	printf( "G/W IP ADDRESS : %s\r\n",str);

 	sprintf(str,"%.3d.%.3d.%.3d.%.3d",
			(int)IINCHIP_READ (SIPR0+0), (int)IINCHIP_READ (SIPR0+1), 
			(int)IINCHIP_READ (SIPR0+2), (int)IINCHIP_READ (SIPR0+3));

	printf( "LOCAL IP ADDRESS : %s\r\n", str);

  //display IP
  evb_set_lcd_text(1,str);	

}

