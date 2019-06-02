/*
 * (c)COPYRIGHT WIZnet Inc. ALL RIGHT RESERVED
 *
 * FileName : TCPIPCore.c
 * Revision History :
 * ----------	-------		------------------------------------------------
 * 	Date			version	  	Description
 * ----------	-------  	------------------------------------------------
 * 08/01/2009	1.0			Release Version
 * ----------	-------		------------------------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "types.h"
#include "W7100.h"
#include "serial.h"
#include "iinchip_conf.h"
#include "wizmemcpy.h"
#include "TCPIPCore.h"


/**
@brief	This function writes the data into W7100 registers.
*/
void IINCHIP_WRITE(uint16 addr,uint8 wizdata) 
{
	uint8 tmpEA;

	tmpEA = EA;
	EA = 0;
	DPX0 = 0xFE;
	*((vuint8 xdata*)(addr)) = wizdata;
	DPX0 = 0x00;
	EA = tmpEA;
}


/**
@brief	This function reads the value from W7100 registers.
*/
uint8 IINCHIP_READ(uint16 addr)
{
	register wizdata;
	uint8 tmpEA;

	tmpEA = EA;
	EA = 0;
	DPX0 = 0xFE;
	wizdata = *((vuint8 xdata*)(addr));
	DPX0 = 0x00;
	EA = tmpEA;
	return wizdata;
}

#ifdef __DEF_IINCHIP_INT__
/**
@brief	Socket interrupt routine
*/ 
static void iinchip_irq(void) interrupt 11
{
	uint8 int_val;
	uint8 i;
	uint8 tmp_STATUS;

	EA = 0;
	int_val = IINCHIP_READ(IR);

	if (int_val & IR_CONFLICT)
	{
		printf("IP conflict : %.2x\r\n", int_val);
	}
	if (int_val & IR_UNREACH)
	{
		printf("INT Port Unreachable : %.2x\r\n", int_val);
		printf("UIPR0 : %d.%d.%d.%d\r\n", IINCHIP_READ(UIPR0), IINCHIP_READ(UIPR0+1), IINCHIP_READ(UIPR0+2), IINCHIP_READ(UIPR0+3));
		printf("UPORT0 : %.2x %.2x\r\n", IINCHIP_READ(UPORT0), IINCHIP_READ(UPORT0+1));
	}
	IINCHIP_WRITE(IR, int_val);

	int_val = IINCHIP_READ(IR2);
	for (i=0; i<MAX_SOCK_NUM; i++) 
	{
         	if (int_val & IR_SOCK(i)) 
		{
         		tmp_STATUS= IINCHIP_READ(Sn_IR(i));
			I_STATUS[i] |= (tmp_STATUS & 0xEF); 
            		IINCHIP_WRITE(Sn_IR(i), (tmp_STATUS & 0xEF));
		 }
	}
	IINCHIP_WRITE(IR2, int_val);

	EIF &= 0xF7;	// Clear TCPIPCore Interrupt Flag
	EA = 1;
}
#endif

uint8 getSn_IR(uint8 s)
{
#ifdef __DEF_IINCHIP_INT__
	return I_STATUS[s];
#else
	return IINCHIP_READ(Sn_IR(s));
#endif
}

void setSn_IR(uint8 s, uint8 val)
{
#ifdef __DEF_IINCHIP_INT__
	I_STATUS[s] = I_STATUS[s] & (~val);
#else
	IINCHIP_WRITE(Sn_IR(s), val);
#endif
}

/**
@brief	This function set the transmit & receive buffer size as per the channels is used

Maximum memory size for Tx, Rx in the W7100 is 16K Bytes,\n
In the range of 16KBytes, the memory size could be allocated dynamically by each channel.\n
Be attentive to sum of memory size shouldn't exceed 16Kbytes\n
and to data transmission and receiption from non-allocated channel may cause some problems.\n
If the 16KBytes memory is already  assigned to centain channel, \n
other channels couldn't be used, for there's no available memory.\n
*/ 
void set_MEMsize(	uint8* tx_size, uint8* rx_size)
{
	int16 xdata i;
	int16 xdata ssum,rsum;

	ssum = 0;
	rsum = 0;
	
	SBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_TXBUF__);		/* Set base address of Tx memory for channel #0 */
	RBUFBASEADDRESS[0] = (uint16)(__DEF_IINCHIP_MAP_RXBUF__);		/* Set base address of Rx memory for channel #0 */

	for (i = 0 ; i < MAX_SOCK_NUM; i++)       // Set the size, masking and base address of Tx & Rx memory by each channel
	{
		IINCHIP_WRITE((Sn_TXMEM_SIZE(i)),tx_size[i]);
		IINCHIP_WRITE((Sn_RXMEM_SIZE(i)),rx_size[i]);

		SSIZE[i] = (int16)(0);
		RSIZE[i] = (int16)(0);

		if (ssum < 16*1024)
		{
	         switch( tx_size[i] )
			{
			case 1:
				SSIZE[i] = (int16)(1024);
				SMASK[i] = (uint16)(0x03FF);
				break;
			case 2:
				SSIZE[i] = (int16)(2048);
				SMASK[i] = (uint16)(0x07FF);
				break;
			case 4:
				SSIZE[i] = (int16)(4096);
				SMASK[i] = (uint16)(0x0FFF);
				break;
			case 8:
				SSIZE[i] = (int16)(8192);
				SMASK[i] = (uint16)(0x1FFF);
				break;
			}
		}

		if (rsum < 16*1024)
		{
       	  switch( rx_size[i] )
			{
			case 1:
				RSIZE[i] = (int16)(1024);
				RMASK[i] = (uint16)(0x03FF);
				break;
			case 2:
				RSIZE[i] = (int16)(2048);
				RMASK[i] = (uint16)(0x07FF);
				break;
			case 4:
				RSIZE[i] = (int16)(4096);
				RMASK[i] = (uint16)(0x0FFF);
				break;
			case 8:
				RSIZE[i] = (int16)(8192);
				RMASK[i] = (uint16)(0x1FFF);
				break;
			}
		}
		ssum += SSIZE[i];
		rsum += RSIZE[i];

        if (i != 0)             // Sets base address of Tx and Rx memory for channel #1,#2,#3
		{
			SBUFBASEADDRESS[i] = SBUFBASEADDRESS[i-1] + SSIZE[i-1];
			RBUFBASEADDRESS[i] = RBUFBASEADDRESS[i-1] + RSIZE[i-1];
		}
	}
}

uint8 getSn_SR(SOCKET s)
{
	uint8 xdata sr=0xff, sr1=0xff;

	while(1)
	{
		sr = (uint8)IINCHIP_READ(Sn_SR(s)); 
		if(sr == sr1) break;                 
		sr1 = sr;                            
	}
	return sr;
}

/**
@brief	get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16 getSn_TX_FSR(SOCKET s)
{
	uint16 xdata val=0;
 	uint16 xdata val1=0;
   
 	while(1)
	{
		val = IINCHIP_READ(Sn_TX_FSR0(s));
		val = (val << 8) + IINCHIP_READ(Sn_TX_FSR0(s) + 1);
		if (val == val1) break;
		val1 = val;
	} 
	return val;
}


/**
@brief	 get socket RX recv buf size

This gives size of received data in receive buffer. 
*/
uint16 getSn_RX_RSR(SOCKET s)
{
	uint16 xdata val=0;
	uint16 xdata val1=0;

	while(1)
	{
		val = IINCHIP_READ(Sn_RX_RSR0(s));
		val = (val << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
	  	if(val == val1) break;
		val1 = val;
	} 
	return val;
}

/**
@brief	 This function is being called by send() and sendto() function also. 

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void IINCHIP_WRITE_BUF(SOCKET s, vuint8 * src, vuint8 * dst, uint16 len)
{
	uint16 xdata size;
	uint16 xdata dst_mask;
	uint16 xdata src_ptr = (uint16)src;
	uint16 xdata dst_ptr;
	
	dst_mask = (uint16)dst & SMASK[s];
	dst_ptr = SBUFBASEADDRESS[s] + dst_mask;

	if (dst_mask + len > SSIZE[s]) 
	{
		size = SSIZE[s] - dst_mask;
		wizmemcpy((0x000000+src_ptr), (0xFE0000+dst_ptr),size);
		src_ptr += size;
		size = len - size;
		dst_ptr = SBUFBASEADDRESS[s];
		wizmemcpy((0x000000+src_ptr),(0xFE0000+dst_ptr),size);
	} 
	else wizmemcpy((0x000000+src_ptr),(0xFE0000+dst_ptr),len);

}


/**
@brief	This function is being called by recv() also.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void IINCHIP_READ_BUF(SOCKET s, vuint8 * src, uint8 * dst, uint16 len)
{
	uint16 xdata size;
	uint16 xdata src_mask;
	uint16 xdata src_ptr;
	uint16 xdata dst_ptr = (uint16)dst;
	
	
	src_mask = (uint16)src & RMASK[s];
	src_ptr = RBUFBASEADDRESS[s] + src_mask;

	if( (src_mask + len) > RSIZE[s] ) 
	{
		size = RSIZE[s] - src_mask;
		wizmemcpy((0xFE0000+src_ptr),(0x000000+dst_ptr),size);
		dst_ptr += size;
		size = len - size;
		src_ptr = RBUFBASEADDRESS[s];
		wizmemcpy((0xFE0000+src_ptr),(0x000000+dst_ptr),size);
	} 
	else
	{
		wizmemcpy((0xFE0000+src_ptr),(0x000000+dst_ptr),len);
	}
	
}

