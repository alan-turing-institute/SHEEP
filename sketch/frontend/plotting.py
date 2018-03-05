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
    output_file = open("templates/results_plots.html","w")
    output_file.write(chart.htmlcontent)
    output_file.close()


def generate_plots(input_dict):
    print("INPUT DICT FOR GENERATE PLOTS")
    print(input_dict)
    query = "SELECT context_name,input_bitwidth,depth,execution_time FROM benchmarks WHERE input_bitwidth=8"
    columns, rdata = database.execute_query_sqlite3(query)
    xdata = []
    ydata = {}

    data_dict = {}
    for row in rdata:
        
        if not row[2] in data_dict.keys():
            data_dict[row[2]] = {}
            pass
        if not row[0] in data_dict[row[2]].keys():
            data_dict[row[2]][row[0]] = row[3]
            pass
        pass
### now have full set of data - need to separate out y-axis vals into lists
    xdata = list(data_dict.keys())
    for k in data_dict[xdata[0]].keys():
        ydata[k] = []
    for xval in xdata:
        for k in ydata.keys():
            ydata[k].append(data_dict[xval][k])
    return xdata, ydata
        
    
        
        

