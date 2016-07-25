#! /usr/bin/env python3
# -*- coding: utf-8; -*-

# might only work with doxygen 1.8.11-3

import sys
import re

from collections import defaultdict

graph_config = '''
edge [fontname="Helvetica",fontsize="10",labelfontname="Helvetica",labelfontsize="10"];
node [fontname="Helvetica",fontsize="10",shape=record];
'''


def edge2str(f, l, t):
    return "%s-%s-%s" % (f, l, t)


EDGE_DEFAULT = 0
EDGE_NOPOINTER = 1
EDGE_KREF = 2
EDGE_NOKREF = 3
EDGE_NOKREF_OK = 4
EDGE_SINGLE_LOCK = 5

edge_type = {
    edge2str('batadv_priv', 'tp_list', 'batadv_tp_vars'): EDGE_KREF,
    edge2str('batadv_tp_vars', 'bat_priv', 'batadv_priv'): EDGE_NOKREF_OK,
    edge2str('batadv_tp_vars', 'unacked_list', 'batadv_tp_unacked'): EDGE_SINGLE_LOCK,
    edge2str('batadv_priv', 'tvlv', 'batadv_priv_tvlv'): EDGE_NOPOINTER,
    edge2str('batadv_priv_tvlv', 'container_list', 'batadv_tvlv_container'): EDGE_KREF,
    edge2str('batadv_priv_tvlv', 'handler_list', 'batadv_tvlv_handler'): EDGE_KREF,
    edge2str('batadv_priv', 'gw', 'batadv_priv_gw'): EDGE_NOPOINTER,
    edge2str('batadv_priv_gw', 'curr_gw', 'batadv_gw_node'): EDGE_KREF,
    edge2str('batadv_priv_gw', 'gateway_list', 'batadv_gw_node'): EDGE_KREF,
    edge2str('batadv_gw_node', 'orig_node', 'batadv_orig_node'): EDGE_KREF,
    edge2str('batadv_orig_node', 'last_bonding_candidate', 'batadv_orig_ifinfo'): EDGE_KREF,
    edge2str('batadv_orig_ifinfo', 'if_outgoing', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_hard_iface', 'neigh_list', 'batadv_hardif_neigh_node'): EDGE_NOKREF,
    edge2str('batadv_hardif_neigh_node', 'if_incoming', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_hard_iface', 'bat_v', 'batadv_hard_iface_bat_v'): EDGE_NOPOINTER,
    edge2str('batadv_hard_iface', 'bat_iv', 'batadv_hard_iface_bat_iv'): EDGE_NOPOINTER,
    edge2str('batadv_orig_ifinfo', 'router', 'batadv_neigh_node'): EDGE_KREF,
    edge2str('batadv_neigh_node', 'hardif_neigh', 'batadv_hardif_neigh_node'): EDGE_KREF,
    edge2str('batadv_neigh_node', 'if_incoming', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_neigh_node', 'ifinfo_list', 'batadv_neigh_ifinfo'): EDGE_KREF,
    edge2str('batadv_neigh_ifinfo', 'bat_v', 'batadv_neigh_ifinfo\\l_bat_v'): EDGE_NOPOINTER,
    edge2str('batadv_neigh_ifinfo', 'if_outgoing', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_neigh_ifinfo', 'bat_iv', 'batadv_neigh_ifinfo\\l_bat_iv'): EDGE_NOPOINTER,
    edge2str('batadv_neigh_node', 'orig_node', 'batadv_orig_node'): EDGE_NOKREF,
    edge2str('batadv_orig_node', 'ifinfo_list', 'batadv_orig_ifinfo'): EDGE_KREF,
    edge2str('batadv_orig_node', 'bat_iv', 'batadv_orig_bat_iv'): EDGE_NOPOINTER,
    edge2str('batadv_orig_node', 'fragments', 'batadv_frag_table_entry'): EDGE_DEFAULT,
    edge2str('batadv_frag_table_entry', 'fragment_list', 'batadv_frag_list_entry'): EDGE_DEFAULT,
    edge2str('batadv_orig_node', 'vlan_list', 'batadv_orig_node_vlan'): EDGE_KREF,
    edge2str('batadv_orig_node_vlan', 'tt', 'batadv_vlan_tt'): EDGE_NOPOINTER,
    edge2str('batadv_orig_node', 'neigh_list', 'batadv_neigh_node'): EDGE_KREF,
    edge2str('batadv_orig_node', 'bat_priv', 'batadv_priv'): EDGE_NOKREF_OK,
    edge2str('batadv_priv', 'dat', 'batadv_priv_dat'): EDGE_NOPOINTER,
    edge2str('batadv_priv_dat', 'hash', 'batadv_dat_entry'): EDGE_KREF,
    edge2str('batadv_priv', 'bat_v', 'batadv_priv_bat_v'): EDGE_NOPOINTER,
    edge2str('batadv_priv', 'primary_if', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_priv', 'nc', 'batadv_priv_nc'): EDGE_NOPOINTER,
    edge2str('batadv_priv_nc', 'decoding_hash', 'batadv_nc_path'): EDGE_KREF,
    edge2str('batadv_priv_nc', 'coding_hash', 'batadv_nc_path'): EDGE_KREF,
    edge2str('batadv_nc_path', 'packet_list', 'batadv_nc_packet'): EDGE_DEFAULT,
    edge2str('batadv_nc_packet', 'nc_path', 'batadv_nc_path'): EDGE_DEFAULT,
    edge2str('batadv_nc_packet', 'neigh_node', 'batadv_neigh_node'): EDGE_NOKREF,
    edge2str('batadv_priv', 'tt', 'batadv_priv_tt'): EDGE_NOPOINTER,
    edge2str('batadv_priv_tt', 'global_hash', 'batadv_tt_global_entry'): EDGE_KREF,
    edge2str('batadv_tt_global_entry', 'common', 'batadv_tt_common_entry'): EDGE_NOPOINTER,
    edge2str('batadv_priv_tt', 'roam_list', 'batadv_tt_roam_node'): EDGE_SINGLE_LOCK,
    edge2str('batadv_priv_tt', 'changes_list', 'batadv_tt_change_node'): EDGE_SINGLE_LOCK,
    edge2str('batadv_priv_tt', 'local_hash', 'batadv_tt_local_entry'): EDGE_KREF,
    edge2str('batadv_tt_local_entry', 'common', 'batadv_tt_common_entry'): EDGE_NOPOINTER,
    edge2str('batadv_tt_local_entry', 'vlan', 'batadv_softif_vlan'): EDGE_KREF,
    edge2str('batadv_softif_vlan', 'tt', 'batadv_vlan_tt'): EDGE_NOPOINTER,
    edge2str('batadv_softif_vlan', 'bat_priv', 'batadv_priv'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_tt', 'req_list', 'batadv_tt_req_node'): EDGE_KREF,
    edge2str('batadv_priv', 'forw_bcast_list', 'batadv_forw_packet'): EDGE_DEFAULT,
    edge2str('batadv_priv', 'forw_bat_list', 'batadv_forw_packet'): EDGE_DEFAULT,
    edge2str('batadv_forw_packet', 'if_outgoing', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_forw_packet', 'if_incoming', 'batadv_hard_iface'): EDGE_KREF,
    edge2str('batadv_priv', 'orig_hash', 'batadv_orig_node'): EDGE_KREF,
    edge2str('batadv_priv', 'debug_log', 'batadv_priv_debug_log'): EDGE_NOKREF_OK,
    edge2str('batadv_priv', 'softif_vlan_list', 'batadv_softif_vlan'): EDGE_KREF,
    edge2str('batadv_priv', 'mcast', 'batadv_priv_mcast'): EDGE_NOPOINTER,
    edge2str('batadv_mcast_mla_flags', 'querier_ipv4', 'batadv_mcast_querier\\l_state'): EDGE_NOPOINTER,
    edge2str('batadv_mcast_mla_flags', 'querier_ipv6', 'batadv_mcast_querier\\l_state'): EDGE_NOPOINTER,
    edge2str('batadv_priv_mcast', 'mla_list', 'batadv_hw_addr'): EDGE_DEFAULT,
    edge2str('batadv_priv_mcast', 'want_all_ipv6_list', 'batadv_orig_node'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_mcast', 'want_all_ipv4_list', 'batadv_orig_node'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_mcast', 'want_all_unsnoopables_list', 'batadv_orig_node'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_mcast', 'want_all_rtr4_list', 'batadv_orig_node'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_mcast', 'want_all_rtr6_list', 'batadv_orig_node'): EDGE_NOKREF_OK,
    edge2str('batadv_priv', 'bla', 'batadv_priv_bla'): EDGE_NOPOINTER,
    edge2str('batadv_priv_bla', 'claim_hash', 'batadv_bla_claim'): EDGE_KREF,
    edge2str('batadv_bla_claim', 'backbone_gw', 'batadv_bla_backbone_gw'): EDGE_KREF,
    edge2str('batadv_bla_backbone_gw', 'bat_priv', 'batadv_priv'): EDGE_NOKREF_OK,
    edge2str('batadv_priv_bla', 'bcast_duplist', 'batadv_bcast_duplist\\l_entry'): EDGE_SINGLE_LOCK,
    edge2str('batadv_priv_bla', 'backbone_hash', 'batadv_bla_backbone_gw'): EDGE_KREF,
    edge2str('batadv_hardif_neigh_node', 'bat_v', 'batadv_hardif_neigh\\l_node_bat_v'): EDGE_NOPOINTER,
    edge2str('batadv_orig_node', 'out_coding_list', 'batadv_nc_node'): EDGE_KREF,
    edge2str('batadv_orig_node', 'in_coding_list', 'batadv_nc_node'): EDGE_KREF,
    edge2str('batadv_nc_node', 'orig_node', 'batadv_orig_node'): EDGE_KREF,
    edge2str('batadv_tt_global_entry', 'orig_list', 'batadv_tt_orig_list\\l_entry'): EDGE_KREF,
    edge2str('batadv_tt_orig_list\\l_entry', 'orig_node', 'batadv_orig_node'): EDGE_KREF,
}


def usage():
    prog = 'filter_graph.py'
    if len(sys.argv) > 0:
        prog = sys.argv[0]

    print('%s INPUT OUTPUT' % (prog))


def read_graph(inpath):
    g = []

    node2name = {}
    graph_raw = []

    r_edge = re.compile(
        r'^\s*(?P<to>[A-Za-z][A-Za-z0-9]*) -> (?P<from>[A-Za-z][A-Za-z0-9]*) \[.*,label="(?P<label>[^"]+)"')
    r_node = re.compile(
        r'^\s*(?P<node>[A-Za-z][A-Za-z0-9]*) \[.*label="(?P<label>[^"]+)"')

    # read input
    lines = open(inpath).readlines()
    for l in lines:
        l = l.strip()
        m_edge = r_edge.match(l)
        m_node = r_node.match(l)

        if not m_edge and not m_node:
            continue

        if m_node:
            node2name[m_node.group('node')] = m_node.group('label')

        if m_edge:
            label = m_edge.group('label').strip()
            labels = label.split('\\n')
            for l in labels:
                e = {
                    'from': m_edge.group('from'),
                    'to': m_edge.group('to'),
                    'label': l}
                graph_raw.append(e)

    # translate graph
    for r in graph_raw:
        e = {
            'from': node2name[r['from']],
            'to': node2name[r['to']],
            'label': r['label']}
        g.append(e)

    return g


def edge2string(e):
    return edge2str(e['from'], e['label'], e['to'])


def get_edgetype(e):
    estring = edge2string(e)
    if not estring in edge_type:
        print(estring)
        return EDGE_DEFAULT
    else:
        return edge_type[estring]


def get_edge_format(e):
    t = get_edgetype(e)

    if t == EDGE_NOPOINTER:
        return "style=\"solid\",color=\"green\""
    elif t == EDGE_KREF:
        return "style=\"dashed\",color=\"green\""
    elif t == EDGE_NOKREF:
        return "style=\"dotted\",color=\"red\""
    elif t == EDGE_NOKREF_OK:
        return "style=\"dotted\",color=\"orange\""
    elif t == EDGE_SINGLE_LOCK:
        return "style=\"dotted\",color=\"darkorchid3\""
    else:
        # t == EDGE_DEFAULT
        return "style=\"dashed\",color=\"blue\""


def write_graph(outpath, g):
    lines = []

    for e in g:
        lines.append(
            '"%s" -> "%s" [label=" %s", %s]' %
            (e['from'], e['to'], e['label'], get_edge_format(e)))

    f = open(outpath, "w")
    f.write('''digraph {
	%s
	%s
}
''' % (graph_config, ";\n".join(lines)))


def get_no_cycle_nodes(nodes):
    no_incoming = filter(lambda n: nodes[n]['in'] == 0, nodes)
    no_outgoing = filter(lambda n: nodes[n]['out'] == 0, nodes)
    return frozenset(no_incoming + no_outgoing)


def remove_non_ref_edges(g):
    return filter(
        lambda e: get_edgetype(e) in
        [EDGE_KREF, EDGE_DEFAULT, EDGE_NOPOINTER],
        g)


def remove_non_cycle_nodes(g):
    g = remove_non_ref_edges(g)

    nodes = defaultdict(lambda: {'in': 0, 'out': 0})
    for e in g:
        nodes[e['from']]['out'] += 1
        nodes[e['to']]['in'] += 1

    while True:
        removable_nodes = get_no_cycle_nodes(nodes)
        if len(removable_nodes) == 0:
            break

        for e in g:
            if e['to'] in removable_nodes:
                nodes[e['from']]['out'] -= 1
            if e['from'] in removable_nodes:
                nodes[e['to']]['in'] -= 1

        for n in removable_nodes:
            del (nodes[n])

        g = filter(
            lambda
            e: e['from'] not in removable_nodes and
            e['to'] not in removable_nodes, g)

    return g


def main():
    if len(sys.argv) < 3:
        usage()
        sys.exit(1)

    inpath = sys.argv[1]
    outpath = sys.argv[2]

    g = read_graph(inpath)
    # g = remove_non_cycle_nodes(g)
    write_graph(outpath, g)


if __name__ == '__main__':
    main()
