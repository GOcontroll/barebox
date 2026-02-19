#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <init.h>
#include <mach/imx/bbu.h>
#include <mach/imx/imx8mm-regs.h>
#include <mach/imx/iomux-mx8mm.h>
#include <envfs.h>


static int gocontroll_moduline_headless_probe(struct device *dev)
{
	imx8m_bbu_internal_mmcboot_register_handler("emmc", "/dev/mmc0",
												BBU_HANDLER_FLAG_DEFAULT);
	defaultenv_append_directory(defaultenv_gocontroll_headless);
	return 0;
}

static const struct of_device_id gocontroll_moduline_headless_of_match[] = {
	{ .compatible = "gocontroll,moduline-iv-306-d" },
	{ .compatible = "gocontroll,moduline-mini-111" },
	{ /* sentinel */ },
};

static struct driver_d gocontroll_moduline_headless_driver = {
	.name = "GOcontroll Moduline Headless",
	.probe = gocontroll_moduline_headless_probe,
	.of_compatible = DRV_OF_COMPAT(gocontroll_moduline_headless_of_match),
};
coredevice_platform_driver(gocontroll_moduline_headless_driver);

BAREBOX_DEEP_PROBE_ENABLE(gocontroll_moduline_headless_of_match);
