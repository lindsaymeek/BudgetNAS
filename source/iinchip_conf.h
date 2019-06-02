#ifndef	_IINCHIP_CONF_H_
#define	_IINCHIP_CONF_H_

#define	MAX_SOCK_NUM		8	/**< Maxmium number of socket  */

/**
* __DEF_IINCHIP_xxx__ : define option for iinchip driver
*/
//#define __DEF_IINCHIP_DBG__ /* involve debug code in driver (socket.c) */
#define __DEF_IINCHIP_INT__ /* involve interrupt service routine (socket.c) */

/**
* __DEF_IINCHIP_MAP_xxx__ : define memory map for iinchip
*/
#define COMMON_BASE 0x0000
#define __DEF_IINCHIP_MAP_TXBUF__ (COMMON_BASE + 0x8000) /* Internal Tx buffer address of the iinchip */
#define __DEF_IINCHIP_MAP_RXBUF__ (COMMON_BASE + 0xc000) /* Internal Rx buffer address of the iinchip */

#ifdef __DEF_IINCHIP_INT__
#define IINCHIP_ISR_DISABLE()	(EINT5 = 0)
#define IINCHIP_ISR_ENABLE()	(EINT5 = 1)
#else
#define IINCHIP_ISR_DISABLE()	
#define IINCHIP_ISR_ENABLE()	
#endif

#endif
