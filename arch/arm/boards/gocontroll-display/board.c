#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <init.h>
#include <mach/imx/bbu.h>
#include <mach/imx/imx8mm-regs.h>
#include <mach/imx/iomux-mx8mm.h>
#include <envfs.h>

extern char __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av101hdt_a10_start[];
extern char __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av123z7m_n17_start[];

static int tx8p_ml81_som_probe(struct device_d *dev)
{
	struct device_node *node;
	int ret;
	imx8m_bbu_internal_mmcboot_register_handler("eMMC", "/dev/mmc0",
						    BBU_HANDLER_FLAG_DEFAULT);
	defaultenv_append_directory(defaultenv_gocontroll_display);

	node = of_unflatten_dtb(__dtbo_imx8mp_tx8p_ml81_moduline_display_106_av101hdt_a10_start, INT_MAX);
	if (IS_ERR(node)) {
		pr_err("Cannot unflatten dtbo\n");
		return PTR_ERR(node);
	}

	ret = of_overlay_apply_tree(of_get_root_node(), node);

	of_delete_node(node);

	if (ret) {
		pr_err("Cannot apply overlay: %pe\n", ERR_PTR(ret));
		return ret;
	}

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
