#pragma once

#include <linux/types.h>
#include <net/rtnetlink.h>

extern struct rtnl_link_ops mesh_link_ops;

struct net_device *mesh_softif_create(struct net *net, const char *name);
bool mesh_softif_valid(struct net_device *dev);