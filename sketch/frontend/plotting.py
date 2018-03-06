"""
generate D3.js plots, using the nvd3 python wrapper, building lists of data using SQL queries.
"""


from nvd3 import multiBarChart
import database

def create_plot(xdata_list, ydata_dict):
    """
    x-data is a list of bins on the x-axis.
    y-data is a dict, with the keys being the category names,
    and the vals being the data value lists (each same length as x-data) 
    """
    type = 'execution time (seconds)'
    chart = multiBarChart(name="SHEEP results",height=450,width=1000,x_axis_format=None)
    chart.set_containerheader("\n\n<h2>" + type + "</h2>\n\n")
    xdata = xdata_list
    for k,v in ydata_dict.items():
        chart.add_serie(name=k, y=v, x=xdata)

    extra_serie = {"tooltip": {"y_start": "", "y_end": " cal"}}
    chart.buildhtml()
    print("Writing results to templates/results_plots.html")
    output_file = open("templates/results_plots.html","w")
    output_file.write(chart.htmlcontent)
    output_file.close()


def build_query(input_dict):
    """
    convert dict on inputs from the web form PlotsForm into an sql query 
    """
    query = "SELECT "
    query += input_dict["x_axis_var"]+","+input_dict["category_field"]
    query += ",execution_time "
    query += " FROM benchmarks"
    if len(input_dict["context_selections"]) > 0 or \
       len(input_dict["gate_selections"]) > 0 or \
       len(input_dict["input_type_selections"]) > 0:
        query+= " WHERE "
        if len(input_dict["context_selections"]) > 0:
            query += "("
            for context in input_dict["context_selections"]:
                query += "context_name='"+context+"' OR "
## now remove the trailing OR and replace with a close-brace.
            query = query[:-4]+")"
        if len(input_dict["gate_selections"]) > 0:
            query += " AND ("
            for gate in input_dict["gate_selections"]:
                query += "gate_name='"+gate+"' OR "
## now remove the trailing OR and replace with a close-brace.
            query = query[:-4]+")"
        if len(input_dict["input_type_selections"]) > 0:
            query += " AND ("
            for itype in input_dict["input_type_selections"]:
                query += "input_bitwidth="+itype+" OR "
## now remove the trailing OR and replace with a close-brace.
            query = query[:-4]+")" 
    return query
                
def generate_plots(input_dict):
    """
    convert input_dict into an sql query,
    then convert the query output into a 
    list (x-axis vals) and a dict (y-axis category labels and val-lists)
    for input to create_plot
    """
    print("INPUT DICT FOR GENERATE PLOTS")
    print(input_dict)
    query = build_query(input_dict)
    columns, rdata = database.execute_query_sqlite3(query)
    xdata = []
    ydata = {}
    data_dict = {}
    ### in each row, row[0] is the x-axis var, row[1] is the category var, and row[2] is execution time
    for row in rdata:
        if not row[0] in data_dict.keys():
            data_dict[row[0]] = {}
            pass
        if not row[1] in data_dict[row[0]].keys():
            data_dict[row[0]][row[1]] = row[2]
            pass
        pass
### now have full set of data - need to separate out y-axis vals into lists
    xdata = list(data_dict.keys())
    for k in data_dict[xdata[0]].keys():
        ydata[k] = []
    for xval in xdata:
        for k in ydata.keys():
            ydata[k].append(data_dict[xval][k])
    create_plot(xdata, ydata)
        
    
        
        

