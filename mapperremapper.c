/**
	Made by Edoardo Mantovani, 2025
	Used for gathering every data available from the PCIe mapped memory in the kernel from userspace
	works with mt7915, mt7916, mt7981, mt7986, mt7921, mt7922 and mt7925.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <elf.h>

#include <sys/mman.h>

/** misc **/

#define REGVAL_OUT_TERMINATOR 	"\n\x1b[1F\x1b[2K"

/** driver based data **/

#define ARRAY_SIZE(ARRAY)	sizeof(ARRAY) / sizeof((ARRAY)[0]) - 1

struct mt76_connac_reg_map {
	uint32_t phys;
	uint32_t maps;
	uint32_t size;
};

#define CONNAC_REGMAP		struct mt76_connac_reg_map

struct mt76_connac_reg_map mt7915_reg_map[] = {
		{ 0x00400000, 0x80000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x90000, 0x10000 }, /* WF_MCU_SYSRAM (configure regs) */
		{ 0x40000000, 0x70000, 0x10000 }, /* WF_UMAC_SYSRAM */
		{ 0x54000000, 0x02000, 0x01000 }, /* WFDMA PCIE0 MCU DMA0 */
		{ 0x55000000, 0x03000, 0x01000 }, /* WFDMA PCIE0 MCU DMA1 */
		{ 0x58000000, 0x06000, 0x01000 }, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
		{ 0x59000000, 0x07000, 0x01000 }, /* WFDMA PCIE1 MCU DMA1 */
		{ 0x7c000000, 0xf0000, 0x10000 }, /* CONN_INFRA */
		{ 0x7c020000, 0xd0000, 0x10000 }, /* CONN_INFRA, WFDMA */
		{ 0x80020000, 0xb0000, 0x10000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0xc0000, 0x10000 }, /* WF_TOP_MISC_ON */
		{ 0x820c0000, 0x08000, 0x04000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x0c000, 0x02000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x0e000, 0x02000 }, /* WF_UMAC_TOP (PP) */
		{ 0x820ce000, 0x21c00, 0x00200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820cf000, 0x22000, 0x01000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820d0000, 0x30000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x820e0000, 0x20000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x20400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e2000, 0x20800, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x20c00, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e4000, 0x21000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e5000, 0x21400, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x820e7000, 0x21e00, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820e9000, 0x23400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x24000, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820eb000, 0x24200, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820ec000, 0x24600, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820ed000, 0x24800, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820f0000, 0xa0000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0xa0600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0xa0800, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0xa0c00, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0xa1000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0xa1400, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0xa1e00, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0xa3400, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0xa4000, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0xa4200, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0xa4600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0xa4800, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
		{ 0x00000000, 0x00000, 0x00000 }, /* End */
};


struct mt76_connac_reg_map mt7916_reg_map[] = {
		{ 0x54000000, 0x02000, 0x01000 }, /* WFDMA_0 (PCIE0 MCU DMA0) */
		{ 0x55000000, 0x03000, 0x01000 }, /* WFDMA_1 (PCIE0 MCU DMA1) */
		{ 0x56000000, 0x04000, 0x01000 }, /* WFDMA_2 (Reserved) */
		{ 0x57000000, 0x05000, 0x01000 }, /* WFDMA_3 (MCU wrap CR) */
		{ 0x58000000, 0x06000, 0x01000 }, /* WFDMA_4 (PCIE1 MCU DMA0) */
		{ 0x59000000, 0x07000, 0x01000 }, /* WFDMA_5 (PCIE1 MCU DMA1) */
		{ 0x820c0000, 0x08000, 0x04000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x0c000, 0x02000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x0e000, 0x02000 }, /* WF_UMAC_TOP (PP) */
		{ 0x820e0000, 0x20000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x20400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e2000, 0x20800, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x20c00, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e4000, 0x21000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e5000, 0x21400, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x820ce000, 0x21c00, 0x00200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820e7000, 0x21e00, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820cf000, 0x22000, 0x01000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820e9000, 0x23400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x24000, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820eb000, 0x24200, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820ec000, 0x24600, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820ed000, 0x24800, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820ca000, 0x26000, 0x02000 }, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
		{ 0x820d0000, 0x30000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x00400000, 0x80000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x90000, 0x10000 }, /* WF_MCU_SYSRAM (configure cr) */
		{ 0x820f0000, 0xa0000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0xa0600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0xa0800, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0xa0c00, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0xa1000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0xa1400, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0xa1e00, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0xa3400, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0xa4000, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0xa4200, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0xa4600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0xa4800, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
		{ 0x820c4000, 0xa8000, 0x01000 }, /* WF_LMAC_TOP (WF_UWTBL ) */
		{ 0x820b0000, 0xae000, 0x01000 }, /* [APB2] WFSYS_ON */
		{ 0x80020000, 0xb0000, 0x10000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0xc0000, 0x10000 }, /* WF_TOP_MISC_ON */
		{ 0x00000000, 0x00000, 0x00000 }, /* End */
};


struct mt76_connac_reg_map mt7986_reg_map[] = {
		{ 0x54000000, 0x402000, 0x01000 }, /* WFDMA_0 (PCIE0 MCU DMA0) */
		{ 0x55000000, 0x403000, 0x01000 }, /* WFDMA_1 (PCIE0 MCU DMA1) */
		{ 0x56000000, 0x404000, 0x01000 }, /* WFDMA_2 (Reserved) */
		{ 0x57000000, 0x405000, 0x01000 }, /* WFDMA_3 (MCU wrap CR) */
		{ 0x58000000, 0x406000, 0x01000 }, /* WFDMA_4 (PCIE1 MCU DMA0) */
		{ 0x59000000, 0x407000, 0x01000 }, /* WFDMA_5 (PCIE1 MCU DMA1) */
		{ 0x820c0000, 0x408000, 0x04000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x40c000, 0x02000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x40e000, 0x02000 }, /* WF_UMAC_TOP (PP) */
		{ 0x820e0000, 0x420000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x420400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e2000, 0x420800, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x420c00, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e4000, 0x421000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e5000, 0x421400, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x820ce000, 0x421c00, 0x00200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820e7000, 0x421e00, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820cf000, 0x422000, 0x01000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820e9000, 0x423400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x424000, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820eb000, 0x424200, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820ec000, 0x424600, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820ed000, 0x424800, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820ca000, 0x426000, 0x02000 }, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
		{ 0x820d0000, 0x430000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x00400000, 0x480000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x490000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x820f0000, 0x4a0000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0x4a0600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0x4a0800, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0x4a0c00, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0x4a1000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0x4a1400, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0x4a1e00, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0x4a3400, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0x4a4000, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0x4a4200, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0x4a4600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0x4a4800, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
		{ 0x820c4000, 0x4a8000, 0x01000 }, /* WF_LMAC_TOP (WF_UWTBL ) */
		{ 0x820b0000, 0x4ae000, 0x01000 }, /* [APB2] WFSYS_ON */
		{ 0x80020000, 0x4b0000, 0x10000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0x4c0000, 0x10000 }, /* WF_TOP_MISC_ON */
		{ 0x89000000, 0x4d0000, 0x01000 }, /* WF_MCU_CFG_ON */
		{ 0x89010000, 0x4d1000, 0x01000 }, /* WF_MCU_CIRQ */
		{ 0x89020000, 0x4d2000, 0x01000 }, /* WF_MCU_GPT */
		{ 0x89030000, 0x4d3000, 0x01000 }, /* WF_MCU_WDT */
		{ 0x80010000, 0x4d4000, 0x01000 }, /* WF_AXIDMA */
		{ 0x00000000, 0x000000, 0x00000 }, /* End */
};

/** this is for the usb adapter **/
struct mt76_connac_reg_map mt7961_reg_map[] = {
		{ 0x915000,   0x915000,    0x59C10 },
		{ 0x2015c00,  0x2015c00,   0x43810 },
		{ 0x404400,   0x404400,    0x3d00  },
		{ 0xe0270000, 0xe0270000,  0xd000  },
		{ 0x00000000, 0x00000000,  0x0000  },
};

struct mt76_connac_reg_map mt7921_reg_map[] = {
		{ 0x820d0000, 0x30000, 0x10000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x820ed000, 0x24800, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820e4000, 0x21000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e7000, 0x21e00, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820eb000, 0x24200, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820e2000, 0x20800, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x20c00, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e5000, 0x21400, 0x00800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x00400000, 0x80000, 0x10000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x90000, 0x10000 }, /* WF_MCU_SYSRAM (configure register) */
		{ 0x40000000, 0x70000, 0x10000 }, /* WF_UMAC_SYSRAM */
		{ 0x54000000, 0x02000, 0x01000 }, /* WFDMA PCIE0 MCU DMA0 */
		{ 0x55000000, 0x03000, 0x01000 }, /* WFDMA PCIE0 MCU DMA1 */
		{ 0x58000000, 0x06000, 0x01000 }, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
		{ 0x59000000, 0x07000, 0x01000 }, /* WFDMA PCIE1 MCU DMA1 */
		{ 0x7c000000, 0xf0000, 0x10000 }, /* CONN_INFRA */
		{ 0x7c020000, 0xd0000, 0x10000 }, /* CONN_INFRA, WFDMA */
		{ 0x7c060000, 0xe0000, 0x10000 }, /* CONN_INFRA, conn_host_csr_top */
		{ 0x80020000, 0xb0000, 0x10000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0xc0000, 0x10000 }, /* WF_TOP_MISC_ON */
		{ 0x820c0000, 0x08000, 0x04000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x0c000, 0x02000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x0e000, 0x01000 }, /* WF_UMAC_TOP (PP) */
		{ 0x820cd000, 0x0f000, 0x01000 }, /* WF_MDP_TOP */
		{ 0x74030000, 0x10000, 0x10000 }, /* PCIE_MAC_IREG */
		{ 0x820ce000, 0x21c00, 0x00200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820cf000, 0x22000, 0x01000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820e0000, 0x20000, 0x00400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x20400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e9000, 0x23400, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x24000, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820ec000, 0x24600, 0x00200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820f0000, 0xa0000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0xa0600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0xa0800, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0xa0c00, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0xa1000, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0xa1400, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0xa1e00, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0xa3400, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0xa4000, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0xa4200, 0x00400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0xa4600, 0x00200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0xa4800, 0x00800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
		{ 0x00000000, 0x00000, 0x00000 }  /* End */
};

struct mt76_connac_reg_map mt7925_reg_map[] = {
		{ 0x830c0000, 0x000000, 0x0001000 }, /* WF_MCU_BUS_CR_REMAP */
		{ 0x54000000, 0x002000, 0x0001000 }, /* WFDMA PCIE0 MCU DMA0 */
		{ 0x55000000, 0x003000, 0x0001000 }, /* WFDMA PCIE0 MCU DMA1 */
		{ 0x56000000, 0x004000, 0x0001000 }, /* WFDMA reserved */
		{ 0x57000000, 0x005000, 0x0001000 }, /* WFDMA MCU wrap CR */
		{ 0x58000000, 0x006000, 0x0001000 }, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
		{ 0x59000000, 0x007000, 0x0001000 }, /* WFDMA PCIE1 MCU DMA1 */
		{ 0x820c0000, 0x008000, 0x0004000 }, /* WF_UMAC_TOP (PLE) */
		{ 0x820c8000, 0x00c000, 0x0002000 }, /* WF_UMAC_TOP (PSE) */
		{ 0x820cc000, 0x00e000, 0x0002000 }, /* WF_UMAC_TOP (PP) */
		{ 0x74030000, 0x010000, 0x0001000 }, /* PCIe MAC */
		{ 0x820e0000, 0x020000, 0x0000400 }, /* WF_LMAC_TOP BN0 (WF_CFG) */
		{ 0x820e1000, 0x020400, 0x0000200 }, /* WF_LMAC_TOP BN0 (WF_TRB) */
		{ 0x820e2000, 0x020800, 0x0000400 }, /* WF_LMAC_TOP BN0 (WF_AGG) */
		{ 0x820e3000, 0x020c00, 0x0000400 }, /* WF_LMAC_TOP BN0 (WF_ARB) */
		{ 0x820e4000, 0x021000, 0x0000400 }, /* WF_LMAC_TOP BN0 (WF_TMAC) */
		{ 0x820e5000, 0x021400, 0x0000800 }, /* WF_LMAC_TOP BN0 (WF_RMAC) */
		{ 0x820ce000, 0x021c00, 0x0000200 }, /* WF_LMAC_TOP (WF_SEC) */
		{ 0x820e7000, 0x021e00, 0x0000200 }, /* WF_LMAC_TOP BN0 (WF_DMA) */
		{ 0x820cf000, 0x022000, 0x0001000 }, /* WF_LMAC_TOP (WF_PF) */
		{ 0x820e9000, 0x023400, 0x0000200 }, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
		{ 0x820ea000, 0x024000, 0x0000200 }, /* WF_LMAC_TOP BN0 (WF_ETBF) */
		{ 0x820eb000, 0x024200, 0x0000400 }, /* WF_LMAC_TOP BN0 (WF_LPON) */
		{ 0x820ec000, 0x024600, 0x0000200 }, /* WF_LMAC_TOP BN0 (WF_INT) */
		{ 0x820ed000, 0x024800, 0x0000800 }, /* WF_LMAC_TOP BN0 (WF_MIB) */
		{ 0x820ca000, 0x026000, 0x0002000 }, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
		{ 0x820d0000, 0x030000, 0x0010000 }, /* WF_LMAC_TOP (WF_WTBLON) */
		{ 0x40000000, 0x070000, 0x0010000 }, /* WF_UMAC_SYSRAM */
		{ 0x00400000, 0x080000, 0x0010000 }, /* WF_MCU_SYSRAM */
		{ 0x00410000, 0x090000, 0x0010000 }, /* WF_MCU_SYSRAM (configure register) */
		{ 0x820f0000, 0x0a0000, 0x0000400 }, /* WF_LMAC_TOP BN1 (WF_CFG) */
		{ 0x820f1000, 0x0a0600, 0x0000200 }, /* WF_LMAC_TOP BN1 (WF_TRB) */
		{ 0x820f2000, 0x0a0800, 0x0000400 }, /* WF_LMAC_TOP BN1 (WF_AGG) */
		{ 0x820f3000, 0x0a0c00, 0x0000400 }, /* WF_LMAC_TOP BN1 (WF_ARB) */
		{ 0x820f4000, 0x0a1000, 0x0000400 }, /* WF_LMAC_TOP BN1 (WF_TMAC) */
		{ 0x820f5000, 0x0a1400, 0x0000800 }, /* WF_LMAC_TOP BN1 (WF_RMAC) */
		{ 0x820f7000, 0x0a1e00, 0x0000200 }, /* WF_LMAC_TOP BN1 (WF_DMA) */
		{ 0x820f9000, 0x0a3400, 0x0000200 }, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
		{ 0x820fa000, 0x0a4000, 0x0000200 }, /* WF_LMAC_TOP BN1 (WF_ETBF) */
		{ 0x820fb000, 0x0a4200, 0x0000400 }, /* WF_LMAC_TOP BN1 (WF_LPON) */
		{ 0x820fc000, 0x0a4600, 0x0000200 }, /* WF_LMAC_TOP BN1 (WF_INT) */
		{ 0x820fd000, 0x0a4800, 0x0000800 }, /* WF_LMAC_TOP BN1 (WF_MIB) */
		{ 0x820c4000, 0x0a8000, 0x0004000 }, /* WF_LMAC_TOP BN1 (WF_MUCOP) */
		{ 0x820b0000, 0x0ae000, 0x0001000 }, /* [APB2] WFSYS_ON */
		{ 0x80020000, 0x0b0000, 0x0010000 }, /* WF_TOP_MISC_OFF */
		{ 0x81020000, 0x0c0000, 0x0010000 }, /* WF_TOP_MISC_ON */
		{ 0x7c020000, 0x0d0000, 0x0010000 }, /* CONN_INFRA, wfdma */
		{ 0x7c060000, 0x0e0000, 0x0010000 }, /* CONN_INFRA, conn_host_csr_top */
		{ 0x7c000000, 0x0f0000, 0x0010000 }, /* CONN_INFRA */
		{ 0x70020000, 0x1f0000, 0x0010000 }, /* Reserved for CBTOP, can't switch */
		{ 0x7c500000, 0x060000, 0x2000000 }, /* remap */
		{ 0x00000000, 0x000000, 0x0000000 }  /* End */
};

/** start of the custom part **/
#define IO_PATH                 	"/sys/kernel/debug/ieee80211/phy%d/mt76/%s"

#define REGIDX  			"regidx"
#define REGVAL				"regval"

#define REGIDX_MAX_LEN			sizeof(IO_PATH) + sizeof(REGIDX)
#define REGVAL_MAX_LEN			sizeof(IO_PATH) + sizeof(REGVAL)

#define REGVAL_BUFSIZE			sizeof("0x80000000")
#define IO_COMMAND_LEN			REGVAL_BUFSIZE
#define IO_MANUAL_CMD_LEN		32

enum{
	_SUCCESS				= 0,
	ERR_MISSING_ARG				= 110,
	ERR_WRONG_FILES				= 111,
	ERR_FAILED_REGVAL_READ			= 112,
	ERR_FAILED_REGVAL_SSCANF		= 113
};

enum{
	UNKNOWN_HW				= 0,
	HW_MT7915				= 1,
	HW_MT7916				= 2,
	HW_MT7981				= 3,
	HW_MT7986				= 4,
	HW_MT7921				= 5,
	HW_MT7925				= 6
};

enum{
	UNKNOWN_MODE				= 0,
	MANUAL_MODE				= 1,
	AUTOMATIC_MODE				= 2
};

/** main ELF context, used for both 'automatic' and 'manual' dumping **/
typedef struct{
	Elf32_Ehdr *ELF_HEADER;
	Elf32_Phdr *ELF_REGIONS;
	void	   *ELF_DATA;
	char       *ELF_NAME;
	int	    ELF_OFFSET_COUNTER;
	int	    ELF_DATA_COUNTER;
	int	    ELF_COMMIT_FD;
}ELF_CTX;

/* create the raw file for the raw dump ('manual' mode) **/
static signed int RAW_CREATE_FILE(ELF_CTX *CTX, unsigned char *CTX_ELF_FINAL_NAME){
	memset(CTX,             0x00,           sizeof(ELF_CTX));
	CTX->ELF_OFFSET_COUNTER			= 0x00;
	/** the handmade dumped file always finish with "_raw" **/
        CTX->ELF_NAME				= strcat(CTX_ELF_FINAL_NAME, "_raw");
        /** now create the final output file **/
        CTX->ELF_COMMIT_FD                      = open(CTX->ELF_NAME, O_CREAT | O_RDWR, 0777);
	close(CTX->ELF_COMMIT_FD);

	return 0;
}

/** generic function used for writing data into both the ELF and RAW file **/
static signed int ELF_COMMIT_FD(ELF_CTX 			       *CTX,
				void 				       *CTX_DATA,
				unsigned int 			 	CTX_OFFSET_COUNTER,
				unsigned int 			 	CTX_DATA_LEN){
	CTX->ELF_COMMIT_FD					= open(CTX->ELF_NAME, O_RDWR, 0777);
	lseek(CTX->ELF_COMMIT_FD, CTX_OFFSET_COUNTER, SEEK_SET);
	write(CTX->ELF_COMMIT_FD, CTX_DATA, CTX_DATA_LEN);
	close(CTX->ELF_COMMIT_FD);

	return 0;
}

/** main function which initialize the elf output, used for 'automatic' mode **/
static signed int INITIALIZE_ELF_OUTPUT(ELF_CTX 		       *CTX,
					unsigned int 			CTX_REGION_NUMBER,
					unsigned int 			CTX_TOTAL_REGION_SIZE,
					unsigned char                  *CTX_ELF_FINAL_NAME,
					struct mt76_connac_reg_map     *CTX_REGION_MAP){
	memset(CTX,		0x00,		sizeof(ELF_CTX));
	CTX->ELF_HEADER				= (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
	memset(CTX->ELF_HEADER,	0x00,		sizeof(Elf32_Ehdr));
	CTX->ELF_REGIONS			= (Elf32_Phdr *)calloc(CTX_REGION_NUMBER, sizeof(Elf32_Phdr));
	memset(CTX->ELF_REGIONS,0x00,		sizeof(Elf32_Phdr) * CTX_REGION_NUMBER);
	CTX->ELF_DATA				= malloc(CTX_TOTAL_REGION_SIZE);
	CTX->ELF_NAME				= CTX_ELF_FINAL_NAME;

	/** now create the final output file **/
	CTX->ELF_COMMIT_FD			= open(CTX->ELF_NAME, O_CREAT | O_RDWR, 0777);
	/** file created, now just close it and returns with the 'normal' operations **/
	close(CTX->ELF_COMMIT_FD);

	/** populate the ELF header **/
        CTX->ELF_HEADER->e_ident[0]             = 0x7f;
        CTX->ELF_HEADER->e_ident[1]             = 'E';
        CTX->ELF_HEADER->e_ident[2]             = 'L';
        CTX->ELF_HEADER->e_ident[3]             = 'F';
        CTX->ELF_HEADER->e_ident[4]             = 0x01;
        CTX->ELF_HEADER->e_ident[5]             = 0x01;
        CTX->ELF_HEADER->e_ident[6]		= 0x01;

        #ifndef EM_NDS32
                #define EM_NDS32        	167
        #endif

        CTX->ELF_HEADER->e_machine              = EM_NDS32;
        CTX->ELF_HEADER->e_shnum                = 0x00;
        CTX->ELF_HEADER->e_shoff                = 0x00;
        CTX->ELF_HEADER->e_phoff                = 0x00 + sizeof(Elf32_Ehdr); /** base address (0x00) + the size of the header **/
        CTX->ELF_HEADER->e_phnum                = CTX_REGION_NUMBER;
        CTX->ELF_HEADER->e_shnum                = 0x00;
        CTX->ELF_HEADER->e_phentsize            = sizeof(Elf32_Phdr);
        CTX->ELF_HEADER->e_shentsize            = sizeof(Elf32_Shdr);
        CTX->ELF_HEADER->e_type                 = ET_EXEC;
        CTX->ELF_HEADER->e_entry                = 0x00;
        CTX->ELF_HEADER->e_version              = 0x01;
        CTX->ELF_HEADER->e_shstrndx             = 0x00;

	/** initialize the offset counter by pointing it directly to the first entry of the ELF's region table **/
	CTX->ELF_OFFSET_COUNTER			= sizeof(Elf32_Ehdr) + CTX_REGION_NUMBER * sizeof(Elf32_Phdr);

	/** now iterate every region of mapped data and create a new region header **/
	for(unsigned char j = 0; j < CTX_REGION_NUMBER; j++){
		if( CTX->ELF_REGIONS[j].p_vaddr == 0x00 && CTX_REGION_MAP[j].size != 0x00 ){
			CTX->ELF_REGIONS[j].p_type	= PT_LOAD;
			CTX->ELF_REGIONS[j].p_vaddr	= CTX_REGION_MAP[j].phys;
			CTX->ELF_REGIONS[j].p_paddr     = CTX_REGION_MAP[j].phys;
			CTX->ELF_REGIONS[j].p_memsz	= CTX_REGION_MAP[j].size;
			CTX->ELF_REGIONS[j].p_filesz	= CTX_REGION_MAP[j].size;
			CTX->ELF_REGIONS[j].p_offset	= CTX->ELF_OFFSET_COUNTER;
			CTX->ELF_REGIONS[j].p_flags	= PF_R | PF_W;
			CTX->ELF_REGIONS[j].p_align	= 0x04;
			#ifdef REGION_DEBUG
			printf("added new region: %d 0x%x 0x%x %d %d\n", j, CTX->ELF_REGIONS[j].p_vaddr,
				CTX->ELF_REGIONS[j].p_paddr, CTX->ELF_REGIONS[j].p_memsz, CTX->ELF_REGIONS[j].p_offset);
			#endif
			CTX->ELF_OFFSET_COUNTER        += CTX->ELF_REGIONS[j].p_memsz;
		}
	}

	#define EHDR_SZ				sizeof(Elf32_Ehdr)
	/**  before quitting, reset 'CTX->ELF_OFFSET_COUNTER' **/
	memcpy(CTX->ELF_DATA	+ 0x00,		CTX->ELF_HEADER,  EHDR_SZ);
	memcpy(CTX->ELF_DATA    + EHDR_SZ,	CTX->ELF_REGIONS, CTX_REGION_NUMBER * sizeof(Elf32_Phdr));
	CTX->ELF_OFFSET_COUNTER		        = EHDR_SZ    +    CTX_REGION_NUMBER * sizeof(Elf32_Phdr);

	CTX->ELF_DATA_COUNTER			= CTX->ELF_OFFSET_COUNTER;
	ELF_COMMIT_FD(CTX,      CTX->ELF_DATA, 	0x00, CTX->ELF_OFFSET_COUNTER);
	return 0;
}

/** free the final ELF file context **/
static signed int FREE_ELF_OUTPUT(ELF_CTX *CTX){
	if( CTX->ELF_HEADER != NULL ){
		free(CTX->ELF_HEADER);
	}
	if( CTX->ELF_DATA != NULL ){
		free(CTX->ELF_DATA);
	}

	if( CTX->ELF_REGIONS != NULL ){
		free(CTX->ELF_REGIONS);
	}
	free(CTX);
	return 0;
}

/** base I/O functions for interacting with the wifi's RAM **/
#define REGIDX_WRITE_FAIL			2

__attribute__((hot)) static signed int REGIDX_SET_VALUE(unsigned char *REGIDX_NAME,
							unsigned int   REGIDX_RET_ADDR) {
	signed int  REGIDX_WRITE_RES		= 0x00;
	signed int  REGIDX_FD			= 0x00;

	signed char REGIDX_WRITE_BUF[32];

	snprintf(REGIDX_WRITE_BUF, sizeof(REGIDX_WRITE_BUF), "0x%x", REGIDX_RET_ADDR);

	REGIDX_FD				= open(REGIDX_NAME, O_RDWR);
	if( REGIDX_FD < 0 ){
		printf("[!] REGIDX: I/O Error!\n");
		return -REGIDX_WRITE_FAIL;
	}
	REGIDX_WRITE_RES			= write(REGIDX_FD, REGIDX_WRITE_BUF, strlen(REGIDX_WRITE_BUF));

	close(REGIDX_FD);

	return REGIDX_WRITE_RES;
}

#define REGVAL_READ_FAIL			3
__attribute__((hot)) static signed int REGVAL_GET_VALUE(unsigned char *REGVAL_NAME,
							unsigned int  *IO_COMMAND_OUTPUT,
							ELF_CTX       *CTX) {
	signed int REGVAL_READ_RES		= 0x00;
	signed int REGVAL_READ_VALUE		= 0x00;
	signed int REGVAL_FD			= 0x00;
	char       REGVAL_BUFF[32];

	REGVAL_FD				= open(REGVAL_NAME, O_RDONLY);

    	REGVAL_READ_RES 			= read(REGVAL_FD, REGVAL_BUFF, sizeof(REGVAL_BUFF) - 1);

    	if (REGVAL_READ_RES < 0) {
		printf("[!] REGVAL FAILED READ\n");
		close(REGVAL_FD);
        	return -ERR_FAILED_REGVAL_READ;
    	}

    	REGVAL_BUFF[REGVAL_READ_RES] 		= '\0';

  	if( sscanf(REGVAL_BUFF, "0x%x", IO_COMMAND_OUTPUT) != 0x1 ){
		close(REGVAL_FD);
        	return -ERR_FAILED_REGVAL_READ;
    	}

	close(REGVAL_FD);

	ELF_COMMIT_FD(CTX, IO_COMMAND_OUTPUT, CTX->ELF_OFFSET_COUNTER, sizeof(uint32_t));

	/** now update the final data buffer **/
	CTX->ELF_OFFSET_COUNTER			+= sizeof(uint32_t);

    	return 0;
}

/** main function **/
int main(int argc, char *argv[]){
	if( argc != 4 ){
		printf("[error] %s <device-number> <hardware-model> <mode>\n", argv[0]);
                printf("\"<device-number>\" is the phyX number in /sys/kernel/debug/ieee80211/phyX/mt76/\n");
		printf("\"<hardware-model>\" can be mt7921 or mt7925\n");
		printf("(note that mt7961 is used for dumping data from the mt7921 usb adapter!)\n");
		printf("\"<mode>\" can be --interactive or --automatic\n");
		printf("	interactive means that the user can write the desired address to dump and see the results\n");
		printf("	automatic means that every memory area defined in the hardware\'s dependent struct are dumped\n");
		return -ERR_MISSING_ARG;
	}

	unsigned char HW_CHOOSE			= UNKNOWN_HW;
	unsigned char MODE_CHOOSE		= UNKNOWN_MODE;

	signed   int  REGIDX_FD			= 0x00;
	signed   int  REGVAL_FD			= 0x00;

	signed   int  REGIDX_READ_RES		= 0x00;
	signed   int  REGIDX_WRITE_RES		= 0x00;
	signed   int  REGVAL_READ_RES		= 0x00;
	signed   int  REGVAL_WRITE_RES  	= 0x00;

	signed   int  EXCEPTION_ERROR   	= 0x00;

        unsigned char REGVAL_BUFF[REGVAL_BUFSIZE];

        unsigned char REGIDX_PATH[REGIDX_MAX_LEN];
        unsigned char REGVAL_PATH[REGVAL_MAX_LEN];

	unsigned char IO_COMMAND[IO_COMMAND_LEN];
	unsigned char IO_MANUAL[IO_MANUAL_CMD_LEN];

	unsigned int  IO_COMMAND_TO_LONG	= 0x00;
	unsigned int  IO_COMMAND_OUTPUT		= 0x00;

	unsigned long MANUAL_MODE_ADDRESS	= 0x00;
	unsigned int  MANUAL_MODE_LEN		= 0x00;

	unsigned int  TOTAL_ELF_REGION_SIZE	= 0x00;
	ELF_CTX       *CTX			= NULL;
	CONNAC_REGMAP *CONNECTED_REGMAP		= NULL;
	unsigned long CONNECTED_REGMAP_SIZE	= 0x00;

	memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
	memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

	memset(REGVAL_BUFF, 0x00, REGVAL_BUFSIZE);

	snprintf(REGIDX_PATH, REGIDX_MAX_LEN, IO_PATH, atoi(argv[1]), REGIDX);
        snprintf(REGVAL_PATH, REGVAL_MAX_LEN, IO_PATH, atoi(argv[1]), REGVAL);

	if( strstr(argv[2], "7915") != NULL ){
		HW_CHOOSE			= HW_MT7915;
		CONNECTED_REGMAP		= mt7915_reg_map;
		CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7915_reg_map);
	}else if( strstr(argv[2], "7916") != NULL ){
		HW_CHOOSE			= HW_MT7916;
		CONNECTED_REGMAP		= mt7916_reg_map;
		CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7916_reg_map);
	}else if( strstr(argv[2], "7981") != NULL ){
                HW_CHOOSE                       = HW_MT7981;
                CONNECTED_REGMAP                = mt7986_reg_map;
                CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7986_reg_map);
	}else if( strstr(argv[2], "7986") != NULL ){
		HW_CHOOSE			= HW_MT7986;
		CONNECTED_REGMAP		= mt7986_reg_map;
		CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7986_reg_map);
	}else if( strstr(argv[2], "7921") != NULL ){
                HW_CHOOSE                       = HW_MT7921;
                CONNECTED_REGMAP                = mt7921_reg_map;
		CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7921_reg_map);
	}else if( strstr(argv[2], "7961") != NULL ){
		HW_CHOOSE			= HW_MT7921;
		CONNECTED_REGMAP		= mt7961_reg_map;
		CONNECTED_REGMAP_SIZE		= ARRAY_SIZE(mt7961_reg_map);
	}else if( strstr(argv[2], "7922") != NULL ){
                HW_CHOOSE                       = HW_MT7921;
                CONNECTED_REGMAP                = mt7921_reg_map;
                CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(mt7921_reg_map);
        }else if( strstr(argv[2], "7925") != NULL ){
                HW_CHOOSE                       = HW_MT7925;
                CONNECTED_REGMAP                = mt7925_reg_map;
		CONNECTED_REGMAP_SIZE		= ARRAY_SIZE(mt7925_reg_map);
        }

	if( strstr(argv[3], "automatic") != NULL ){
		printf("[*] entering automatic mode...\n");
		MODE_CHOOSE			= AUTOMATIC_MODE;
	}else{
		printf("[*] entering manual mode...\n");
		MODE_CHOOSE			= MANUAL_MODE;
	}

	/** try to open the files and verify if they exists **/
	REGIDX_FD				= open(REGIDX_PATH, O_RDWR);
	REGVAL_FD				= open(REGVAL_PATH, O_RDWR);

	/** control the FD if exists or not. **/
	if( REGIDX_FD < 0 || REGVAL_FD < 0 ){
		EXCEPTION_ERROR			= ERR_WRONG_FILES;
		goto end;
	}else{
		printf("[*] opened the 2 debugfs file: %s %s\n", REGIDX_PATH, REGVAL_PATH);
	}

	close(REGIDX_FD);
	close(REGVAL_FD);

        CTX                                    	= (ELF_CTX *)malloc(sizeof(ELF_CTX));

	if( MODE_CHOOSE == AUTOMATIC_MODE ){
		/** first iteration: count the total size of the regions and their number **/
                for(unsigned char j = 0; j < CONNECTED_REGMAP_SIZE && CONNECTED_REGMAP[j].size != 0x00; j++){
			TOTAL_ELF_REGION_SIZE	       += CONNECTED_REGMAP[j].size;
		}

		/** before dumping everything, make sure to create the ELF which contains the dumped data **/
		INITIALIZE_ELF_OUTPUT(CTX, CONNECTED_REGMAP_SIZE, TOTAL_ELF_REGION_SIZE, argv[2], CONNECTED_REGMAP);

		for(unsigned char j = 0; j < CONNECTED_REGMAP_SIZE && CONNECTED_REGMAP[j].size != 0x00; j++){
			for(unsigned int Z = 0x00; Z < CONNECTED_REGMAP[j].size; Z += 0x4){
				IO_COMMAND_TO_LONG	= CONNECTED_REGMAP[j].phys + Z;
        	                REGIDX_SET_VALUE(REGIDX_PATH, IO_COMMAND_TO_LONG);
				usleep(500);
 	                	if( REGVAL_GET_VALUE(REGVAL_PATH, &IO_COMMAND_OUTPUT, CTX) < 0 ){
					break;
				}
                		printf("[region %d][0x%08x] 0x%08x" REGVAL_OUT_TERMINATOR, j, IO_COMMAND_TO_LONG, IO_COMMAND_OUTPUT);
			}
		}
	}
	if( MODE_CHOOSE == MANUAL_MODE ) {
		printf("[*] for dumping: <address> <len>\n");
		fgets(IO_MANUAL, IO_MANUAL_CMD_LEN, stdin);
		sscanf(IO_MANUAL, "0x%x %d", &MANUAL_MODE_ADDRESS, &MANUAL_MODE_LEN);
		printf("[*] 0x%x %d\n", MANUAL_MODE_ADDRESS, MANUAL_MODE_LEN);
		if( MANUAL_MODE_ADDRESS == 0x00 || MANUAL_MODE_LEN <= 0 ){
                        printf("[!] wrong address given or missing length!\n");
                        goto end;
		}else{
			/** don't waste resources by creating an ELF file for some bytes dumped, write everything into a raw file **/
			RAW_CREATE_FILE(CTX, argv[2]);
			for(unsigned int Z = 0x00; Z < MANUAL_MODE_LEN; Z += 0x4){
                                IO_COMMAND_TO_LONG      = MANUAL_MODE_ADDRESS + Z;
                                REGIDX_SET_VALUE(REGIDX_PATH, IO_COMMAND_TO_LONG);
                                usleep(500);
                                if( REGVAL_GET_VALUE(REGVAL_PATH, &IO_COMMAND_OUTPUT, CTX) < 0 ){
					printf("[!] error\n");
                                        break;
                                }
                                printf("[0x%08x] 0x%08x\n", IO_COMMAND_TO_LONG, IO_COMMAND_OUTPUT);
			}
			printf("[*] manual mode dump finished!\n");
		}
	}

	end:

	/** free the ELF context **/
	if( CTX != NULL ){
		FREE_ELF_OUTPUT(CTX);
	}

	/** free everything and clear the array **/
        memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
        memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

	if( REGIDX_FD > 0 ){
		close(REGIDX_FD);
	}

	if( REGVAL_FD > 0 ){
		close(REGVAL_FD);
	}

	printf("[!] returning with error %d\n", EXCEPTION_ERROR);
	return EXCEPTION_ERROR;
}

