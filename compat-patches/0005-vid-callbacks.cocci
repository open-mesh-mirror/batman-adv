@ add_assignment @
identifier batadv_interface_add_vid, batadv_netdev_ops;
@@

 struct net_device_ops batadv_netdev_ops = {
 	.ndo_vlan_rx_add_vid = batadv_interface_add_vid,
 };

@ kill_assignment @
identifier batadv_interface_kill_vid, batadv_netdev_ops;
@@

 struct net_device_ops batadv_netdev_ops = {
 	.ndo_vlan_rx_kill_vid = batadv_interface_kill_vid,
 };

@ add_vid @
identifier add_assignment.batadv_interface_add_vid;
type be16;
identifier dev, proto, vid;
@@

-batadv_interface_add_vid
+batadv_interface_add_vid_orig
			       (struct net_device *dev, be16 proto,
				unsigned short vid)
 { ... }

+static __vid_api_returntype batadv_interface_add_vid(struct net_device *dev,
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
+			   			      be16 proto,
+#endif
+			   			      unsigned short vid)
+{
+#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
+    be16 proto = htons(ETH_P_8021Q);
+#endif
+
+#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
+	batadv_interface_add_vid_orig(dev, proto, vid);
+#else
+	return batadv_interface_add_vid_orig(dev, proto, vid);
+#endif
+}


@ kill_vid @
identifier kill_assignment.batadv_interface_kill_vid;
type be16;
identifier dev, proto, vid;
@@

-batadv_interface_kill_vid
+batadv_interface_kill_vid_orig
			       (struct net_device *dev, be16 proto,
				unsigned short vid)
 { ... }

+static __vid_api_returntype batadv_interface_kill_vid(struct net_device *dev,
+#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
+			   			       be16 proto,
+#endif
+			   			       unsigned short vid)
+{
+#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
+    be16 proto = htons(ETH_P_8021Q);
+#endif
+
+#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
+	batadv_interface_kill_vid_orig(dev, proto, vid);
+#else
+	return batadv_interface_kill_vid_orig(dev, proto, vid);
+#endif
+}
