/*
 * Authors:
 *  Claudio Scordino <claudio@evidence.eu.com>
 */

#include <inmate.h>
#include "i210.h"

//#define MSIX

void irq_handler(void);

// ============================ Data structures ================================

static u8 buffer[RX_DESCR_NB * RX_BUFFER_SIZE];
static struct rxd rx_ring[RX_DESCR_NB] __attribute__((aligned(128)));
static struct txd tx_ring[TX_DESCR_NB] __attribute__((aligned(128)));
static unsigned int rx_idx, tx_idx;
static struct eth_device devs [DEVS_MAX_NB];
static u16 devs_nb = 0;	// Number of found devices

// ============================ I-MECH API =====================================

static struct eth_device* get_device(u16 dev)
{
	if (dev >= devs_nb)
		return NULL;
	else
		return &(devs[dev]);
}


static u16 mdic_read(u16 dev, unsigned int reg)
{
	u32 val;

	mmio_write32(devs[dev].bar_addr + E1000_MDIC,
		     (reg << E1000_MDIC_REGADD_SHFT) |
		     E1000_MDIC_OP_READ);
	do {
		val = mmio_read32(devs[dev].bar_addr + E1000_MDIC);
		cpu_relax();
	} while (!(val & E1000_MDIC_READY));

	return (u16)val;
}

static void mdic_write(u16 dev, unsigned int reg, u16 val)
{
	mmio_write32(devs[dev].bar_addr + E1000_MDIC,
		     val | (reg << E1000_MDIC_REGADD_SHFT) | E1000_MDIC_OP_WRITE);
	while (!(mmio_read32(devs[dev].bar_addr + E1000_MDIC) & E1000_MDIC_READY))
		cpu_relax();
}




static void print_ring_regs(u16 dev, int queue)
{
	printk("%d.RDBAL(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RDBAL(queue)));
	printk("%d.RDBAH(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RDBAH(queue)));
	printk("%d.RDLEN(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RDLEN(queue)));
	printk("%d.RDH(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RDH(queue)));
	printk("%d.RDT(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RDT(queue)));
	printk("%d.RXDCTL(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_RXDCTL(queue)));

	printk("%d.TDBAL(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TDBAL(queue)));
	printk("%d.TDBAH(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TDBAH(queue)));
	printk("%d.TDLEN(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TDLEN(queue)));
	printk("%d.TDH(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TDH(queue)));
	printk("%d.TDT(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TDT(queue)));
	printk("%d.TXDCTL(%d): %x\n", dev, queue, mmio_read32(devs[dev].bar_addr + E1000_TXDCTL(queue)));
}

static void print_regs(u16 dev)
{
	u32 val;

	printk("~~~~~~~~~~~~~~~~~~~~~~~\n");
	printk("%d.CTRL:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_CTRL));
	printk("%d.CTRL_EXT:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_CTRL_EXT));
	printk("%d.STATUS:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_STATUS));
	printk("%d.TCTL:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_TCTL));
	printk("%d.TIPG:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_TIPG));
	printk("%d.GPIE:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_GPIE));
	printk("%d.IVAR:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_IVAR));
	printk("%d.IVAR_MISC:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_IVAR_MISC));
	printk("%d.EIMS:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_EIMS));
	printk("%d.EICR:\t%x\n", dev, mmio_read32(devs[dev].bar_addr + E1000_EICR));
	printk("PCI Control register = %x\n", pci_read_config(devs[dev].bdf, 0x4, 2));
	printk("PCI Status register = %x\n", pci_read_config(devs[dev].bdf, 0x6, 2));

	// Check speeds
	val = mmio_read32(devs[dev].bar_addr + E1000_STATUS);
	val &= E1000_STATUS_SPEED_MSK;
	if (val == E1000_STATUS_SPEED_10)
		printk("Speed:\t10 Mb/s\n");
	else if (val == E1000_STATUS_SPEED_100)
		printk("Speed:\t100 Mb/s\n");
	else
		printk("Speed:\t1000 Mb/s\n");

	val = mmio_read32(devs[dev].bar_addr + E1000_PCS_LCTL);
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

	for (int i = 0; i < NUM_QUEUES; ++i)
		print_ring_regs(dev, i);
}


static void eth_get_mac_addr(u16 dev)
{
	if (mmio_read32(devs[dev].bar_addr + E1000_RAH) & E1000_RAH_AV) {
		*(u32 *)devs[dev].mac = mmio_read32(devs[dev].bar_addr + E1000_RAL);
		*(u16 *)&(devs[dev].mac[4]) = mmio_read32(devs[dev].bar_addr + E1000_RAH);

		printk("%d.MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", dev,
				devs[dev].mac[0], devs[dev].mac[1], devs[dev].mac[2],
				devs[dev].mac[3], devs[dev].mac[4], devs[dev].mac[5]);
	} else {
		printk("ERROR: need to get MAC through EERD\n");
	}
}

void irq_handler(void)
{
	while (true)
        	printk("FIRE!!\n");
}



static int eth_discover_devices(void)
{
	u64 bar, bar3;
	int bdf = 0;
	u16 ctrl;

	int_init();

	while (devs_nb < DEVS_MAX_NB) {

		bdf = pci_find_device(ETH_VENDORID, ETH_DEVICEID, bdf);
		if (bdf < 0)
			break;
		devs[devs_nb].bdf = bdf;

		print("found %04x:%04x at %02x:%02x.%x\n",
				pci_read_config(bdf, PCI_CFG_VENDOR_ID, 2),
				pci_read_config(bdf, PCI_CFG_DEVICE_ID, 2),
				bdf >> 8, (bdf >> 3) & 0x1f, bdf & 0x3);

		ctrl = pci_read_config(bdf, 0x4, 2);
		printk("PCI Control register = %x\n", ctrl);
		ctrl |= 6; // Enable BME and other stuff
		ctrl |= (1 << 10); // No Interrupt Disable
		pci_write_config(bdf, 0x4, ctrl, 2);
		printk("PCI Control register = %x\n", pci_read_config(bdf, 0x4, 2));

		// Read Base Address Register in the configuration
		bar = pci_read_config(bdf, PCI_CFG_BAR, 4);
		if ((bar & BAR_TYPE_MSK) == BAR_TYPE_32BIT)
			printk ("bar0 (%llx) is at 32-bit\n", bar);
		else
			printk ("bar0 (%llx) is at 64-bit\n", bar);
		if ((bar & 0x6) == 0x4)
			bar |= (u64)pci_read_config(bdf, PCI_CFG_BAR + 4, 4) << 32;

		// Map BAR0 in the virtual memory
		devs[devs_nb].bar_addr = (void *)(bar & ~0xfUL);

		map_range(devs[devs_nb].bar_addr, BAR0_SIZE, MAP_UNCACHED);
		print("BAR0 at %p\n", devs[devs_nb].bar_addr);

		// Map BAR3 in the virtual memory
		bar3 = pci_read_config(bdf, BAR3_OFFST, 4);
		if ((bar3 & BAR_TYPE_MSK) == BAR_TYPE_32BIT)
			printk ("bar3 (%llx) is at 32-bit\n", bar3);
		else
			printk ("bar3 (%llx) is at 64-bit\n", bar3);
		if ((bar3 & 0x6) == 0x4)
			bar3 |= (u64)pci_read_config(bdf, BAR3_OFFST + 4, 4) << 32;
		devs[devs_nb].bar3_addr = (void *)(bar3 & ~0xfUL);

		map_range(devs[devs_nb].bar3_addr, BAR3_SIZE, MAP_UNCACHED);
		print("BAR3 at %p\n", devs[devs_nb].bar3_addr);

		pci_write_config(bdf, PCI_CFG_COMMAND,
				PCI_CMD_MEM | PCI_CMD_MASTER, 2);

		// Software reset
		mmio_write32(devs[devs_nb].bar_addr + E1000_CTRL, E1000_CTRL_RST);
		while (!(mmio_read32(devs[devs_nb].bar_addr + E1000_STATUS) | E1000_STATUS_RST_DONE))
			cpu_relax();

		eth_get_mac_addr(devs_nb);

		devs_nb++;
		bdf++;
	}

 	if (devs_nb < 1) {
		print("ERROR: no device found\n");
		return -1;
	}

 	print("%d PCI devices succesfully initialized\n", devs_nb);

 	return 0;
}

static void interrupt_enable(u16 dev)
{
		// Set MSI IRQ vector
		int_set_handler(ETH_IRQ_VECTOR, irq_handler);

#ifdef MSIX
		pci_msix_set_vector(devs[dev].bdf, ETH_IRQ_VECTOR, 0);
#else
		pci_msi_set_vector(devs[dev].bdf, ETH_IRQ_VECTOR);
#endif
		asm volatile("sti");
}

static void eth_set_speed(u16 dev, u16 speed)
{
	u32 val;

	val = mmio_read32(devs[dev].bar_addr + E1000_CTRL_EXT);

	// Disable low power modes
	// Linux value (dumped): 0x101400C0
	val &= ~(E1000_CTRL_EXT_SD_LP | E1000_CTRL_EXT_PHY_LP);
	if (speed == 100)
		val |= E1000_CTRL_EXT_BYPS; // Bypass speed detection
	mmio_write32(devs[dev].bar_addr + E1000_CTRL_EXT, val);

	if (speed == 100){
		// Linux value (dumped): 0x0A10000C
		val = mmio_read32(devs[dev].bar_addr + E1000_PCS_LCTL);
		val &= ~(E1000_PCS_LCTL_FSV_MSK);
		val |= E1000_PCS_LCTL_FSV_100;
		mmio_write32(devs[dev].bar_addr + E1000_PCS_LCTL, val);

		// Disable 1000 Mb/s in all power modes:
		// Linux value (dumped): 0x0000009D
		mmio_write32(devs[dev].bar_addr + E1000_PHPM,
				mmio_read32(devs[dev].bar_addr + E1000_PHPM) | E1000_PHPM_NO_1000);
	}

	// Linux value (dumped): 0x581F0241
	val = mmio_read32(devs[dev].bar_addr + E1000_CTRL);
	printk("CTRL (before changing speed):\t%x\n", val);
        val |= E1000_CTRL_SLU; // Set link up
	if (speed == 100) {
        	val &= ~(E1000_CTRL_SPEED_MSK);
		val |= E1000_CTRL_SPEED_100; // Set link to 100 Mp/s
		val |= E1000_CTRL_FRCSPD; // Force speed
	} else {
		val &= ~(E1000_CTRL_FRCSPD); // Enable PHY to control MAC speed
	}
	mmio_write32(devs[dev].bar_addr + E1000_CTRL, val);
	delay_us(20000);

	val = mdic_read(dev, E1000_MDIC_CCR);
	if (speed == 100) {
		val &= ~(E1000_MDIC_CCR_SPEED_MSK);
		val |= E1000_MDIC_CCR_SPEED_100;
	}
	val &= ~(E1000_MDIC_CCR_POWER_DOWN); // Power up
	mdic_write(dev, E1000_MDIC_CCR, val);


	printk("Waiting for link...");
	while (!(mmio_read32(devs[dev].bar_addr + E1000_STATUS) & E1000_STATUS_LU))
		cpu_relax();

	devs[dev].speed = speed;

	printk(" ok\n");
}

#define REGSET (devs_, dev_, reg_, ...) \
	mmio_write32(devs_[dev_].bar_addr + reg_\
			mmio_read32(devs_[dev_].bar_addr + reg_) __VAR_ARGS__ );

static void eth_setup_rx(u16 dev)
{
	u32 val;
	// Values taken from pag. 327 and 333 of i210 datasheet for MSI-X single vector

	// Disable interrupt moderation:
	// (i210 datasheet says "A null value is not a valid setting")
	mmio_write32(devs[dev].bar_addr + E1000_EITR_0, 0x08);
	mmio_write32(devs[dev].bar_addr + E1000_EITR_1, 0x08);
	mmio_write32(devs[dev].bar_addr + E1000_EITR_2, 0x08);
	mmio_write32(devs[dev].bar_addr + E1000_EITR_3, 0x08);
	mmio_write32(devs[dev].bar_addr + E1000_EITR_4, 0x08);

#ifdef MSIX
	// Enable all interrupts:
	mmio_write32(devs[dev].bar_addr + E1000_IMS, 0x0);

	mmio_write32(devs[dev].bar_addr + E1000_IAM, 0x0);

	mmio_write32(devs[dev].bar_addr + E1000_EIMS, 0xFFFFFFFF);
	mmio_write32(devs[dev].bar_addr + E1000_EICS, 0xFFFFFFFF);

	mmio_write32(devs[dev].bar_addr + E1000_EIAC, 0);

	mmio_write32(devs[dev].bar_addr + E1000_EIAM, 0xFFFFFFFF);

#else
	// Enable all interrupts:
	mmio_write32(devs[dev].bar_addr + E1000_IMS, 0xFFFFFFFF);
	mmio_write32(devs[dev].bar_addr + E1000_ICS, 0xFFFFFFFF);

	mmio_write32(devs[dev].bar_addr + E1000_IAM, 0xFFFFFFFF);

	mmio_write32(devs[dev].bar_addr + E1000_EIMS, 0x0);
	mmio_write32(devs[dev].bar_addr + E1000_EICS, 0x0);

	mmio_write32(devs[dev].bar_addr + E1000_EIAC, 0x0);

	mmio_write32(devs[dev].bar_addr + E1000_EIAM, 0x0);
#endif




	// Linux value (dumped): 0x50080004
	val = mmio_read32(devs[dev].bar_addr + E1000_GPIE);
	val |= ((1 << 0) | (1 << 30) | (1 << 31));
	val &= ~(1 << 4);
#if !defined(MSIX)
	val = 0;
#endif
 	mmio_write32(devs[dev].bar_addr + E1000_GPIE, val);


	// Linux value (dumped): 0x82828181
	mmio_write32(devs[dev].bar_addr + E1000_IVAR,
			mmio_read32(devs[dev].bar_addr + E1000_IVAR) | (1 << 7));
	mmio_write32(devs[dev].bar_addr + E1000_IVAR,
			mmio_read32(devs[dev].bar_addr + E1000_IVAR) & ~(0x7));

	// Linux value (dumped): 0x00008000
	mmio_write32(devs[dev].bar_addr + E1000_IVAR_MISC, (1 << 15));
	mmio_write32(devs[dev].bar_addr + E1000_IVAR_MISC,
			mmio_read32(devs[dev].bar_addr + E1000_IVAR_MISC) & ~(0x7));

	// Disable all RX queues (TODO: write 0 ?)
	for (int i=0; i < NUM_QUEUES; ++i){
		mmio_write32(devs[dev].bar_addr + E1000_RXDCTL(i),
			mmio_read32(devs[dev].bar_addr + E1000_RXDCTL(i)) & ~(E1000_RXDCTL_ENABLE));
	}

	// Make the ring point to the buffer
	for (int i = 0; i < RX_DESCR_NB; ++i)
		rx_ring[i].addr = (u64) &buffer [i * RX_BUFFER_SIZE];

	// These must be programmed when the queue is still disabled:
        mmio_write32(devs[dev].bar_addr + E1000_RDBAL(0), (unsigned long)&rx_ring);
        mmio_write32(devs[dev].bar_addr + E1000_RDBAH(0), 0);
        mmio_write32(devs[dev].bar_addr + E1000_RDLEN(0), sizeof(rx_ring));
        mmio_write32(devs[dev].bar_addr + E1000_RDH(0), 0);
        mmio_write32(devs[dev].bar_addr + E1000_RDT(0), 0); // Overwritten below

	// Linux value (dumped): 0x04448022
	val = mmio_read32(devs[dev].bar_addr + E1000_RCTL);
	val &= ~(E1000_RCTL_BSIZE);
	val &= ~(E1000_RCTL_BAM); // Accept Broadcast Packets
	val |= (E1000_RCTL_RXEN | E1000_RCTL_SECRC | E1000_RCTL_BSIZE_2048);
	mmio_write32(devs[dev].bar_addr + E1000_RCTL, val);

	// Enable only the first queue
        mmio_write32(devs[dev].bar_addr + E1000_RXDCTL(0),
			mmio_read32(devs[dev].bar_addr + E1000_RXDCTL(0)) | E1000_RXDCTL_ENABLE);

	mmio_write32(devs[dev].bar_addr + E1000_RDT(0), RX_DESCR_NB - 1);
}


static void eth_setup_tx(u16 dev)
{
	// Disable all TX queues (TODO: write 0 ?)
	for (int i=0; i < NUM_QUEUES; ++i){
		mmio_write32(devs[dev].bar_addr + E1000_TXDCTL(i),
			mmio_read32(devs[dev].bar_addr + E1000_TXDCTL(i)) & ~(E1000_TXDCTL_ENABLE));
	}

	// These must be programmed when the queue is still disabled:
	mmio_write32(devs[dev].bar_addr + E1000_TDBAL(0), (unsigned long)&tx_ring);
	mmio_write32(devs[dev].bar_addr + E1000_TDBAH(0), 0);
	mmio_write32(devs[dev].bar_addr + E1000_TDLEN(0), sizeof(tx_ring));
	mmio_write32(devs[dev].bar_addr + E1000_TDH(0), 0);
	mmio_write32(devs[dev].bar_addr + E1000_TDT(0), 0);

	// Enable only the first queue
	mmio_write32(devs[dev].bar_addr + E1000_TXDCTL(0),
		mmio_read32((devs[dev].bar_addr)  + E1000_TXDCTL(0)) | E1000_TXDCTL_ENABLE);

	mmio_write32(devs[dev].bar_addr + E1000_TCTL, mmio_read32(devs[dev].bar_addr + E1000_TCTL) |
		E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT_IEEE);

/* 	mmio_write32(devs[dev].bar_addr + E1000_TIPG, */
/* 		     E1000_TIPG_IPGT_DEF | E1000_TIPG_IPGR1_DEF | */
/* 		     E1000_TIPG_IPGR2_DEF); */


}


static void send_packet(u16 dev, void *pkt, unsigned int size)
{
	unsigned int idx = tx_idx;
	memset(&tx_ring[idx], 0, sizeof(struct txd));
	tx_ring[idx].addr = (unsigned long)pkt;
	tx_ring[idx].len = size;
	tx_ring[idx].rs = 1;
	tx_ring[idx].ifcs = 1;
	tx_ring[idx].eop = 1;

	tx_idx = (tx_idx + 1) % TX_DESCR_NB;
	mmio_write32(devs[dev].bar_addr + E1000_TDT(0), tx_idx);

	while (!tx_ring[idx].dd)
		cpu_relax();
}



void inmate_main(void)
{
	struct eth_packet tx_packet;
	printk("Starting...\n");

	if (eth_discover_devices() < 0)
		goto error;

	if (get_device(0) == NULL) {
		printk("ERROR: no devices found\n");
		goto error;
	} else if (get_device(1) != NULL) {
		printk("WARNING: more than one device found\n");
	}

	print_regs(0);
	eth_set_speed(0, 1000);
	eth_setup_rx(0);
	eth_setup_tx(0);
	interrupt_enable(0);
	print_regs(0);

	// Forge a packet:
	memcpy(tx_packet.src, devs[0].mac, sizeof(tx_packet.src));
	memset(tx_packet.dst, 0xff, sizeof(tx_packet.dst));
	tx_packet.data[0] = 0x1;
	tx_packet.data[1] = 0x2;
	tx_packet.data[2] = 0x4;
	tx_packet.data[3] = 0x8;
	tx_packet.type = ETHERTYPE_ETHERCAT;
	for (int i = 0; i < 100; ++i)
		send_packet(0, &tx_packet, sizeof(tx_packet));
	printk("Finished!\n");

error:
	while (true) {
/* 		if (rx_ring[rx_idx].dd) { */
/* 			unsigned int idx = rx_idx; */
/*  */
/*         		rx_ring[idx].dd = 0; */
/*         		rx_idx = (rx_idx + 1) % RX_DESCR_NB; */
/*         		mmio_write32(devs[0].bar_addr + E1000_RDT(0), idx); */
/* 			printk("Received!\n"); */
/* 		} else { */
/* 			u16 status = pci_read_config(devs[0].bdf, 0x6, 2); */
/* 			printk("Status = %x\n", status); */
			cpu_relax();
/* 		} */
	}
}
