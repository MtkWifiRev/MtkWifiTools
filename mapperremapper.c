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

/** driver based data **/

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
		{ 0x0, 0x0, 0x0 }, 		  /* imply end of search */
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
		{ 0x0, 0x0, 0x0 }, 		  /* imply end of search */
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
		{ 0x0, 0x0, 0x0 }, 		   /* imply end of search */
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
		{ 0x0, 0x0, 0x0 } 		     /* End */
};

/** start of the custom part **/
#define IO_PATH                 	"/sys/kernel/debug/ieee80211/phy%d/mt76/%s"

#define REGIDX  			"regidx"
#define REGVAL				"regval"

#define REGIDX_MAX_LEN			sizeof(IO_PATH) + sizeof(REGIDX)
#define REGVAL_MAX_LEN			sizeof(IO_PATH) + sizeof(REGVAL)

#define REGVAL_BUFSIZE			sizeof("0x80000000")

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

__attribute__((hot)) static signed int REGIDX_SET_VALUE(signed int REGIDX_FD, unsigned int REGIDX_RET_ADDR) {
	signed int  REGIDX_WRITE_RES		= 0x00;

	signed char REGIDX_WRITE_BUF[REGVAL_BUFSIZE];

	memset(&REGIDX_WRITE_BUF, 0x00, REGVAL_BUFSIZE);

	snprintf(REGIDX_WRITE_BUF, REGVAL_BUFSIZE, "0x%x", REGIDX_RET_ADDR);

	REGIDX_WRITE_RES			= write(REGIDX_FD, REGIDX_WRITE_BUF, REGVAL_BUFSIZE);

	return REGIDX_WRITE_RES;
}

__attribute__((hot)) static signed int REGVAL_GET_VALUE(signed int REGVAL_FD, unsigned char *REGVAL_RET_BUFFER) {
	signed int REGVAL_READ_RES		= 0x00;
	signed int REGVAL_READ_VALUE		= 0x00;

    	REGVAL_READ_RES 			= read(REGVAL_FD, REGVAL_RET_BUFFER, REGVAL_BUFSIZE - 1);
    	if (REGVAL_READ_RES < 0) {
        	return -ERR_FAILED_REGVAL_READ;
    	}

    	REGVAL_RET_BUFFER[REGVAL_READ_RES] 	= '\0';

    	if( sscanf(REGVAL_RET_BUFFER, "0x%x", &REGVAL_READ_VALUE) != 0x1 ){
        	return -ERR_FAILED_REGVAL_READ;
    	}

	printf("0x%08x\n", REGVAL_READ_VALUE);

    	return REGVAL_READ_VALUE;
}

int main(int argc, char *argv[]){
	if( argc != 4 ){
		printf("[error] %s <device-number> <hardware-model> <mode>\n", argv[0]);
		printf("\"<hardware-model>\" can be mt7921 or mt7925\n");
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

	CONNAC_REGMAP *CONNECTED_REGMAP		= NULL;

	memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
	memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

	memset(REGVAL_BUFF, 0x00, REGVAL_BUFSIZE);

	snprintf(REGIDX_PATH, REGIDX_MAX_LEN, IO_PATH, atoi(argv[1]), REGIDX);
        snprintf(REGVAL_PATH, REGVAL_MAX_LEN, IO_PATH, atoi(argv[1]), REGVAL);

	if( strstr(argv[2], "7915") != NULL ){
		HW_CHOOSE			= HW_MT7915;
		CONNECTED_REGMAP		= &mt7915_reg_map;
	}else if( strstr(argv[2], "mt7916") != NULL ){
		HW_CHOOSE			= HW_MT7916;
		CONNECTED_REGMAP		= &mt7916_reg_map;
	}else if( strstr(argv[2], "mt7986") != NULL ){
		HW_CHOOSE			= HW_MT7986;
		CONNECTED_REGMAP		= &mt7986_reg_map;
	}else if( strstr(argv[2], "mt7921") != NULL ){
                HW_CHOOSE                       = HW_MT7921;
                CONNECTED_REGMAP                = &mt7921_reg_map;
        }else if( strstr(argv[2], "mt7925") != NULL ){
                HW_CHOOSE                       = HW_MT7925;
                CONNECTED_REGMAP                = &mt7925_reg_map;
        }


	/** try to open the files and verify if they exists **/
	REGIDX_FD				= open(REGIDX_PATH, O_RDWR);
	REGVAL_FD				= open(REGVAL_PATH, O_RDWR);

	if( REGIDX_FD < 0 || REGVAL_FD < 0 ){
		EXCEPTION_ERROR			= ERR_WRONG_FILES;
		goto end;
	}else{
		printf("[*] opened the 2 debugfs file: %s %s\n", REGIDX_PATH, REGVAL_PATH);
	}

	REGIDX_SET_VALUE(REGIDX_FD, 0x800008);
	REGVAL_GET_VALUE(REGVAL_FD, REGVAL_BUFF);

	end:

        memset(REGIDX_PATH, 0x00, REGIDX_MAX_LEN);
        memset(REGVAL_PATH, 0x00, REGVAL_MAX_LEN);

	if( REGIDX_FD > 0 ){
		close(REGIDX_FD);
	}

	if( REGVAL_FD > 0 ){
		close(REGVAL_FD);
	}

	printf("returning with error %d\n", EXCEPTION_ERROR);
	return EXCEPTION_ERROR;
}

