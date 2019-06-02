
#ifndef ATAOE_H
#define ATAOE_H

/*

Circuit Cellar WizNet Contest 2010
Project 3035
Budget Network Attached Storage Unit
ATAOE driver

 */

#include "types.h"

#define ETHER_ADDR_LEN 6

#define SECTOR_SIZE			512
#define SECTOR_SHIFT		9

#define MAX_SHELF			0xfffe
#define MAX_SLOT			0xfe

typedef struct _AtaHdr
{
 	uint8  dst_mac[ETHER_ADDR_LEN ];	// ff.ff.ff.ff.ff.ff
	uint8  src_mac[ETHER_ADDR_LEN ];	
	uint16  msg_type;   	// ATAOE (0x88a2)

	uint8	flags;
	uint8	error;
	uint16	major;
	uint8	minor;
	uint8	cmd;
	uint32	tag;
} AtaHdr;

/// Config command packet

#define ATAOE_CCMD_READ 0
#define ATAOE_CCMD_TEST 1
#define ATAOE_CCMD_TEST_PREFIX 2
#define ATAOE_CCMD_SET 2
#define ATAOE_CCMD_FORCE 3

typedef struct _AtaConfig
{
	AtaHdr  header;
	uint16	buffer_count;
	uint16	firmware_version;
	uint8   sectors;
	uint8	aoe_ccmd;
	uint16	length;

	uint8	str[1];

} AtaConfig;

/// Main ATA command issue

typedef struct _AtaIssue
 
{
	AtaHdr  header;

	uint8	aflag;
	uint8	err;
	uint8	sectors;
	uint8	acmd;
	uint8	lba0,lba1,lba2,lba3,lba4,lba5;
	uint8	resvd[2];

	uint8	dat[SECTOR_SIZE*2];
} AtaIssue;
   
/// MAC mask packet

#define ATAOE_DCMD_ADD 1
#define ATAOE_DCMD_REMOVE 2

typedef struct _AtaDirective
{
	uint8	resvd;
	uint8	dcmd;
	uint8	mac_addr[ETHER_ADDR_LEN];
} AtaDirective;

typedef struct _AtaMACMask
{
	AtaHdr  header;

	uint8	resvd;
	uint8	mcmd;
	uint8	merror;
	uint8	directive_count;

	AtaDirective	directive[1];

} AtaMACMask;

/// Reserve / release a series of mac addresses

#define ATAOE_RCMD_READ 0
#define ATAOE_RCMD_SET 1
#define ATAOE_RCMD_FORCE 2

typedef struct _AtaReserveRelease
{
	AtaHdr	header;
	uint8	rcmd;
	uint8	nmacs;
	uint8	mac_addr[ETHER_ADDR_LEN];
} AtaReserveRelease;

/// Hardware ATA interface

#define IDE_DATA_LSB P2
#define IDE_DATA_MSB P3

#define IDE_CS_MASK (1<<3)
#define IDE_RD_MASK (1<<6)
#define IDE_WR_MASK (1<<5)
#define IDE_DIR_MASK (1<<7)
#define IDE_RESET_MASK (1<<4)

void init_ata_hardware(void);

void ataoe(SOCKET s,uint8 *mac);

#endif
