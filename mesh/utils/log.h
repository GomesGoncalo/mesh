#pragma once

#include <main.h>
#include <types.h>

#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/printk.h>

/**
 * enum mesh_dbg_level - available log levels
 */
enum mesh_dbg_level {
	/** Print regardless */
	MESH_DBG			= 0,

	/** @MESH_DBG_MODULE: Kernel related operations */
	MESH_DBG_MODULE		= BIT(0),

	/** @MESH_DBG_ALL: the union of all the above log levels */
	MESH_DBG_ALL		= 255,
};

#define _mesh_dbg(ctx, type, ratelimited, fmt, arg...)						\
		do {															\
			if ((!type || (atomic_read(&ctx->log_level) & (type))) && 	\
				(!(ratelimited) || net_ratelimit()))					\
				printk("%s" fmt "\n", ctx->soft_iface->name, ## arg);								\
		} while(0)

#define mesh_dbg(ctx, type, arg...)				_mesh_dbg(ctx, type, 0, ## arg)

#define mesh_dbg_ratelimited(ctx, type, arg...)	_mesh_dbg(ctx, type, 1, ## arg)
