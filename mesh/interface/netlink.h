#pragma once

#include <main.h>

#include <linux/types.h>
#include <net/genetlink.h>

struct nlmsghdr;

void mesh_netlink_register(void);
void mesh_netlink_unregister(void);

extern struct genl_family mesh_netlink_family;