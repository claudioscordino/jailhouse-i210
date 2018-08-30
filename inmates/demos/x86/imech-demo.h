#ifndef INMATE_ETH_H
#define INMATE_ETH_H

/* Taken using "sudo lspci -nn" */
#define ETH_VENDORID		0x8086
#define ETH_DEVICEID		0x1533

/* TODO: check */
#define ETH_IRQ_VECTOR		42

#define E1000_CTRL     0x00000  /* Device Control - RW */
#define E1000_STATUS   0x00008  /* Device Status - RO */
#define E1000_EECD     0x00010  /* EEPROM/Flash Control - RW */
#define E1000_EERD     0x00014  /* EEPROM Read - RW */
#define E1000_CTRL_EXT 0x00018  /* Extended Device Control - RW */
#define E1000_MDIC     0x00020  /* MDI Control - RW */
#define E1000_MDICNFG  0x00E04  /* MDI Config - RW */
#define E1000_SCTL     0x00024  /* SerDes Control - RW */
#define E1000_FCAL     0x00028  /* Flow Control Address Low - RW */
#define E1000_FCAH     0x0002C  /* Flow Control Address High -RW */
#define E1000_FCT      0x00030  /* Flow Control Type - RW */
#define E1000_CONNSW   0x00034  /* Copper/Fiber switch control - RW */
#define E1000_VET      0x00038  /* VLAN Ether Type - RW */
#define E1000_TSSDP    0x0003C  /* Time Sync SDP Configuration Register - RW */
#define E1000_ICR      0x000C0  /* Interrupt Cause Read - R/clr */
#define E1000_ITR      0x000C4  /* Interrupt Throttling Rate - RW */
#define E1000_ICS      0x000C8  /* Interrupt Cause Set - WO */
#define E1000_IMS      0x000D0  /* Interrupt Mask Set - RW */
#define E1000_IMC      0x000D8  /* Interrupt Mask Clear - WO */
#define E1000_IAM      0x000E0  /* Interrupt Acknowledge Auto Mask */
#define E1000_RCTL     0x00100  /* RX Control - RW */
#define E1000_FCTTV    0x00170  /* Flow Control Transmit Timer Value - RW */
#define E1000_TXCW     0x00178  /* TX Configuration Word - RW */
#define E1000_EICR     0x01580  /* Ext. Interrupt Cause Read - R/clr */
#define E1000_EITR(_n) (0x01680 + (0x4 * (_n)))
#define E1000_EICS     0x01520  /* Ext. Interrupt Cause Set - W0 */
#define E1000_EIMS     0x01524  /* Ext. Interrupt Mask Set/Read - RW */
#define E1000_EIMC     0x01528  /* Ext. Interrupt Mask Clear - WO */
#define E1000_EIAC     0x0152C  /* Ext. Interrupt Auto Clear - RW */
#define E1000_EIAM     0x01530  /* Ext. Interrupt Ack Auto Clear Mask - RW */
#define E1000_GPIE     0x01514  /* General Purpose Interrupt Enable - RW */
#define E1000_IVAR0    0x01700  /* Interrupt Vector Allocation (array) - RW */
#define E1000_IVAR_MISC 0x01740 /* IVAR for "other" causes - RW */
#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TCTL_EXT 0x00404  /* Extended TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_AIT      0x00458  /* Adaptive Interframe Spacing Throttle - RW */
#define E1000_LEDCTL   0x00E00  /* LED Control - RW */
#define E1000_LEDMUX   0x08130  /* LED MUX Control */
#define E1000_PBA      0x01000  /* Packet Buffer Allocation - RW */
#define E1000_PBS      0x01008  /* Packet Buffer Size */
#define E1000_EEMNGCTL 0x01010  /* MNG EEprom Control */
#define E1000_EEMNGCTL_I210 0x12030  /* MNG EEprom Control */
#define E1000_EEARBC_I210 0x12024  /* EEPROM Auto Read Bus Control */
#define E1000_EEWR     0x0102C  /* EEPROM Write Register - RW */
#define E1000_I2CCMD   0x01028  /* SFPI2C Command Register - RW */
#define E1000_FRTIMER  0x01048  /* Free Running Timer - RW */
#define E1000_TCPTIMER 0x0104C  /* TCP Timer - RW */
#define E1000_FCRTL    0x02160  /* Flow Control Receive Threshold Low - RW */
#define E1000_FCRTH    0x02168  /* Flow Control Receive Threshold High - RW */
#define E1000_FCRTV    0x02460  /* Flow Control Refresh Timer Value - RW */
#define E1000_I2CPARAMS        0x0102C /* SFPI2C Parameters Register - RW */
#define E1000_I2CBB_EN      0x00000100  /* I2C - Bit Bang Enable */
#define E1000_I2C_CLK_OUT   0x00000200  /* I2C- Clock */
#define E1000_I2C_DATA_OUT  0x00000400  /* I2C- Data Out */
#define E1000_I2C_DATA_OE_N 0x00000800  /* I2C- Data Output Enable */
#define E1000_I2C_DATA_IN   0x00001000  /* I2C- Data In */
#define E1000_I2C_CLK_OE_N  0x00002000  /* I2C- Clock Output Enable */
#define E1000_I2C_CLK_IN    0x00004000  /* I2C- Clock In */
#define E1000_MPHY_ADDR_CTRL    0x0024 /* GbE MPHY Address Control */
#define E1000_MPHY_DATA         0x0E10 /* GBE MPHY Data */
#define E1000_MPHY_STAT         0x0E0C /* GBE MPHY Statistics */

#define E1000_TSYNCRXCTL 0x0B620 /* Rx Time Sync Control register - RW */
#define E1000_TSYNCTXCTL 0x0B614 /* Tx Time Sync Control register - RW */
#define E1000_TSYNCRXCFG 0x05F50 /* Time Sync Rx Configuration - RW */
#define E1000_RXSTMPL    0x0B624 /* Rx timestamp Low - RO */
#define E1000_RXSTMPH    0x0B628 /* Rx timestamp High - RO */
#define E1000_RXSATRL    0x0B62C /* Rx timestamp attribute low - RO */
#define E1000_RXSATRH    0x0B630 /* Rx timestamp attribute high - RO */
#define E1000_TXSTMPL    0x0B618 /* Tx timestamp value Low - RO */
#define E1000_TXSTMPH    0x0B61C /* Tx timestamp value High - RO */
#define E1000_SYSTIML    0x0B600 /* System time register Low - RO */
#define E1000_SYSTIMH    0x0B604 /* System time register High - RO */
#define E1000_TIMINCA    0x0B608 /* Increment attributes register - RW */
#define E1000_TSAUXC     0x0B640 /* Timesync Auxiliary Control register */
#define E1000_TRGTTIML0  0x0B644 /* Target Time Register 0 Low  - RW */
#define E1000_TRGTTIMH0  0x0B648 /* Target Time Register 0 High - RW */
#define E1000_TRGTTIML1  0x0B64C /* Target Time Register 1 Low  - RW */
#define E1000_TRGTTIMH1  0x0B650 /* Target Time Register 1 High - RW */
#define E1000_FREQOUT0   0x0B654 /* Frequency Out 0 Control Register - RW */
#define E1000_FREQOUT1   0x0B658 /* Frequency Out 1 Control Register - RW */
#define E1000_AUXSTMPL0  0x0B65C /* Auxiliary Time Stamp 0 Register Low  - RO */
#define E1000_AUXSTMPH0  0x0B660 /* Auxiliary Time Stamp 0 Register High - RO */
#define E1000_AUXSTMPL1  0x0B664 /* Auxiliary Time Stamp 1 Register Low  - RO */
#define E1000_AUXSTMPH1  0x0B668 /* Auxiliary Time Stamp 1 Register High - RO */
#define E1000_SYSTIMR    0x0B6F8 /* System time register Residue */
#define E1000_TSICR      0x0B66C /* Interrupt Cause Register */
#define E1000_TSIM       0x0B674 /* Interrupt Mask Register */


struct eth_device {
	void *bar_addr;
};



#define print(...)          printk("Ethernet: " __VA_ARGS__)


#endif /* INMATE_ETH_H */
