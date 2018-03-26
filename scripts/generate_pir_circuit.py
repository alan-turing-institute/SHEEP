"""
Script to generate a circuit for single server PIR, using the recursive pir trick.
"""

import os

VAR_COUNT = 0

if not "SHEEP_HOME" in os.environ.keys():
    BASE_DIR = os.environ["HOME"]+"/SHEEP"
else:
    BASE_DIR = os.environ["SHEEP_HOME"]

    
CIRCUIT_DIR = BASE_DIR+"/benchmark_inputs/mid_level/circuits/"


def new_var():
    global VAR_COUNT
    VAR_COUNT = VAR_COUNT + 1
    return 'w{}'.format(VAR_COUNT - 1)

def generate_circuit(filename, database_size, alphas):
    """
    Generate the circuit.
    """
    sizes = []
    l = database_size
    for i, v in enumerate(alphas):
        l, x = divmod(l, v)
        sizes.append(l)
        #l = l2
        assert x == 0
    output_length = sizes[len(alphas) - 1]
    outputs = None
    inputs = ['d_0_{}_{}'.format(i, j) for i in range(alphas[0]) for j in range(sizes[0])]
    inputs += ['s_{}_{}'.format(i, j) for i in range(len(alphas)) for j in range(alphas[i])]
    operations = []
    
    for i in range(len(alphas)):
        # component-wise multiplication
        for j in range(alphas[i]):
            #print i, sizes
            for z in range(sizes[i]):
                op = ('c_{}_{}_{}'.format(i, j, z),
                      'MULTIPLY' if i == 0 else 'MULTIPLY',  # First one should be MULTBYCONST
                      's_{}_{}'.format(i, j), 'd_{}_{}_{}'.format(i, j, z))
                operations.append(op)
        # component-wise sum
        next_database = []
        for j in range(sizes[i]):
            op = ('e_{}_{}'.format(i, j),
                  'ADD',
                  ['c_{}_{}_{}'.format(i, z, j) for z in range(alphas[i])])
            operations.append(op)
            next_database.append('e_{}_{}'.format(i, j))
        # rename
        index = 0
        #print next_database
        if i < len(alphas) - 1:
            for j in range(alphas[i+1]):
                for z in range(sizes[i+1]):
                    #print j, z
                    op = ('d_{}_{}_{}'.format(i + 1, j, z),
                          'ALIAS',
                          next_database[index])
                    index += 1
                    operations.append(op)
        else:
            outputs = next_database
                    

    #for op in operations:
    #    print op

    write_output(operations, inputs, outputs, filename)

def binary_add_tree(assignment):
    lhs, op, arg_list = assignment
    if len(arg_list) == 2:
        return ['{} {} {} {}'.format(arg_list[0], arg_list[1], op, lhs)]
    if len(arg_list) == 1:
        return ['{} {} {}'.format(arg_list[0], 'ALIAS', lhs)]
    else:
        arg_list_1 = arg_list[: len(arg_list) / 2]
        arg_list_2 = arg_list[len(arg_list) / 2 :]
        x1 = new_var()
        x2 = new_var()
        l1 = binary_add_tree((x1, op, arg_list_1))
        l2 = binary_add_tree((x2, op, arg_list_2))
        return ['{} {} {} {}'.format(x1, x2, op, lhs)] + l1 + l2
    
def write_output(operations, inputs, outputs, filename):
    """
    write the circuit to a file.
    """
    outfile = open(filename,"w")
    outfile.write("INPUTS ")
    for i in inputs:
        outfile.write(" "+i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" "+o)
    outfile.write("\n")
    for t in operations:
        lhs = t[0]
        op = t[1]
        assert not op == 'MULTIPLY' or len(t) == 4
        assert not op == 'MULTIPLY' or len(t) == 4 # Should be MULTBYCONST
        assert not op == 'ADD' or (len(t) == 3 and isinstance(t[2], list))
        if op in {'MULTIPLY', 'MULTIPLY'}: # One should be MULTBYCONST
            outfile.write('{} {} {} {}\n'.format(t[2], t[3], op, lhs))
        elif op in ('ALIAS'):
            outfile.write('{} {} {}\n'.format(t[2],  op, lhs))
        else:
            assert op == 'ADD'
            ops = binary_add_tree(t)
            ops.reverse()
            for s in ops:
                outfile.write(s)
                outfile.write('\n')
    outfile.close()

def generate(dbsize, alphas):
    outputfile = CIRCUIT_DIR+"/circuit-pir-"+str(dbsize)
    for a in alphas:
        outputfile+="_"+str(a)
    outputfile+= ".sheep"
    generate_circuit(outputfile,dbsize,alphas)
    return outputfile

if __name__ == "__main__":
    #generate_circuit('pir_32_2_2_2_2_2.sheep', 32, [2, 2, 2, 2, 2])
   # generate_circuit('pir_49_7_7.sheep', 49, [7, 7])
  #  generate_circuit('pir_32_32.sheep', 32, [32])
    #generate_circuit('pir_4_2_2.sheep', 4, [2, 2])
    generate_circuit('pir_2_2.sheep', 2, [2])
