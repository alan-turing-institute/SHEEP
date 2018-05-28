import sys
import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_agraph import graphviz_layout


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
    def __init__(self, node):
        self.output = node
        self.G = nx.DiGraph()
        self.labeldict = {}
        self.G.add_node(self.construct(node))

    def construct(self, curr_node):
        for child in curr_node.children:
            self.G.add_edge(curr_node.val, self.construct(
                child))
        self.labeldict[curr_node.val] = curr_node.op
        return curr_node.val

    def draw(self):
        nx.draw_networkx(self.G, labels=self.labeldict,
                         pos=graphviz_layout(self.G),
                         with_label=True,
                         node_size=20)
        plt.show()


def get_circuit_graph(filename):
    inps = []
    outs = []
    complete = node(val='OUTPUT',
                    node_type='COMPLETE')
    nodes = {'OUTPUT': complete}
    with open(filename, 'rb') as f:
        for idx, line in enumerate(f.readlines()):
            args = line.split()
            if idx == 0:
                inps = args[1:]
                for vals in inps:
                    nodes[vals] = node(val=vals,
                                       node_type='INPUT',
                                       op='INPUT')
            elif idx == 1:
                outs = args[1:]
                for vals in outs:
                    nodes[vals] = node(val=vals,
                                       node_type='OUTPUT')
                    nodes['OUTPUT'].children.append(nodes[vals])
            else:
                if len(args) == 4:
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
    print(complete_node)
    ng = networkx_graph(complete_node)
    ng.draw()
