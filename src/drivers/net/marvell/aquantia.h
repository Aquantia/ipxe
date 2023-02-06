#ifndef _ATLANTIC_H
#define _ATLANTIC_H

/** @file
*
* Marvell AQC network card driver
*
*/

FILE_LICENCE(GPL2_OR_LATER_OR_UBDL);

#include <stdint.h>
#include <ipxe/if_ether.h>
#include <ipxe/nvs.h>

#define ATL_BAR_SIZE   0x10000
#define ATL2_BAR_SIZE   0x40000
#define ATL_RING_SIZE  64
#define ATL_RING_ALIGN 128
#define ATL_RX_MAX_LEN 2048

#define ATL_IRQ_TX   0x00000001U
#define ATL_IRQ_RX   0x00000002U

/*IRQ Status Register*/
#define ATL_IRQ_STAT_REG 0x00002000U

/* Interrupt Vector Allocation Register */
#define ATL_IRQ_CTRL			 0x00002300U
#define ATL_IRQ_CTRL_COR_EN	  0x00000080U /*IRQ clear on read */
#define ATL_IRQ_CTRL_REG_RST_DIS 0x20000000U /*Register reset disable */

/*TX/RX Interruprt Mapping*/
#define ATL_IRQ_MAP_REG1		0x00002100U /*IRQ mapping register */

#define ATL_IRQ_MAP_REG1_RX0_EN 0x00008000U /*IRQ RX0 enable*/
#define ATL_IRQ_MAP_REG1_RX0	0x00000100U /*IRQ RX0*/

#define ATL_IRQ_MAP_REG1_TX0_EN 0x80000000U /*IRQ TX0 enable*/
#define ATL_IRQ_MAP_REG1_TX0	0x00000000U /*IRQ TX0*/

/*TX interrupt ctrl reg*/
#define ATL_TX_IRQ_CTRL	   0x00007B40U
#define ATL_TX_IRQ_CTRL_WB_EN 0x00000002U
//#define ATL_TX_IRQ_CTRL_PCKT_TRANSM_EN 0x00000008U

/*RX interrupt ctrl reg*/
#define ATL_RX_IRQ_CTRL	   0x00005A30U
#define ATL_RX_IRQ_CTRL_WB_EN 0x00000004U
//#define ATL_RX_IRQ_CTRL_PCKT_TRANSM_EN 0x00000008U

#define ATL_GLB_CTRL  0x00000000U

#define ATL_PCI_CTRL		 0x00001000U
#define ATL_PCI_CTRL_RST_DIS 0x20000000U

#define ATL_RX_CTRL		 0x00005000U
#define ATL_RX_CTRL_RST_DIS 0x20000000U /*RPB reset disable */
#define ATL_TX_CTRL		 0x00007000U
#define ATL_TX_CTRL_RST_DIS 0x20000000U /*TPB reset disable */

/*RX data path control registers*/
#define ATL_RPF2_CTRL	0x00005040U
#define ATL_RPF2_CTRL_EN 0x000F0000U /* RPF2 enable*/

#define ATL_RPF_CTRL1			0x00005100U
#define ATL_RPF_CTRL1_BRC_EN	 0x00000001U /*Allow broadcast receive*/
#define ATL_RPF_CTRL1_L2_PROMISC 0x00000008U /*L2 promiscious*/
#define ATL_RPF_CTRL1_ACTION	 0x00001000U /*Action to host*/
#define ATL_RPF_CTRL1_BRC_TSH	0x00010000U /*Broadcast threshold in 256 units per sec*/

#define ATL_RPF_CTRL2			  0x00005280U
#define ATL_RPF_CTRL2_VLAN_PROMISC 0x00000002U /*VLAN promisc*/

#define ATL_RPB_CTRL		 0x00005700U
#define ATL_RPB_CTRL_EN	  0x00000001U /*RPB Enable*/
#define ATL_RPB_CTRL_FC	  0x00000010U /*RPB Enable*/
#define ATL_RPB_CTRL_TC_MODE 0x00000100U /*RPB Traffic Class Mode*/

#define ATL_RPB0_CTRL1	  0x00005710U
#define ATL_RPB0_CTRL1_SIZE 0x00000140U /*RPB size (in unit 1KB) \*/

#define ATL_RPB0_CTRL2		  0x00005714U
#define ATL_RPB0_CTRL2_LOW_TSH  0x00000C00U /*Buffer Low Threshold (70% of RPB size in unit 32B)*/
#define ATL_RPB0_CTRL2_HIGH_TSH 0x1C000000U /*Buffer High Threshold(30% of RPB size in unit 32B)*/
#define ATL_RPB0_CTRL2_FC_EN	0x80000000U /*Flow control Enable*/

#define ATL_RX_DMA_DESC_BUF_SIZE 0x00005b18U
#define ATL_RX_DMA_DESC_ADDR 0x00005b00U

/*TX data path  control registers*/
#define ATL_TPO2_CTRL 0x00007040U
#define ATL_TPO2_EN   0x00010000U /*TPO2 Enable*/

#define ATL_TPB_CTRL		 0x00007900U
#define ATL_TPB_CTRL_EN	  0x00000001U /*TPB enable*/
#define ATL_TPB_CTRL_PAD_EN  0x00000004U /*Tx pad insert enable*/
#define ATL_TPB_CTRL_TC_MODE 0x00000100U /*Tx traffic Class Mode*/

#define ATL_TPB0_CTRL1	  0x00007910U
#define ATL_TPB0_CTRL1_SIZE 0x000000A0U /*TPB Size (in unit 1KB)*/

#define ATL_TPB0_CTRL2		  0x00007914U
#define ATL_TPB0_CTRL2_LOW_TSH  0x00000600U /*Buffer High Threshold(30% of RPB size in unit 32B)*/
#define ATL_TPB0_CTRL2_HIGH_TSH 0x0E000000U /*Buffer Low Threshold (70% of RPB size in unit 32B)*/

#define ATL_TX_DMA_DESC_ADDR 0x00007c00U

/*Rings control registers*/
#define ATL_RING_TX_CTRL	0x00007c08U
#define ATL_RING_TX_CTRL_EN 0x80000000U /*Tx descriptor Enable*/

#define ATL_RING_RX_CTRL	0x00005b08U
#define ATL_RING_RX_CTRL_EN 0x80000000U /*Rx descriptor Enable*/

#define ATL_RING_TAIL	 0x00007c10U
#define ATL_RING_TAIL_PTR 0x00005b10U

/*IRQ control registers*/
#define ATL_ITR_MSKS	 0x00002060U
#define ATL_ITR_MSKS_LSW 0x0000000CU
#define ATL_ITR_MSKC	 0x00002070U
#define ATL_ITR_MSKC_LSW 0x0000000CU

/*Link advertising*/
#define ATL_LINK_ADV		   0x00000368U
#define ATL_LINK_ADV_AUTONEG   0xF20U
/*#define ATL_LINK_ADV_DOWNSHIFT 0xC0000000U
#define ATL_LINK_ADV_CMD	   0x00000002U*/

//#define ATL_LINK_ADV_EN 0xFFFF0002U /*??????????????*/
#define ATL_LINK_ST	 0x00000370U

/*Semaphores*/
#define ATL_SEM_RAM 0x000003a8U

/*Mailbox*/
#define ATL_MBOX_ADDR  0x00000360U
#define ATL_MBOX_CTRL1 0x00000200U
#define ATL_MBOX_CTRL3 0x00000208U
#define ATL_MBOX_CTRL5 0x0000020cU

#define ATL_FLAG_A1 0x1
#define ATL_FLAG_A2 0x2

#define ATL_WRITE_REG(VAL, REG)	writel(VAL, nic->regs + (REG)) /*write register*/
#define ATL_READ_REG(REG)	readl(nic->regs + (REG)) /*read register*/

struct atl_desc_tx {
	uint64_t address;
	union {
		struct {
			uint32_t dx_type : 3;
			uint32_t rsvd1 : 1;
			uint32_t buf_len : 16;
			uint32_t dd : 1;
			uint32_t eop : 1;
			uint32_t cmd : 8;
			uint32_t rsvd2 : 2;
			uint32_t rsvd3 : 14;
			uint32_t pay_len : 18;
		};
		uint64_t flags;
	};
} __attribute__((packed));

struct atl_desc_tx_wb {
	uint64_t rsvd1;
	uint32_t rsvd2 : 20;
	uint32_t dd : 1;
	uint32_t rsvd3 : 11;
	uint32_t rsvd4;
} __attribute__((packed));

struct atl_desc_rx {
	uint64_t data_addr;
	uint64_t hdr_addr;

} __attribute__((packed));

struct atl_desc_rx_wb {
	uint64_t rsvd2;
	uint16_t dd : 1;
	uint16_t eop : 1;
	uint16_t rsvd3 : 14;
	uint16_t pkt_len;
	uint32_t rsvd4;
} __attribute__((packed));

struct atl_ring {
	unsigned int sw_tail;
	unsigned int sw_head;
	void * ring;
	unsigned int length;
};

struct atl_nic;

struct atl_hw_ops {
	int (*reset) (struct atl_nic *nic);
	int (*start) (struct atl_nic *nic);
	int (*stop) (struct atl_nic *nic);
	int (*get_link) (struct atl_nic *nic);
	int (*get_mac) (struct atl_nic *, uint8_t *mac);
};

/** An aQuanita network card */
struct atl_nic {
	/** Registers */
	void *regs;
	/** Port number (for multi-port devices) */
	unsigned int port;
	/** Flags */
	unsigned int flags;
	struct atl_ring tx_ring;
	struct atl_ring rx_ring;
	struct io_buffer *iobufs[ATL_RING_SIZE];
	uint32_t link_state;
	uint32_t mbox_addr;
	struct atl_hw_ops *hw_ops;
};

struct atl_hw_stats
{
	uint32_t version;
	uint32_t tid;
};

#endif /* _AQUANTIA_H */
