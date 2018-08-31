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



void inmate_main(void)
{
	struct eth_device dev;
	u32 reg;
	int ret;

	printk("Hello world!\n");
	printk("Hello world!\n");
	printk("Hello world!\n");
	printk("Hello world!\n");

	ret = eth_pci_probe(&dev);
	if (ret >= 0) {
		reg = mmio_read32((dev.bar_addr) + E1000_RCTL);
		printk("RCTL: %x\n", reg);

		reg = mmio_read32((dev.bar_addr) + E1000_RXDCTL(0));
		printk("RXDCTL(0): %x\n", reg);
		reg |= 0x02000000;
		printk("RXDCTL(0): writing %x\n", reg);
		mmio_write32((dev.bar_addr) + E1000_RXDCTL(0), reg);
		reg = mmio_read32((dev.bar_addr) + E1000_RXDCTL(0));
		printk("RXDCTL(0): %x\n", reg);

		reg = mmio_read32((dev.bar_addr) + E1000_RXDCTL(1));
		printk("RXDCTL(1): %x\n", reg);
		reg = mmio_read32((dev.bar_addr) + E1000_RXDCTL(2));
		printk("RXDCTL(2): %x\n", reg);
		reg = mmio_read32((dev.bar_addr) + E1000_RXDCTL(3));
		printk("RXDCTL(3): %x\n", reg);
	}

	asm volatile("hlt");
}
