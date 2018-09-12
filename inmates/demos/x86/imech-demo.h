#ifndef INMATE_ETH_H
#define INMATE_ETH_H

/* ============================ Hardware ==================================== */
/* Taken using "sudo lspci -nn" */
#define ETH_VENDORID		0x8086
#define ETH_DEVICEID		0x1533

/* TODO: check */
#define ETH_IRQ_VECTOR		42

/* ============================ Data structures ============================= */
#define IGB_DEFAULT_RXD		256
#define IGB_DEFAULT_TXD		256

#define RX_DESCRIPTORS          8
#define RX_BUFFER_SIZE          2048
#define TX_DESCRIPTORS          8

/* ============================ Registers =================================== */
#define E1000_CTRL     0x00000  /* Device Control - RW */
	#define E1000_CTRL_SLU      0x00000040  /* Set link up (Force Link) */
	#define E1000_CTRL_FRCSPD   0x00000800  /* Force Speed */
#define E1000_STATUS   0x00008  /* Device Status - RO */
	#define E1000_STATUS_SPEED	0x000000C0	/* Link speed */
	#define E1000_STATUS_SPEED_10	0x00000000      /* Speed 10  Mb/s */
	#define E1000_STATUS_SPEED_100	0x00000040      /* Speed 100 Mb/s */

#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_RXDCTL(_n)  ((_n) < 4 ? (0x02828 + ((_n) * 0x100)) \
                                    : (0x0C028 + ((_n) * 0x40)))

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
	void *bar_addr;
};



#define print(...)          printk("Ethernet: " __VA_ARGS__)


#endif /* INMATE_ETH_H */
