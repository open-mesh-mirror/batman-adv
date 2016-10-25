@@
identifier netdev, fallback_net;
@@

 static struct net *batadv_getlink_net(const struct net_device *netdev,
				       struct net *fallback_net)
 {
+#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
+	return fallback_net;
+#else
 ...
+#endif
 }
