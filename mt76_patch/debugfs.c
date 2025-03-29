// SPDX-License-Identifier: ISC
/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 */

#include <linux/kernel.h>
#include <linux/version.h>

#include "mt76.h"
#include "mt76_connac.h"
#include <linux/sched/clock.h>

/** custom modifications made by Edoardo Mantovani, 2025 **/

/** read the PC on mt7921 USB **/

unsigned char
usb_read_wifi_mcu_pc(struct mt76_dev *dev,  uint8_t ucPcLogSel, uint32_t *pu4RetVal)
{

        #define CONNAC2X_UDMA_BASE              	0x74000000
        #define CONNAC2X_UDMA_TX_QSEL           	(CONNAC2X_UDMA_BASE + 0x08) /* 0008 */
        #define CONNAC2X_UDMA_RESET             	(CONNAC2X_UDMA_BASE + 0x14) /* 0014 */
        #define CONNAC2X_UDMA_WLCFG_1           	(CONNAC2X_UDMA_BASE + 0x0C) /* 000c */
        #define CONNAC2X_UDMA_WLCFG_0           	(CONNAC2X_UDMA_BASE + 0x18) /* 0018 */

        /* For support mcu debug mechanism. +*/
        #define USB_CTRL_EN          	           	(1 << 31)
        #define CONNAC2X_UDMA_CONDBGCR_DATA     	(CONNAC2X_UDMA_BASE + 0xA18) /* 0A18 */
        #define CONNAC2X_UDMA_CONDBGCR_SEL      	(CONNAC2X_UDMA_BASE + 0xA1C) /* 0A1C */

	#define CONNAC2X_UDMA_MCU_PC_LOG_MASK   	(0x3F)
	#define CONNAC2X_UDMA_MCU_PC_LOG_SHIFT  	(16)

	#define PC_IDX_SWH(val, idx, mask, shift) 	((val & (~(mask << shift))) | ((mask & idx) << shift))

        uint32_t u4Val 		= 0x00;

        if (pu4RetVal == NULL){
                return 0x00;
	}

        u4Val			= __mt76_rr(dev, CONNAC2X_UDMA_CONDBGCR_SEL);

        u4Val 			= PC_IDX_SWH(u4Val, ucPcLogSel, CONNAC2X_UDMA_MCU_PC_LOG_MASK, CONNAC2X_UDMA_MCU_PC_LOG_SHIFT);

        __mt76_wr(dev, CONNAC2X_UDMA_CONDBGCR_SEL, u4Val);
        *pu4RetVal		= __mt76_rr(dev, CONNAC2X_UDMA_CONDBGCR_DATA);

        return 0x00;
}

/** read the PC on mt7921e **/
static uint8_t
pcie_read_wifi_mcu_pc(struct mt76_dev *dev, uint8_t ucPcLogSel, uint32_t *pu4RetVal)
{

	#define PCIE_CTRL_EN            (1 << 28)
	#define CONNAC2X_PCIE_CONDBGCR_CTRL             (0xE009C)
	#define CONNAC2X_PCIE_CONDBGCR_DATA             (0xE0204)
	#define CONNAC2X_PCIE_CONDBGCR_LR_DATA          (0xE0208)
	#define CONNAC2X_PCIE_CONDBGCR_SEL              (0xE0090)
	#define CONNAC2X_PCIE_MCU_PC_LOG_MASK   (0x3F)
	#define CONNAC2X_PCIE_MCU_PC_LOG_SHIFT  (2)
	#define CONNAC2X_PCIE_MCU_LR_LOG_MASK   (0x3F)
	#define CONNAC2X_PCIE_MCU_LR_LOG_SHIFT  (8)

        uint32_t u4Val            = 0x00;

        if (pu4RetVal == NULL){
                return 0x00;
        }

        u4Val                     = __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_SEL);
        u4Val                     = PC_IDX_SWH(u4Val, ucPcLogSel, CONNAC2X_PCIE_MCU_PC_LOG_MASK, CONNAC2X_PCIE_MCU_PC_LOG_SHIFT);
        __mt76_wr(dev, CONNAC2X_PCIE_CONDBGCR_SEL, u4Val);
        *pu4RetVal                = __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_DATA);

        return 0x1;
}

static u_int8_t
pcie_read_wifi_mcu_lr(struct mt76_dev *dev, uint8_t ucPcLogSel, uint32_t *pu4RetVal)
{
        uint32_t u4Val            = 0x00;

        if (pu4RetVal == NULL){
                return 0x00;
        }

        u4Val                     = __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_SEL);
        u4Val                     = PC_IDX_SWH(u4Val, ucPcLogSel, CONNAC2X_PCIE_MCU_LR_LOG_MASK, CONNAC2X_PCIE_MCU_LR_LOG_SHIFT);
        __mt76_wr(dev, CONNAC2X_PCIE_CONDBGCR_SEL, u4Val);
        *pu4RetVal                = __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_LR_DATA);

        return 0x1;
}


static int
mt76_cpu_state_get(struct seq_file *s, void *val)
{
	struct mt76_dev *dev 	= (struct mt76_dev *)dev_get_drvdata(s->private);
	uint32_t i   		= 0x00;
        uint32_t u4Val    	= 0x00;

	/** switch between the right device for dumping the PC, special registers and so on.. **/

	if( is_mt7915(dev) ){

	}

	if( is_mt7986(dev) ){	/** apply also to mt7981 **/

	}

	if( is_mt7921(dev) ){
		/** for mt7921/22 we need to choose if the device is USB or PCIe based. **/
		if( mt76_is_usb(dev) ){

			#define CONNAC2X_UDMA_BASE        	0x74000000
			#define CONNAC2X_UDMA_TX_QSEL     	(CONNAC2X_UDMA_BASE + 0x08) /* 0008 */
			#define CONNAC2X_UDMA_RESET       	(CONNAC2X_UDMA_BASE + 0x14) /* 0014 */
			#define CONNAC2X_UDMA_WLCFG_1     	(CONNAC2X_UDMA_BASE + 0x0C) /* 000c */
			#define CONNAC2X_UDMA_WLCFG_0     	(CONNAC2X_UDMA_BASE + 0x18) /* 0018 */

			/* For support mcu debug mechanism. +*/
			#define USB_CTRL_EN             	(1 << 31)
			#define CONNAC2X_UDMA_CONDBGCR_DATA     (CONNAC2X_UDMA_BASE + 0xA18) /* 0A18 */
			#define CONNAC2X_UDMA_CONDBGCR_SEL      (CONNAC2X_UDMA_BASE + 0xA1C) /* 0A1C */
			#define CONNAC2X_UDMA_WM_MONITER_SEL    (~(0x40000000))
			#define CONNAC2X_UDMA_PC_MONITER_SEL    (~(0x20000000))
			#define CONNAC2X_UDMA_LR_MONITER_SEL    (0x20000000)
			#define CONNAC2X_UDMA_MCU_PC_LOG_MASK   (0x3F)
			#define CONNAC2X_UDMA_MCU_PC_LOG_SHIFT  (16)
			/* For support mcu debug mechanism. -*/

			/* For support CFG_SUPPORT_DEBUG_SOP +*/
			#define CONNAC2X_UDMA_BT_DBG_STATUS     (CONNAC2X_UDMA_BASE + 0xA00) /* 0A00 */
			#define CONNAC2X_UDMA_BT_DBG_SEL        (CONNAC2X_UDMA_BASE + 0xA04) /* 0A04 */
			#define CONNAC2X_UDMA_DBG_STATUS        (CONNAC2X_UDMA_BASE + 0xA10) /* 0A10 */
			#define CONNAC2X_UDMA_DBG_SEL           (CONNAC2X_UDMA_BASE + 0xA14) /* 0A14 */

			#define CURRENT_PC 			0x3F
			#define PC_LOG_IDX 			0x20
			#define PC_LOG_NUM 			32

        		/* Enable USB mcu debug function. */
        		u4Val	= __mt76_rr(dev, CONNAC2X_UDMA_CONDBGCR_SEL);
        		u4Val  |= USB_CTRL_EN;
        		u4Val  &= CONNAC2X_UDMA_WM_MONITER_SEL;
        		u4Val  &= CONNAC2X_UDMA_PC_MONITER_SEL;
        		__mt76_wr(dev, CONNAC2X_UDMA_CONDBGCR_SEL, u4Val);

        		usb_read_wifi_mcu_pc(dev, CURRENT_PC, &u4Val);

        		seq_printf(s, "mt7921u: Current PC LOG: 0x%08x\n", u4Val);

                	for (i = 0; i < PC_LOG_NUM; i++) {
                        	usb_read_wifi_mcu_pc(dev, i, &u4Val);
                        	seq_printf(s, "mt7921u: PC log(%d)=0x%08x\n", i, u4Val);
                	}

                	/* Switch to LR. */
                	u4Val	= __mt76_rr(dev, CONNAC2X_UDMA_CONDBGCR_SEL);
                	u4Val  |= CONNAC2X_UDMA_LR_MONITER_SEL;
                	__mt76_wr(dev, CONNAC2X_UDMA_CONDBGCR_SEL, u4Val);

                	usb_read_wifi_mcu_pc(dev, PC_LOG_IDX, &u4Val);
                	seq_printf(s, "mt7921u: LR log contorl=0x%08x\n", u4Val);
                	for (i = 0; i < PC_LOG_NUM; i++) {
                        	usb_read_wifi_mcu_pc(dev, i, &u4Val);
                        	seq_printf(s, "mt7921u: LR log(%d)=0x%08x\n", i, u4Val);
                	}
        		/* Disable USB mcu debug function. */
        		u4Val	= __mt76_rr(dev, CONNAC2X_UDMA_CONDBGCR_SEL);
        		u4Val  &= ~USB_CTRL_EN;
        		__mt76_wr(dev, CONNAC2X_UDMA_CONDBGCR_SEL, u4Val);

		}else if( dev_is_pci(dev) ){	/** exclude SDIO based hardware **/

      			/* Enable PCIE mcu debug function. */
        		u4Val	= __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_CTRL);
        		u4Val  |= PCIE_CTRL_EN;
        		__mt76_wr(dev, CONNAC2X_PCIE_CONDBGCR_CTRL, u4Val);

        		pcie_read_wifi_mcu_pc(dev, CURRENT_PC, &u4Val);

        		seq_printf(s, "mt7921e: Current PC LOG: 0x%08x\n", u4Val);
            pcie_read_wifi_mcu_pc(dev, PC_LOG_IDX, &u4Val);
            seq_printf(s, "mt7921e: PC log contorl=0x%08x\n", u4Val);
            for (i = 0; i < PC_LOG_NUM; i++) {
                pcie_read_wifi_mcu_pc(dev, i, &u4Val);
                seq_printf(s, "mt7921e: PC log(%d)=0x%08x\n", i, u4Val);
            }
	                /* Read LR log. */
        	        pcie_read_wifi_mcu_lr(dev, PC_LOG_IDX, &u4Val);
                	seq_printf(s, "mt7921e: LR log contorl=0x%08x\n", u4Val);
                	for (i = 0; i < PC_LOG_NUM; i++) {
                        	pcie_read_wifi_mcu_lr(dev, i, &u4Val);
	                        seq_printf(s, "mt7921e: LR log(%d)=0x%08x\n", i, u4Val);
                	}

        		/* Disable PCIE mcu debug function. */
        		u4Val	= __mt76_rr(dev, CONNAC2X_PCIE_CONDBGCR_CTRL);
        		u4Val  &= ~PCIE_CTRL_EN;
        		__mt76_wr(dev, CONNAC2X_PCIE_CONDBGCR_CTRL, u4Val);
		}
	}

	if( is_mt7925(dev) ){
		#define CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_ADDR \
        		CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_ADDR
		#define CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_MASK \
        		0x00000007
		#define CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_SHFT \
        		0
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR \
        		CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_MASK \
        		0xFFFFFFFF
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_SHFT \
		        0
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_ADDR \
		        CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_ADDR
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_MASK \
	       	 	0x0000003F
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_SHFT \
      	  		0
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR \
	        	CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_MASK \
	        	0xFFFFFFFF
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_WF_MCU_GPR_BUS_DBGOUT_LOG_SHFT \
	        	0
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_ADDR \
	        	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_ADDR
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_MASK \
	        	0x0000003F
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_WF_MCU_DBG_PC_LOG_SEL_SHFT \
	        	0
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_ADDR \
        		CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_ADDR
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_MASK \
        		0xFFFFFFFF
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_SHFT \
        		0
		#define CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_ADDR \
        		CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_ADDR
		#define CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_MASK \
        		0x0000003F
		#define CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_SHFT \
        		0
		#define CONN_DBG_CTL_WF_CORE_PC_INDEX_FR_HIF_WF_CORE_PC_INDEX_FR_HIF_ADDR \
        		CONN_DBG_CTL_WF_CORE_PC_INDEX_FR_HIF_ADDR
		#define CONN_DBG_CTL_WF_CORE_PC_INDEX_FR_HIF_WF_CORE_PC_INDEX_FR_HIF_MASK \
        		0x00001FFF
		#define CONN_DBG_CTL_WF_CORE_PC_INDEX_FR_HIF_WF_CORE_PC_INDEX_FR_HIF_SHFT \
        		0
		#define CONN_INFRA_REMAPPING_OFFSET                    0x64000000
		#define CONN_DBG_CTL_BASE 							\
	        	(0x18023000 + CONN_INFRA_REMAPPING_OFFSET)
		#define CONN_DBG_CTL_CONN_INFRA_BUS_CLK_DETECT_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x000)
		#define CONN_DBG_CTL_CONN_INFRA_BUS_TIMEOUT_IRQ_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x400)
		#define CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_ADDR 					\
        		(CONN_DBG_CTL_BASE + 0x604)
		#define CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x608)
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_SEL_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x60C)
		#define CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_ADDR 					\
        		(CONN_DBG_CTL_BASE + 0x610)
		#define CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x614)
		#define CONN_DBG_CTL_WF_CORE_PC_INDEX_FR_HIF_ADDR 				\
        		(CONN_DBG_CTL_BASE + 0x620)
		#define CONN_DBG_CTL_WF_MCUSYS_INFRA_VDNR_GEN_DEBUG_CTRL_AO_DEBUGSYS_CTRL_ADDR 	\
       			(CONN_DBG_CTL_BASE + 0x628)
		#define CONN_DBG_CTL_WF_MCUSYS_INFRA_VDNR_GEN_DEBUG_CTRL_AO_BUS_TIMEOUT_IRQ_ADDR \
        		(CONN_DBG_CTL_BASE + 0x62C)
		#define CONN_DBG_CTL_WF_VON_DEBUG_OUT_ADDR 					\
		        (CONN_DBG_CTL_BASE + 0x638)

		#undef  PC_LOG_NUM
        	#define PC_LOG_NUM                      35
        	#define GPR_LOG_NUM                     35

		#define HAL_MCR_WR_FIELD(_prAdapter, _u4Offset, _u4FieldVal, _ucShft, _u4Mask) 	\
		{ 										\
        	uint32_t u4CrValue = 0; 							\
        	u4CrValue = __mt76_rr(_prAdapter, _u4Offset); 					\
        	u4CrValue &= (~_u4Mask); 							\
        	u4CrValue |= ((_u4FieldVal << _ucShft) & _u4Mask); 				\
        	__mt76_wr(_prAdapter, _u4Offset, u4CrValue); 					\
		}

		#define CPUPCR_LOG_NUM  5
		#define CPUPCR_BUF_SZ   50

        	uint32_t var_pc 	= 0;
        	uint32_t var_lp 	= 0;
        	uint64_t log_sec 	= 0;
        	uint64_t log_nsec 	= 0;
        	char log_buf_pc[CPUPCR_LOG_NUM][CPUPCR_BUF_SZ];
        	char log_buf_lp[CPUPCR_LOG_NUM][CPUPCR_BUF_SZ];

        	HAL_MCR_WR_FIELD(dev,
                	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_ADDR,
                	0x3F,
                	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_SHFT,
                	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_MASK);

		      HAL_MCR_WR_FIELD(dev,
                	CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_ADDR,
                	0x3F,
                	CONN_DBG_CTL_WF_MCU_DBG_GPR_LOG_SEL_WF_MCU_DBG_GPR_LOG_SEL_SHFT,
                	CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_WF_MCU_DBG_PC_LOG_MASK);

        	HAL_MCR_WR_FIELD(dev,
                	CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_ADDR,
                	0x0,
                	CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_SHFT,
                	CONN_DBG_CTL_WF_MCU_DBGOUT_SEL_WF_MCU_DBGOUT_SEL_MASK);

        	for (i = 0; i < CPUPCR_LOG_NUM; i++) {
                	log_sec 	= local_clock();
                	log_nsec 	= do_div(log_sec, 1000000000)/1000;

                	var_pc		= __mt76_rr(dev, CONN_DBG_CTL_WF_MCU_DBG_PC_LOG_ADDR);

                	var_lp		= __mt76_rr(dev, CONN_DBG_CTL_WF_MCU_GPR_BUS_DBGOUT_LOG_ADDR);

                	snprintf(log_buf_pc[i],
                        	CPUPCR_BUF_SZ,
                            	"%llu.%06llu/0x%08x;",
                            	log_sec,
                            	log_nsec,
                            	var_pc
			);

                	snprintf(log_buf_lp[i],
                            	CPUPCR_BUF_SZ,
                            	"%llu.%06llu/0x%08x;",
                            	log_sec,
                            	log_nsec,
                            	var_lp);
		}
       		seq_printf(s, "mt7925: wm pc=%s%s%s%s%s\n", log_buf_pc[0], log_buf_pc[1], log_buf_pc[2], log_buf_pc[3], log_buf_pc[4]);
		      seq_printf(s, "mt7925: wm lp=%s%s%s%s%s\n", log_buf_lp[0], log_buf_lp[1], log_buf_lp[2], log_buf_lp[3], log_buf_lp[4]);
	}
	return 0;
}

static int
mt76_reg_set(void *data, u64 val)
{
	struct mt76_dev *dev = data;

	__mt76_wr(dev, dev->debugfs_reg, val);
	return 0;
}

static int
mt76_reg_get(void *data, u64 *val)
{
	struct mt76_dev *dev = data;

	*val = __mt76_rr(dev, dev->debugfs_reg);
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_regval, mt76_reg_get, mt76_reg_set,
			 "0x%08llx\n");

static int
mt76_napi_threaded_set(void *data, u64 val)
{
	struct mt76_dev *dev = data;

	if (!mt76_is_mmio(dev))
		return -EOPNOTSUPP;

	#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 10, 0)
	if (dev->napi_dev->threaded != val)
	#else
	if (dev->napi_dev.threaded != val)
	#endif
		#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 10, 0)
		return dev_set_threaded(dev->napi_dev, val);
		#else
		return dev_set_threaded(&dev->napi_dev, val);
		#endif
	return 0;
}

static int
mt76_napi_threaded_get(void *data, u64 *val)
{
	struct mt76_dev *dev = data;

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(6, 10, 0)
	*val = dev->napi_dev.threaded;
	#else
	*val = dev->napi_dev->threaded;
	#endif
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(fops_napi_threaded, mt76_napi_threaded_get,
			 mt76_napi_threaded_set, "%llu\n");

int mt76_queues_read(struct seq_file *s, void *data)
{
	struct mt76_dev *dev = dev_get_drvdata(s->private);
	int i;

	seq_puts(s, "     queue | hw-queued |      head |      tail |\n");
	for (i = 0; i < ARRAY_SIZE(dev->phy.q_tx); i++) {
		struct mt76_queue *q = dev->phy.q_tx[i];

		if (!q)
			continue;

		seq_printf(s, " %9d | %9d | %9d | %9d |\n",
			   i, q->queued, q->head, q->tail);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(mt76_queues_read);

static int mt76_rx_queues_read(struct seq_file *s, void *data)
{
	struct mt76_dev *dev = dev_get_drvdata(s->private);
	int i, queued;

	seq_puts(s, "     queue | hw-queued |      head |      tail |\n");
	mt76_for_each_q_rx(dev, i) {
		struct mt76_queue *q = &dev->q_rx[i];

		queued = mt76_is_usb(dev) ? q->ndesc - q->queued : q->queued;
		seq_printf(s, " %9d | %9d | %9d | %9d |\n",
			   i, queued, q->head, q->tail);
	}

	return 0;
}

void mt76_seq_puts_array(struct seq_file *file, const char *str,
			 s8 *val, int len)
{
	int i;

	seq_printf(file, "%10s:", str);
	for (i = 0; i < len; i++)
		seq_printf(file, " %2d", val[i]);
	seq_puts(file, "\n");
}
EXPORT_SYMBOL_GPL(mt76_seq_puts_array);

struct dentry *
mt76_register_debugfs_fops(struct mt76_phy *phy,
			   const struct file_operations *ops)
{
	const struct file_operations *fops = ops ? ops : &fops_regval;
	struct mt76_dev *dev = phy->dev;
	struct dentry *dir;

	dir = debugfs_create_dir("mt76", phy->hw->wiphy->debugfsdir);
	debugfs_create_u8("led_pin", 0600, dir, &phy->leds.pin);
	debugfs_create_bool("led_active_low", 0600, dir, &phy->leds.al);
	debugfs_create_u32("regidx", 0600, dir, &dev->debugfs_reg);
	debugfs_create_file_unsafe("regval", 0600, dir, dev, fops);
	debugfs_create_file_unsafe("napi_threaded", 0600, dir, dev,
				   &fops_napi_threaded);

	debugfs_create_blob("eeprom", 0400, dir, &dev->eeprom);
	if (dev->otp.data)
		debugfs_create_blob("otp", 0400, dir, &dev->otp);
	debugfs_create_devm_seqfile(dev->dev, "rx-queues", dir,
				    mt76_rx_queues_read);

        debugfs_create_devm_seqfile(dev->dev, "mt76_show_pc", dir,
                        	    mt76_cpu_state_get);

	return dir;
}
EXPORT_SYMBOL_GPL(mt76_register_debugfs_fops);
