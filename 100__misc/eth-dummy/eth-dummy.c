// SPDX-License-Identifier: GPL-2.0+
/*
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define PLATFORM_DRIVER_NAME "lothars-eth-dummy"

struct eth_struct {
	int aaa;
	int bbb;
	struct net_device *dummy_ndev;
};

static int eth_open(struct net_device *ndev)
{
	pr_info("%s(): called\n", __func__);
	// now ready to accept transmit requests from the queueing
	// layer of the networking
	netif_start_queue(ndev);
	return 0;
}

static int eth_release(struct net_device *ndev)
{
	pr_info("%s(): called\n", __func__);
	netif_stop_queue(ndev);
	return 0;
}

static int eth_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	pr_info("%s(): called\n", __func__);
	ndev->stats.tx_bytes += skb->len;
	ndev->stats.tx_packets++;

	// timestamp the socketbuffer
	skb_tx_timestamp(skb);
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static int eth_init(struct net_device *dev)
{
	pr_info("%s(): called\n", __func__);
	return 0;
}

static const struct net_device_ops my_netdev_ops = {
	.ndo_init = eth_init,
	.ndo_open = eth_open,
	.ndo_stop = eth_release,
	.ndo_start_xmit = eth_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

static const struct of_device_id eth_dt_ids[] = {
	{ .compatible = "lothars,eth-dummy", },
	{ }
};

static int eth_probe(struct platform_device *pdev)
{
	int ret;
	struct eth_struct *priv;
	struct net_device *dummy_ndev;

	pr_info("%s(): called\n", __func__);
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		pr_err("%s(): failed to allocate memory\n", __func__);
		return -ENOMEM;
	}
	dummy_ndev = alloc_etherdev(sizeof(*priv));
	dummy_ndev->if_port = IF_PORT_10BASET;
	dummy_ndev->netdev_ops = &my_netdev_ops;

	// if needed, dev->ethtool_ops = &ethtool_ops;
	ret = register_netdev(dummy_ndev);
	if (ret) {
		pr_err("%s(): %d - failed to initialize card ...\n",
		       __func__, ret);
		return ret;
	}
	priv->dummy_ndev = dummy_ndev;
	platform_set_drvdata(pdev, priv);

	return 0;
}


static int eth_remove(struct platform_device *pdev)
{
	struct eth_struct *priv;

	pr_info("%s(): called\n", __func__);
	priv = platform_get_drvdata(pdev);
	unregister_netdev(priv->dummy_ndev);
	free_netdev(priv->dummy_ndev);

	return 0;
}

static struct platform_driver pf_drv = {
	.probe      = eth_probe,
	.remove     = eth_remove,
	.driver     = {
		.name     = PLATFORM_DRIVER_NAME,
		.of_match_table = of_match_ptr(eth_dt_ids),
		.owner    = THIS_MODULE,
	},
};
module_platform_driver(pf_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <l.rubusch@gmail.com>");
MODULE_DESCRIPTION("Messing with platform drivers.");
