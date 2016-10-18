@@
expression E;
@@

+#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,3,0)
 E->priv_flags |= IFF_NO_QUEUE;
+#else
+E->tx_queue_len = 0;
+#endif
