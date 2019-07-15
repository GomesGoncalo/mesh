#include <interface/hard.h>
#include <interface/soft.h>

#include <main.h>
#include <types.h>
#include <utils/log.h>

#include <linux/notifier.h>
#include <linux/netdevice.h>

void mesh_hardif_release(struct kref *ref)
{
	struct mesh_hard_iface *hard_iface;

	hard_iface = container_of(ref, struct mesh_hard_iface, refcount);
	printk("[%s] deleted\n", hard_iface->net_dev->name);

	dev_put(hard_iface->net_dev);

	kfree_rcu(hard_iface, rcu);
}

void mesh_hardif_put(struct mesh_hard_iface *hard_iface)
{
	kref_put(&hard_iface->refcount, mesh_hardif_release);
}

struct mesh_hard_iface *mesh_hardif_get_by_netdev(struct net_device *net_dev)
{
	struct mesh_priv *ctx = NULL;
	struct net_device *upper_dev = NULL;
	struct mesh_hard_iface *hard_iface = NULL;

	rcu_read_lock();

	upper_dev = netdev_master_upper_dev_get_rcu(net_dev);

	if (!upper_dev)
		goto out;

	ctx = netdev_priv(upper_dev);

	if (!ctx)
		goto out;

	list_for_each_entry_rcu(hard_iface, &ctx->mesh_hardif_list, list) {
		if (hard_iface->net_dev == net_dev &&
		    kref_get_unless_zero(&hard_iface->refcount))
			goto out;
	}

	hard_iface = NULL;

out:
	rcu_read_unlock();
	return hard_iface;
}

struct mesh_hard_iface *mesh_hardif_create_interface(struct net_device *net_dev, struct net_device *soft_iface)
{
	struct mesh_hard_iface *hard_iface = NULL;

	ASSERT_RTNL();

	dev_hold(net_dev);

	if (netdev_master_upper_dev_link(net_dev, soft_iface, NULL, NULL, NULL))
		goto release_dev;

	hard_iface = kzalloc(sizeof(struct mesh_hard_iface), GFP_ATOMIC);
	if (!hard_iface)
		goto release_dev;

	hard_iface->net_dev = net_dev;
	hard_iface->soft_iface = soft_iface;

	kref_init(&hard_iface->refcount);

	return hard_iface;
//free_if:
//	kfree(hard_iface);
release_dev:
	dev_put(net_dev);
//out:
	return NULL;
}

static int  __release_slave(struct net_device *lower_dev, void *data) {
	struct net_device *net_dev = data;

	if (net_dev && net_dev->netdev_ops && net_dev->netdev_ops->ndo_del_slave)
		net_dev->netdev_ops->ndo_del_slave(net_dev, lower_dev);

	return 0;
}

static int mesh_hard_if_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct net_device *net_dev = netdev_notifier_info_to_dev(ptr);
	struct net_device *master_dev = NULL;
	struct mesh_priv *ctx = NULL;

	rcu_read_lock();

	if (!net_dev)
		goto done;

	if (!mesh_softif_valid(net_dev)) {
		master_dev = netdev_master_upper_dev_get_rcu(net_dev);
	} else
		master_dev = net_dev;

	if (!master_dev)
		goto done;

	ctx = netdev_priv(master_dev);

	if (!ctx)
		goto done;

	switch (event) {
	case NETDEV_UP:
	case NETDEV_REGISTER:
		mesh_dbg(ctx, MESH_DBG, "[%s]: %s", net_dev->name, netdev_cmd_to_name(event));
		break;

	case NETDEV_DOWN:
	case NETDEV_UNREGISTER:
		mesh_dbg(ctx, MESH_DBG, "[%s]: %s", net_dev->name, netdev_cmd_to_name(event));
		if (mesh_softif_valid(net_dev))
			netdev_walk_all_lower_dev_rcu(net_dev, __release_slave, net_dev);
		break;

	default:
		break;
	}

done:
	rcu_read_unlock();

	return NOTIFY_DONE;
}

struct notifier_block mesh_hard_if_notifier = {
	.notifier_call = mesh_hard_if_event,
};
