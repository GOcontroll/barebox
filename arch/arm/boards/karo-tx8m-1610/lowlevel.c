// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <debug_ll.h>
#include <mach/imx/debug_ll.h>
#include <asm/barebox-arm.h>
#include <pbl/i2c.h>
#include <pbl/pmic.h>
#include <mach/imx/esdctl.h>
#include <mach/imx/atf.h>
#include <mach/imx/generic.h>
#include <mach/imx/iomux-mx8mm.h>
#include <mach/imx/imx8m-ccm-regs.h>
#include <mfd/bd71837.h>
#include <mach/imx/xload.h>
#include <soc/imx8m/ddr.h>

#include "lowlevel.h"

extern char __dtb_z_imx8mm_tx8m_1610_test_start[];

#define UART_PAD_CTRL	MUX_PAD_CTRL(PAD_CTL_DSE_3P3V_45_OHM)

static void setup_uart(void)
{
	void __iomem *uart = IOMEM(MX8M_UART3_BASE_ADDR);

	imx8m_early_setup_uart_clock();

	imx8mm_setup_pad(IMX8MM_PAD_UART3_TXD_UART3_TX | UART_PAD_CTRL);
	imx8mm_setup_pad(IMX8MM_PAD_UART3_RXD_UART3_RX | UART_PAD_CTRL);
	imx8m_uart_setup(uart);

	pbl_set_putc(imx_uart_putc, uart);
	putc_ll('>');
}

static struct pmic_config bd71837_cfg[] = {
	/* decrease RESET key long push time from the default 10s to 10ms */
	{ BD718XX_PWRONCONFIG1, 0x0 },
	/* unlock the PMIC regs */
	{ BD718XX_REGLOCK, BD718XX_REGLOCK_PWRSEQ },

	{ BD718XX_1ST_NODVS_BUCK_CTRL, 0x00 },
	{ BD718XX_2ND_NODVS_BUCK_CTRL, 0x00 },
	{ BD718XX_3RD_NODVS_BUCK_CTRL, 0x00 },

	/* vdd soc 0.9v */
	{ BD718XX_BUCK1_VOLT_RUN, 0x14 },

	/* vdd arm 0.9v */
	{ BD718XX_BUCK2_VOLT_RUN, 0x0f },
	{ BD718XX_BUCK2_VOLT_IDLE, 0x0b },

	/* vdd dram 0.9v */
	{ BD718XX_1ST_NODVS_BUCK_VOLT, 0x02 },

	/* 3v3 */
	{ BD718XX_2ND_NODVS_BUCK_VOLT, 0x03 },

	/* 1v8 */
	{ BD718XX_3RD_NODVS_BUCK_VOLT, 0x03 },

	/* nvcc dram 1v35 */
	{ BD718XX_4TH_NODVS_BUCK_VOLT, 0x1E },

	{ BD718XX_BUCK1_CTRL, 0xc1 },
	{ BD718XX_BUCK2_CTRL, 0xc1 },

	{ BD718XX_1ST_NODVS_BUCK_CTRL, 0x01 },
	{ BD718XX_2ND_NODVS_BUCK_CTRL, 0x01 },
	{ BD718XX_3RD_NODVS_BUCK_CTRL, 0x01 },
	{ BD718XX_4TH_NODVS_BUCK_CTRL, 0x01 },

	{ BD718XX_LDO1_VOLT, 0x62 },
	{ BD718XX_LDO2_VOLT, 0x60 },
	{ BD718XX_LDO3_VOLT, 0x40 },
	{ BD718XX_LDO4_VOLT, 0x40 },
	{ BD718XX_LDO6_VOLT, 0x43 },

	{ BD718XX_REGLOCK, BD718XX_REGLOCK_VREG | BD718XX_REGLOCK_PWRSEQ },
};

void karo_tx8m_1610_power_init_board(void)
{
	struct pbl_i2c *i2c;

	imx8mm_setup_pad(IMX8MM_PAD_I2C1_SCL_I2C1_SCL);
	imx8mm_setup_pad(IMX8MM_PAD_I2C1_SDA_I2C1_SDA);

	imx8m_ccgr_clock_enable(IMX8M_CCM_CCGR_I2C1);

	i2c = imx8m_i2c_early_init(IOMEM(MX8MM_I2C1_BASE_ADDR));

	pmic_configure(i2c, 0x4b, bd71837_cfg, ARRAY_SIZE(bd71837_cfg));
}

ENTRY_FUNCTION(start_karo_tx8m_1610_test, r0, r1, r2)
{
	imx8mm_cpu_lowlevel_init();

	relocate_to_current_adr();
	setup_c();

	setup_uart();

	/*
	 * If we are in EL3 we are running for the first time out of OCRAM,
	 * we'll need to initialize the DRAM and run TF-A (BL31). The TF-A
	 * will then jump to DRAM in EL2
	 */
	if (current_el() == 3) {
		imx8mm_early_clock_init();

		karo_tx8m_1610_power_init_board();

		imx8mm_ddr_init(&tx8m_1610_dram_timing, DRAM_TYPE_DDR3);

		imx8mm_load_and_start_image_via_tfa();
	}

	/* Standard entry we hit once we initialized both DDR and ATF */
	imx8mm_barebox_entry(__dtb_z_imx8mm_tx8m_1610_test_start);
}
