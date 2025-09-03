/**
        Made by Edoardo Mantovani, 2025
        Used for gathering every data available from the AXI mapped memory in the kernel from userspace (android version)
        works with soc2, soc3, soc5 and soc7.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <elf.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/wireless.h>

/** misc **/

#define REGVAL_OUT_TERMINATOR   	"\n\x1b[1F\x1b[2K"

#define REGVAL_BUFSIZE                  sizeof("0x80000000")
#define IO_COMMAND_LEN                  REGVAL_BUFSIZE
#define IO_MANUAL_CMD_LEN               32

/** driver based data **/

#define ARRAY_SIZE(ARRAY)       	sizeof(ARRAY) / sizeof((ARRAY)[0]) - 1

#define PRIV_SUPPORT_IOCTL              (SIOCDEVPRIVATE + 2)

#if 0
#define IOCTL_SET_INT                   0x8be0
#define IOCTL_GET_INT                   0x8be1
#define IOCTL_SET_STRUCT                0x8beb
#endif

#define PRIV_CMD_OID                    15
#define PRIV_CMD_ACCESS_MCR             19
#define OID_CUSTOM_MCR_RW               0xFFA0C801


#define IOCTL_SET_INT                   (SIOCIWFIRSTPRIV + 0)
#define IOCTL_GET_INT                   (SIOCIWFIRSTPRIV + 1)
#define IOCTL_QA_TOOL_DAEMON            (SIOCIWFIRSTPRIV + 16)
#define IOCTL_SET_STRUCT                (SIOCIWFIRSTPRIV + 8)
#define IOCTL_GET_STRUCT                (SIOCIWFIRSTPRIV + 9)
#define PRIV_CMD_ACCESS_MCR             19
#define PRIV_CMD_DUMP_MEM               27
#define CMD_OID_BUF_LENGTH              4096


struct mt76_connac_reg_map {
        uint32_t phys;
        uint32_t maps;
        uint32_t size;
};

#define CONNAC_REGMAP	           	struct mt76_connac_reg_map

/*****
the following tables (apart the soc2_2x2) works with AXI bus only, I am not sure if is necessary to include
also an option for supporting PCIe based hw..
******/

CONNAC_REGMAP soc2_2x2_bus2chip_cr_mapping[] = {
        /* chip addr, bus addr, range */
        {0x80000000, 0x00002000, 0x00001000}, /* MCU_CFG */

        {0x50000000, 0x00004000, 0x00004000}, /* CONN_HIF (PDMA) */
        {0x50002000, 0x00005000, 0x00001000}, /* CONN_HIF (Reserved) */
        {0x5000A000, 0x00006000, 0x00001000}, /* CONN_HIF (DMASHDL) */
        {0x000E0000, 0x00007000, 0x00001000}, /* CONN_HIF_ON (HOST_CSR) */

        {0x82060000, 0x00008000, 0x00004000}, /* WF_UMAC_TOP (PLE) */
        {0x82068000, 0x0000C000, 0x00002000}, /* WF_UMAC_TOP (PSE) */
        {0x8206C000, 0x0000E000, 0x00002000}, /* WF_UMAC_TOP (PP) */
        {0x82070000, 0x00010000, 0x00010000}, /* WF_PHY */
        {0x820F0000, 0x00020000, 0x00010000}, /* WF_LMAC_TOP */
        {0x820E0000, 0x00030000, 0x00010000}, /* WF_LMAC_TOP (WF_WTBL) */

        {0x81000000, 0x00050000, 0x00010000}, /* BTSYS_OFF */
        {0x80070000, 0x00060000, 0x00010000}, /* GPSSYS */
        {0x40000000, 0x00070000, 0x00010000}, /* WF_SYSRAM */
        {0x00300000, 0x00080000, 0x00010000}, /* MCU_SYSRAM */

        {0x80010000, 0x000A1000, 0x00001000}, /* CONN_MCU_DMA */
        {0x80030000, 0x000A2000, 0x00001000}, /* CONN_MCU_BTIF0 */
        {0x81030000, 0x000A3000, 0x00001000}, /* CONN_MCU_CFG_ON */
        {0x80050000, 0x000A4000, 0x00001000}, /* CONN_UART_PTA */
        {0x81040000, 0x000A5000, 0x00001000}, /* CONN_MCU_CIRQ */
        {0x81050000, 0x000A6000, 0x00001000}, /* CONN_MCU_GPT */
        {0x81060000, 0x000A7000, 0x00001000}, /* CONN_PTA */
        {0x81080000, 0x000A8000, 0x00001000}, /* CONN_MCU_WDT */
        {0x81090000, 0x000A9000, 0x00001000}, /* CONN_MCU_PDA */
        {0x810A0000, 0x000AA000, 0x00001000}, /* CONN_RDD_AHB_WRAP0 */
        {0x810B0000, 0x000AB000, 0x00001000}, /* BTSYS_ON */
        {0x810C0000, 0x000AC000, 0x00001000}, /* CONN_RBIST_TOP */
        {0x810D0000, 0x000AD000, 0x00001000}, /* CONN_RDD_AHB_WRAP0 */
        {0x820D0000, 0x000AE000, 0x00001000}, /* WFSYS_ON */
        {0x60000000, 0x000AF000, 0x00001000}, /* CONN_MCU_PDA */

        {0x80020000, 0x000B0000, 0x00010000}, /* CONN_TOP_MISC_OFF */
        {0x81020000, 0x000C0000, 0x00010000}, /* CONN_TOP_MISC_ON */

        {0x0, 0x0, 0x0}
};

CONNAC_REGMAP soc3_0_bus2chip_cr_mapping[] = {
        /*
         * For example:
         * WFDMA PCIE0 MCU DMA0:
         *   {0x54000000, 0x54000FFF} => {0x18402000, 0x18402FFF}
         */
        /* chip addr, bus addr, range */
        {0x54000000, 0x402000, 0x1000}, /* WFDMA PCIE0 MCU DMA0 */
        {0x55000000, 0x403000, 0x1000}, /* WFDMA PCIE0 MCU DMA1 */
        {0x56000000, 0x404000, 0x1000}, /* WFDMA reserved */
        {0x57000000, 0x405000, 0x1000}, /* WFDMA MCU wrap CR */
        {0x58000000, 0x406000, 0x1000}, /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
        {0x59000000, 0x407000, 0x1000}, /* WFDMA PCIE1 MCU DMA1 */
        {0x820c0000, 0x408000, 0x4000}, /* WF_UMAC_TOP (PLE) */
        {0x820c8000, 0x40c000, 0x2000}, /* WF_UMAC_TOP (PSE) */
        {0x820cc000, 0x40e000, 0x2000}, /* WF_UMAC_TOP (PP) */
        {0x820e0000, 0x420000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_CFG) */
        {0x820e1000, 0x420400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_TRB) */
        {0x820e2000, 0x420800, 0x0400}, /* WF_LMAC_TOP BN0 (WF_AGG) */
        {0x820e3000, 0x420c00, 0x0400}, /* WF_LMAC_TOP BN0 (WF_ARB) */
        {0x820e4000, 0x421000, 0x0400}, /* WF_LMAC_TOP BN0 (WF_TMAC) */
        {0x820e5000, 0x421400, 0x0800}, /* WF_LMAC_TOP BN0 (WF_RMAC) */
        {0x820ce000, 0x421c00, 0x0200}, /* WF_LMAC_TOP (WF_SEC) */
        {0x820e7000, 0x421e00, 0x0200}, /* WF_LMAC_TOP BN0 (WF_DMA) */
        {0x820cf000, 0x422000, 0x1000}, /* WF_LMAC_TOP (WF_PF) */
        {0x820e9000, 0x423400, 0x0200}, /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
        {0x820ea000, 0x424000, 0x0200}, /* WF_LMAC_TOP BN0 (WF_ETBF) */
        {0x820eb000, 0x424200, 0x0400}, /* WF_LMAC_TOP BN0 (WF_LPON) */
        {0x820ec000, 0x424600, 0x0200}, /* WF_LMAC_TOP BN0 (WF_INT) */
        {0x820ed000, 0x424800, 0x0800}, /* WF_LMAC_TOP BN0 (WF_MIB) */
        {0x820ca000, 0x426000, 0x2000}, /* WF_LMAC_TOP BN0 (WF_MUCOP) */
        {0x820d0000, 0x430000, 0x10000}, /* WF_LMAC_TOP (WF_WTBLON) */
        {0x40000000, 0x470000, 0x10000}, /* WF_UMAC_SYSRAM */
        {0x00400000, 0x480000, 0x10000}, /* WF_MCU_SYSRAM */
        {0x00410000, 0x490000, 0x10000}, /* WF_MCU_SYSRAM (config register) */
        {0x820f0000, 0x4a0000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_CFG) */
        {0x820f1000, 0x4a0600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_TRB) */
        {0x820f2000, 0x4a0800, 0x0400}, /* WF_LMAC_TOP BN1 (WF_AGG) */
        {0x820f3000, 0x4a0c00, 0x0400}, /* WF_LMAC_TOP BN1 (WF_ARB) */
        {0x820f4000, 0x4a1000, 0x0400}, /* WF_LMAC_TOP BN1 (WF_TMAC) */
        {0x820f5000, 0x4a1400, 0x0800}, /* WF_LMAC_TOP BN1 (WF_RMAC) */
        {0x820f7000, 0x4a1e00, 0x0200}, /* WF_LMAC_TOP BN1 (WF_DMA) */
        {0x820f9000, 0x4a3400, 0x0200}, /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
        {0x820fa000, 0x4a4000, 0x0200}, /* WF_LMAC_TOP BN1 (WF_ETBF) */
        {0x820fb000, 0x4a4200, 0x0400}, /* WF_LMAC_TOP BN1 (WF_LPON) */
        {0x820fc000, 0x4a4600, 0x0200}, /* WF_LMAC_TOP BN1 (WF_INT) */
        {0x820fd000, 0x4a4800, 0x0800}, /* WF_LMAC_TOP BN1 (WF_MIB) */
        {0x820c4000, 0x4a8000, 0x4000}, /* WF_LMAC_TOP (WF_UWTBL)  */
        {0x820b0000, 0x4ae000, 0x1000}, /* [APB2] WFSYS_ON */
        {0x80020000, 0x4b0000, 0x10000}, /* WF_TOP_MISC_OFF */
        {0x81020000, 0x4c0000, 0x10000}, /* WF_TOP_MISC_ON */
        {0x7c500000, 0x500000, 0x200000}, /* remap, for fwdl */
        {0x7c000000, 0x00000,  0x10000}, /* CONN_INFRA, conn_infra_on */
        {0x7c020000, 0x20000,  0x10000}, /* CONN_INFRA, wfdma */
        {0x7c050000, 0x50000,  0x10000}, /* CONN_INFRA, conn infra sysram */
        {0x7c060000, 0x60000,  0x10000}, /* CONN_INFRA, conn_host_csr_top */
        {0x0, 0x0, 0x0} /* End */
};

CONNAC_REGMAP soc5_0_bus2chip_cr_mapping[] = {
        /*
         * For example:
         * WF_MCU_BUS_CR_REMAP:
         *   {0x830c0000, 0x830c0FFF} => {0x18400000, 0x18400FFF}
         */
        /* chip addr, bus addr, range */
        {0x830c0000, 0x400000, 0x1000},    /* WF_MCU_BUS_CR_REMAP */
        {0x54000000, 0x402000, 0x1000},    /* WFDMA PCIE0 MCU DMA0 */
        {0x55000000, 0x403000, 0x1000},    /* WFDMA PCIE0 MCU DMA1 */
        {0x56000000, 0x404000, 0x1000},    /* WFDMA reserved */
        {0x57000000, 0x405000, 0x1000},    /* WFDMA MCU wrap CR */
        {0x58000000, 0x406000, 0x1000},    /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
        {0x59000000, 0x407000, 0x1000},    /* WFDMA PCIE1 MCU DMA1 */
        {0x820c0000, 0x408000, 0x4000},    /* WF_UMAC_TOP (PLE) */
        {0x820c8000, 0x40c000, 0x2000},    /* WF_UMAC_TOP (PSE) */
        {0x820cc000, 0x40e000, 0x2000},    /* WF_UMAC_TOP (PP) */
        {0x83000000, 0x410000, 0x10000},   /* WF_PHY_MAP3 */
        {0x820e0000, 0x420000, 0x0400},    /* WF_LMAC_TOP BN0 (WF_CFG) */
        {0x820e1000, 0x420400, 0x0200},    /* WF_LMAC_TOP BN0 (WF_TRB) */
        {0x820e2000, 0x420800, 0x0400},    /* WF_LMAC_TOP BN0 (WF_AGG) */
        {0x820e3000, 0x420c00, 0x0400},    /* WF_LMAC_TOP BN0 (WF_ARB) */
        {0x820e4000, 0x421000, 0x0400},    /* WF_LMAC_TOP BN0 (WF_TMAC) */
        {0x820e5000, 0x421400, 0x0800},    /* WF_LMAC_TOP BN0 (WF_RMAC) */
        {0x820ce000, 0x421c00, 0x0200},    /* WF_LMAC_TOP (WF_SEC) */
        {0x820e7000, 0x421e00, 0x0200},    /* WF_LMAC_TOP BN0 (WF_DMA) */
        {0x820cf000, 0x422000, 0x1000},    /* WF_LMAC_TOP (WF_PF) */
        {0x820e9000, 0x423400, 0x0200},    /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
        {0x820ea000, 0x424000, 0x0200},    /* WF_LMAC_TOP BN0 (WF_ETBF) */
        {0x820eb000, 0x424200, 0x0400},    /* WF_LMAC_TOP BN0 (WF_LPON) */
        {0x820ec000, 0x424600, 0x0200},    /* WF_LMAC_TOP BN0 (WF_INT) */
        {0x820ed000, 0x424800, 0x0800},    /* WF_LMAC_TOP BN0 (WF_MIB) */
        {0x820ca000, 0x426000, 0x2000},    /* WF_LMAC_TOP BN0 (WF_MUCOP) */
        {0x820d0000, 0x430000, 0x10000},   /* WF_LMAC_TOP (WF_WTBLON) */
        {0x83080000, 0x450000, 0x10000},   /* WF_PHY_MAP1 */
        {0x83090000, 0x460000, 0x10000},   /* WF_PHY_MAP2 */
        {0xE0200000, 0x470000, 0x10000},   /* WF_UMAC_SYSRAM */
        {0x00400000, 0x480000, 0x10000},   /* WF_MCU_SYSRAM */
        {0x00410000, 0x490000, 0x10000},   /* WF_MCU_SYSRAM (config register) */
        {0x820f0000, 0x4a0000, 0x0400},    /* WF_LMAC_TOP BN1 (WF_CFG) */
        {0x820f1000, 0x4a0600, 0x0200},    /* WF_LMAC_TOP BN1 (WF_TRB) */
        {0x820f2000, 0x4a0800, 0x0400},    /* WF_LMAC_TOP BN1 (WF_AGG) */
        {0x820f3000, 0x4a0c00, 0x0400},    /* WF_LMAC_TOP BN1 (WF_ARB) */
        {0x820f4000, 0x4a1000, 0x0400},    /* WF_LMAC_TOP BN1 (WF_TMAC) */
        {0x820f5000, 0x4a1400, 0x0800},    /* WF_LMAC_TOP BN1 (WF_RMAC) */
        {0x820f7000, 0x4a1e00, 0x0200},    /* WF_LMAC_TOP BN1 (WF_DMA) */
        {0x820f9000, 0x4a3400, 0x0200},    /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
        {0x820fa000, 0x4a4000, 0x0200},    /* WF_LMAC_TOP BN1 (WF_ETBF) */
        {0x820fb000, 0x4a4200, 0x0400},    /* WF_LMAC_TOP BN1 (WF_LPON) */
        {0x820fc000, 0x4a4600, 0x0200},    /* WF_LMAC_TOP BN1 (WF_INT) */
        {0x820fd000, 0x4a4800, 0x0800},    /* WF_LMAC_TOP BN1 (WF_MIB) */
        {0x820cc000, 0x4a5000, 0x2000},    /* WF_LMAC_TOP BN1 (WF_MUCOP) */
        {0x820c4000, 0x4a8000, 0x4000},    /* WF_LMAC_TOP BN1 (WF_MIB) */
        {0x820b0000, 0x4ae000, 0x1000},    /* [APB2] WFSYS_ON */
        {0x80020000, 0x4b0000, 0x10000},   /* WF_TOP_MISC_OFF */
        {0x81020000, 0x4c0000, 0x10000},   /* WF_TOP_MISC_ON */
        {0x80010000, 0x4d4000, 0x1000},    /* WF_AXIDMA */
        {0x83010000, 0x4e0000, 0x10000},   /* WF_PHY_MAP4 */
        {0x88000000, 0x4f0000, 0x10000},   /* WF_MCU_CFG_LS */
        {0x7c000000, 0x000000, 0x100000},  /* CONN_INFRA */
        {0x7c500000, 0x500000, 0x200000},  /* remap, for fwdl */
        {0x0, 0x0, 0x0} /* End */
};

CONNAC_REGMAP soc7_0_bus2chip_cr_mapping[] = {
        /*
         * For example:
         * WF_MCU_BUS_CR_REMAP:
         *   {0x830c0000, 0x830c0FFF} => {0x18400000, 0x18400FFF}
         */
        /* chip addr, bus addr, range */
        {0x830c0000, 0x400000, 0x1000},    /* WF_MCU_BUS_CR_REMAP */
        {0x54000000, 0x402000, 0x1000},    /* WFDMA PCIE0 MCU DMA0 */
        {0x55000000, 0x403000, 0x1000},    /* WFDMA PCIE0 MCU DMA1 */
        {0x56000000, 0x404000, 0x1000},    /* WFDMA reserved */
        {0x57000000, 0x405000, 0x1000},    /* WFDMA MCU wrap CR */
        {0x58000000, 0x406000, 0x1000},    /* WFDMA PCIE1 MCU DMA0 (MEM_DMA) */
        {0x59000000, 0x407000, 0x1000},    /* WFDMA PCIE1 MCU DMA1 */
        {0x820c0000, 0x408000, 0x4000},    /* WF_UMAC_TOP (PLE) */
        {0x820c8000, 0x40c000, 0x2000},    /* WF_UMAC_TOP (PSE) */
        {0x820cc000, 0x40e000, 0x2000},    /* WF_UMAC_TOP (PP) */
        {0x83000000, 0x410000, 0x10000},   /* WF_PHY_MAP3 */
        {0x820e0000, 0x420000, 0x0400},    /* WF_LMAC_TOP BN0 (WF_CFG) */
        {0x820e1000, 0x420400, 0x0200},    /* WF_LMAC_TOP BN0 (WF_TRB) */
        {0x820e2000, 0x420800, 0x0400},    /* WF_LMAC_TOP BN0 (WF_AGG) */
        {0x820e3000, 0x420c00, 0x0400},    /* WF_LMAC_TOP BN0 (WF_ARB) */
        {0x820e4000, 0x421000, 0x0400},    /* WF_LMAC_TOP BN0 (WF_TMAC) */
        {0x820e5000, 0x421400, 0x0800},    /* WF_LMAC_TOP BN0 (WF_RMAC) */
        {0x820ce000, 0x421c00, 0x0200},    /* WF_LMAC_TOP (WF_SEC) */
        {0x820e7000, 0x421e00, 0x0200},    /* WF_LMAC_TOP BN0 (WF_DMA) */
        {0x820cf000, 0x422000, 0x1000},    /* WF_LMAC_TOP (WF_PF) */
        {0x820e9000, 0x423400, 0x0200},    /* WF_LMAC_TOP BN0 (WF_WTBLOFF) */
        {0x820ea000, 0x424000, 0x0200},    /* WF_LMAC_TOP BN0 (WF_ETBF) */
        {0x820eb000, 0x424200, 0x0400},    /* WF_LMAC_TOP BN0 (WF_LPON) */
        {0x820ec000, 0x424600, 0x0200},    /* WF_LMAC_TOP BN0 (WF_INT) */
        {0x820ed000, 0x424800, 0x0800},    /* WF_LMAC_TOP BN0 (WF_MIB) */
        {0x820ca000, 0x426000, 0x2000},    /* WF_LMAC_TOP BN0 (WF_MUCOP) */
        {0x820d0000, 0x430000, 0x10000},   /* WF_LMAC_TOP (WF_WTBLON) */
        {0x83080000, 0x450000, 0x10000},   /* WF_PHY_MAP1 */
        {0x83090000, 0x460000, 0x10000},   /* WF_PHY_MAP2 */
        {0xE0200000, 0x470000, 0x10000},   /* WF_UMAC_SYSRAM */
        {0x00400000, 0x480000, 0x10000},   /* WF_MCU_SYSRAM */
        {0x00410000, 0x490000, 0x10000},   /* WF_MCU_SYSRAM (config register) */
        {0x820f0000, 0x4a0000, 0x0400},    /* WF_LMAC_TOP BN1 (WF_CFG) */
        {0x820f1000, 0x4a0600, 0x0200},    /* WF_LMAC_TOP BN1 (WF_TRB) */
        {0x820f2000, 0x4a0800, 0x0400},    /* WF_LMAC_TOP BN1 (WF_AGG) */
        {0x820f3000, 0x4a0c00, 0x0400},    /* WF_LMAC_TOP BN1 (WF_ARB) */
        {0x820f4000, 0x4a1000, 0x0400},    /* WF_LMAC_TOP BN1 (WF_TMAC) */
        {0x820f5000, 0x4a1400, 0x0800},    /* WF_LMAC_TOP BN1 (WF_RMAC) */
        {0x820f7000, 0x4a1e00, 0x0200},    /* WF_LMAC_TOP BN1 (WF_DMA) */
        {0x820f9000, 0x4a3400, 0x0200},    /* WF_LMAC_TOP BN1 (WF_WTBLOFF) */
        {0x820fa000, 0x4a4000, 0x0200},    /* WF_LMAC_TOP BN1 (WF_ETBF) */
        {0x820fb000, 0x4a4200, 0x0400},    /* WF_LMAC_TOP BN1 (WF_LPON) */
        {0x820fc000, 0x4a4600, 0x0200},    /* WF_LMAC_TOP BN1 (WF_INT) */
        {0x820fd000, 0x4a4800, 0x0800},    /* WF_LMAC_TOP BN1 (WF_MIB) */
        {0x820c4000, 0x4a8000, 0x4000},    /* WF_LMAC_TOP BN1 (WF_UWTBL) */
        {0x820b0000, 0x4ae000, 0x1000},    /* [APB2] WFSYS_ON */
        {0x80020000, 0x4b0000, 0x10000},   /* WF_TOP_MISC_OFF */
        {0x81020000, 0x4c0000, 0x10000},   /* WF_TOP_MISC_ON */
        {0x80010000, 0x4d4000, 0x1000},    /* WF_AXIDMA */
        {0x83010000, 0x4e0000, 0x10000},   /* WF_PHY_MAP4 */
        {0x88000000, 0x4f0000, 0x10000},   /* WF_MCU_CFG_LS */
        {0x7c000000, 0x000000, 0x100000},  /* CONN_INFRA */
        {0x7c500000, 0x500000, 0x200000},  /* remap, for fwdl */
        {0x0, 0x0, 0x0} /* End */
};

enum{
        _SUCCESS                                = 0,
        ERR_MISSING_ARG                         = 110,
        ERR_WRONG_FILES                         = 111,
        ERR_FAILED_REG_READ	                = 112
};

enum{
        UNKNOWN_HW                              = 0,
	HW_SOC2					= 1,
        HW_SOC3					= 2,
	HW_SOC5					= 3,
	HW_SOC7					= 4
};

enum{
        UNKNOWN_MODE                            = 0,
        MANUAL_MODE                             = 1,
        AUTOMATIC_MODE                          = 2
};

/** main ELF context, used for both 'automatic' and 'manual' dumping **/
typedef struct{
        Elf32_Ehdr *ELF_HEADER;
        Elf32_Phdr *ELF_REGIONS;
        void       *ELF_DATA;
        char       *ELF_NAME;
        int         ELF_OFFSET_COUNTER;
        int         ELF_DATA_COUNTER;
        int         ELF_COMMIT_FD;
}ELF_CTX;

typedef struct	    MTK_NDIS_TRANSPORT {
        uint32_t    NDISOIDCMD;
        uint32_t    INPUT_NDISOIDLENGTH;
        uint32_t    OUTPUT_NDISOIDLENGTH;
        uint8_t     NDISOID_CONTENT[16];
}MTK_NDIS_TRANSPORT;

typedef struct      MTK_IOCTL_PARAM_MCR_IO {
        uint32_t    MCR_OFFSET;
        uint32_t    MCR_DATA;
}MTK_IOCTL_PARAM_MCR_IO;


/* create the raw file for the raw dump ('manual' mode) **/
static signed int RAW_CREATE_FILE(ELF_CTX *CTX, unsigned char *CTX_ELF_FINAL_NAME){
        memset(CTX,             0x00,           sizeof(ELF_CTX));
        CTX->ELF_OFFSET_COUNTER                 = 0x00;
        /** the handmade dumped file always finish with "_raw" **/
        CTX->ELF_NAME                           = strcat(CTX_ELF_FINAL_NAME, "_raw");
        /** now create the final output file **/
        CTX->ELF_COMMIT_FD                      = open(CTX->ELF_NAME, O_CREAT | O_RDWR, 0777);
        close(CTX->ELF_COMMIT_FD);

        return 0;
}

/** generic function used for writing data into both the ELF and RAW file **/
static signed int ELF_COMMIT_FD(ELF_CTX                                *CTX,
                                void                                   *CTX_DATA,
                                unsigned int                            CTX_OFFSET_COUNTER,
                                unsigned int                            CTX_DATA_LEN){
        CTX->ELF_COMMIT_FD                                      = open(CTX->ELF_NAME, O_RDWR, 0777);
        lseek(CTX->ELF_COMMIT_FD, CTX_OFFSET_COUNTER, SEEK_SET);
        write(CTX->ELF_COMMIT_FD, CTX_DATA, CTX_DATA_LEN);
        close(CTX->ELF_COMMIT_FD);

        return 0;
}

/** main function which initialize the elf output, used for 'automatic' mode **/
static signed int INITIALIZE_ELF_OUTPUT(ELF_CTX                        *CTX,
                                        unsigned int                    CTX_REGION_NUMBER,
                                        unsigned int                    CTX_TOTAL_REGION_SIZE,
                                        unsigned char                  *CTX_ELF_FINAL_NAME,
                                        struct mt76_connac_reg_map     *CTX_REGION_MAP){
        memset(CTX,             0x00,           sizeof(ELF_CTX));
        CTX->ELF_HEADER                         = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
        memset(CTX->ELF_HEADER, 0x00,           sizeof(Elf32_Ehdr));
        CTX->ELF_REGIONS                        = (Elf32_Phdr *)calloc(CTX_REGION_NUMBER, sizeof(Elf32_Phdr));
        memset(CTX->ELF_REGIONS,0x00,           sizeof(Elf32_Phdr) * CTX_REGION_NUMBER);
        CTX->ELF_DATA                           = malloc(CTX_TOTAL_REGION_SIZE);
        CTX->ELF_NAME                           = CTX_ELF_FINAL_NAME;

        /** now create the final output file **/
        CTX->ELF_COMMIT_FD                      = open(CTX->ELF_NAME, O_CREAT | O_RDWR, 0777);
        /** file created, now just close it and returns with the 'normal' operations **/
        close(CTX->ELF_COMMIT_FD);

        /** populate the ELF header **/
        CTX->ELF_HEADER->e_ident[0]             = 0x7f;
        CTX->ELF_HEADER->e_ident[1]             = 'E';
        CTX->ELF_HEADER->e_ident[2]             = 'L';
        CTX->ELF_HEADER->e_ident[3]             = 'F';
        CTX->ELF_HEADER->e_ident[4]             = 0x01;
        CTX->ELF_HEADER->e_ident[5]             = 0x01;
        CTX->ELF_HEADER->e_ident[6]             = 0x01;

        #ifndef EM_NDS32
                #define EM_NDS32                167
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
        CTX->ELF_OFFSET_COUNTER                 = sizeof(Elf32_Ehdr) + CTX_REGION_NUMBER * sizeof(Elf32_Phdr);

        /** now iterate every region of mapped data and create a new region header **/
        for(unsigned char j = 0; j < CTX_REGION_NUMBER; j++){
                if( CTX->ELF_REGIONS[j].p_vaddr == 0x00 && CTX_REGION_MAP[j].size != 0x00 ){
                        CTX->ELF_REGIONS[j].p_type      = PT_LOAD;
                        CTX->ELF_REGIONS[j].p_vaddr     = CTX_REGION_MAP[j].phys;
                        CTX->ELF_REGIONS[j].p_paddr     = CTX_REGION_MAP[j].phys;
                        CTX->ELF_REGIONS[j].p_memsz     = CTX_REGION_MAP[j].size;
                        CTX->ELF_REGIONS[j].p_filesz    = CTX_REGION_MAP[j].size;
                        CTX->ELF_REGIONS[j].p_offset    = CTX->ELF_OFFSET_COUNTER;
                        CTX->ELF_REGIONS[j].p_flags     = PF_R | PF_W;
                        CTX->ELF_REGIONS[j].p_align     = 0x04;
                        #ifdef REGION_DEBUG
                        printf("added new region: %d 0x%x 0x%x %d %d\n", j, CTX->ELF_REGIONS[j].p_vaddr,
                                CTX->ELF_REGIONS[j].p_paddr, CTX->ELF_REGIONS[j].p_memsz, CTX->ELF_REGIONS[j].p_offset);
                        #endif
                        CTX->ELF_OFFSET_COUNTER        += CTX->ELF_REGIONS[j].p_memsz;
                }
        }

        #define EHDR_SZ                         sizeof(Elf32_Ehdr)
        /**  before quitting, reset 'CTX->ELF_OFFSET_COUNTER' **/
        memcpy(CTX->ELF_DATA    + 0x00,         CTX->ELF_HEADER,  EHDR_SZ);
        memcpy(CTX->ELF_DATA    + EHDR_SZ,      CTX->ELF_REGIONS, CTX_REGION_NUMBER * sizeof(Elf32_Phdr));
        CTX->ELF_OFFSET_COUNTER                 = EHDR_SZ    +    CTX_REGION_NUMBER * sizeof(Elf32_Phdr);

        CTX->ELF_DATA_COUNTER                   = CTX->ELF_OFFSET_COUNTER;
        ELF_COMMIT_FD(CTX,      CTX->ELF_DATA,  0x00, CTX->ELF_OFFSET_COUNTER);
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

__attribute__((hot)) static signed int MTK_GEN4M_IO(signed int SOCK_CONNECTION, uint32_t IO_OFFSET, ELF_CTX *CTX){
        signed int ret				= 0;
        struct iwreq wrq			= {};
        MTK_NDIS_TRANSPORT ndis_struct		= {};

        /* zeroize */
        memset(&wrq, 0, sizeof(struct iwreq));
        ndis_struct.NDISOIDCMD 			= OID_CUSTOM_MCR_RW;

        MTK_IOCTL_PARAM_MCR_IO *ptr_to_mcr 	= (struct MTK_IOCTL_PARAM_MCR_IO *)ndis_struct.NDISOID_CONTENT;
        ptr_to_mcr->MCR_OFFSET 			= IO_OFFSET;

        ndis_struct.INPUT_NDISOIDLENGTH 	= sizeof(MTK_IOCTL_PARAM_MCR_IO);
        ndis_struct.OUTPUT_NDISOIDLENGTH	= sizeof(MTK_IOCTL_PARAM_MCR_IO);

        /* configure struct iwreq */
        wrq.u.data.pointer 			= &ndis_struct;
        wrq.u.data.length 			= sizeof(MTK_NDIS_TRANSPORT);
        wrq.u.data.flags 			= PRIV_CMD_OID;

	/** "wlan0" should be the default interface, need to think if in future an additional option will be added for selecting
	     the interface from cmdline
	**/
        snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", "wlan0");

        ioctl(SOCK_CONNECTION, IOCTL_GET_STRUCT, &wrq);
	/** introduce a small delay, unsure on the timing range... **/
	usleep(50);
	ELF_COMMIT_FD(CTX, &ptr_to_mcr->MCR_DATA, CTX->ELF_OFFSET_COUNTER, sizeof(uint32_t));

        /** now update the final data buffer **/
        CTX->ELF_OFFSET_COUNTER                 += sizeof(uint32_t);

	return ptr_to_mcr->MCR_DATA;
}

/** main function **/
int main(int argc, char *argv[]){
        if( argc != 3 ){
                printf("[error] %s <hardware-model> <mode>\n", argv[0]);
                printf("\"<hardware-model>\" can be soc3, soc5 or soc7\n");
                printf("\"<mode>\" can be --interactive or --automatic\n");
                printf("        interactive means that the user can write the desired address to dump and see the results\n");
                printf("        automatic means that every memory area defined in the hardware\'s dependent struct are dumped\n");
                return -ERR_MISSING_ARG;
        }

	unsigned char HW_CHOOSE                 = UNKNOWN_HW;
        unsigned char MODE_CHOOSE               = UNKNOWN_MODE;

	signed   int  SOCK_CONNECTION		= 0x00;
        signed   int  EXCEPTION_ERROR           = 0x00;
        unsigned long MANUAL_MODE_ADDRESS       = 0x00;
        unsigned int  MANUAL_MODE_LEN           = 0x00;
        unsigned int  IO_COMMAND_TO_LONG        = 0x00;
	unsigned int  IO_READ_RES		= 0x00;

        unsigned int  TOTAL_ELF_REGION_SIZE     = 0x00;
        ELF_CTX       *CTX                      = NULL;
        CONNAC_REGMAP *CONNECTED_REGMAP         = NULL;
        unsigned long CONNECTED_REGMAP_SIZE     = 0x00;

        unsigned char IO_COMMAND[IO_COMMAND_LEN];
        unsigned char IO_MANUAL[IO_MANUAL_CMD_LEN];

	if( argv[1] == NULL || strlen(argv[1]) < 3 ){
		printf("[!] wrong <hardware> parameter!\n");
		return -ERR_MISSING_ARG;
	}

	if( argv[2] == NULL ){
		printf("[!] missing <mode> parameter!\n");
		return -ERR_MISSING_ARG;
	}

	if( strstr(argv[1], "soc2") != NULL ){
		HW_CHOOSE			= HW_SOC2;
		CONNECTED_REGMAP		= soc2_2x2_bus2chip_cr_mapping;
		CONNECTED_REGMAP_SIZE		= ARRAY_SIZE(soc2_2x2_bus2chip_cr_mapping);
        }else if( strstr(argv[1], "soc3") != NULL ){
                HW_CHOOSE                       = HW_SOC3;
                CONNECTED_REGMAP                = soc3_0_bus2chip_cr_mapping;
                CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(soc3_0_bus2chip_cr_mapping);
        }else if( strstr(argv[1], "soc5") != NULL ){
                HW_CHOOSE                       = HW_SOC5;
                CONNECTED_REGMAP                = soc5_0_bus2chip_cr_mapping;
                CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(soc5_0_bus2chip_cr_mapping);
        }else if( strstr(argv[1], "soc7") != NULL ){
                HW_CHOOSE                       = HW_SOC7;
                CONNECTED_REGMAP                = soc7_0_bus2chip_cr_mapping;
                CONNECTED_REGMAP_SIZE           = ARRAY_SIZE(soc7_0_bus2chip_cr_mapping);
	}

        if( strstr(argv[2], "automatic") != NULL ){
                printf("[*] entering automatic mode...\n");
                MODE_CHOOSE                     = AUTOMATIC_MODE;
        }else{
                printf("[*] entering manual mode...\n");
                MODE_CHOOSE                     = MANUAL_MODE;
        }

	CTX                                     = (ELF_CTX *)malloc(sizeof(ELF_CTX));

	/** before starting, open the socket **/
	SOCK_CONNECTION 			= socket(PF_INET, SOCK_DGRAM, 0);

        if( MODE_CHOOSE == AUTOMATIC_MODE ){
                /** first iteration: count the total size of the regions and their number **/
                for(unsigned char j = 0; j < CONNECTED_REGMAP_SIZE && CONNECTED_REGMAP[j].size != 0x00; j++){
                        TOTAL_ELF_REGION_SIZE          += CONNECTED_REGMAP[j].size;
                }

                /** before dumping everything, make sure to create the ELF which contains the dumped data **/
                INITIALIZE_ELF_OUTPUT(CTX, CONNECTED_REGMAP_SIZE, TOTAL_ELF_REGION_SIZE, argv[1], CONNECTED_REGMAP);

                for(unsigned char j = 0; j < CONNECTED_REGMAP_SIZE && CONNECTED_REGMAP[j].size != 0x00; j++){
                        for(unsigned int Z = 0x00; Z < CONNECTED_REGMAP[j].size; Z += 0x4){
                                IO_COMMAND_TO_LONG      = CONNECTED_REGMAP[j].phys + Z;
                                IO_READ_RES		= MTK_GEN4M_IO(SOCK_CONNECTION, IO_COMMAND_TO_LONG, CTX);
                                usleep(500);
                                printf("[region %d][0x%08x] 0x%08x" REGVAL_OUT_TERMINATOR, j, IO_COMMAND_TO_LONG, IO_READ_RES);
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
                        RAW_CREATE_FILE(CTX, argv[1]);
                        for(unsigned int Z = 0x00; Z < MANUAL_MODE_LEN; Z += 0x4){
                                IO_COMMAND_TO_LONG      = MANUAL_MODE_ADDRESS + Z;
				IO_READ_RES             = MTK_GEN4M_IO(SOCK_CONNECTION, IO_COMMAND_TO_LONG, CTX);
                                usleep(500);
                                printf("[0x%08x] 0x%08x\n", IO_COMMAND_TO_LONG, IO_READ_RES);
                        }
                        printf("[*] manual mode dump finished!\n");
                }
        }

        end:

	/** close the socket **/
	close(SOCK_CONNECTION);

        /** free the ELF context **/
        if( CTX != NULL ){
                FREE_ELF_OUTPUT(CTX);
        }

        printf("[!] returning with error %d\n", EXCEPTION_ERROR);
        return EXCEPTION_ERROR;
}
