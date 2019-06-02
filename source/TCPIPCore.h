/*
@file		TCPIPCore.h
*/

#ifndef	_TCPIPCore_H_
#define	_TCPIPCore_H_

#include "iinchip_conf.h"
#include "types.h"

static vuint8 xdata I_STATUS[MAX_SOCK_NUM];
static uint16 xdata SMASK[MAX_SOCK_NUM]; /**< Variable for Tx buffer MASK in each channel */
static uint16 xdata RMASK[MAX_SOCK_NUM]; /**< Variable for Rx buffer MASK in each channel */
static uint16 xdata SSIZE[MAX_SOCK_NUM]; /**< Max Tx buffer size by each channel */
static uint16 xdata RSIZE[MAX_SOCK_NUM]; /**< Max Rx buffer size by each channel */
static uint16 xdata SBUFBASEADDRESS[MAX_SOCK_NUM]; /**< Tx buffer base address by each channel */
static uint16 xdata RBUFBASEADDRESS[MAX_SOCK_NUM]; /**< Rx buffer base address by each channel */


#define MR 		COMMON_BASE

/*************************************************************/
/*					COMMON REGISTERS						  */	
/*************************************************************/

//Gateway IP Register address
#define GAR0		((volatile uint8 xdata *)(COMMON_BASE + 0x0001))

//Subnet mask Register address
#define SUBR0		((volatile uint8 xdata *)(COMMON_BASE + 0x0005))

//Source MAC Register address
#define SHAR0		((volatile uint8 xdata *)(COMMON_BASE + 0x0009))

//Source IP Register address
#define SIPR0		((volatile uint8 xdata *)(COMMON_BASE + 0x000F))

//Interrupt Register
#define IR		((volatile uint8 xdata *)(COMMON_BASE + 0x0015))

//Interrupt mask register
#define IMR		((volatile uint8 xdata *)(COMMON_BASE + 0x0016))

//Timeout register address( 1 is 100us )
#define RTR		((volatile uint8 xdata *)(COMMON_BASE + 0x0017))

//Retry count reigster
#define RCR		((volatile uint8 xdata *)(COMMON_BASE + 0x0019))

//Authentication type register address in PPPoE mode
#define PATR0		((volatile uint8 xdata *)(COMMON_BASE + 0x001C))
#define PPPALGO		((volatile uint8 xdata *)(COMMON_BASE + 0x001E))

//Chip version register address
#define VERSIONR	((volatile uint8 xdata *)(COMMON_BASE + 0x001F))

//PPP Link control protocol request timer
#define PTIMER 		((volatile uint8 xdata *)(COMMON_BASE + 0x0028))

//PPP LCP magic number register
#define PMAGIC 		((volatile uint8 xdata *)(COMMON_BASE + 0x0029))

//Unreachable IP register address in UDP mode
#define UIPR0		((volatile uint8 xdata *)(COMMON_BASE + 0x002A))

//Unreachable Port register address in UDP mode
#define UPORT0		((volatile uint8 xdata *)(COMMON_BASE + 0x002E))

//SOCKET Interrupt Register
#define IR2		((volatile uint8 xdata *)(COMMON_BASE + 0x0034))


/*************************************************************/
/*					SOCKET REGISTERS						  */	
/*************************************************************/

//Size of each channel register map
#define CH_BASE		(COMMON_BASE + 0x4000)

//Size of each channel register map
#define CH_SIZE		0x0100

//Socket Mode register
#define Sn_MR(ch)		(CH_BASE + ch * CH_SIZE + 0x0000)

//Channel Sn_CR register
#define Sn_CR(ch)		(CH_BASE + ch * CH_SIZE + 0x0001)

//Channel interrupt register
#define Sn_IR(ch)		(CH_BASE + ch * CH_SIZE + 0x0002)

//Channel status register
#define Sn_SR(ch)		(CH_BASE + ch * CH_SIZE + 0x0003)

//Source port register
#define Sn_PORT0(ch)		(CH_BASE + ch * CH_SIZE + 0x0004)

//Peer MAC register address
#define Sn_DHAR0(ch)		(CH_BASE + ch * CH_SIZE + 0x0006)

//Peer IP register address
#define Sn_DIPR0(ch)		(CH_BASE + ch * CH_SIZE + 0x000C)

//Peer port register address
#define Sn_DPORT0(ch)		(CH_BASE + ch * CH_SIZE + 0x0010)

//Maximum Segment Size(Sn_MSSR0) register address
#define Sn_MSSR0(ch)		(CH_BASE + ch * CH_SIZE + 0x0012)

//Protocol of IP Header field register in IP raw mode
#define Sn_PROTO(ch)		(CH_BASE + ch * CH_SIZE + 0x0014)

//IP Type of Service(TOS) Register 
#define Sn_TOS(ch)		(CH_BASE + ch * CH_SIZE + 0x0015)

//IP Time to live(TTL) Register 
#define Sn_TTL(ch)		(CH_BASE + ch * CH_SIZE + 0x0016)

//Receive memory size reigster
#define Sn_RXMEM_SIZE(ch)	(CH_BASE + ch * CH_SIZE + 0x001E)

//Transmit memory size reigster
#define Sn_TXMEM_SIZE(ch)	(CH_BASE + ch * CH_SIZE + 0x001F)

//Transmit free memory size register
#define Sn_TX_FSR0(ch)		(CH_BASE + ch * CH_SIZE + 0x0020)

//Transmit memory read pointer register address
#define Sn_TX_RD0(ch)		(CH_BASE + ch * CH_SIZE + 0x0022)

//Transmit memory write pointer register address
#define Sn_TX_WR0(ch)		(CH_BASE + ch * CH_SIZE + 0x0024)

//Received data size register
#define Sn_RX_RSR0(ch)		(CH_BASE + ch * CH_SIZE + 0x0026)

//Read point of Receive memory
#define Sn_RX_RD0(ch)		(CH_BASE + ch * CH_SIZE + 0x0028)

//Write point of Receive memory
#define Sn_RX_WR0(ch)		(CH_BASE + ch * CH_SIZE + 0x002A)

//Socket interrupt mask register
#define Sn_IMR(ch)		(CH_BASE + ch * CH_SIZE + 0x002C)

//Frag field value in IP header register
#define Sn_FRAG0(ch)		(CH_BASE + ch * CH_SIZE + 0x002D)

/*************************************************************/
/*					BIT of REGISTERS	 						  */	
/*************************************************************/
//MODE register values
#define MR_RST			0x80 		/**< reset */
#define MR_PB			0x10 		/**< ping block */
#define MR_PPPOE		0x08		/**< enable pppoe */

//IR register values
#define IR_CONFLICT		0x80 		/**< check ip confict */
#define IR_UNREACH		0x40 		/**< get the destination unreachable message in UDP sending */
#define IR_PPPoE		0x20 		/**< get the PPPoE close message */
#define IR_SOCK(ch)		(0x01 << ch) /**< check socket interrupt */

//Sn_MR values
#define Sn_MR_MULTI		0x80		/**< support multicating */
#define Sn_MR_ND		0x20		/**< No Delayed Ack(TCP) flag */
#define Sn_MR_PPPOE		0x05		/**< PPPoE */
#define Sn_MR_MACRAW		0x04		/**< MAC LAYER RAW SOCK */
#define Sn_MR_IPRAW		0x03		/**< IP LAYER RAW SOCK */
#define Sn_MR_UDP		0x02		/**< UDP */
#define Sn_MR_TCP		0x01		/**< TCP */
#define Sn_MR_CLOSE		0x00		/**< unused socket */

//Sn_CR values
#define Sn_CR_OPEN		0x01		/**< initialize or open socket */
#define Sn_CR_LISTEN		0x02		/**< wait connection request in tcp mode(Server mode) */
#define Sn_CR_CONNECT		0x04		/**< send connection request in tcp mode(Client mode) */
#define Sn_CR_DISCON		0x08		/**< send closing reqeuset in tcp mode */
#define Sn_CR_CLOSE		0x10		/**< close socket */
#define Sn_CR_SEND		0x20		/**< update txbuf pointer, send data */
#define Sn_CR_SEND_MAC		0x21		/**< send data with MAC address, so without ARP process */
#define Sn_CR_SEND_KEEP 	0x22		/**<  send keep alive message */
#define Sn_CR_RECV		0x40		/**< update rxbuf pointer, recv data */

#define Sn_CR_PCON		0x23		/**< ADSL connection begins by transmitting PPPoE discovery packet */		 
#define Sn_CR_PDISCON		0x24		/**< close ADSL connection */		 
#define Sn_CR_PCR		0x25		/**< In each phase, it transmits REQ message */
#define Sn_CR_PCN		0x26		/**< In each phase, it transmits NAK message */	
#define Sn_CR_PCJ		0x27		/**< In each phase, it transmits REJECT message */	

//Sn_IR values
#define Sn_IR_PRECV		0x80		/**< PPP receive interrupt */
#define Sn_IR_PFAIL		0x40		/**< PPP fail interrupt */
#define Sn_IR_PNEXT		0x20		/**< PPP next phase interrupt */
#define Sn_IR_SEND_OK		0x10		/**< complete sending */
#define Sn_IR_TIMEOUT		0x08		/**< assert timeout */
#define Sn_IR_RECV		0x04		/**< receiving data */
#define Sn_IR_DISCON		0x02		/**< closed socket */
#define Sn_IR_CON		0x01		/**< established connection */

//Sn_SR values
#define SOCK_CLOSED		0x00		/**< closed */
#define SOCK_INIT 		0x13		/**< init state */
#define SOCK_LISTEN		0x14		/**< listen state */
#define SOCK_SYNSENT		0x15		/**< connection state */
#define SOCK_SYNRECV	   	0x16		/**< connection state */
#define SOCK_ESTABLISHED	0x17		/**< success to connect */
#define SOCK_FIN_WAIT		0x18		/**< closing state */
#define SOCK_CLOSING	   	0x1A		/**< closing state */
#define SOCK_TIME_WAIT		0x1B		/**< closing state */
#define SOCK_CLOSE_WAIT		0x1C		/**< closing state */
#define SOCK_LAST_ACK		0x1D		/**< closing state */
#define SOCK_UDP		0x22		/**< udp socket */
#define SOCK_IPRAW		0x32		/**< ip raw mode socket */
#define SOCK_MACRAW		0x42		/**< mac raw mode socket */
#define SOCK_PPPOE		0x5F		/**< pppoe socket */


/*************************************************************/
/*					W7100 ACCESS FUNCTIONS					  */	
/*************************************************************/

uint8 IINCHIP_READ(uint16 addr); 
void IINCHIP_WRITE(uint16 addr,uint8 wizdata);
void set_MEMsize(uint8* tx_size, uint8* rx_size); // setting TX/RX buffer size
uint8 getSn_IR(uint8 s);
void setSn_IR(uint8 s, uint8 val);
uint8 getSn_SR(SOCKET s); // get socket status
uint16 getSn_TX_FSR(SOCKET s); // get socket TX free buffer size
uint16 getSn_RX_RSR(SOCKET s); // get socket RX recv buffer size
void IINCHIP_WRITE_BUF(SOCKET s, vuint8 * src, vuint8 * dst, uint16 len);
void IINCHIP_READ_BUF(SOCKET s, vuint8 * src, uint8 * dst, uint16 len);

#endif
