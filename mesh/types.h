#pragma once

/**
 * enum mesh_counters - indices for traffic counters
 */
enum mesh_counters {
	/** @MESH_CNT_TX: transmitted payload traffic packet counter */
	MESH_CNT_TX,

	/** @MESH_CNT_TX_BYTES: transmitted payload traffic bytes counter */
	MESH_CNT_TX_BYTES,

	/**
	 * @MESH_CNT_TX_DROPPED: dropped transmission payload traffic packet
	 *  counter
	 */
	MESH_CNT_TX_DROPPED,

	/** @MESH_CNT_RX: received payload traffic packet counter */
	MESH_CNT_RX,

	/** @MESH_CNT_RX_BYTES: received payload traffic bytes counter */
	MESH_CNT_RX_BYTES,

	/** @MESH_CNT_FORWARD: forwarded payload traffic packet counter */
	MESH_CNT_FORWARD,

	/**
	 * @MESH_CNT_FORWARD_BYTES: forwarded payload traffic bytes counter
	 */
	MESH_CNT_FORWARD_BYTES,

	/**
	 * @MESH_CNT_MGMT_TX: transmitted routing protocol traffic packet
	 *  counter
	 */
	MESH_CNT_MGMT_TX,

	/**
	 * @MESH_CNT_MGMT_TX_BYTES: transmitted routing protocol traffic bytes
	 *  counter
	 */
	MESH_CNT_MGMT_TX_BYTES,

	/**
	 * @MESH_CNT_MGMT_RX: received routing protocol traffic packet counter
	 */
	MESH_CNT_MGMT_RX,

	/**
	 * @MESH_CNT_MGMT_RX_BYTES: received routing protocol traffic bytes
	 *  counter
	 */
	MESH_CNT_MGMT_RX_BYTES,

	/** @MESH_CNT_FRAG_TX: transmitted fragment traffic packet counter */
	MESH_CNT_FRAG_TX,

	/**
	 * @MESH_CNT_FRAG_TX_BYTES: transmitted fragment traffic bytes counter
	 */
	MESH_CNT_FRAG_TX_BYTES,

	/** @MESH_CNT_FRAG_RX: received fragment traffic packet counter */
	MESH_CNT_FRAG_RX,

	/**
	 * @MESH_CNT_FRAG_RX_BYTES: received fragment traffic bytes counter
	 */
	MESH_CNT_FRAG_RX_BYTES,

	/** @MESH_CNT_FRAG_FWD: forwarded fragment traffic packet counter */
	MESH_CNT_FRAG_FWD,

	/**
	 * @MESH_CNT_FRAG_FWD_BYTES: forwarded fragment traffic bytes counter
	 */
	MESH_CNT_FRAG_FWD_BYTES,

	/**
	 * @MESH_CNT_TT_REQUEST_TX: transmitted tt req traffic packet counter
	 */
	MESH_CNT_TT_REQUEST_TX,

	/** @MESH_CNT_TT_REQUEST_RX: received tt req traffic packet counter */
	MESH_CNT_TT_REQUEST_RX,

	/**
	 * @MESH_CNT_TT_RESPONSE_TX: transmitted tt resp traffic packet
	 *  counter
	 */
	MESH_CNT_TT_RESPONSE_TX,

	/**
	 * @MESH_CNT_TT_RESPONSE_RX: received tt resp traffic packet counter
	 */
	MESH_CNT_TT_RESPONSE_RX,

	/**
	 * @MESH_CNT_TT_ROAM_ADV_TX: transmitted tt roam traffic packet
	 *  counter
	 */
	MESH_CNT_TT_ROAM_ADV_TX,

	/**
	 * @MESH_CNT_TT_ROAM_ADV_RX: received tt roam traffic packet counter
	 */
	MESH_CNT_TT_ROAM_ADV_RX,

	/** @MESH_CNT_NUM: number of traffic counters */
	MESH_CNT_NUM,
};

struct mesh_hard_iface {
	struct list_head list;
	struct net_device *net_dev;
	struct kref refcount;
	struct net_device *soft_iface;
	struct rcu_head rcu;
};

enum mesh_mesh_state {
	MESH_MESH_INACTIVE,
	MESH_MESH_ACTIVE,
};

struct proc_handle;
struct mesh_priv {
	atomic_t log_level;
	atomic_t mesh_state;
	struct net_device *soft_iface;
	struct proc_handle *proc;
	u64 __percpu *counters;
	struct list_head mesh_hardif_list;
};
