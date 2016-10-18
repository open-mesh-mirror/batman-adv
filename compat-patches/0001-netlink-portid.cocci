@@
struct netlink_notify *notify;
@@
-notify->portid
+netlink_notify_portid(notify)

@@
struct genl_info *info;
@@
-info->snd_portid
+genl_info_snd_portid(info)

@@
expression skb;
@@
-NETLINK_CB(skb).portid
+NETLINK_CB_PORTID(skb)
