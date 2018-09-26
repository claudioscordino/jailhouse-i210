/*
 * Authors:
 *  Claudio Scordino <claudio@evidence.eu.com>
 */

#include <inmate.h>
#include "i210.h"

static u16 mdic_read(struct eth_device *dev, unsigned int reg)
{
	u32 val;

	mmio_write32(dev->bar_addr + E1000_MDIC,
		     (reg << E1000_MDIC_REGADD_SHFT) |
		     E1000_MDIC_OP_READ);
	do {
		val = mmio_read32(dev->bar_addr + E1000_MDIC);
		cpu_relax();
	} while (!(val & E1000_MDIC_READY));

	return (u16)val;
}

static void mdic_write(struct eth_device *dev, unsigned int reg, u16 val)
{
	mmio_write32(dev->bar_addr + E1000_MDIC,
		     val | (reg << E1000_MDIC_REGADD_SHFT) | E1000_MDIC_OP_WRITE);
	while (!(mmio_read32(dev->bar_addr + E1000_MDIC) & E1000_MDIC_READY))
		cpu_relax();
}




static u8 buffer[RX_DESCR_NB * RX_BUFFER_SIZE];
static struct rxd rx_ring[RX_DESCR_NB] __attribute__((aligned(128)));
static struct txd tx_ring[TX_DESCR_NB] __attribute__((aligned(128)));
static unsigned int rx_idx, tx_idx;


static void print_ring_regs(struct eth_device *dev, int i)
{
	printk("RDBAL(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RDBAL(i)));
	printk("RDBAH(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RDBAH(i)));
	printk("RDLEN(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RDLEN(i)));
	printk("RDH(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RDH(i)));
	printk("RDT(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RDT(i)));
	printk("RXDCTL(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_RXDCTL(i)));

	printk("TDBAL(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TDBAL(i)));
	printk("TDBAH(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TDBAH(i)));
	printk("TDLEN(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TDLEN(i)));
	printk("TDH(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TDH(i)));
	printk("TDT(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TDT(i)));
	printk("TXDCTL(%d): %x\n", i, mmio_read32(dev->bar_addr + E1000_TXDCTL(i)));
}

static void print_regs(struct eth_device* dev)
{
	u32 val;

	printk("~~~~~~~~~~~~~~~~~~~~~~~\n");
	printk("CTRL:\t%x\n", mmio_read32(dev->bar_addr + E1000_CTRL));
	printk("CTRL_EXT:\t%x\n", mmio_read32(dev->bar_addr + E1000_CTRL_EXT));
	printk("STATUS:\t%x\n", mmio_read32(dev->bar_addr + E1000_STATUS));
	printk("TCTL:\t%x\n", mmio_read32(dev->bar_addr + E1000_TCTL));
	printk("TIPG:\t%x\n", mmio_read32(dev->bar_addr + E1000_TIPG));
/* 	printk("RAL:\t%x\n", mmio_read32(dev->bar_addr + E1000_RAL)); */
/* 	printk("RAH:\t%x\n", mmio_read32(dev->bar_addr + E1000_RAH)); */


	// Check speeds
	val = mmio_read32(dev->bar_addr + E1000_STATUS);
	val &= E1000_STATUS_SPEED_MSK;
	if (val == E1000_STATUS_SPEED_10)
		printk("Speed:\t10 Mb/s\n");
	else if (val == E1000_STATUS_SPEED_100)
		printk("Speed:\t100 Mb/s\n");
	else
		printk("Speed:\t1000 Mb/s\n");

	val = mmio_read32(dev->bar_addr + E1000_PCS_LCTL);
	val &= E1000_PCS_LCTL_FSV_MSK;
	if (val == E1000_PCS_LCTL_FSV_10)
		printk("Link speed:\t10 Mb/s\n");
	else if (val == E1000_PCS_LCTL_FSV_100)
		printk("Link speed:\t100 Mb/s\n");
	else
		printk("Link speed:\t1000 Mb/s\n");

	val = mdic_read(dev, E1000_MDIC_CCR);
	val &= E1000_MDIC_CCR_SPEED_MSK;
	if (val == E1000_MDIC_CCR_SPEED_10)
		printk("MDIC Link speed:\t10 Mb/s\n");
	else if (val == E1000_MDIC_CCR_SPEED_100)
		printk("MDIC Link speed:\t100 Mb/s\n");
	else
		printk("MDIC Link speed:\t1000 Mb/s\n");

	for (int i = 0; i < 4; ++i)
		print_ring_regs(dev, i);
}

static int eth_pci_probe(struct eth_device *dev)
{
        u64 bar;
        int bdf;

        bdf = pci_find_device(ETH_VENDORID, ETH_DEVICEID, 0);
        if (bdf < 0) {
                print("ERROR: no device found\n");
                return -1;
        }
        print("found %04x:%04x at %02x:%02x.%x\n",
                        pci_read_config(bdf, PCI_CFG_VENDOR_ID, 2),
                        pci_read_config(bdf, PCI_CFG_DEVICE_ID, 2),
                        bdf >> 8, (bdf >> 3) & 0x1f, bdf & 0x3);

        // Read Base Address Register in the configuration
        bar = pci_read_config(bdf, PCI_CFG_BAR, 4);
        if ((bar & 0x6) == 0x4)
                bar |= (u64)pci_read_config(bdf, PCI_CFG_BAR + 4, 4) << 32;

        // Map BAR in the virtual memory
	// TODO: check which one
        dev->bar_addr = (void *)(bar & ~0xfUL);
/*         map_range(dev->bar_addr, PAGE_SIZE, MAP_UNCACHED); */
        map_range(dev->bar_addr, 128 * 1024, MAP_UNCACHED);
        print("BAR at %p\n", dev->bar_addr);

        // Set MSI IRQ vector
	// TODO: missing in e1000
        pci_msi_set_vector(bdf, ETH_IRQ_VECTOR);

        pci_write_config(bdf, PCI_CFG_COMMAND,
                        PCI_CMD_MEM | PCI_CMD_MASTER, 2);

	// Software reset
	mmio_write32(dev->bar_addr + E1000_CTRL, E1000_CTRL_RST);
	while (!(mmio_read32(dev->bar_addr + E1000_STATUS) | E1000_STATUS_RST_DONE))
		cpu_relax();

        print("PCI device succesfully initialized\n");
        return 0;
}


static void eth_set_speed(struct eth_device *dev, u16 speed)
{
	u32 val;

	val = mmio_read32(dev->bar_addr + E1000_CTRL_EXT);

	// Disable low power modes
	val &= ~(E1000_CTRL_EXT_SD_LP | E1000_CTRL_EXT_PHY_LP);
	if (speed == 100)
		val |= E1000_CTRL_EXT_BYPS; // Bypass speed detection
	mmio_write32(dev->bar_addr + E1000_CTRL_EXT, val);

	if (speed == 100){
		val = mmio_read32(dev->bar_addr + E1000_PCS_LCTL);
		val &= ~(E1000_PCS_LCTL_FSV_MSK);
		val |= E1000_PCS_LCTL_FSV_100;
		mmio_write32(dev->bar_addr + E1000_PCS_LCTL, val);

		// Disable 1000 Mb/s in all power modes:
		mmio_write32(dev->bar_addr + E1000_PHPM,
				mmio_read32(dev->bar_addr + E1000_PHPM) | E1000_PHPM_NO_1000);
	}

	val = mmio_read32(dev->bar_addr + E1000_CTRL);
	printk("CTRL (before changing speed):\t%x\n", val);
        val |= E1000_CTRL_SLU; // Set link up
	if (speed == 100) {
        	val &= ~(E1000_CTRL_SPEED_MSK);
		val |= E1000_CTRL_SPEED_100; // Set link to 100 Mp/s
		val |= E1000_CTRL_FRCSPD; // Force speed
	} else {
		val &= ~(E1000_CTRL_FRCSPD); // Enable PHY to control MAC speed
	}
	mmio_write32(dev->bar_addr + E1000_CTRL, val);
	delay_us(20000);

	val = mdic_read(dev, E1000_MDIC_CCR);
	if (speed == 100) {
		val &= ~(E1000_MDIC_CCR_SPEED_MSK);
		val |= E1000_MDIC_CCR_SPEED_100;
	}
	val &= ~(E1000_MDIC_CCR_POWER_DOWN); // Power up
	mdic_write(dev, E1000_MDIC_CCR, val);


	printk("Waiting for link...");
	while (!(mmio_read32(dev->bar_addr + E1000_STATUS) & E1000_STATUS_LU))
		cpu_relax();

	dev->speed = speed;

	printk(" ok\n");
}


static void eth_print_mac_addr(struct eth_device *dev)
{
	if (mmio_read32(dev->bar_addr + E1000_RAH) & E1000_RAH_AV) {
		*(u32 *)dev->mac = mmio_read32(dev->bar_addr + E1000_RAL);
		*(u16 *)&(dev->mac[4]) = mmio_read32(dev->bar_addr + E1000_RAH);

		printk("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
				dev->mac[0], dev->mac[1], dev->mac[2], dev->mac[3], dev->mac[4], dev->mac[5]);
	} else {
		printk("ERROR: need to get MAC through EERD\n");
	}
}


static void eth_setup_rx(struct eth_device *dev)
{
	u32 val;

	// Disable all RX queues (TODO: write 0 ?)
	for (int i=0; i< NUM_QUEUES; ++i){
		mmio_write32(dev->bar_addr + E1000_RXDCTL(i),
			mmio_read32(dev->bar_addr + E1000_RXDCTL(i)) & ~(E1000_RXDCTL_ENABLE));
	}

	// Make the ring point to the buffer
	for (int i = 0; i < RX_DESCR_NB; ++i)
		rx_ring[i].addr = (u64) &buffer [i * RX_BUFFER_SIZE];

	// These must be programmed when the queue is still disabled:
        mmio_write32(dev->bar_addr + E1000_RDBAL(0), (unsigned long)&rx_ring);
        mmio_write32(dev->bar_addr + E1000_RDBAH(0), 0);
        mmio_write32(dev->bar_addr + E1000_RDLEN(0), sizeof(rx_ring));
        mmio_write32(dev->bar_addr + E1000_RDH(0), 0);
        mmio_write32(dev->bar_addr + E1000_RDT(0), 0); // Overwritten below

	// Enable only the first queue
        mmio_write32(dev->bar_addr + E1000_RXDCTL(0),
                  	mmio_read32(dev->bar_addr + E1000_RXDCTL(0)) | E1000_RXDCTL_ENABLE);

	val = mmio_read32(dev->bar_addr + E1000_RCTL);
	val &= ~(E1000_RCTL_BAM | E1000_RCTL_BSIZE);
	val |= (E1000_RCTL_RXEN | E1000_RCTL_SECRC | E1000_RCTL_BSIZE_2048);
	mmio_write32(dev->bar_addr + E1000_RCTL, val);

	mmio_write32(dev->bar_addr + E1000_RDT(0), RX_DESCR_NB - 1);
}


static void eth_setup_tx(struct eth_device *dev)
{
	// Disable all TX queues (TODO: write 0 ?)
	for (int i=0; i< NUM_QUEUES; ++i){
		mmio_write32(dev->bar_addr + E1000_TXDCTL(i),
			mmio_read32(dev->bar_addr + E1000_TXDCTL(i)) & ~(E1000_TXDCTL_ENABLE));
	}

	// These must be programmed when the queue is still disabled:
	mmio_write32(dev->bar_addr + E1000_TDBAL(0), (unsigned long)&tx_ring);
	mmio_write32(dev->bar_addr + E1000_TDBAH(0), 0);
	mmio_write32(dev->bar_addr + E1000_TDLEN(0), sizeof(tx_ring));
	mmio_write32(dev->bar_addr + E1000_TDH(0), 0);
	mmio_write32(dev->bar_addr + E1000_TDT(0), 0);

	// Enable only the first queue
	mmio_write32(dev->bar_addr + E1000_TXDCTL(0),
		mmio_read32((dev->bar_addr)  + E1000_TXDCTL(0)) | E1000_TXDCTL_ENABLE);

	mmio_write32(dev->bar_addr + E1000_TCTL, mmio_read32(dev->bar_addr + E1000_TCTL) |
		E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT_IEEE);

/* 	mmio_write32(dev->bar_addr + E1000_TIPG, */
/* 		     E1000_TIPG_IPGT_DEF | E1000_TIPG_IPGR1_DEF | */
/* 		     E1000_TIPG_IPGR2_DEF); */


}


static void send_packet(struct eth_device *dev, void *pkt, unsigned int size)
{
	unsigned int idx = tx_idx;
	memset(&tx_ring[idx], 0, sizeof(struct txd));
	tx_ring[idx].addr = (unsigned long)pkt;
	tx_ring[idx].len = size;
	tx_ring[idx].rs = 1;
	tx_ring[idx].ifcs = 1;
	tx_ring[idx].eop = 1;

	tx_idx = (tx_idx + 1) % TX_DESCR_NB;
	mmio_write32(dev->bar_addr + E1000_TDT(0), tx_idx);

	while (!tx_ring[idx].dd)
		cpu_relax();
}



void inmate_main(void)
{
	struct eth_header tx_packet;
	struct eth_device dev;
	int ret;

	printk("Starting...\n");

	ret = eth_pci_probe(&dev);
	if (ret < 0)
		goto error;

	print_regs(&dev);
	eth_set_speed(&dev, 100);
	eth_print_mac_addr(&dev);
	eth_setup_rx(&dev);
	eth_setup_tx(&dev);
	print_regs(&dev);

	printk("Size = %ld\n", sizeof(struct rxd));
	printk("Size = %ld\n", sizeof(rx_ring));

	memcpy(tx_packet.src, dev.mac, sizeof(tx_packet.src));
	memset(tx_packet.dst, 0xff, sizeof(tx_packet.dst));
	tx_packet.type = ETH_FRAME_TYPE_ANNOUNCE;
	for (int i = 0; i < 100; ++i)
		send_packet(&dev, &tx_packet, sizeof(tx_packet));
	printk("Finished!\n");

error:
/* 	asm volatile("hlt"); */
	cpu_relax();
}
