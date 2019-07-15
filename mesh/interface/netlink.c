#include <utils/log.h>
#include <interface/netlink.h>

struct net;

struct genl_family mesh_netlink_family;

static const struct genl_multicast_group mesh_netlink_mcgrps[] = {};

static const struct nla_policy mesh_netlink_policy[] = {};

static const struct genl_ops mesh_netlink_ops[] = {};

static int mesh_pre_doit(const struct genl_ops *ops, struct sk_buff *skb,
			   struct genl_info *info)
{
	return 0;
}

static void mesh_post_doit(const struct genl_ops *ops, struct sk_buff *skb,
			     struct genl_info *info)
{}

struct genl_family mesh_netlink_family __ro_after_init = {
	.hdrsize = 0,
	.name = "meshif",
	.version = 1,
	.maxattr = 0,
	.policy = mesh_netlink_policy,
	.netnsok = true,
	.pre_doit = mesh_pre_doit,
	.post_doit = mesh_post_doit,
	.module = THIS_MODULE,
	.ops = NULL, //mesh_netlink_ops,
	.n_ops = 0, //ARRAY_SIZE(mesh_netlink_ops),
	.mcgrps = NULL, //mesh_netlink_mcgrps,
	.n_mcgrps = 0, //ARRAY_SIZE(mesh_netlink_mcgrps),
};

void __init mesh_netlink_register(void)
{
	int ret;

	ret = genl_register_family(&mesh_netlink_family);
	if (ret)
		pr_warn("unable to register netlink family");
}

void mesh_netlink_unregister(void)
{
	genl_unregister_family(&mesh_netlink_family);
}
