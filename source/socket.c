/*
*
@file		socket.c
@brief	setting chip register for socket
*
*/

#include "types.h"
#include "W7100.h"
#include "TCPIPCore.h"
#include "socket.h"
#include "serial.h"
#include "iinchip_conf.h"

uint16 xdata local_port;

/**
@brief	This Socket function initialize the channel in perticular mode, and set the port and wait for W7100 done it.
@return 	1 for sucess else 0.
*/  
uint8 socket(
	SOCKET s, 		/**< for socket number */
	uint8 protocol, 	/**< for socket protocol */
	uint16 port, 		/**< the source port for the socket */
	uint8 flag		/**< the option for the socket */
	)
{
	uint8 xdata ret;

	if ((protocol == Sn_MR_TCP) || (protocol == Sn_MR_UDP) || (protocol == Sn_MR_IPRAW) || (protocol == Sn_MR_MACRAW) || (protocol == Sn_MR_PPPOE))
	{
		close(s);
		IINCHIP_WRITE(Sn_MR(s),protocol | flag);
		if (port != 0) {
			IINCHIP_WRITE(Sn_PORT0(s),(uint8)((port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(port & 0x00ff));
		} else {
			local_port++; // if don't set the source port, set local_port number.
			IINCHIP_WRITE(Sn_PORT0(s),(uint8)((local_port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8)(local_port & 0x00ff));
		}
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
		while ( IINCHIP_READ(Sn_CR(s)) ) ; // wait for completion CR
		ret = 1;
	}
	else ret = 0;

	return ret;

}


/**
@brief	This function close the socket and parameter is "s" which represent the socket number
*/ 
void close(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	while ( IINCHIP_READ(Sn_CR(s)) ) ; // wait for completion CR
	setSn_IR(s, 0xff); // clear socket interrupt
}


/**
@brief	This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
@return	1 for success else 0.
*/ 
uint8 listen(SOCKET s)	/**< s : socket number */
{
	uint8 xdata ret;
	
	if (getSn_SR(s) == SOCK_INIT)
	{
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_LISTEN);
		while( IINCHIP_READ(Sn_CR(s)) ) ;
		ret = 1;
	}
	else ret = 0;

	return ret;
}


/**
@brief	This function established  the connection for the channel in Active (client) mode. 
		This function waits for the untill the connection is established.
		
@return	1 for success else 0.
*/ 
uint8 connect(SOCKET s, uint8 * addr, uint16 port)
{
	uint8 xdata ret;

	if 
		(
			((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		 	((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		 	(port == 0x00) 
		) 
 	{
 		ret = 0;
	}
	else
	{
		ret = 1;
		// set destination IP
		IINCHIP_WRITE(Sn_DIPR0(s),addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(port & 0x00ff));
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_CONNECT);
		while ( IINCHIP_READ(Sn_CR(s)) ) ; // wait for completion CR
	}

	return ret;
}


/**
@brief	This function used for disconnect the socket and parameter is "s" which represent the socket number
@return	1 for success else 0.
*/ 
void disconnect(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_DISCON);
	while ( IINCHIP_READ(Sn_CR(s)) ) ; // wait for completion CR
}


/**
@brief	This function used to send the data in TCP mode
@return	1 for success else 0.
*/ 
uint16 send(
	SOCKET s, 		/**< the socket index */
	const uint8 * buf, 	/**< a pointer to data */
	uint16 len		/**< the data size to be send */
	)
{
	uint8 xdata status = 0;
	uint16 xdata ptr = 0;
	uint16 xdata freesize = 0;
	uint16 xdata ret = 0;
	
	if (len > SSIZE[s]) ret = SSIZE[s]; // check size not to exceed MAX size.
	else ret = len;

	// if freebuf is available, start.
	do 
	{
		freesize = getSn_TX_FSR(s);
		status = getSn_SR(s);
		if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
		{
			ret = 0; 
			break;
		}
	} while (freesize < ret);

	// read TX write pointer
	ptr = IINCHIP_READ(Sn_TX_WR0(s));
	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR0(s) + 1);

      	// copy data
	IINCHIP_WRITE_BUF(s, buf, (uint8*)ptr, len);

	// Update TX write pointer
	ptr += len;
	IINCHIP_WRITE(Sn_TX_WR0(s),(uint8)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8)(ptr & 0x00ff));

	// Excute SEND command
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
	while( IINCHIP_READ(Sn_CR(s)) ) ;

	// wait for completion
	while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
	{
		if ( IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED )
		{
			close(s);
			return 0;
		}
  	}
	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);	// clear send_ok interrupt

	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in TCP mode.
		It continues to wait for data as much as the application wants to receive.
		
@return	received data size for success else -1.
*/ 
uint16 recv(
	SOCKET s, 	/**< socket index */
	uint8 * buf, 	/**< a pointer to copy the data to be received */
	uint16 len	/**< the data size to be read */
	)
{
	uint16 xdata ptr = 0;
	uint16 xdata ret = 0;
	
	if ( len > 0 )
	{
		// read RX read pointer
		ptr = IINCHIP_READ(Sn_RX_RD0(s));
		ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);

		// copy data
		IINCHIP_READ_BUF(s,(uint8*)ptr,buf, len);

		// Update RX read pointer
		ptr += len;
		IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));

		// Excute RECV command
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		while( IINCHIP_READ(Sn_CR(s)) ) ;

		ret = len;
	}

	return ret;
}

/**
@brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
		Unlike TCP transmission, The peer's destination address and the port is needed.
		
@return	This function return send data size for success else -1.
*/ 
uint16 sendto(
	SOCKET s, 		/**< socket index */
	const uint8 * buf, 	/**< a pointer to the data */
	uint16 len, 		/**< the data size to send */
	uint8 * addr, 		/**< the peer's Destination IP address */
	uint16 port		/**< the peer's destination port number */
	)
{
	uint16 xdata ret=0;
	uint16 xdata ptr=0;
		
	if (len > SSIZE[s]) ret = SSIZE[s]; // check size not to exceed MAX size.
	else ret = len;

	if
		(
		 	((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		 	((port == 0x00)) ||(ret == 0)
		) 
 	{
		ret = 0;
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s),addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),(uint8)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8)(port & 0x00ff));

 		// read TX write pointer
		ptr = IINCHIP_READ(Sn_TX_WR0(s));
		ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR0(s) + 1);

	      	// copy data
		IINCHIP_WRITE_BUF(s, buf, (uint8*)ptr, len);

		// Update TX write pointer
		ptr += len;
		IINCHIP_WRITE(Sn_TX_WR0(s),(uint8)((ptr & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8)(ptr & 0x00ff));

		// Excute SEND command
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		while( IINCHIP_READ(Sn_CR(s)) );

		// wait for completion
		while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
		{
			if (getSn_IR(s) & Sn_IR_TIMEOUT)
			{
				setSn_IR(s,Sn_IR_TIMEOUT);		// clear TIMEOUT Interrupt
				IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);	// clear send_ok interrupt
				return 0;
			}
		}

		IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);	// clear send_ok interrupt
	}

	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in other then
	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
	
@return	This function return received data size for success else -1.
*/ 
uint16 recvfrom(
	SOCKET s, 	/**< the socket number */
	uint8 * buf, 	/**< a pointer to copy the data to be received */
	uint16 len, 	/**< the data size to read */
	uint8 * addr, 	/**< a pointer to store the peer's IP address */
	uint16 *port	/**< a pointer to store the peer's port number. */
	)
{
	uint8 xdata head[8];
	uint16 xdata data_len=0;
	uint16 xdata ptr=0;
	
	if ( len > 0 )
	{
		ptr = IINCHIP_READ(Sn_RX_RD0(s));
	      	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);

		switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
	      	{
	      	case Sn_MR_UDP :
				
				IINCHIP_READ_BUF(s,(uint8*)ptr, head, 0x08);
	      			ptr += 8;
	      			// read peer's IP address, port number.
	      			addr[0] = head[0];
	      			addr[1] = head[1];
	      			addr[2] = head[2];
	      			addr[3] = head[3];
	      			*port = head[4];
	      			*port = (*port << 8) + head[5];
	      			data_len = head[6];
	      			data_len = (data_len << 8) + head[7];

	   			IINCHIP_READ_BUF(s,(uint8*)ptr, buf, data_len); // data copy.
	   			ptr += data_len;

				IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
   				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));
      			
	      			break;
	      
	      	case Sn_MR_IPRAW :
	      			IINCHIP_READ_BUF(s, (uint8 *)ptr, head, 0x06);
      				ptr += 6;
	      				      
	      			addr[0] = head[0];
	      			addr[1] = head[1];
	      			addr[2] = head[2];
	      			addr[3] = head[3];
	      			data_len = head[4];
	      			data_len = (data_len << 8) + head[5];
	      	
	   			IINCHIP_READ_BUF(s, (uint8 *)ptr, buf, data_len); // data copy.
   				ptr += data_len;
   
	   			IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
   				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));

	      			break;
					
	      	case Sn_MR_MACRAW :
	      			IINCHIP_READ_BUF(s,(uint8*)ptr,head,0x02);
				ptr+=2;

	      			data_len = head[0];
	      			data_len = (data_len<<8) + head[1] - 2;
	   
	      			IINCHIP_READ_BUF(s,(uint8*) ptr,buf,data_len);
      				ptr += data_len;
	      			IINCHIP_WRITE(Sn_RX_RD0(s),(uint8)((ptr & 0xff00) >> 8));
      				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8)(ptr & 0x00ff));
      			      			
	   			break;
	   
	      	default :
	      			break;
	      	}

		IINCHIP_WRITE(Sn_CR(s),Sn_CR_RECV);
		while ( IINCHIP_READ(Sn_CR(s)) ) ; // wait for completion CR
	}

 	return data_len;
}

