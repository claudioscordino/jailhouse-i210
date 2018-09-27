#ifndef I210_H
#define I210_H

// ============================ Hardware =======================================
/* Taken using "sudo lspci -nn" */
#define ETH_VENDORID		0x8086
#define ETH_DEVICEID		0x1533

#define NUM_QUEUES		4 // i210 has 4 rx and 4 tx queues

// TODO: check
#define ETH_IRQ_VECTOR		42

// ============================ Data structures ================================
#define RX_BUFFER_SIZE          2048	// This must be written into RCTL.BSIZE
#define RX_DESCR_NB		8	// Number of descriptors in the rx queue
					// It affects RDT
#define TX_DESCR_NB		8
#define DEVS_MAX_NB		3	// Maximum number of handled devices

// ============================ Registers ======================================
#define E1000_CTRL	0x00000	// Device Control - RW
	#define E1000_CTRL_FD		(1 << 0)	// Full-Duplex
	#define E1000_CTRL_SLU		(1 << 6)	// Set link up (Force Link)
	#define E1000_CTRL_SPEED_MSK	(3 << 8)	// Link speed mask
	#define E1000_CTRL_SPEED_100	(1 << 8)	// 100 Mb/s speed
	#define E1000_CTRL_FRCSPD	(1 << 11)	// Force Speed
	#define E1000_CTRL_FRCDPLX	(1 << 12)	// Force Duplex
	#define E1000_CTRL_RST		(1 << 26)	// Reset
#define E1000_STATUS	0x00008	// Device Status - RO
	#define E1000_STATUS_FD		(1 << 0)	// Full Duplex (TODO: check if needed)
	#define E1000_STATUS_LU		(1 << 1)	// Link up
	#define E1000_STATUS_SPEED_MSK	(3 << 6)	// Link speed mask
	#define E1000_STATUS_SPEED_10	(0 << 6)	// Speed 10  Mb/s
	#define E1000_STATUS_SPEED_100	(1 << 6)	// Speed 100 Mb/s
	#define E1000_STATUS_RST_DONE	(1 << 21)	// Reset done
#define E1000_CTRL_EXT	0x00018	// Extended Device Control - RW
	#define E1000_CTRL_EXT_BYPS	(1 << 15)	// Speed Select Bypass
	#define E1000_CTRL_EXT_SD_LP	(1 << 18)	// SerDes Low Power Enable
	#define E1000_CTRL_EXT_PHY_LP	(1 << 20)	// PHY Power Down
#define E1000_RCTL	0x00100  // RX Control - RW
	#define E1000_RCTL_RXEN		(1 << 1)
	#define E1000_RCTL_BAM		(1 << 15)	// Accept broadcast packets
	#define E1000_RCTL_BSIZE	0x30000		// Buffer size
	#define E1000_RCTL_BSIZE_2048	0x00000 	// Buffer size = 2048
	#define E1000_RCTL_SECRC	(1 << 26)	// Strip Ethernet CRC From Incoming Packet
#define E1000_TCTL	0x00400  // TX Control - RW
	#define E1000_TCTL_EN		(1 << 1)
	#define E1000_TCTL_PSP		(1 << 3)	// Pad Short Packets
	#define E1000_TCTL_CT_IEEE	(0xf << 4)	// IEEE Collision threshold
#define E1000_TIPG	0x00410  // TX Control - Inter-Packet Gap
	#define E1000_TIPG_IPGT_DEF	(10 << 0)
	#define E1000_TIPG_IPGR1_DEF	(10 << 10)
	#define E1000_TIPG_IPGR2_DEF	(10 << 20)
#define E1000_PHPM	0x00E14  // PHY Power Management
	#define E1000_PHPM_NO_1000	(1 << 6)	// Disable 1000 Mb/s
#define E1000_PCS_LCTL	0x4208	// PCS Link Control
	#define E1000_PCS_LCTL_FSV_MSK	(3 << 1)	// Forced Speed Value mask
	#define E1000_PCS_LCTL_FSV_10	(0 << 1)	// Forced Speed 10 Mb/s
	#define E1000_PCS_LCTL_FSV_100	(1 << 1)	// Forced Speed 100 Mb/s
	#define E1000_PCS_LCTL_FDV	(1 << 3)	// Forced Duplex Value
	#define E1000_PCS_LCTL_FSD	(1 << 4)	// Force Speed and Duplex
#define E1000_PCS_LSTS	0x420C	// PCS Link Status
	#define E1000_PCS_LSTS_LINK	(1 << 0)	// Link OK
	#define E1000_PCS_LSTS_SPEED_MSK (3 << 1)	// Speed mask
	#define E1000_PCS_LSTS_SPEED_10	 (0 << 1)	// 10 Mb/s
	#define E1000_PCS_LSTS_SPEED_100 (1 << 1)	// 100 Mb/s
	#define E1000_PCS_LSTS_DUPLEX	 (1 << 3)

#define E1000_RAL	0x05400
#define E1000_RAH	0x05404
	#define E1000_RAH_AV		(1 << 31)

#define E1000_RDBAL(_n)		(0x0C000 + ((_n) * 0x040)) // Descriptor lower bits
#define E1000_RDBAH(_n)		(0x0C004 + ((_n) * 0x040)) // Descriptor higher bits
#define E1000_RDLEN(_n)		(0x0C008 + ((_n) * 0x040))
#define E1000_RDH(_n)		(0x0C010 + ((_n) * 0x040))
#define E1000_RDT(_n)		(0x0C018 + ((_n) * 0x040))
#define E1000_RXDCTL(_n)	(0x0C028 + ((_n) * 0x040))
	#define E1000_RXDCTL_ENABLE	(1<<25)

#define E1000_TDBAL(_n)		(0x0E000 + ((_n) * 0x040)) // Descriptor lower bits
#define E1000_TDBAH(_n)		(0x0E004 + ((_n) * 0x040)) // Descriptor higher bits
#define E1000_TDLEN(_n)		(0x0E008 + ((_n) * 0x040))
#define E1000_TDH(_n)		(0x0E010 + ((_n) * 0x040))
#define E1000_TDT(_n)		(0x0E018 + ((_n) * 0x040))
#define E1000_TXDCTL(_n)	(0x0E028 + ((_n) * 0x040))
	#define E1000_TXDCTL_ENABLE	(1<<25)

#define E1000_MDIC		0x0020
	#define E1000_MDIC_REGADD_SHFT		16
	#define E1000_MDIC_OP_WRITE		(0x1 << 26)
	#define E1000_MDIC_OP_READ		(0x2 << 26)
	#define E1000_MDIC_READY		(0x1 << 28)
	#define E1000_MDIC_CCR			0
		#define E1000_MDIC_CCR_POWER_DOWN	(1 << 11)
		#define E1000_MDIC_CCR_SPEED_MSK	((1 << 6)|(1 << 13))
		#define E1000_MDIC_CCR_SPEED_10		(0 << 13)
		#define E1000_MDIC_CCR_SPEED_100	(1 << 13)

struct rxd {
	u64	addr;
	u16	len;
	u16	crc;
	u8	dd:1,
		eop:1,
		ixsm:1,
		vp:1,
		udpcs:1,
		tcpcs:1,
		ipcs:1,
		pif:1;
	u8	errors;
	u16	vlan_tag;
} __attribute__((packed));

struct txd {
	u64	addr;
	u16	len;
	u8	cso;
	u8	eop:1,
		ifcs:1,
		ic:1,
		rs:1,
		rps:1,
		dext:1,
		vle:1,
		ide:1;
	u8	dd:1,
		ec:1,
		lc:1,
		tu:1,
		rsv:4;
	u8	css;
	u16	special;
} __attribute__((packed));

struct eth_device {
	void	*bar_addr;
	u8	mac[6];
	u16	speed;
};

//	Structure of a IEEE 802.3 Ethernet II frame:
//
//	     7           1        6     6         2        46-1500    4
//	------------------------------------------------------------------
//	| Preamble |   Frame   | MAC | MAC |    Type     | Body    | CRC |
//	|          | delimiter | Src | Dst | (Ethertype) |         |     |
//	------------------------------------------------------------------
//
//	Ethertypes:
//	0x88A4		EtherCAT protocol

// TODO: switch endianness (e.g. using htons() )
#define ETHERTYPE_ETHERCAT	0xA488 // EtherCAT is 0x88A4

struct eth_packet {
	u8	dst[6];
	u8	src[6];
	u16	type;
	u8	data[4];
} __attribute__((packed));



#define print(...)          printk("Ethernet: " __VA_ARGS__)


#endif // I210_H
