/*
 * Implements driver for INVENSENSE MPU6050 Chip
 *
 * Copyright (C) 2011 Texas Instruments
 * Author: Kishore Kadiyala <kishore.kadiyala@ti.com>
 * Contributor: Leed Aguilar <leed.aguilar@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/input/mpu6050.h>
#include <linux/platform_device.h>

#include "mpu6050x.h"

#define MPU6050_DEBUG

#ifdef MPU6050_DEBUG
struct mpu6050_reg {
	u8 *name;
	uint8_t reg;
} mpu6050_regs[] = {
	{ "MPU6050_REG_SMPLRT_DIV", MPU6050_REG_SMPLRT_DIV },
	{ "MPU6050_REG_CONFIG", MPU6050_REG_CONFIG },
	{ "MPU6050_REG_GYRO_CONFIG", MPU6050_REG_GYRO_CONFIG },
	{ "MPU6050_REG_ACCEL_CONFIG", MPU6050_REG_ACCEL_CONFIG },
	{ "MPU6050_REG_ACCEL_FF_THR", MPU6050_REG_ACCEL_FF_THR },
	{ "MPU6050_REG_ACCEL_FF_DUR", MPU6050_REG_ACCEL_FF_DUR },
	{ "MPU6050_REG_ACCEL_MOT_THR", MPU6050_REG_ACCEL_MOT_THR },
	{ "MPU6050_REG_ACCEL_MOT_DUR", MPU6050_REG_ACCEL_MOT_DUR },
	{ "MPU6050_REG_ACCEL_ZRMOT_THR", MPU6050_REG_ACCEL_ZRMOT_THR },
	{ "MPU6050_REG_ACCEL_ZRMOT_DUR", MPU6050_REG_ACCEL_ZRMOT_DUR },
	{ "MPU6050_REG_FIFO_EN", MPU6050_REG_FIFO_EN },
	{ "MPU6050_REG_I2C_MST_CTRL", MPU6050_REG_I2C_MST_CTRL },
	{ "MPU6050_REG_I2C_SLV0_ADDR", MPU6050_REG_I2C_SLV0_ADDR },
	{ "MPU6050_REG_I2C_SLV0_REG", MPU6050_REG_I2C_SLV0_REG },
	{ "MPU6050_REG_I2C_SLV0_CTRL", MPU6050_REG_I2C_SLV0_CTRL },
	{ "MPU6050_REG_I2C_SLV1_ADDR", MPU6050_REG_I2C_SLV1_ADDR },
	{ "MPU6050_REG_I2C_SLV1_REG", MPU6050_REG_I2C_SLV1_REG },
	{ "MPU6050_REG_I2C_SLV1_CTRL", MPU6050_REG_I2C_SLV1_CTRL },
	{ "MPU6050_REG_I2C_SLV2_ADDR", MPU6050_REG_I2C_SLV2_ADDR },
	{ "MPU6050_REG_I2C_SLV2_REG", MPU6050_REG_I2C_SLV2_REG },
	{ "MPU6050_REG_I2C_SLV2_CTRL", MPU6050_REG_I2C_SLV2_CTRL },
	{ "MPU6050_REG_I2C_SLV3_ADDR", MPU6050_REG_I2C_SLV3_ADDR },
	{ "MPU6050_REG_I2C_SLV3_REG", MPU6050_REG_I2C_SLV3_REG },
	{ "MPU6050_REG_I2C_SLV3_CTRL", MPU6050_REG_I2C_SLV3_CTRL },
	{ "MPU6050_REG_I2C_SLV4_ADDR", MPU6050_REG_I2C_SLV4_ADDR },
	{ "MPU6050_REG_I2C_SLV4_REG", MPU6050_REG_I2C_SLV4_REG },
	{ "MPU6050_REG_I2C_SLV4_DO", MPU6050_REG_I2C_SLV4_DO },
	{ "MPU6050_REG_I2C_SLV4_CTRL", MPU6050_REG_I2C_SLV4_CTRL },
	{ "MPU6050_REG_I2C_SLV4_DI", MPU6050_REG_I2C_SLV4_DI },
	{ "MPU6050_REG_I2C_MST_STATUS", MPU6050_REG_I2C_MST_STATUS },
	{ "MPU6050_REG_INT_PIN_CFG", MPU6050_REG_INT_PIN_CFG },
	{ "MPU6050_REG_INT_ENABLE", MPU6050_REG_INT_ENABLE },
	{ "MPU6050_REG_INT_STATUS", MPU6050_REG_INT_STATUS },
	{ "MPU6050_REG_ACCEL_XOUT_H", MPU6050_REG_ACCEL_XOUT_H },
	{ "MPU6050_REG_ACCEL_XOUT_L", MPU6050_REG_ACCEL_XOUT_L },
	{ "MPU6050_REG_ACCEL_YOUT_H", MPU6050_REG_ACCEL_YOUT_H },
	{ "MPU6050_REG_ACCEL_YOUT_L", MPU6050_REG_ACCEL_YOUT_L },
	{ "MPU6050_REG_ACCEL_ZOUT_H", MPU6050_REG_ACCEL_ZOUT_H },
	{ "MPU6050_REG_ACCEL_ZOUT_L", MPU6050_REG_ACCEL_ZOUT_L },
	{ "MPU6050_REG_TEMP_OUT_H", MPU6050_REG_TEMP_OUT_H },
	{ "MPU6050_REG_TEMP_OUT_L", MPU6050_REG_TEMP_OUT_L },
	{ "MPU6050_REG_GYRO_XOUT_H", MPU6050_REG_GYRO_XOUT_H },
	{ "MPU6050_REG_GYRO_XOUT_L", MPU6050_REG_GYRO_XOUT_L },
	{ "MPU6050_REG_GYRO_YOUT_H", MPU6050_REG_GYRO_YOUT_H },
	{ "MPU6050_REG_GYRO_YOUT_L", MPU6050_REG_GYRO_YOUT_L },
	{ "MPU6050_REG_GYRO_ZOUT_H", MPU6050_REG_GYRO_ZOUT_H },
	{ "MPU6050_REG_GYRO_ZOUT_L", MPU6050_REG_GYRO_ZOUT_L },
	{ "MPU6050_REG_I2C_SLV0_DO", MPU6050_REG_I2C_SLV0_DO },
	{ "MPU6050_REG_I2C_SLV1_DO", MPU6050_REG_I2C_SLV1_DO },
	{ "MPU6050_REG_I2C_SLV2_DO", MPU6050_REG_I2C_SLV2_DO },
	{ "MPU6050_REG_I2C_SLV3_DO", MPU6050_REG_I2C_SLV3_DO },
	{ "MPU6050_REG_I2C_MST_DELAY_CTRL", MPU6050_REG_I2C_MST_DELAY_CTRL },
	{ "MPU6050_REG_SIGNAL_PATH_RESET", MPU6050_REG_SIGNAL_PATH_RESET },
	{ "MPU6050_REG_ACCEL_INTEL_CTRL", MPU6050_REG_ACCEL_INTEL_CTRL },
	{ "MPU6050_REG_USER_CTRL", MPU6050_REG_USER_CTRL },
	{ "MPU6050_REG_PWR_MGMT_1", MPU6050_REG_PWR_MGMT_1 },
	{ "MPU6050_REG_PWR_MGMT_2", MPU6050_REG_PWR_MGMT_2 },
	{ "MPU6050_REG_FIFO_COUNTH", MPU6050_REG_FIFO_COUNTH },
	{ "MPU6050_REG_FIFO_COUNTL", MPU6050_REG_FIFO_COUNTL },
	{ "MPU6050_REG_FIFO_R_W", MPU6050_REG_FIFO_R_W },
	{ "MPU6050_REG_WHOAMI", MPU6050_REG_WHOAMI },
};

static ssize_t mpu6050_registers_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mpu6050_data *data = platform_get_drvdata(pdev);
	unsigned i, n, reg_count;
	uint8_t value;

	reg_count = sizeof(mpu6050_regs) / sizeof(mpu6050_regs[0]);
	for (i = 0, n = 0; i < reg_count; i++) {
		MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
			mpu6050_regs[i].reg, 1, &value, mpu6050_regs[i].name);
		n += scnprintf(buf + n, PAGE_SIZE - n,
			       "%-20s = 0x%02X\n",
			       mpu6050_regs[i].name, value);
	}

	return n;
}

static ssize_t mpu6050_registers_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mpu6050_data *data = platform_get_drvdata(pdev);
	unsigned i, reg_count, value;
	char name[30];

	if (count >= 30) {
		pr_err("%s:input too long\n", __func__);
		return -1;
	}

	if (sscanf(buf, "%s %x", name, &value) != 2) {
		pr_err("%s:unable to parse input\n", __func__);
		return -1;
	}

	reg_count = sizeof(mpu6050_regs) / sizeof(mpu6050_regs[0]);
	for (i = 0; i < reg_count; i++) {
		if (!strcmp(name, mpu6050_regs[i].name)) {
			MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
				mpu6050_regs[i].reg, value,
				mpu6050_regs[i].name);
			return count;
		}
	}

	pr_err("%s:no such register %s\n", __func__, name);
	return -1;
}

static DEVICE_ATTR(registers, S_IWUSR | S_IRUGO,
		mpu6050_registers_show, mpu6050_registers_store);

static struct attribute *mpu6050_attrs[] = {
	&dev_attr_registers.attr,
	NULL
};

static const struct attribute_group mpu6050_attr_group = {
	.attrs = mpu6050_attrs,
};

#endif

static int mpu6050_enable_sleep(struct mpu6050_data *data, int action)
{
	unsigned char val = 0;

	MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1, 1, &val, "Enable Sleep");
	if (action)
		val |= MPU6050_DEVICE_SLEEP_MODE;
	else
		val &= ~MPU6050_DEVICE_SLEEP_MODE;

	MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1,	val, "Enable Sleep");

	MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1, 1, &val, "Enable Sleep");

	if (val & MPU6050_DEVICE_SLEEP_MODE)
		dev_err(data->dev, "Enable Sleep failed\n");
	return 0;
}

static int mpu6050_reset(struct mpu6050_data *data)
{
	unsigned char val = 0;
	int error;

	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1,	1, &val, "Reset");
	if (error) {
		dev_err(data->dev, "fail to read PWR_MGMT_1 reg\n");
		return error;
	}

	val &= ~MPU6050_CLOCKSEL_SRC_INT;
	val |= MPU6050_CLOCKSEL_SRC_PLL_X;
    error = MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1,	val, "PLL_X");
	if (error) {
		dev_err(data->dev, "fail to set reset bit\n");
		return error;
	}

	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
			MPU6050_REG_PWR_MGMT_1, 1, &val, "Reset");
	if (error) {
		dev_err(data->dev, "fail to read PWR_MGMT_1 reg\n");
		return error;
	}

    mpu6050_enable_sleep(data, 0);

	return 0;
}

static int mpu6050_set_dplf_mode(struct mpu6050_data *data)
{
	int error;
	unsigned char val = 0;
    dev_info(data->dev, "%s\n", __func__);

    /* dplf Configuration */
	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_CONFIG,
				1, &val, "MPU6050 register configuration");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to read REG_CONFIG register\n");
		return error;
	}

    val &= ~MPU6050_REG_CONFIG_DLPF;
    val |= MPU6050_BANDWIDTH_GYRO_42HZ_ACC_44HZ;
	error = MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_CONFIG,
				val, "MPU6050 register configuration");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to set dplf register\n");
		return error;
	}

	return 0;
}

static int mpu6050_set_bypass_mode(struct mpu6050_data *data)
{
	int error;
	unsigned char val = 0;

    dev_info(data->dev, "%s\n", __func__);
	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_I2C_MST_STATUS,
				1, &val, "Pass Through");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to read MST_STATUS register\n");
		return error;
	}
	val |= MPU6050_PASS_THROUGH;
	error = MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_I2C_MST_STATUS,
				val, "Pass Through");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to set pass through\n");
		return error;
	}

		/* Bypass Enable Configuration */
	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_INT_PIN_CFG,
				1, &val, "Bypass enabled");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to read INT_PIN_CFG register\n");
		return error;
	}

	val |= MPU6050_I2C_BYPASS_EN;
	error = MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_INT_PIN_CFG,
				val, "Bypass enabled");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to set i2c bypass mode\n");
		return error;
	}
	error = MPU6050_READ(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_USER_CTRL,
				1, &val, "I2C MST mode");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to read USER_CTRL register\n");
		return error;
	}
	val &= ~MPU6050_I2C_MST_EN;
	error = MPU6050_WRITE(data, MPU6050_CHIP_I2C_ADDR,
				MPU6050_REG_USER_CTRL,
				val, "I2C MST mode");
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to set i2c master enable bit\n");
		return error;
	}

	return 0;
}

int mpu6050_init(struct mpu6050_data *data, const struct mpu6050_bus_ops *bops)
{
	struct mpu6050_platform_data *pdata = data->client->dev.platform_data;
	int error = 0;

	/* Verify platform data */
	if (!pdata) {
		dev_err(&data->client->dev, "platform data not found\n");
		return -EINVAL;
	}

	/* if no IRQ return error */
	if (data->client->irq == 0) {
		dev_err(&data->client->dev, "Irq not set\n");
		return -EINVAL;
	}

	/* Initialize device data */
	data->dev = &data->client->dev;
	data->bus_ops = bops;
	data->pdata = pdata;
	data->irq = data->client->irq;
	mutex_init(&data->mutex);

	/* Reset MPU6050 device */
	error = mpu6050_reset(data);
	if (error) {
		dev_err(data->dev,
			"MPU6050: fail to reset device\n");
		return error;
	}

	/* Verify if pass-through mode is required */
	if (pdata->flags & MPU6050_PASS_THROUGH_EN) {
		error = mpu6050_set_bypass_mode(data);
		if (error) {
			dev_err(data->dev,
				"MPU6050: fail to set bypass mode\n");
			return error;
		}
	}
    else
    {
        error = mpu6050_set_dplf_mode(data);
        if (error) {
			dev_err(data->dev,
				"MPU6050: fail to set fsync mode\n");
			return error;
		}
    }

	/* Initializing built-in sensors on MPU6050 */
    if (IS_ERR(mpu6050_accel_gyro_init(data)))
    {
        dev_err(data->dev,
                "MPU6050: mpu6050_accel_gyro_init failed\n");
        if (!(pdata->flags & MPU6050_PASS_THROUGH_EN))
            return -ENODEV;
    }

#ifdef MPU6050_DEBUG
	error = sysfs_create_group(&data->client->dev.kobj, &mpu6050_attr_group);
#endif

	return error;
}
EXPORT_SYMBOL(mpu6050_init);

void mpu6050_exit(struct mpu6050_data *data)
{
#ifdef MPU6050_DEBUG
	sysfs_remove_group(&data->client->dev.kobj, &mpu6050_attr_group);
#endif

    mpu6050_enable_sleep(data, 1);

    mpu6050_accel_gyro_exit(data);
}
EXPORT_SYMBOL(mpu6050_exit);

MODULE_AUTHOR("Kishore Kadiyala <kishore.kadiyala@ti.com");
MODULE_DESCRIPTION("MPU6050 Driver");
MODULE_LICENSE("GPL");
