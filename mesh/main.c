#include <main.h>
#include <utils/log.h>

#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>

#include <linux/genetlink.h>
#include <net/rtnetlink.h>

#include <interface/hard.h>
#include <interface/soft.h>
#include <interface/netlink.h>

static int __init mesh_init(void)
{
	register_netdevice_notifier(&mesh_hard_if_notifier);
	rtnl_link_register(&mesh_link_ops);
	mesh_netlink_register();

	return 0;
}

static void __exit mesh_exit(void)
{
	mesh_netlink_unregister();
	rtnl_link_unregister(&mesh_link_ops);
	unregister_netdevice_notifier(&mesh_hard_if_notifier);
}

module_init(mesh_init);
module_exit(mesh_exit);

MODULE_AUTHOR(MESH_DRIVER_AUTHOR);
MODULE_DESCRIPTION(MESH_DRIVER_DESC);
MODULE_LICENSE("GPL");

MODULE_ALIAS_RTNL_LINK("meshif");
MODULE_ALIAS_GENL_FAMILY("meshif");