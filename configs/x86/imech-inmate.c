#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[4];
	//struct jailhouse_irqchip irqchips[1];
	struct jailhouse_cache cache_regions[1];
	__u8 pio_bitmap[0x2000];
	struct jailhouse_pci_device pci_devices[1];
        struct jailhouse_pci_capability pci_caps[7];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
        	.revision = JAILHOUSE_CONFIG_REVISION,
        	.flags = JAILHOUSE_CELL_PASSIVE_COMMREG | \
			JAILHOUSE_CELL_VIRTUAL_CONSOLE_PERMITTED,

		.name = "imech",

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_cache_regions = ARRAY_SIZE(config.cache_regions),
		//.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_irqchips = 0,
		.pio_bitmap_size = ARRAY_SIZE(config.pio_bitmap),
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
                .num_pci_caps = ARRAY_SIZE(config.pci_caps),

		.console = {
			.type = JAILHOUSE_CON_TYPE_8250,
			.flags = JAILHOUSE_CON_ACCESS_PIO,
			.address = 0x3f8,
		},

	},

	.cpus = {
		0x8,
	},

	.mem_regions = {
		/* RAM */ {
			.phys_start = 0x3e000000,
			.virt_start = 0,
			.size = 0x00100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE |
				JAILHOUSE_MEM_DMA,
		},

		/* communication region */ {
			.virt_start = 0x00100000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_COMM_REGION,
		},

                /* Ethernet BAR0 */ {
                        .phys_start = 0xdf000000,
                        .virt_start = 0xdf000000,
                        .size = 0x00200000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE,
                },
                /* Ethernet BAR3 */ {
                        .phys_start = 0xdf200000,
                        .virt_start = 0xdf200000,
                        .size = 0x00100000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE,
                },
	},

	.cache_regions = {
		{
			.start = 0,
			.size = 2,
			.type = JAILHOUSE_CACHE_L3,
		},
	},

/* 	.irqchips = { */
/* 		{ */
/* 			.address = 0xfec00000, */
/* 			.id = 0x1f0f8, */
/* 			.pin_bitmap = { */
/* 				0x00000000, 0x00000000, 0x00000000, 0x000E0000 */
/* 			}, */
/* 		}, */
/* 	}, */

	.pio_bitmap = {
		[     0/8 ...  0x2f7/8] = -1,
		[ 0x2f8/8 ...  0x2ff/8] = 0, /* serial2 */
		[ 0x300/8 ...  0x3f7/8] = -1,
		[ 0x3f8/8 ...  0x3ff/8] = 0, /* serial1 */
		[ 0x400/8 ... 0xe00f/8] = -1,
		[0xe010/8 ... 0xe017/8] = 0, /* OXPCIe952 serial1 */
		[0xe018/8 ... 0xffff/8] = -1,
	},


        .pci_devices = {
                  { /* Ethernet @03:00.0 */
			.type = JAILHOUSE_PCI_TYPE_DEVICE,
			//.iommu = 1,
			.domain = 0x0,
			.bdf = 0x300,
			.caps_start = 0, // Root cell says 43
			.num_caps = 7,
			.num_msi_vectors = 1,
			.msi_64bits = 1,
			.num_msix_vectors = 1, // Root cell says 5
			.msix_region_size = 0x4000,
			.msix_address = 0xdf200000,
                  },
          },

          .pci_caps = {
		/* PCIDevice: 03:00.0 */
		{
			.id = 0x1,
			.start = 0x40,
			.len = 8,
			.flags = JAILHOUSE_PCICAPS_WRITE,
		},
		{
			.id = 0x5,
			.start = 0x50,
			.len = 24,
			.flags = JAILHOUSE_PCICAPS_WRITE,
		},
		{
			.id = 0x11,
			.start = 0x70,
			.len = 12,
			.flags = JAILHOUSE_PCICAPS_WRITE,
		},
		{
			.id = 0x10,
			.start = 0xa0,
			.len = 60,
			.flags = 0,
		},
		{
			.id = 0x1 | JAILHOUSE_PCI_EXT_CAP,
			.start = 0x100,
			.len = 4,
			.flags = 0,
		},
		{
			.id = 0x3 | JAILHOUSE_PCI_EXT_CAP,
			.start = 0x140,
			.len = 4,
			.flags = 0,
		},
		{
			.id = 0x17 | JAILHOUSE_PCI_EXT_CAP,
			.start = 0x1a0,
			.len = 4,
			.flags = 0,
		},
          },
};
