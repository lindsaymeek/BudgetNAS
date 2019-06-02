#include <stdio.h>
#include "arp.h"
#include "TCPIPCore.h"
#include "socket.h"
#include "serial.h"
#include "sockutil.h"
#include "delay.h" 


uint16 xdata mac_remain_time;
uint16 xdata mac_arp_received = 0;
uint8* xdata pMACRAW;

ARPMSG xdata pARPMSG;
ARPMSG xdata * aARPMSG;
	
void arp(SOCKET s, uint16 aPort, uint8 * SrcIp, uint8 *SrcMac, uint8 *TgtIp, uint8 count)
{
	uint16 xdata len = 0;
	uint16 xdata   i= 0;
	uint16 xdata rlen =0;
	uint16 xdata cnt =0;

	for ( i=0 ; i<count +1; i++ ){	
				
		switch(getSn_SR(s))
			{
				case SOCK_CLOSED:
					close(s);                                                     // close the SOCKET
					socket(s,Sn_MR_MACRAW,aPort,0);            // open the SOCKET with MACRAW mode
				break;
	
				case SOCK_MACRAW:
	
					 wait_1ms(1000); 
						
					    mac_arp_received	= 0;
						arp_request(s, aPort, SrcIp, SrcMac, TgtIp);
						while(1){
							if ( (rlen = getSn_RX_RSR(s) ) > 0){
									arp_reply(s, rlen);	
								if (mac_arp_received)  break;
							}
							if ( (cnt > 100) ) {
								printf("Request Time out.\n"); 									
								break;
							}else { 
								cnt++; 	
								wait_1ms(20);
							}
					     }
	
					break;
				default:		
					break;
			}
		}
}

void arp_request(SOCKET s, uint16 port, uint8 *SrcIp, uint8 *SrcMac, uint8 *TgtIp)
{
		uint32 xdata tip = 0xFFFFFFFF;
		uint16 xdata i =0;
	
		for( i=0; i<6 ; i++) {
			pARPMSG.dst_mac[i] = 0xFF;//BROADCAST_MAC[i];
			pARPMSG.src_mac[i] = SrcMac[i];//BROADCAST_MAC[i];
			pARPMSG.sender_mac[i] = SrcMac[i];//BROADCAST_MAC[i];
			pARPMSG.tgt_mac[i] = 0;	
		}
	
		pARPMSG.msg_type = htons(ARP_TYPE);
		pARPMSG.hw_type   = htons(ETHER_TYPE);
		pARPMSG.pro_type  = htons(PRO_TYPE);        // IP	(0x0800)
		pARPMSG.hw_size    =  HW_SIZE;		        // 6
		pARPMSG.pro_size   = PRO_SIZE;		        // 4
		pARPMSG.opcode      =  htons(ARP_REQUEST);		// request (0x0001), reply(0x0002)
		for( i=0; i<4 ; i++) {
			pARPMSG.sender_ip[i] = SrcIp[i];//BROADCAST_MAC[i];
			pARPMSG.tgt_ip[i] = TgtIp[i];
		}

	if( sendto(s,(uint8*)&pARPMSG,sizeof(pARPMSG),(uint8 *)&tip,port) ==0){
			printf( "\r\n Fail to send ping-reply packet  r\n") ;				
	}else{
				if(pARPMSG.opcode==ARP_REQUEST){
					printf("\r\nWho has ");
					printf( "%d.%d.%d.%d ? ", (int)(pARPMSG.tgt_ip[0]), (int)(pARPMSG.tgt_ip[1]), (int)(pARPMSG.tgt_ip[2]), (int)(pARPMSG.tgt_ip[3])) ;
					printf( "  Tell %d.%d.%d.%d ? \r\n", (int)(pARPMSG.sender_ip[0]) , (int)(pARPMSG.sender_ip[1]), (int)(pARPMSG.sender_ip[2]), (int)(pARPMSG.sender_ip[3]));							
				}else{
					printf("opcode has wrong value. check opcode !\r\n");
				}
	}

}


void arp_reply(SOCKET s, uint16 rlen)
{
	uint16 xdata mac_destport;
	uint8 xdata * data_buf = 0x007000;	
	uint16 xdata len =0;
	uint8 xdata mac_destip[4];
	
		/* receive data from a destination */
	len = recvfrom(s,(uint8 *)data_buf,rlen,mac_destip,&mac_destport);
	if( data_buf[12]==ARP_TYPE_HI && data_buf[13]==ARP_TYPE_LO ){
			aARPMSG = 0x007000;	
				  if( ((aARPMSG->opcode) == ARP_REPLY) &&(mac_arp_received == 0)  ) {
							mac_arp_received = 1;
							printf( "%d.%d.%d.%d ", 
								(int)(aARPMSG->sender_ip[0]), (int)(aARPMSG->sender_ip[1]), 
								(int)(aARPMSG->sender_ip[2]), (int)(aARPMSG->sender_ip[3])) ;
							printf(" is at  ");
							printf( "%.2X.%.2X.%.2X.%.2X.%.2X.%.2X \r\b", 
								(int)(aARPMSG->sender_mac[0]), (int)(aARPMSG->sender_mac[1]), (int)(aARPMSG->sender_mac[2]), 
								(int)(aARPMSG->sender_mac[3]),(int)(aARPMSG->sender_mac[4]), (int)(aARPMSG->sender_mac[5])) ;
                  }else if( (aARPMSG->opcode) == ARP_REQUEST && (mac_arp_received == 1)  ) {
		 				    //mac_arp_received = 1;
							printf( "Who has %d.%d.%d.%d ? ", 
								(int)(aARPMSG->sender_ip[0]), (int)(aARPMSG->sender_ip[1]), 
								(int)(aARPMSG->sender_ip[2]), (int)(aARPMSG->sender_ip[3])) ;
							printf(" Tell  ");
							printf( "%.2X.%.2X.%.2X.%.2X%.2X%.2X \r\b", 
								(int)(aARPMSG->sender_mac[0]), (int)(aARPMSG->sender_mac[1]), (int)(aARPMSG->sender_mac[2]), 
								(int)(aARPMSG->sender_mac[3]),(int)(aARPMSG->sender_mac[4]), (int)(aARPMSG->sender_mac[5])) ;

				  }else{
					        //printf(" This msg is not ARP reply : opcode is not 0x02 \n");	
				  }
	}else{
			 //printf(" This msg is not ARP TYPE : ARP TYPE is 0x0806 \n");
	}
  
}

