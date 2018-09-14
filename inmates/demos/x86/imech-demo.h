#ifndef INMATE_ETH_H
#define INMATE_ETH_H

/* ============================ Hardware ==================================== */
/* Taken using "sudo lspci -nn" */
#define ETH_VENDORID		0x8086
#define ETH_DEVICEID		0x1533

/* TODO: check */
#define ETH_IRQ_VECTOR		42
#define NUM_QUEUES	4 // i210 has 4 rx and 4 tx queues

/* ============================ Data structures ============================= */
#define RX_BUFFER_SIZE          2048 // This must be written into RCTL.BSIZE

/* ============================ Registers =================================== */
#define E1000_CTRL	0x00000	/* Device Control - RW */
	#define E1000_CTRL_SLU		0x00000040	/* Set link up (Force Link) */
	#define E1000_CTRL_SPEED	0x00000300	/* Link speed */
	#define E1000_CTRL_SPEED_100	0x00000100	/* 100 Mb/s speed */
	#define E1000_CTRL_FRCSPD	0x00000800	/* Force Speed */
#define E1000_STATUS	0x00008	/* Device Status - RO */
	#define E1000_STATUS_SPEED	0x000000C0	/* Link speed */
	#define E1000_STATUS_SPEED_10	0x00000000      /* Speed 10  Mb/s */
	#define E1000_STATUS_SPEED_100	0x00000040      /* Speed 100 Mb/s */
#define E1000_CTRL_EXT	0x00018	/* Extended Device Control - RW */
	#define E1000_CTRL_EXT_BYPS	0x00008000	/* Speed Select Bypass */
#define E1000_RCTL	0x00100  /* RX Control - RW */
	#define E1000_RCTL_EN		(1 << 1)
	#define E1000_RCTL_BAM		(1 << 15)	// Accept broadcast packets
	#define E1000_RCTL_BSIZE	0x30000		// Buffer size
	#define E1000_RCTL_BSIZE_2048	0x00000 	// Buffer size = 2048
	#define E1000_RCTL_SECRC	(1 << 26)	// Strip Ethernet CRC From Incoming Packet
#define E1000_RAL	0x05400
#define E1000_RAH	0x05404
	#define E1000_RAH_AV		(1 << 31)

#define E1000_RDBAL(_n)		(0x0C000 + ((_n) * 0x040)) // Descriptor lower bits
#define E1000_RDBAH(_n)		(0x0C004 + ((_n) * 0x040)) // Descriptor higher bits
#define E1000_RDLEN(_n)		(0x0C008 + ((_n) * 0x040))
#define E1000_RDH(_n)		(0x0C010 + ((_n) * 0x040))
#define E1000_RDT(_n)		(0x0C018 + ((_n) * 0x040))
#define E1000_RXDCTL(_n)	(0x0C028 + ((_n) * 0x100))
	#define E1000_RXDCTL_ENABLE	(1<<25)

/* Receive Descriptor - Advanced */
/* From linux/drivers/net/ethernet/intel/igb/e1000_82575.h */
union e1000_adv_rx_desc {
        struct {
                u64 pkt_addr;             /* Packet buffer address */
                u64 hdr_addr;             /* Header buffer address */
        } read;
        struct {
                struct {
                        struct {
                                u16 pkt_info;   /* RSS type, Packet type */
                                u16 hdr_info;   /* Split Head, buf len */
                        } lo_dword;
                        union {
                                u32 rss;          /* RSS Hash */
                                struct {
                                        u16 ip_id;    /* IP id */
                                        u16 csum;     /* Packet Checksum */
                                } csum_ip;
                        } hi_dword;
                } lower;
                struct {
                        u32 status_error;     /* ext status/error */
                        u16 length;           /* Packet length */
                        u16 vlan;             /* VLAN tag */
                } upper;
        } wb;  /* writeback */
};

/* Transmit Descriptor - Advanced */
/* From linux/drivers/net/ethernet/intel/igb/e1000_82575.h */
union e1000_adv_tx_desc {
        struct {
                u64 buffer_addr;    /* Address of descriptor's data buf */
                u32 cmd_type_len;
                u32 olinfo_status;
        } read;
        struct {
                u64 rsvd;       /* Reserved */
                u32 nxtseq_seed;
                u32 status;
        } wb;
};


struct eth_device {
	void	*bar_addr;
	u16	speed;
};



#define print(...)          printk("Ethernet: " __VA_ARGS__)


#endif /* INMATE_ETH_H */
