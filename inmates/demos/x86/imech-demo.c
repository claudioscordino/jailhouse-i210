/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2013-2016
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <inmate.h>
#include "imech-demo.h"

static union e1000_adv_rx_desc rx_ring[RX_DESCRIPTORS] __attribute__((aligned(128)));
static union e1000_adv_tx_desc tx_ring[TX_DESCRIPTORS] __attribute__((aligned(128)));

static void print_ring_regs(struct eth_device* dev, int i)
{
	u32 val;
	val = mmio_read32((dev->bar_addr) + E1000_RXDCTL(i));
	printk("RXDCTL(%d): %x\n", i, val);
}

static void print_regs(struct eth_device* dev)
{
	u32 val;
	printk("~~~~~~~~~~~~~~~~~~~~~~~\n");
	val = mmio_read32((dev->bar_addr) + E1000_RCTL);
	printk("RCTL:\t%x\n", val);
	val = mmio_read32((dev->bar_addr) + E1000_CTRL);
	printk("CTRL:\t%x\n", val);
	val = mmio_read32((dev->bar_addr) + E1000_STATUS);
	printk("STATUS:\t%x\n", val);

	for (int i = 0; i < 4; ++i)
		print_ring_regs(dev, i);
}

static int eth_pci_probe (struct eth_device *dev)
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

        /* Read Base Address Register in the configuration */
        bar = pci_read_config(bdf, PCI_CFG_BAR, 4);
        if ((bar & 0x6) == 0x4)
                bar |= (u64)pci_read_config(bdf, PCI_CFG_BAR + 4, 4) << 32;

        /* Map BAR in the virtual memory */
        dev->bar_addr = (void *)(bar & ~0xfUL);
        map_range(dev->bar_addr, PAGE_SIZE, MAP_UNCACHED);
        print("BAR at %p\n", dev->bar_addr);

        /* Set MSI IRQ vector */
        pci_msi_set_vector(bdf, ETH_IRQ_VECTOR);

        pci_write_config(bdf, PCI_CFG_COMMAND,
                        PCI_CMD_MEM | PCI_CMD_MASTER, 2);

        print("PCI device succesfully initialized\n");
        return 0;
}


static void eth_setup(struct eth_device* dev)
{
	u32 val;

	if (dev->speed == 100) {
		// Bypass all speed detection mechanisms.
		val = mmio_read32((dev->bar_addr) + E1000_CTRL_EXT);
        	val |= E1000_CTRL_EXT_BYPS;
		mmio_write32((dev->bar_addr) + E1000_CTRL_EXT, val);
	}

	val = mmio_read32((dev->bar_addr) + E1000_CTRL);
	printk("CTRL (before changing speed):\t%x\n", val);
        val |= E1000_CTRL_SLU; // Set link up
	if (dev->speed == 100) {
        	val &= ~(E1000_CTRL_SPEED);
        	val |= E1000_CTRL_SPEED_100; // Set link to 100 Mp/s (TODO: set also PHY ?)
		val |= E1000_CTRL_FRCSPD; // Force speed
	} else {
		val &= ~(E1000_CTRL_FRCSPD); // Enable PHY to control MAC speed
	}
	mmio_write32((dev->bar_addr) + E1000_CTRL, val);
	delay_us(20000);

	val = mmio_read32((dev->bar_addr) + E1000_CTRL);
	printk("CTRL (after changing speed):\t%x\n", val);


	// Check link speed
	val = mmio_read32((dev->bar_addr) + E1000_STATUS);
	val &= E1000_STATUS_SPEED;
	if (val == E1000_STATUS_SPEED_10)
		printk("Link speed: 10 Mb/s\n");
	else if (val == E1000_STATUS_SPEED_100)
		printk("Link speed: 100 Mb/s\n");
	else
		printk("Link speed: 1000 Mb/s\n");

	// Disable all queues (TODO: write 0 ?)
	for (int i=0; i< 4; ++i){
		val = mmio_read32((dev->bar_addr) + E1000_RXDCTL(i));
		val &= ~(0x02000000);
		mmio_write32((dev->bar_addr) + E1000_RXDCTL(i), val);
	}

#if 0
        /* Set DMA base address registers */
	u64 rdba = ring->dma;
        wr32(E1000_RDBAL(reg_idx),
             rdba & 0x00000000ffffffffULL);
        wr32(E1000_RDBAH(reg_idx), rdba >> 32);
        wr32(E1000_RDLEN(reg_idx),
             ring->count * sizeof(union e1000_adv_rx_desc));
#endif



	// Enable only the first queue
	val = mmio_read32((dev->bar_addr) + E1000_RXDCTL(0));
	val |= 0x02000000;
	mmio_write32((dev->bar_addr) + E1000_RXDCTL(0), val);
}





void inmate_main(void)
{
	struct eth_device dev;
	int ret, size;
	dev.speed = 100;

	printk("Starting...\n");

	ret = eth_pci_probe(&dev);
	if (ret < 0)
		goto error;

	print_regs(&dev);
	eth_setup(&dev);
	print_regs(&dev);

	size = IGB_DEFAULT_RXD * sizeof(union e1000_adv_rx_desc);
	printk("Size = %ld\n", sizeof(union e1000_adv_rx_desc));
	printk("Size = %d\n", size);

error:
	asm volatile("hlt");
}
