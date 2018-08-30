#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[1];
	struct jailhouse_cache cache_regions[1];
	__u8 pio_bitmap[0x2000];
#if 0
	struct jailhouse_pci_device pci_devices[1];
        struct jailhouse_pci_capability pci_caps[1];
#endif
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
        	.revision = JAILHOUSE_CONFIG_REVISION,
        	.flags = JAILHOUSE_CELL_PASSIVE_COMMREG|JAILHOUSE_CELL_VIRTUAL_CONSOLE_ACTIVE,

		.name = "imech",

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_cache_regions = ARRAY_SIZE(config.cache_regions),
		.num_irqchips = 0,
		.pio_bitmap_size = ARRAY_SIZE(config.pio_bitmap),
#if 0
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
                .num_pci_caps = ARRAY_SIZE(config.pci_caps),
#else
		.num_pci_devices = 0,
                .num_pci_caps = 0,
#endif

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
			.size = 0x00200000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},

#if 0
                /* Ethernet BAR0 */ {
                        .phys_start = 0xdf100000,
                        .virt_start = 0xdf100000,
                        .size = 0x00100000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE,
                },
#endif
	},

	.cache_regions = {
		{
			.start = 0,
			.size = 2,
			.type = JAILHOUSE_CACHE_L3,
		},
	},

#if 0

	.pio_bitmap = {
		[     0/8 ...  0x3f7/8] = 0,
		[ 0x3f8/8 ...  0x3ff/8] = 0,
		[ 0x400/8 ...  0xa2f/8] = 0,
		[ 0xa30/8 ...  0xa37/8] = 0,
		[ 0xa38/8 ... 0xe00f/8] = 0,
		[0xe010/8 ... 0xe017/8] = 0,
		[0xe018/8 ... 0xffff/8] = 0,
		[0x10000/8 ... 0x1a000/8] = 0,
	},

#else

	.pio_bitmap = {
		[     0/8 ...   0x3f/8] = -1,
		[  0x40/8 ...   0x47/8] = -1,
		[  0x48/8 ...   0x5f/8] = -1,
		[  0x60/8 ...   0x67/8] = -1,
		[  0x68/8 ...   0x6f/8] = -1,
		[  0x70/8 ...   0x77/8] = -1,
		[  0x78/8 ...  0x3af/8] = -1,
		[ 0x3b0/8 ...  0x3df/8] = -1,
		[ 0x3e0/8 ...  0x3f7/8] = -1,
		[ 0x3f8/8 ...  0x3ff/8] = 0,
		[ 0x400/8 ... 0xffff/8] = -1,
	},
#endif

#if 0
        .pci_devices = {
                  { /* Ethernet @03:00.0 */
                          .type = JAILHOUSE_PCI_TYPE_DEVICE,
                          .domain = 0x0000,
                          .bdf = 0x0300, // 03:00.0
                          .caps_start = 0,
                          .num_caps = 1,
                          .num_msi_vectors = 1,
                          .msi_64bits = 1,
                  },
          },

          .pci_caps = {
                  { /* Ethernet @03:00.0 */
                          .id = 0x5,
                          .start = 0x50, // Capabilities: [50] MSI: Enable- Count=1/1 Maskable+ 64bit+
                          .len = 14,
                          .flags = JAILHOUSE_PCICAPS_WRITE,
                  },
          },
#endif


};
