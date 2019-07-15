#include <interface/soft.h>
#include <interface/hard.h>

#include <utils/log.h>
#include <utils/proc.h>
#include <types.h>
#include <main.h>

#include <linux/etherdevice.h>

static const struct {
	const char name[ETH_GSTRING_LEN];
} mesh_counters_strings[] = {
	{ "tx" },
	{ "tx_bytes" },
	{ "tx_dropped" },
	{ "rx" },
	{ "rx_bytes" },
	{ "forward" },
	{ "forward_bytes" },
	{ "mgmt_tx" },
	{ "mgmt_tx_bytes" },
	{ "mgmt_rx" },
	{ "mgmt_rx_bytes" },
	{ "frag_tx" },
	{ "frag_tx_bytes" },
	{ "frag_rx" },
	{ "frag_rx_bytes" },
	{ "frag_fwd" },
	{ "frag_fwd_bytes" },
	{ "tt_request_tx" },
	{ "tt_request_rx" },
	{ "tt_response_tx" },
	{ "tt_response_rx" },
	{ "tt_roam_adv_tx" },
	{ "tt_roam_adv_rx" },
};

static void mesh_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	if (stringset == ETH_SS_STATS)
		memcpy(data, mesh_counters_strings,
		       sizeof(mesh_counters_strings));
}

static int mesh_get_sset_count(struct net_device *dev, int stringset)
{
	if (stringset == ETH_SS_STATS)
		return MESH_CNT_NUM;

	return -EOPNOTSUPP;
}

static u64 mesh_sum_counter(struct mesh_priv *ctx,  size_t idx)
{
	u64 *counters, sum = 0;
	int cpu;

	for_each_possible_cpu(cpu) {
		counters = per_cpu_ptr(ctx->counters, cpu);
		sum += counters[idx];
	}

	return sum;
}

static void mesh_get_ethtool_stats(struct net_device *dev,
				     struct ethtool_stats *stats, u64 *data)
{
	struct mesh_priv *ctx = netdev_priv(dev);
	int i;

	for (i = 0; i < MESH_CNT_NUM; i++)
		data[i] = mesh_sum_counter(ctx, i);
}

static void mesh_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, "Mesh", sizeof(info->driver));
	strlcpy(info->version, "N/A", sizeof(info->version));
	strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
	strlcpy(info->bus_info, "mesh", sizeof(info->bus_info));
}

static const struct ethtool_ops mesh_ethtool_ops = {
	.get_drvinfo = mesh_get_drvinfo,
	.get_link = ethtool_op_get_link,
	.get_strings = mesh_get_strings,
	.get_ethtool_stats = mesh_get_ethtool_stats,
	.get_sset_count = mesh_get_sset_count,
};

static int mesh_softif_write_handler(void *priv_data, struct seq_file *m)
{
	unsigned count = 0;
	struct mesh_priv *ctx = priv_data;
	struct mesh_hard_iface *hard_iface = NULL;

	seq_printf(m, "Mesh interface: %s\n", ctx->soft_iface->name);
	seq_printf(m, "\tStats:\n");

	#define __printstat(stat) \
		do { \
			seq_printf(m, "\t\t%s: %llu\n", # stat, mesh_sum_counter(ctx, stat)); \
		} while (0)

	__printstat(MESH_CNT_TX);
	__printstat(MESH_CNT_TX_BYTES);
	__printstat(MESH_CNT_TX_DROPPED);
	__printstat(MESH_CNT_RX);
	__printstat(MESH_CNT_RX_BYTES);
	#undef __printstat

	seq_printf(m, "\n\n");

	seq_printf(m, "Registered interfaces:\n");
	rcu_read_lock();

	list_for_each_entry_rcu(hard_iface, &ctx->mesh_hardif_list, list) {
		if (kref_get_unless_zero(&hard_iface->refcount)) {
			seq_printf(m, "\t%u - %s: \n", count, hard_iface->net_dev->name);
			mesh_hardif_put(hard_iface);
		}
	}

	rcu_read_unlock();

	return 0;
}

static int mesh_softif_init_late(struct net_device *dev)
{
	struct mesh_priv *ctx;
	size_t cnt_len = sizeof(u64) * MESH_CNT_NUM;

	ctx = netdev_priv(dev);
	ctx->soft_iface = dev;

	if (!(ctx->counters = __alloc_percpu(cnt_len, __alignof__(u64))))
		return -ENOMEM;

	if (!(ctx->proc = proc_register(dev->name, ctx, mesh_softif_write_handler)))
		return -ENOMEM;

	atomic_set(&ctx->log_level, 0);

	INIT_LIST_HEAD(&ctx->mesh_hardif_list);

	return 0;
}

static int mesh_interface_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static int mesh_interface_release(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static struct net_device_stats *mesh_interface_stats(struct net_device *dev)
{
	struct mesh_priv *ctx = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;

	stats->tx_packets = mesh_sum_counter(ctx, MESH_CNT_TX);
	stats->tx_bytes = mesh_sum_counter(ctx, MESH_CNT_TX_BYTES);
	stats->tx_dropped = mesh_sum_counter(ctx, MESH_CNT_TX_DROPPED);
	stats->rx_packets = mesh_sum_counter(ctx, MESH_CNT_RX);
	stats->rx_bytes = mesh_sum_counter(ctx, MESH_CNT_RX_BYTES);
	return stats;
}

static int mesh_interface_set_mac_addr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	ether_addr_copy(dev->dev_addr, addr->sa_data);

	return 0;
}

static int mesh_interface_change_mtu(struct net_device *dev, int new_mtu)
{
	/* check ranges */
	if (new_mtu < 68 || new_mtu > 1500)
		return -EINVAL;

	dev->mtu = new_mtu;

	return 0;
}

static void mesh_interface_set_rx_mode(struct net_device *dev)
{}

static netdev_tx_t mesh_interface_tx(struct sk_buff *skb,
				       struct net_device *soft_iface)
{
	return NETDEV_TX_OK;
}

static int mesh_softif_slave_add(struct net_device *dev,
				   struct net_device *slave_dev,
				   struct netlink_ext_ack *extack)
{
	struct mesh_hard_iface *hard_iface = NULL;
	struct mesh_priv *ctx = netdev_priv(dev);
	int ret = -EINVAL;

	hard_iface = mesh_hardif_get_by_netdev(slave_dev);
	if (hard_iface && hard_iface->soft_iface)
		goto out;
	else if (!hard_iface && !(hard_iface = mesh_hardif_create_interface(slave_dev, dev)))
		goto out;

	ret = 0;
	kref_get(&hard_iface->refcount);
	list_add_tail_rcu(&hard_iface->list, &ctx->mesh_hardif_list);

out:
	if (hard_iface)
		mesh_hardif_put(hard_iface);

	return ret;
}

static int mesh_softif_slave_del(struct net_device *dev,
				   struct net_device *slave_dev)
{
	struct mesh_hard_iface *hard_iface;
	int ret = -EINVAL;

	hard_iface = mesh_hardif_get_by_netdev(slave_dev);
	if (!hard_iface || hard_iface->soft_iface != dev)
		goto out;

	ret = 0;
	netdev_upper_dev_unlink(slave_dev, dev);
	list_del_rcu(&hard_iface->list);
	mesh_hardif_put(hard_iface);

out:
	if (hard_iface)
		mesh_hardif_put(hard_iface);

	return ret;
}

static const struct net_device_ops mesh_netdev_ops = {
	.ndo_init = mesh_softif_init_late,
	.ndo_open = mesh_interface_open,
	.ndo_stop = mesh_interface_release,
	.ndo_get_stats = mesh_interface_stats,
//	.ndo_vlan_rx_add_vid = mesh_interface_add_vid,
//	.ndo_vlan_rx_kill_vid = mesh_interface_kill_vid,
	.ndo_set_mac_address = mesh_interface_set_mac_addr,
	.ndo_change_mtu = mesh_interface_change_mtu,
	.ndo_set_rx_mode = mesh_interface_set_rx_mode,
	.ndo_start_xmit = mesh_interface_tx,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_add_slave = mesh_softif_slave_add,
	.ndo_del_slave = mesh_softif_slave_del,
};

bool mesh_softif_valid(struct net_device *dev)
{
	return dev->netdev_ops->ndo_start_xmit == mesh_netdev_ops.ndo_start_xmit;
}

static void mesh_softif_free(struct net_device *dev)
{
	struct mesh_priv *ctx = netdev_priv(dev);
	proc_unregister(ctx->proc);
	free_percpu(ctx->counters);
	ctx->counters = NULL;
	rcu_barrier();
	printk("[%s] deleted\n", dev->name);
}

static void mesh_softif_init_early(struct net_device *dev)
{
	ether_setup(dev);

	dev->netdev_ops = &mesh_netdev_ops;
	dev->needs_free_netdev = true;
	dev->priv_destructor = mesh_softif_free;
	dev->features |= NETIF_F_NETNS_LOCAL;
	dev->features |= NETIF_F_LLTX;
	dev->priv_flags |= IFF_NO_QUEUE;

	/* can't call min_mtu, because the needed variables
	 * have not been initialized yet
	 */
	dev->mtu = ETH_DATA_LEN;

	/* generate random address */
	eth_hw_addr_random(dev);

	dev->ethtool_ops = &mesh_ethtool_ops;
}

static void mesh_softif_destroy_netlink(struct net_device *soft_iface,
					  struct list_head *head)
{
	unregister_netdevice_queue(soft_iface, head);
}

struct rtnl_link_ops mesh_link_ops __read_mostly = {
	.kind		= "meshif",
	.priv_size	= sizeof(struct mesh_priv),
	.setup		= mesh_softif_init_early,
	.dellink	= mesh_softif_destroy_netlink,
};
