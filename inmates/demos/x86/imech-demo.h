#ifndef INMATE_ETH_H
#define INMATE_ETH_H

/* Taken using "sudo lspci -nn" */
#define ETH_VENDORID		0x8086
#define ETH_DEVICEID		0x1533

/* TODO: check */
#define ETH_IRQ_VECTOR		42

#define E1000_RCTL     0x00100  /* RX Control - RW */

#define E1000_RXDCTL(_n)  ((_n) < 4 ? (0x02828 + ((_n) * 0x100)) \
                                    : (0x0C028 + ((_n) * 0x40)))

struct eth_device {
	void *bar_addr;
};



#define print(...)          printk("Ethernet: " __VA_ARGS__)


#endif /* INMATE_ETH_H */
