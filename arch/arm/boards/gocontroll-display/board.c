#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <init.h>
#include <mach/imx/bbu.h>
#include <mach/imx/imx8mm-regs.h>
#include <mach/imx/iomux-mx8mm.h>
#include <envfs.h>


static int tx8p_ml81_som_probe(struct device_d *dev)
{
	imx8m_bbu_internal_mmcboot_register_handler("eMMC", "/dev/mmc0",
												BBU_HANDLER_FLAG_DEFAULT);
	defaultenv_append_directory(defaultenv_gocontroll_display);
	return 0;
}

static const struct of_device_id tx8p_ml81_of_match[] = {
	{ .compatible = "karo,imx8mp-tx8p-ml81" },
	{ /* sentinel */ },
};

static struct driver_d tx8p_ml81_som_driver = {
	.name = "karo-tx8p-ml81",
	.probe = tx8p_ml81_som_probe,
	.of_compatible = DRV_OF_COMPAT(tx8p_ml81_of_match),
};
coredevice_platform_driver(tx8p_ml81_som_driver);

BAREBOX_DEEP_PROBE_ENABLE(tx8p_ml81_of_match);
