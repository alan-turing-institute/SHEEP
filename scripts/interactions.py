from particles import variables
from multiprocessing import Process, Queue
import collections
from tqdm import tqdm
import sys
import numpy as np


class assignments(object):
    def __init__(self, ass_type, lhs, rhs):
        '''
        All kinds of assignments statements
        will be contained here
        '''

        self.ass_type = ass_type
        assert isinstance(lhs, list)
        assert isinstance(rhs, variables)
        for elems in lhs:
            assert isinstance(elems, variables), elems + \
                " should be a variable."
        self.nargs = len(lhs)
        self.lhs = lhs
        self.rhs = rhs

        assert self.rhs not in self.lhs, "RHS cannot be in LHS"

    def __str__(self):
        pass

    def __add__(self, x):
        return str(self) + x

    def __radd__(self, x):
        return x + str(self)


class mono_assign(assignments):
    def __init__(self, ass_type, lhs, rhs):
        assignments.__init__(self, ass_type, lhs, rhs)
        assert self.nargs == 1, "Cannot have more than one element : " + \
            rhs + " <= " + ass_type + ' ' + \
            ','.join(map(lambda x: str(x), lhs))
        self.ass_comms = {'not': 'NEGATE',
                          'alias': 'ALIAS'}

    def interpret(self):

        try:
            assert self.ass_type is not None
            self.command = self.ass_comms[self.ass_type]
        except KeyError:
            print("Maybe use one of  : " + ' | '.join(self.ass_comms))
            sys.exit()

    def __str__(self):
        self.interpret()
        return self.lhs[0] + ' ' + self.command + ' ' + self.rhs


class bi_assign(assignments):
    def __init__(self, ass_type, lhs, rhs):
        assignments.__init__(self, ass_type, lhs, rhs)
        assert self.nargs == 2, "Must have 2 elements: " + \
            rhs + " <= " + ass_type + ' ' + \
            ','.join(map(lambda x: str(x), lhs))
        self.ass_comms = {'xor': 'ADD',
                          'and': 'MULTIPLY',
                          'or': 'MAXIMUM',
                          'constand': 'MULTBYCONST'}

    def interpret(self):
        try:
            assert self.ass_type is not None
            self.command = self.ass_comms[self.ass_type]
        except KeyError:
            print("Maybe use one of  : " + ' | '.join(self.ass_comms))
            sys.exit()

    def __str__(self):
        self.interpret()
        lhs_args = self.lhs[0] + ' ' + self.lhs[1]
        return lhs_args + ' ' + self.command + ' ' + self.rhs


class tri_assign(assignments):
    def __init__(self, ass_type, lhs, rhs):
        assignments.__init__(self, ass_type, lhs, rhs)
        assert self.nargs == 3, "Must have 3 elements: " + \
            rhs + " <= " + ass_type + ' ' + \
            ','.join(map(lambda x: str(x), lhs))
        self.ass_comms = {'mux': 'SELECT'}

    def interpret(self):
        try:
            assert self.ass_type is not None
            self.command = self.ass_comms[self.ass_type]
        except KeyError:
            print("Maybe use one of  : " + ' | '.join(self.ass_comms))
            sys.exit()

    def __str__(self):
        self.interpret()
        lhs_args = self.lhs[0] + ' ' + self.lhs[1] + ' ' + self.lhs[2]
        return lhs_args + ' ' + self.command + ' ' + self.rhs


class mini_mod(object):
    def __init__(self, name, inputs, outputs, parallel=False):
        self.name = name
        self.inputs = inputs
        self.outputs = outputs
        self.commands = collections.OrderedDict()
        self.zero = variables(name='FALSE')
        self.one = variables(name='TRUE')
        self.parallel = parallel

    def add(self, op):
        try:
            if isinstance(op, mini_mod):
                self.commands[op.name] = op
            elif isinstance(op, assignments):
                self.commands[op.ass_type +
                              '_'.join(map(lambda x: str(x),
                                           op.lhs)) + '||' + op.rhs] = op
        except Exception:
            print("I am not quite sure, what you're trying\
                   to add. I will just exit!")

    def __str__(self):
        def local_str(output, idx, curr_module):
            output.put((idx, str(curr_module)))
            print "HI"
        vals = self.commands.values()

        if self.parallel:
            tgt_list = ['' for s in vals]
            output = Queue()
            proc_arr = []
            num_coms = len(vals)
            for i in tqdm(range(num_coms)):
                proc_arr.append(
                    Process(target=local_str, args=(output, i, vals[i])))
                proc_arr[i].start()
            for i in tqdm(range(num_coms)):
                proc_arr[i].join()
            results = [output.get() for p in proc_arr]
            for idx, val in enumerate(results):
                tgt_list[val[0]] = str(val[1])
        else:
            tgt_list = [str(x) for x in vals]
        return '\n'.join(tgt_list)


if __name__ == '__main__':
    a = variables('a')
    b = variables('b')
    c = variables('c')
    d = variables('d')
    t1 = mono_assign(ass_type='not', lhs=[a], rhs=b)
    print(t1)
    t2 = mono_assign(ass_type='alias', lhs=[b], rhs=d)
    print(t2)
    t3 = bi_assign(ass_type='or', lhs=[d, c], rhs=b)
    print(t3)

    print("MODULE below")
    t = mini_mod('addition', [a, c], b)
    t.add(t1)
    t.add(t2)
    t.add(t3)
    ts = mini_mod('addition', [a, c], b)
    print(t)
