#include <bootsource.h>
#include <common.h>
#include <deep-probe.h>
#include <envfs.h>
#include <globalvar.h>
#include <gpio.h>
#include <i2c/i2c.h>
#include <init.h>
#include <mach/imx/bbu.h>
#include <mach/imx/generic.h>

#define AV101_TOUCHSCREEN_ADDR 0x24
#define AV101_TOUCHSCREEN_RESET_PIN IMX_GPIO_NR(4,13)

extern char __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av101hdt_a10_start[];
extern char __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av123z7m_n17_start[];

static int moduline_display_apply_overlay(char * dtbo) {
	struct device_node *node;
	int ret;

	node = of_unflatten_dtb(dtbo, INT_MAX);
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

static int gocontroll_moduline_display_som_probe(struct device *dev)
{
	struct i2c_adapter * i2c4;
	struct i2c_client client;
	char *dtbo = NULL;
	int ret;
	uint8_t value;

	imx8m_bbu_internal_mmcboot_register_handler("emmc", "/dev/mmc0",
							BBU_HANDLER_FLAG_DEFAULT);
	defaultenv_append_directory(defaultenv_gocontroll_display);
	
	of_device_ensure_probed_by_alias("gpio3");
	of_device_ensure_probed_by_alias("i2c3");
	i2c4 = i2c_get_adapter(3);
	if (!i2c4) {
		pr_err("Could not get i2c4 adapter\n");
		return -ENODEV;
	}

	ret = gpio_direction_output(AV101_TOUCHSCREEN_RESET_PIN, 1);
	if (ret) {
		pr_err("av101 touchscreen reset: failed to request pin\n");
		return ret;
	}

	client.adapter = i2c4;
	client.addr = AV101_TOUCHSCREEN_ADDR;

	mdelay(10);

	ret = i2c_read_reg(&client, 0x00, &value, 1);

	if (ret < 0) {
		dtbo = __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av123z7m_n17_start;
		globalvar_set("of.overlay.pattern", "*av123*.dtbo");
	} else {
		dtbo = __dtbo_imx8mp_tx8p_ml81_moduline_display_106_av101hdt_a10_start;
		globalvar_set("of.overlay.pattern", "*av101*.dtbo");
	}

	ret = moduline_display_apply_overlay(dtbo);

	return ret;
}

static const struct of_device_id gocontroll_moduline_display_of_match[] = {
	{ .compatible = "gocontroll,moduline-display-106" },
	{ .compatible = "gocontroll,moduline-display-107" },
	{ /* sentinel */ },
};

static struct driver_d gocontroll_moduline_display_driver = {
	.name = "GOcontroll Moduline Display",
	.probe = gocontroll_moduline_display_som_probe,
	.of_compatible = DRV_OF_COMPAT(gocontroll_moduline_display_of_match),
};
coredevice_platform_driver(gocontroll_moduline_display_driver);

BAREBOX_DEEP_PROBE_ENABLE(gocontroll_moduline_display_of_match);
