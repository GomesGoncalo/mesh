#pragma once

extern struct notifier_block mesh_hard_if_notifier;

struct net_device;

struct mesh_hard_iface *mesh_hardif_create_interface(struct net_device *, struct net_device *);
struct mesh_hard_iface *mesh_hardif_get_by_netdev(struct net_device *net_dev);
void mesh_hardif_put(struct mesh_hard_iface *hard_iface);
