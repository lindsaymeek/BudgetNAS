/*

Circuit Cellar WizNet Contest 2010
Project 3035
Budget Network Attached Storage Unit
ATAOE driver

 */

#include "ataoe.h"
#include "socket.h" 		// W7100A driver file
#include "w7100.h"
#include "Wizmemcpy.h"
#include "TCPIPcore.h"
#include "delay.h"
#include "sockutil.h"
#include <stdio.h>
  
#define MY_MAJOR 1
#define MY_MINOR 0

#define AOE_TYPE 0x88a2

#define AOE_CMD_ATA 0
#define AOE_CMD_CONFIG 1
#define AOE_CMD_MAC_MASK 2
#define AOE_CMD_RESERVE_RELEASE 3

#define ATA_CMD_PIO_EREAD		0x24
#define ATA_CMD_PIO_READ		0x20
#define ATA_CMD_PIO_WRITE		0x30
#define ATA_CMD_ID_ATA			0xEC

///
///IDE status register bits
///
#define IDE_BSY		(1<<7)
#define IDE_DRDY	(1<<6)
#define IDE_DF      (1<<5)
#define IDE_DRQ		(1<<3)
#define IDE_ERR		(1<<0)

#define LBA_SIZE					4
#define AOE_TAG_SIZE				4

static uint8 xdata ide_status=0;

static AtaIssue xdata aATA _at_ 0x007000;


static void DumpAtaHdr(AtaHdr *p)
{
  printf("DEST %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
   				(int)p->dst_mac[0],(int)p->dst_mac[1],(int)p->dst_mac[2],(int)p->dst_mac[3],(int)p->dst_mac[4],(int)p->dst_mac[5]);
 
  printf("SRC %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
   				(int)p->src_mac[0],(int)p->src_mac[1],(int)p->src_mac[2],(int)p->src_mac[3],(int)p->src_mac[4],(int)p->src_mac[5]);
   
	printf("FLAGS %x\r\nMAJOR %u\r\nMINOR %u\r\nCMD %x\r\n", 
			(int)p->flags,htons(p->major),(int)p->minor,(int)p->cmd);

}

static void DumpATAIssue(AtaIssue *p)
{
   	   DumpAtaHdr(&p->header);
  
	    printf("ATA CMD %X\r\n", (int)p->acmd);
		printf("SECTORS %u\r\n", (int)p->sectors);
		printf("LBA %02X:%02X:%02X:%02X:%02X:%02X\r\n",
				(int)p->lba0,(int)p->lba1,(int)p->lba2,(int)p->lba3,(int)p->lba4,(int)p->lba5);


}

static void DumpATAConfig(AtaConfig *p)
{
   	   DumpAtaHdr(&p->header);

	   printf("Max Queue Length %u\r\n", htons(p->buffer_count));
	   printf("Firmware Version %u\r\n", htons(p->firmware_version));
 	   printf("Max Sectors %u\r\n",(int)p->sectors);
	   printf("AOE Protocol %u\n\r",(int)((p->aoe_ccmd>>4)&15));
	   printf("CCMD %u\r\n",(int)(p->aoe_ccmd&15));
	   printf("String length %u\r\n",htons(p->length));


}

static void DumpATAMacMask( AtaMACMask *p)
{
   	   DumpAtaHdr(&p->header);
 
}

static void DumpATAReserveRelease( AtaReserveRelease *p)
{
   	   DumpAtaHdr(&p->header);
 
}

AtaConfig xdata ConfigResp;


static uint8 SendConfigAck(SOCKET s,uint8 *src_mac,AtaConfig *req)
{
	uint8 i;
   	uint32 xdata tip = 0xFFFFFFFF;

	DumpATAConfig(req);

	ConfigResp.header.msg_type = AOE_TYPE;
	ConfigResp.header.flags = 0x18;
	ConfigResp.header.error = 0; 
    ConfigResp.header.major = MY_MAJOR;
	ConfigResp.header.minor = MY_MINOR;
	ConfigResp.header.cmd = AOE_CMD_CONFIG;

  	ConfigResp.header.dst_mac[0] = aATA.header.src_mac[0];
  	ConfigResp.header.dst_mac[1] = aATA.header.src_mac[1];
	ConfigResp.header.dst_mac[2] = aATA.header.src_mac[2];
	ConfigResp.header.dst_mac[3] = aATA.header.src_mac[3];
	ConfigResp.header.dst_mac[4] = aATA.header.src_mac[4];
	ConfigResp.header.dst_mac[5] = aATA.header.src_mac[5];

	ConfigResp.header.src_mac[0] = src_mac[0];
	ConfigResp.header.src_mac[1] = src_mac[1];
	ConfigResp.header.src_mac[2] = src_mac[2];
	ConfigResp.header.src_mac[3] = src_mac[3];
	ConfigResp.header.src_mac[4] = src_mac[4];
	ConfigResp.header.src_mac[5] = src_mac[5];

	ConfigResp.header.tag = req->header.tag;

	// Maximum # of queued commands	
	ConfigResp.buffer_count = htons(1);
	ConfigResp.firmware_version= 0;

	ConfigResp.sectors=0;	// actually means 2 sectors max
	ConfigResp.aoe_ccmd=0x10;
	ConfigResp.length=0; 	// no config string response

	if( sendto(s,(uint8*)&ConfigResp,sizeof(ConfigResp),(uint8 *)&tip,3000) ==0)
		return 0;
	else
		return 1;
}

	
static uint8 ReadReg8(uint8 reg)
{
	static uint8 t;
	  
	reg &= 7;

	// Load address
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK) ;
	
	wait_1us(1);
	// CS=0
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_DIR_MASK) ;
	 	
		
	wait_1us(1);

	// Set the I/O pin to input (open collector with pullup)
	IDE_DATA_MSB = 0xFF;
	IDE_DATA_LSB = 0xFF;

	// DIR=0 RD=0 CS=0
	P1 = reg | (IDE_RESET_MASK|IDE_WR_MASK) ;
	
	// 1.25us 

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
	
	wait_1us(1);

	t = IDE_DATA_LSB;
				
	wait_1us(1);

	// DIR=0 RD=1 CS=0
	P1 = reg | IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK ;

	wait_1us(1);

	// DIR=1 RD=1 CS=1
	P1 = reg | IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK ;

	wait_1us(1);
		
	return	t;
}
		
static void ResetBus(void)
{

	// DIR=1 RD=1 CS=1 WR=1 RESET=0
	P1 = (IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK) ;

	wait_1us(100);
			  
	// DIR=1 RD=1 CS=1 WR=1 RESET=1
	P1 = (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK) ;

	wait_1ms(4);

}

static uint8 WaitBusy(void)
{
	uint16 i=0;
	uint8 j=0;

	do
	{

		do
		{
			ide_status = ReadReg8(7);
		
			if(!(ide_status & IDE_BSY))
				return 1;

		} while(++i!=0); 

	} while(++j != 10);

   	return 0;
}

static void WriteReg8(uint8 reg,uint8 x)
{
	reg &= 7;

   	// Load addr
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK) ;
			
	wait_1us(1);

  	// CS=0	DIR=1
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_DIR_MASK) ;
	 		 
	wait_1us(1);

	IDE_DATA_LSB = x;
	IDE_DATA_MSB = 0xFF;
 			 
	wait_1us(2);

#pragma ASM
		NOP
		NOP
		NOP
		NOP
		NOP
#pragma ENDASM
			
  	// CS=0 WR=0 DIR=1
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_DIR_MASK) ;
	 	
	wait_1us(1);

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

	// WR=1 CS=0 DIR=1
	P1 = reg | (IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_DIR_MASK) ;
	  	   	   	 
	wait_1us(1);

	IDE_DATA_LSB = 0xFF;
	IDE_DATA_MSB = 0xFF;
				 
	wait_1us(1);

	// WR=1 CS=1 DIR=1
	P1 = reg | IDE_RESET_MASK|IDE_RD_MASK|IDE_WR_MASK|IDE_CS_MASK|IDE_DIR_MASK ;
	 
	wait_1us(1);
}

static void DumpRegs(void)
{
	uint8 i;

	printf("Regs: ");

	for(i=1;i<8;i++)
	{
		printf("%02x ", (int)ReadReg8(i));
	}

	printf("\r\n");

}

// pointer dereferencing

static uint8 ATAHandler(SOCKET s,uint8 *src_mac)
{
	uint16 byte_cnt;
	uint8  blk_cnt;
	uint8 blocks;
	unsigned long m;
  	uint32 xdata tip = 0xFFFFFFF;
	uint8 i,j,*p;

//	DumpATAIssue(&aATA);

	// Wait for interface to be free
	WaitBusy();
		
//	DumpRegs();

	// extended or normal access?
	if(aATA.aflag & 0x40)
	{
	//	printf("Extended\r\n");

		WriteReg8(6,(aATA.aflag & 0x50) | 0xa0); 		
  
  		// Wait for device to respond
		WaitBusy();
				
		WriteReg8(3, aATA.lba3);  	// 	LBA low

		WriteReg8(3, aATA.lba0);

 		WriteReg8(4, aATA.lba4); 	// LBA Mid

		WriteReg8(4, aATA.lba1);

 		WriteReg8(5,aATA.lba5);	// LBA High

		WriteReg8(5,aATA.lba2);
     
		WriteReg8(2, 0);	 		// Sector count
	}
	else  // normal access
	{
	//	printf("Normal\r\n");

	 	WriteReg8(6, aATA.lba3);	// head

	  	// Wait for device to respond
		WaitBusy();

		WriteReg8(3,aATA.lba0); 	// 	LBA low

		WriteReg8(4,aATA.lba1);	// LBA Mid

		WriteReg8(5,aATA.lba2);	// LBA High
	}

	WriteReg8(2,aATA.sectors);	 	// Sector count
		
	WriteReg8(1,aATA.err);	  		// Err/Features

//	printf("Write values %02X %02X %02X %02X %02X %02X\r\n",(int)aATA.err,(int)aATA.sectors,(int)aATA.lba0,
//							(int)aATA.lba1,(int)aATA.lba2,(int)aATA.lba3);

//	DumpRegs();

	// Set up block copy registers
	m=(unsigned long)aATA.dat;
	DPX1 = (uint8)(m>>16);
	DPH1 = (uint8)(m>>8);
	DPL1 = (uint8)(m);
			 
	p=aATA.dat;

	WaitBusy();

	// Writing to ATA?
	if(aATA.aflag & 1)
	{
		blocks=0;
 
		if(ide_status & (IDE_DRDY|IDE_DRQ))
	    {

			// Address zero

			P1 = IDE_CS_MASK | IDE_RD_MASK | IDE_WR_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
			
			wait_1us(1);
			// CS=0
			P1 = IDE_RD_MASK | IDE_WR_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
			
			wait_1us(1);
			blk_cnt=aATA.sectors; 

			// Send the data to the device
			while(blk_cnt!=0)
			{
				--blk_cnt;
				    
				byte_cnt=0;
				do
				{

					IDE_DATA_LSB = *p++;
					IDE_DATA_MSB = *p++;
			  
					wait_1us(1);
					// WR=0 RD=1 RESET=1 DIR=1 CS=0
					P1 = IDE_RD_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
			 
					wait_1us(2);

					// WR=1 RD=1 RESET=1 CS=0 DIR=1
					P1 = IDE_WR_MASK | IDE_RD_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
			 
					wait_1us(1);

				} while(++byte_cnt != 256);
			}	

		    IDE_DATA_MSB=0xFF;
			IDE_DATA_LSB=0xFF;
			   
			wait_1us(1);
			P1 = IDE_CS_MASK | IDE_WR_MASK | IDE_RD_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
				
			wait_1us(1);
	 
			WaitBusy();
		}

	}
	else
	{
		blocks=0;
			 	 
		if(ide_status & (IDE_DRDY|IDE_DRQ))
	    {
		
			blocks=aATA.sectors;
			blk_cnt=aATA.sectors;
	 
			// Fetch data from the device
			while(blk_cnt!=0)
			{
				--blk_cnt;
			
				// This seems to set the I/O pin to input, must be open collector with pullup
				IDE_DATA_MSB = 0xFF;
				IDE_DATA_LSB = 0xFF;

				// Address zero
				P1 = IDE_DIR_MASK | IDE_CS_MASK | IDE_RD_MASK | IDE_WR_MASK | IDE_RESET_MASK;
					   		
				wait_1us(1);
							 
				for(byte_cnt=256;byte_cnt!=0;--byte_cnt)
				{
	 
					// CS=0 DIR=0 RD=1 WR=1 RESET=1
					P1 = IDE_RD_MASK | IDE_WR_MASK | IDE_RESET_MASK;
			   
					wait_1us(1);
					// CS=0	RD=0 DIR=0 RESET=1 WR=1
					P1 = IDE_WR_MASK | IDE_RESET_MASK;
	
					wait_1us(2);			
	
					*p++ = IDE_DATA_LSB;
					*p++ = IDE_DATA_MSB;
		   
					wait_1us(1);
					// CS=0	RD=1 DIR=0 WR=1 RESET=1
					P1 = IDE_WR_MASK | IDE_RD_MASK | IDE_RESET_MASK;
			
					wait_1us(1);
	 
					// CS=1	RD=1 DIR=1 WR=1 RESET=1 
					P1 = IDE_CS_MASK | IDE_WR_MASK | IDE_RD_MASK | IDE_RESET_MASK ;
					   
					wait_1us(1);			

				} 
			}
			
	// CS=1	RD=1 DIR=1 WR=1 RESET=1 
	P1 = IDE_CS_MASK | IDE_WR_MASK | IDE_RD_MASK | IDE_RESET_MASK | IDE_DIR_MASK ;
					   
	wait_1us(1);	
			
				}

#if 0
	// dump contents of packet
	 
	 for(i=0;i<32;i++)
	 {
		for(j=0;j<16;j++)
		{
			printf("%02X ",(int)aATA.dat[j+(i<<4)]);
		}
		printf("\r\n");
	 }
#endif

	}

	// Copy registers from device to response packet
	aATA.err = ReadReg8(1);   	// Err/Features
 	aATA.sectors = ReadReg8(2); // Sector count
 	aATA.lba0 = ReadReg8(3); 	// 	LBA low
	aATA.lba1 = ReadReg8(4);	// LBA Mid
	aATA.lba2 = ReadReg8(5);	// LBA High
	aATA.acmd =ReadReg8(7);		// Status

	aATA.header.flags = 0x18;

    aATA.header.major = MY_MAJOR;
	aATA.header.minor = MY_MINOR;
	aATA.header.msg_type = AOE_TYPE;

	aATA.header.dst_mac[0] = aATA.header.src_mac[0];
  	aATA.header.dst_mac[1] = aATA.header.src_mac[1];
	aATA.header.dst_mac[2] = aATA.header.src_mac[2];
	aATA.header.dst_mac[3] = aATA.header.src_mac[3];
	aATA.header.dst_mac[4] = aATA.header.src_mac[4];
	aATA.header.dst_mac[5] = aATA.header.src_mac[5];

	aATA.header.src_mac[0] = src_mac[0];
	aATA.header.src_mac[1] = src_mac[1];
	aATA.header.src_mac[2] = src_mac[2];
	aATA.header.src_mac[3] = src_mac[3];
	aATA.header.src_mac[4] = src_mac[4];
	aATA.header.src_mac[5] = src_mac[5];
				
	byte_cnt=sizeof(AtaIssue)-(SECTOR_SIZE*2)+(blocks<<SECTOR_SHIFT);

 	return sendto(s,(uint8*)&aATA,byte_cnt,(uint8 *)&tip,3000);

}

void init_ata_hardware(void)
{

	// Set DDRs to input

	IDE_DATA_LSB=0xFF;
  	IDE_DATA_MSB=0xFF;

 	// CS=1 RD=1 WR=1 RESET=1 DIR=1 ADDR=0
	P1 = IDE_CS_MASK | IDE_RD_MASK | IDE_WR_MASK | IDE_RESET_MASK | IDE_DIR_MASK;
	
}


uint8 init_ata(void)
{

	init_ata_hardware();

	ResetBus();

	return WaitBusy();

}


void ataoe(SOCKET s,uint8 *mac)
{
	uint16 xdata rlen =0;
 	uint16 xdata mac_destport;
	uint16 xdata len =0;
	uint8 xdata mac_destip[4];

	init_ata();

	  	while(1)
		{
			switch(getSn_SR(s))
			{
				case SOCK_CLOSED:
					close(s);                              // close the SOCKET
					socket(s,Sn_MR_MACRAW,3000,0);            // open the SOCKET with MACRAW mode
					break;
	
				case SOCK_MACRAW:
	
					if ( (rlen = getSn_RX_RSR(s) ) > 0)
					{
						/* receive data from a destination */
						len = recvfrom(s,(uint8 *)&aATA,rlen,mac_destip,&mac_destport);

						if(aATA.header.msg_type == AOE_TYPE)
						{
						    switch(aATA.header.cmd)
							{
								default: break;
 
								case AOE_CMD_ATA:

									ATAHandler(s,mac);
									break;
								case AOE_CMD_CONFIG:

									SendConfigAck(s,mac,(AtaConfig *)&aATA);
									break;
								case AOE_CMD_MAC_MASK:
									DumpATAMacMask((AtaMACMask *)&aATA);
									break;
								case AOE_CMD_RESERVE_RELEASE:
									DumpATAReserveRelease((AtaReserveRelease *)&aATA);
									break;
							}
						}
					
					}

					break;
				default:		
					break;
			}
	   }


}
	
