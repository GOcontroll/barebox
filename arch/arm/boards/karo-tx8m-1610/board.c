#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <init.h>
#include <mach/imx/bbu.h>

static int tx8m_1610_som_probe(struct device_d *dev)
{
	int emmc_bbu_flag = 0;
        int sd_bbu_flag = 0;

        if (bootsource_get() == BOOTSOURCE_MMC && bootsource_get_instance() == 1) {
                of_device_enable_path("/chosen/environment-sd");
                sd_bbu_flag = BBU_HANDLER_FLAG_DEFAULT;
        } else {
                of_device_enable_path("/chosen/environment-emmc");
                emmc_bbu_flag = BBU_HANDLER_FLAG_DEFAULT;
        }

        imx8m_bbu_internal_mmcboot_register_handler("eMMC", "/dev/mmc0", emmc_bbu_flag);
        imx8m_bbu_internal_mmc_register_handler("SD", "/dev/mmc1.barebox", sd_bbu_flag);

        return 0;
}

static const struct of_device_id tx8m_1610_of_match[] = {
	{ .compatible = "karo,tx8m-1610" },
	{ /* sentinel */ },
};

static struct driver_d tx8m_1610_som_driver = {
	.name = "karo-tx8m-1610-test",
	.probe = tx8m_1610_som_probe,
	.of_compatible = DRV_OF_COMPAT(tx8m_1610_of_match),
};
coredevice_platform_driver(tx8m_1610_som_driver);
