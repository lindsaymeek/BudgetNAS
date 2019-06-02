/*
*
@file		socket.h
@brief	define function of socket API 
*
*/

#ifndef	__SOCKET_H__
#define	__SOCKET_H__

#include "types.h"

uint8 socket(SOCKET s, uint8 protocol, uint16 port, uint8 flag); // Opens a socket(TCP or UDP or IP_RAW mode)
void close(SOCKET s); // Close socket
uint8 connect(SOCKET s, uint8 * addr, uint16 port); // Establish TCP connection (Active connection)
void disconnect(SOCKET s); // disconnect the connection
uint8 listen(SOCKET s);	// Establish TCP connection (Passive connection)
uint16 send(SOCKET s,	const uint8 * buf, uint16 len);
uint16 recv(SOCKET s,uint8 * buf,uint16 len);
uint16 sendto(SOCKET s, const uint8 * buf, uint16 len, uint8 * addr, uint16 port); // Send data (UDP/IP RAW)
uint16 recvfrom(SOCKET s, uint8 * buf, uint16 len, uint8 * addr, uint16  *port); // Receive data (UDP/IP RAW)

#endif
/* _SOCKET_H_ */
