import sys
import networkx as nx
from networkx.drawing.nx_agraph import graphviz_layout, to_agraph
from PIL import Image


class node(object):

    def __init__(self, val, node_type, op='GOD'):
        self.val = val
        self.node_type = node_type
        self.mono = False
        self.bi = False
        if self.val in ['ALIAS', 'NEGATE']:
            self.mono = True
        else:
            self.bi = True
        self.children = []
        self.op = op
        self.name = self.val + "(" + self.op + ")"

    def __str__(self, level=0):
        ret = "\t" * level + repr(self.val) + "(" + self.op + ")\n"
        for child in self.children:
            ret += child.__str__(level + 1)
        return ret

    def __repr__(self):
        return '<tree node representation>'


class networkx_graph(object):
    def __init__(self, node, filename='test'):
        self.output = node
        self.filename = filename
        self.G = nx.DiGraph()
        self.labeldict = {}
        self.edge_label = {}
        self.short_names = {'NEGATE': '~',
                            'ALIAS': '=',
                            'MULTIPLY': '&',
                            'ADD': 'XR',
                            'MAXIMUM': '|',
                            'GOD': 'OUT',
                            'SELECT': 'SEL',
                            'MULTBYCONST': '&c'}
        self.G.add_node(self.construct(node))

    def construct(self, curr_node):
        if curr_node.op != 'INPUT' and curr_node.op != 'OUTPUT':
            self.G.add_node(
                curr_node.val, label=self.short_names[curr_node.op])
            self.labeldict[curr_node.val] = self.short_names[curr_node.op]
        elif curr_node.op == 'OUTPUT':
            self.G.add_node(
                curr_node.val,
                color='red',
                label=curr_node.val[:-3])
            self.labeldict[curr_node.val] = curr_node.val[:-3]
        else:
            self.G.add_node(
                curr_node.val,
                color='red',
                label=curr_node.val[:-3])
            self.labeldict[curr_node.val] = curr_node.val[:-3]
        for idx, child in enumerate(curr_node.children):
            target_node = self.construct(child)
            self.G.add_edge(target_node, curr_node.val,
                            label=str(idx))
            self.edge_label[(target_node, curr_node.val)
                            ] = target_node[-4:]

        return curr_node.val

    def draw(self):
        options = {
            'node_color': 'blue',
            'node_size': 10,
            'width': 1,
            'font_size': 9
        }
        nx.draw_networkx(self.G, labels=self.labeldict,
                         pos=graphviz_layout(self.G, prog='neato'),
                         with_label=True, **options)
        nx.draw_networkx_edges(self.G, pos=graphviz_layout(
            self.G, prog='neato'), arrowstyle='->', arrow=True)
        nx.draw_networkx_edge_labels(self.G, pos=graphviz_layout(
            self.G, prog='neato'), edge_labels=self.edge_label,
            arrowstyle='-')

        self.G.graph['graph'] = {'rankdir': 'TD', 'size': "20.,20"}
        self.G.graph['edges'] = {'arrowsize': '10.0'}
        G_agraph = to_agraph(self.G)
        G_agraph.layout(prog='dot')
        G_agraph.draw(self.filename.split('.')[0] + '.png')
        Image.open(self.filename.split('.')[0] + '.png').show()


def get_circuit_graph(filename):
    inps = []
    outs = []
    complete = node(val='OUTPUT',
                    node_type='COMPLETE')
    nodes = {'OUTPUT': complete}
    with open(filename, 'rb') as f:
        for idx, line in enumerate(f.readlines()):
            args = line.split()
            if idx <= 1:
                inps = args[1:]
                for vals in inps:
                    nodes[vals] = node(val=vals,
                                       node_type='INPUT',
                                       op='INPUT')
            elif idx == 2:
                outs = args[1:]
                for vals in outs:
                    nodes[vals] = node(val=vals,
                                       node_type='OUTPUT',
                                       op='OUTPUT')
                    nodes['OUTPUT'].children.append(nodes[vals])
            else:
                if len(args) == 5:
                    if args[-1] not in nodes.keys():
                        nodes[args[-1]] = node(val=args[-1],
                                               node_type='TRI_OP_OUT')
                    assert(args[0] in nodes.keys()), "Can't find " + args[0]
                    assert(args[1] in nodes.keys()), "Can't find " + args[1]
                    assert(args[2] in nodes.keys()), "Can't find " + args[2]
                    nodes[args[-1]].op = args[3]
                    nodes[args[-1]].children.append(nodes[args[0]])
                    nodes[args[-1]].children.append(nodes[args[1]])
                    nodes[args[-1]].children.append(nodes[args[2]])
                if len(args) == 4:
                    if args[-1] not in nodes.keys():
                        nodes[args[-1]] = node(val=args[-1],
                                               node_type='BI_OP_OUT')
                    assert(args[0] in nodes.keys()), "Can't find " + args[0]
                    assert(args[1] in nodes.keys()), "Can't find " + args[1]
                    nodes[args[-1]].op = args[2]
                    nodes[args[-1]].children.append(nodes[args[0]])
                    nodes[args[-1]].children.append(nodes[args[1]])
                elif len(args) == 3:
                    if args[-1] not in nodes.keys():
                        nodes[args[-1]] = node(val=args[-1],
                                               node_type='MONO_OP_OUT')
                    assert(args[0] in nodes.keys()), "Can't find " + args[0]
                    nodes[args[-1]].op = args[1]
                    nodes[args[-1]].children.append(nodes[args[0]])
    return nodes['OUTPUT']


if __name__ == '__main__':
    assert(len(sys.argv) == 2)
    complete_node = get_circuit_graph(sys.argv[-1])
    # print(complete_node)
    ng = networkx_graph(complete_node)
    ng.draw()
