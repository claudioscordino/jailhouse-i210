/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2014
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Alternatively, you can use or redistribute this file under the following
 * BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <inmate.h>

#define PCI_REG_ADDR_PORT	0xcf8
#define PCI_REG_DATA_PORT	0xcfc

#define PCI_CONE		(1 << 31)

u32 pci_read_config(u16 bdf, unsigned int addr, unsigned int size)
{
	outl(PCI_CONE | ((u32)bdf << 8) | (addr & 0xfc), PCI_REG_ADDR_PORT);
	switch (size) {
	case 1:
		return inb(PCI_REG_DATA_PORT + (addr & 0x3));
	case 2:
		return inw(PCI_REG_DATA_PORT + (addr & 0x3));
	case 4:
		return inl(PCI_REG_DATA_PORT);
	default:
		return -1;
	}
}

void pci_write_config(u16 bdf, unsigned int addr, u32 value, unsigned int size)
{
	outl(PCI_CONE | ((u32)bdf << 8) | (addr & 0xfc), PCI_REG_ADDR_PORT);
	switch (size) {
	case 1:
		outb(value, PCI_REG_DATA_PORT + (addr & 0x3));
		break;
	case 2:
		outw(value, PCI_REG_DATA_PORT + (addr & 0x3));
		break;
	case 4:
		outl(value, PCI_REG_DATA_PORT);
		break;
	}
}

/* For MSI-X capability see: https://goo.gl/CPTujK */
void pci_msix_set_vector(u16 bdf, unsigned int vector, u32 index)
{
	int cap = pci_find_cap(bdf, PCI_CAP_MSIX); // Offset of capability (0x70)
	unsigned int bar;
	printk ("cap = %d\n", cap);
	u64 msix_table = 0;
	u32 addr;
	u16 ctrl;
	u32 table;

	if (cap < 0)
		return;
	// Message Control Register:
	ctrl = pci_read_config(bdf, cap + 2, 4);
	printk ("ctrl = %x\n", ctrl);
	/* bounds check */
	if (index > (ctrl & 0x3ff)){
		printk("ERROR: out of bound index!\n");
		return;
	}
	table = pci_read_config(bdf, cap + 4, 4);

	/* Bits 0-2 of table indicate which BAR maps the MSI-X table. */
	bar = (table & 7) * 4 + PCI_CFG_BAR;
	printk("BAR retrieved from table = %x\n", bar);
	addr = pci_read_config(bdf, bar, 4);

	if ((addr & 6) == PCI_BAR_64BIT) {
		printk("MSI-X bar is at 64 bit\n");
		msix_table = pci_read_config(bdf, bar + 4, 4);
		msix_table <<= 32;
	} else {
		printk("MSI-X bar is at 32 bit\n");
	}

	/* Bits 0-3 of BARs include info about 32/64-bit and prefetching */
	msix_table |= addr & ~0xf;
	msix_table += table & ~7;

	/* enable and mask */
	ctrl |= (MSIX_CTRL_ENABLE | MSIX_CTRL_FMASK);
	pci_write_config(bdf, cap + 2, ctrl, 2);

	msix_table += 16 * index;
	/* This is the XAPIC_BASE */
	mmio_write32((u32 *)msix_table, 0xfee00000 | cpu_id() << 12);
	mmio_write32((u32 *)(msix_table + 4), 0);
	mmio_write32((u32 *)(msix_table + 8), vector);
	mmio_write32((u32 *)(msix_table + 12), 0);

	/* enable and unmask */
	ctrl &= ~MSIX_CTRL_FMASK;
	pci_write_config(bdf, cap + 2, ctrl, 2);
}

void pci_msi_set_vector(u16 bdf, unsigned int vector)
{
	int cap = pci_find_cap(bdf, PCI_CAP_MSI);
	u16 ctl, data;

	if (cap < 0){
		printk("ERROR: MSI capability not found!\n");
		return;
	}

	pci_write_config(bdf, cap + 0x04, 0xfee00000 | (cpu_id() << 12), 4);

	ctl = pci_read_config(bdf, cap + 0x02, 2);
	printk("PCI ctl: %x\n", ctl);
	if (ctl & (1 << 7)) {
		printk("MSI is 64-bit capable\n");
		pci_write_config(bdf, cap + 0x08, 0, 4);
		data = cap + 0x0c;
	} else {
		printk("MSI not 64-bit capable\n");
		data = cap + 0x08;
	}
	pci_write_config(bdf, data, vector, 2);

	pci_write_config(bdf, cap + 0x10, 0x0000, 2);
	pci_write_config(bdf, cap + 0x02, 0x0001, 2);
}
