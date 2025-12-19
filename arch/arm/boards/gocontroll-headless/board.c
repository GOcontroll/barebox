#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <init.h>
#include <mach/imx/bbu.h>
#include <mach/imx/imx8mm-regs.h>
#include <mach/imx/iomux-mx8mm.h>
#include <envfs.h>


static int tx8m_1610_som_probe(struct device_d *dev)
{
	imx8m_bbu_internal_mmcboot_register_handler("emmc", "/dev/mmc0",
												BBU_HANDLER_FLAG_DEFAULT);
	defaultenv_append_directory(defaultenv_gocontroll_headless);
	return 0;
}

static const struct of_device_id tx8m_1610_of_match[] = {
	{ .compatible = "karo,imx8mm-tx8m-1610" },
	{ /* sentinel */ },
};

static struct driver_d tx8m_1610_som_driver = {
	.name = "karo-tx8m-1610",
	.probe = tx8m_1610_som_probe,
	.of_compatible = DRV_OF_COMPAT(tx8m_1610_of_match),
};
coredevice_platform_driver(tx8m_1610_som_driver);

BAREBOX_DEEP_PROBE_ENABLE(tx8m_1610_of_match);
