// SPDX-License-Identifier: GPL-2.0
/*
 * Intel(R) Trace Hub Memory Storage Unit (MSU) data structures
 *
 * Copyright (C) 2014-2015 Intel Corporation.
 */

#ifndef __INTEL_TH_MSU_H__
#define __INTEL_TH_MSU_H__

#include "intel_th.h"

enum {
	REG_MSU_MSUPARAMS	= 0x0000,
	REG_MSU_MSUSTS		= 0x0008,
	REG_MSU_MSC0CTL		= 0x0100, /* MSC0 control */
	REG_MSU_MSC0STS		= 0x0104, /* MSC0 status */
	REG_MSU_MSC0BAR		= 0x0108, /* MSC0 output base address */
	REG_MSU_MSC0SIZE	= 0x010c, /* MSC0 output size */
	REG_MSU_MSC0MWP		= 0x0110, /* MSC0 write pointer */
	REG_MSU_MSC0NWSA	= 0x011c, /* MSC0 next window start address */

	REG_MSU_MSC1CTL		= 0x0200, /* MSC1 control */
	REG_MSU_MSC1STS		= 0x0204, /* MSC1 status */
	REG_MSU_MSC1BAR		= 0x0208, /* MSC1 output base address */
	REG_MSU_MSC1SIZE	= 0x020c, /* MSC1 output size */
	REG_MSU_MSC1MWP		= 0x0210, /* MSC1 write pointer */
	REG_MSU_MSC1NWSA	= 0x021c, /* MSC1 next window start address */
};

/* MSUSTS bits */
#define MSUSTS_MSU_INT	BIT(0)

/* MSCnCTL bits */
#define MSC_EN		BIT(0)
#define MSC_WRAPEN	BIT(1)
#define MSC_RD_HDR_OVRD	BIT(2)
#define MSC_MODE	(BIT(4) | BIT(5))
#define MSC_LEN		(BIT(8) | BIT(9) | BIT(10))

/* MSC operating modes (MSC_MODE) */
enum {
	MSC_MODE_SINGLE	= 0,
	MSC_MODE_MULTI,
	MSC_MODE_EXI,
	MSC_MODE_DEBUG,
};

/* MSCnSTS bits */
#define MSCSTS_WRAPSTAT	BIT(1)	/* Wrap occurred */
#define MSCSTS_PLE	BIT(2)	/* Pipeline Empty */

/*
 * Multiblock/multiwindow block descriptor
 */
struct msc_block_desc {
	u32	sw_tag;
	u32	block_sz;
	u32	next_blk;
	u32	next_win;
	u32	res0[4];
	u32	hw_tag;
	u32	valid_dw;
	u32	ts_low;
	u32	ts_high;
	u32	res1[4];
} __packed;

#define MSC_BDESC	sizeof(struct msc_block_desc)
#define DATA_IN_PAGE	(PAGE_SIZE - MSC_BDESC)

/* MSC multiblock sw tag bits */
#define MSC_SW_TAG_LASTBLK	BIT(0)
#define MSC_SW_TAG_LASTWIN	BIT(1)

/* MSC multiblock hw tag bits */
#define MSC_HW_TAG_TRIGGER	BIT(0)
#define MSC_HW_TAG_BLOCKWRAP	BIT(1)
#define MSC_HW_TAG_WINWRAP	BIT(2)
#define MSC_HW_TAG_ENDBIT	BIT(3)

static inline unsigned long msc_data_sz(struct msc_block_desc *bdesc)
{
	if (!bdesc->valid_dw)
		return 0;

	return bdesc->valid_dw * 4 - MSC_BDESC;
}

static inline bool msc_block_wrapped(struct msc_block_desc *bdesc)
{
	if (bdesc->hw_tag & MSC_HW_TAG_BLOCKWRAP)
		return true;

	return false;
}

static inline bool msc_block_last_written(struct msc_block_desc *bdesc)
{
	if ((bdesc->hw_tag & MSC_HW_TAG_ENDBIT) ||
	    (msc_data_sz(bdesc) != DATA_IN_PAGE))
		return true;

	return false;
}

/* waiting for Pipeline Empty bit(s) to assert for MSC */
#define MSC_PLE_WAITLOOP_DEPTH	10000

/* API */
struct msc_probe_rem_cb {
	void (*probe)(struct intel_th_device *thdev);
	void (*remove)(struct intel_th_device *thdev);
};

int msc_register_callbacks(struct msc_probe_rem_cb cbs);
void msc_unregister_callbacks(void);
unsigned int msc_max_blocks(struct intel_th_device *thdev);
unsigned int msc_block_max_size(struct intel_th_device *thdev);
int msc_switch_window(struct intel_th_device *thdev);
int msc_sg_oldest_win(struct intel_th_device *thdev,
		      struct scatterlist *sg_array);
int msc_current_win_bytes(struct intel_th_device *thdev);

#endif /* __INTEL_TH_MSU_H__ */
