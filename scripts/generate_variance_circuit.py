"""
Script to generate a circuit to calculate the variance of a set of inputs.
The number of input vals needs to be passed to the circuit as a "CONST_INPUT"
The circuit then calculates 

Sum_i=0..N ( (N.xbar - N.x_i)^2 )    
(where N.xbar is the sum of all the x_i,  i.e. N*mean(x_i) )

The output will be N^3 * variance.
"""

def generate_circuit(num_inputs):
    """
    Generate the circuit.
    """
    const_inputs = ["N"]
    inputs = []
    outputs = ["varianceN3"]
    assignments = []
    last_output = ""

    ## first, add all the inputs, to get the sum N.xbar    
    for i in range(num_inputs):
        inputs.append("x_"+str(i))
        if i == 0:
            continue
        elif i == 1:
            assignments.append("x_0 x_1 ADD y_0")
        else:
            assignments.append("y_"+str(i-2)+" x_"+str(i)+" ADD y_"+str(i-1))
            pass
        pass
    last_output = assignments[-1].split()[-1]
    assignments.append(last_output+" ALIAS Nxbar")

    ## now multiply each of the inputs by N and subtract from Nxbar
    for i in range(num_inputs):
        assignments.append("x_"+str(i)+" N  MULTBYCONST Nx_"+str(i))
        assignments.append("Nxbar Nx_"+str(i)+" SUBTRACT v_"+str(i))
    ## now square each of these outputs by multiplying with an ALIAS of themself
        assignments.append("v_"+str(i)+" ALIAS vv_"+str(i))
        assignments.append("v_"+str(i)+" vv_"+str(i)+" MULTIPLY s_"+str(i))
    ## finally sum these squares
    for i in range(1,num_inputs):
        if i == 1:
            assignments.append("s_0 s_1 ADD ss_0")
        else:
            assignments.append("ss_"+str(i-2)+" s_"+str(i)+" ADD ss_"+str(i-1))
            pass
        pass
    last_output = assignments[-1].split()[-1]
    assignments.append(last_output+" ALIAS varianceN3")

    return const_inputs, inputs, outputs, assignments
    

def write_output(filename, const_inputs,inputs,outputs,assignments):
    """
    write the circuit to a file.
    """
    outfile = open(filename,"w")
    outfile.write("CONST_INPUTS ")
    for ci in const_inputs:
        outfile.write(" "+ci)
    outfile.write("\n")
    outfile.write("INPUTS ")
    for i in inputs:
        outfile.write(" "+i)
    outfile.write("\n")
    outfile.write("OUTPUTS ")
    for o in outputs:
        outfile.write(" "+o)
    outfile.write("\n")
    for a in assignments:
        outfile.write(a+"\n")
    outfile.close()



if __name__ == "__main__":
    const_inputs, inputs, outputs, assignments = generate_circuit(4)
    write_output("test-variance.sheep",const_inputs,inputs,outputs,assignments)
