/*
 *  linux/drivers/mfd/mcp-core.c
 *
 *  Copyright (C) 2001 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 *  Generic MCP (Multimedia Communications Port) layer.  All MCP locking
 *  is solely held within this file.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <asm/dma.h>
#include <asm/system.h>

#include "mcp.h"

#define to_mcp(d)		container_of(d, struct mcp, attached_device)
#define to_mcp_driver(d)	container_of(d, struct mcp_driver, drv)

static int mcp_bus_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}

static int mcp_bus_probe(struct device *dev)
{
	struct mcp *mcp = to_mcp(dev);
	struct mcp_driver *drv = to_mcp_driver(dev->driver);

	return drv->probe(mcp);
}

static int mcp_bus_remove(struct device *dev)
{
	struct mcp *mcp = to_mcp(dev);
	struct mcp_driver *drv = to_mcp_driver(dev->driver);

	drv->remove(mcp);
	return 0;
}

static int mcp_bus_suspend(struct device *dev, pm_message_t state)
{
	struct mcp *mcp = to_mcp(dev);
	int ret = 0;

	if (dev->driver) {
		struct mcp_driver *drv = to_mcp_driver(dev->driver);

		ret = drv->suspend(mcp, state);
	}
	return ret;
}

static int mcp_bus_resume(struct device *dev)
{
	struct mcp *mcp = to_mcp(dev);
	int ret = 0;

	if (dev->driver) {
		struct mcp_driver *drv = to_mcp_driver(dev->driver);

		ret = drv->resume(mcp);
	}
	return ret;
}

static struct bus_type mcp_bus_type = {
	.name		= "mcp",
	.match		= mcp_bus_match,
	.suspend	= mcp_bus_suspend,
	.resume		= mcp_bus_resume,
};

/**
 *	mcp_set_telecom_divisor - set the telecom divisor
 *	@mcp: MCP interface structure
 *	@div: SIB clock divisor
 *
 *	Set the telecom divisor on the MCP interface.  The resulting
 *	sample rate is SIBCLOCK/div.
 */
void mcp_set_telecom_divisor(struct mcp *mcp, unsigned int div)
{
	spin_lock_irq(&mcp->lock);
	mcp->ops->set_telecom_divisor(mcp, div);
	spin_unlock_irq(&mcp->lock);
}
EXPORT_SYMBOL(mcp_set_telecom_divisor);

/**
 *	mcp_set_audio_divisor - set the audio divisor
 *	@mcp: MCP interface structure
 *	@div: SIB clock divisor
 *
 *	Set the audio divisor on the MCP interface.
 */
void mcp_set_audio_divisor(struct mcp *mcp, unsigned int div)
{
	spin_lock_irq(&mcp->lock);
	mcp->ops->set_audio_divisor(mcp, div);
	spin_unlock_irq(&mcp->lock);
}
EXPORT_SYMBOL(mcp_set_audio_divisor);

/**
 *	mcp_reg_write - write a device register
 *	@mcp: MCP interface structure
 *	@reg: 4-bit register index
 *	@val: 16-bit data value
 *
 *	Write a device register.  The MCP interface must be enabled
 *	to prevent this function hanging.
 */
void mcp_reg_write(struct mcp *mcp, unsigned int reg, unsigned int val)
{
	unsigned long flags;

	spin_lock_irqsave(&mcp->lock, flags);
	mcp->ops->reg_write(mcp, reg, val);
	spin_unlock_irqrestore(&mcp->lock, flags);
}
EXPORT_SYMBOL(mcp_reg_write);

/**
 *	mcp_reg_read - read a device register
 *	@mcp: MCP interface structure
 *	@reg: 4-bit register index
 *
 *	Read a device register and return its value.  The MCP interface
 *	must be enabled to prevent this function hanging.
 */
unsigned int mcp_reg_read(struct mcp *mcp, unsigned int reg)
{
	unsigned long flags;
	unsigned int val;

	spin_lock_irqsave(&mcp->lock, flags);
	val = mcp->ops->reg_read(mcp, reg);
	spin_unlock_irqrestore(&mcp->lock, flags);

	return val;
}
EXPORT_SYMBOL(mcp_reg_read);

/**
 *	mcp_enable - enable the MCP interface
 *	@mcp: MCP interface to enable
 *
 *	Enable the MCP interface.  Each call to mcp_enable will need
 *	a corresponding call to mcp_disable to disable the interface.
 */
void mcp_enable(struct mcp *mcp)
{
	spin_lock_irq(&mcp->lock);
	if (mcp->use_count++ == 0)
		mcp->ops->enable(mcp);
	spin_unlock_irq(&mcp->lock);
}
EXPORT_SYMBOL(mcp_enable);

/**
 *	mcp_disable - disable the MCP interface
 *	@mcp: MCP interface to disable
 *
 *	Disable the MCP interface.  The MCP interface will only be
 *	disabled once the number of calls to mcp_enable matches the
 *	number of calls to mcp_disable.
 */
void mcp_disable(struct mcp *mcp)
{
	unsigned long flags;

	spin_lock_irqsave(&mcp->lock, flags);
	if (--mcp->use_count == 0)
		mcp->ops->disable(mcp);
	spin_unlock_irqrestore(&mcp->lock, flags);
}
EXPORT_SYMBOL(mcp_disable);

static void mcp_release(struct device *dev)
{
	struct mcp *mcp = container_of(dev, struct mcp, attached_device);

	kfree(mcp);
}

struct mcp *mcp_host_alloc(struct device *parent, size_t size)
{
	struct mcp *mcp;

	mcp = kmalloc(sizeof(struct mcp) + size, GFP_KERNEL);
	if (mcp) {
		memset(mcp, 0, sizeof(struct mcp) + size);
		spin_lock_init(&mcp->lock);
		mcp->attached_device.parent = parent;
		mcp->attached_device.bus = &mcp_bus_type;
		mcp->attached_device.dma_mask = parent->dma_mask;
		mcp->attached_device.release = mcp_release;
		mcp->dev = &mcp->attached_device;
	}
	return mcp;
}
EXPORT_SYMBOL(mcp_host_alloc);

int mcp_host_register(struct mcp *mcp)
{
	strcpy(mcp->attached_device.bus_id, "mcp0");
	return device_register(&mcp->attached_device);
}
EXPORT_SYMBOL(mcp_host_register);

void mcp_host_unregister(struct mcp *mcp)
{
	device_unregister(&mcp->attached_device);
}
EXPORT_SYMBOL(mcp_host_unregister);

int mcp_driver_register(struct mcp_driver *mcpdrv)
{
	mcpdrv->drv.bus = &mcp_bus_type;
	mcpdrv->drv.probe = mcp_bus_probe;
	mcpdrv->drv.remove = mcp_bus_remove;
	return driver_register(&mcpdrv->drv);
}
EXPORT_SYMBOL(mcp_driver_register);

void mcp_driver_unregister(struct mcp_driver *mcpdrv)
{
	driver_unregister(&mcpdrv->drv);
}
EXPORT_SYMBOL(mcp_driver_unregister);

static int __init mcp_init(void)
{
	return bus_register(&mcp_bus_type);
}

static void __exit mcp_exit(void)
{
	bus_unregister(&mcp_bus_type);
}

module_init(mcp_init);
module_exit(mcp_exit);

MODULE_AUTHOR("Russell King <rmk@arm.linux.org.uk>");
MODULE_DESCRIPTION("Core multimedia communications port driver");
MODULE_LICENSE("GPL");
