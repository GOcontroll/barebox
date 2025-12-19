// SPDX-License-Identifier: GPL-2.0

#include <io.h>
#include <common.h>
#include <debug_ll.h>
#include <mach/imx/debug_ll.h>
#include <firmware.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/sections.h>
#include <asm/barebox-arm.h>
#include <asm/barebox-arm-head.h>
#include <pbl/i2c.h>
#include <pbl/pmic.h>
#include <linux/sizes.h>
#include <mach/imx/atf.h>
#include <mach/imx/xload.h>
#include <mach/imx/esdctl.h>
#include <mach/imx/generic.h>
#include <mach/imx/imx8mp-regs.h>
#include <mach/imx/iomux-mx8mp.h>
#include <mach/imx/imx8m-ccm-regs.h>
#include <mach/imx/imx-gpio.h>
#include <mfd/pca9450.h>
#include <soc/imx8m/ddr.h>
#include <soc/fsl/fsl_udc.h>

extern char __dtb_z_imx8mp_tx8p_ml81_moduline_display_106_start[];
extern char __dtb_z_imx8mp_tx8p_ml81_moduline_display_107_start[];

#define IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK	GENMASK(18, 16)
#define ENET_QOS_RGMII_EN							BIT(21)
#define ENET_QOS_CLK_TX_CLK_SEL						BIT(20)
#define ENET_QOS_CLK_GEN_EN							BIT(19)
#define ENET_QOS_INTF_SEL_RMII						BIT(18)

#define UART_PAD_CTRL	MUX_PAD_CTRL(MX8MP_PAD_CTL_PUE | MX8MP_PAD_CTL_PE)

#define I2C_PAD_CTRL	MUX_PAD_CTRL(MX8MP_PAD_CTL_DSE6 | \
				     MX8MP_PAD_CTL_HYS | \
				     MX8MP_PAD_CTL_PUE | \
				     MX8MP_PAD_CTL_PE)

static void setup_uart(void)
{
	void __iomem *uart = IOMEM(MX8M_UART1_BASE_ADDR);

	imx8m_early_setup_uart_clock();

	imx8mp_setup_pad(MX8MP_PAD_UART1_TXD__UART1_DCE_TX | UART_PAD_CTRL);
	imx8mp_setup_pad(MX8MP_PAD_UART1_RXD__UART1_DCE_RX | UART_PAD_CTRL);
	imx8m_uart_setup(uart);

	pbl_set_putc(imx_uart_putc, uart);
	putc_ll('>');
}

static struct pmic_config pca9450_cfg[] = {
	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	{ PCA9450_BUCK123_DVS, 0x29 },
	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	{ PCA9450_BUCK1OUT_DVS0, 0x1C },
	{ PCA9450_BUCK1OUT_DVS1, 0x14 },
	/*
	 * Increase VDD_ARM and VDD_DRAM to 0.95V to avoid issues in case
	 * software after Barebox switches to the OD ARM frequency without
	 * reprogramming the PMIC first.
	 */
	{ PCA9450_BUCK2OUT_DVS0, 0x1C },
	{ PCA9450_BUCK3OUT_DVS0, 0x1C },
	{ PCA9450_BUCK1CTRL, 0x59 },
	/* set WDOG_B_CFG to cold reset */
	{ PCA9450_RESET_CTRL, 0xA1 },
};

static void karo_tx8p_ml81_power_init_board(void)
{
	struct pbl_i2c *i2c;

	imx8mp_setup_pad(MX8MP_PAD_I2C1_SCL__I2C1_SCL | I2C_PAD_CTRL);
	imx8mp_setup_pad(MX8MP_PAD_I2C1_SDA__I2C1_SDA | I2C_PAD_CTRL);

	imx8m_ccgr_clock_enable(IMX8M_CCM_CCGR_I2C1);

	i2c = imx8m_i2c_early_init(IOMEM(MX8MP_I2C1_BASE_ADDR));

	pmic_configure(i2c, 0x25, pca9450_cfg, ARRAY_SIZE(pca9450_cfg));
}

extern struct dram_timing_info tx8p_ml81_dram_timing;

int gocontroll_display_lowlevel(void) {
	imx8mp_cpu_lowlevel_init();

	relocate_to_current_adr();
	setup_c();

	setup_uart();

	/*
	 * If we are in EL3 we are running for the first time out of OCRAM,
	 * we'll need to initialize the DRAM and run TF-A (BL31). The TF-A
	 * will then jump to DRAM in EL2
	 */
	if (current_el() == 3) {
		void __iomem *gpio4 = IOMEM(MX8MP_GPIO4_BASE_ADDR);
		imx8mp_early_clock_init();

		/* set KEEP ALIVE */
		imx8mp_setup_pad(MX8MP_PAD_SAI1_RXD2__GPIO4_IO04);
		imx8m_gpio_direction_output(gpio4, 4, 1);

		karo_tx8p_ml81_power_init_board();

		imx8mp_ddr_init(&tx8p_ml81_dram_timing, DRAM_TYPE_LPDDR4);

		imx8mp_load_and_start_image_via_tfa();
	}
}

ENTRY_FUNCTION(start_gocontroll_display_106, r0, r1, r2)
{
	gocontroll_display_lowlevel();

	imx8mp_barebox_entry(__dtb_z_imx8mp_tx8p_ml81_moduline_display_106_start);
}

ENTRY_FUNCTION(start_gocontroll_display_107, r0, r1, r2)
{
	gocontroll_display_lowlevel();

	imx8mp_barebox_entry(__dtb_z_imx8mp_tx8p_ml81_moduline_display_107_start);
}
