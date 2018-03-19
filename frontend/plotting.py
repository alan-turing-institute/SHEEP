"""
generate D3.js plots, using the nvd3 python wrapper, building lists of data using SQL queries.
"""


from nvd3 import multiBarChart
from database import BenchmarkMeasurement, session
import uuid
from sqlalchemy import and_, or_

def create_plot(xdata_list, ydata_dict, xtitle=""):
    """
    x-data is a list of bins on the x-axis.
    y-data is a dict, with the keys being the category names,
    and the vals being the data value lists (each same length as x-data) 
    """
    type = 'Results'
    chart = multiBarChart(name="SHEEP results",height=450,width=1000,x_axis_format=None)
    chart.set_containerheader("\n\n<h2>" + type + "</h2>\n\n")
#    chart.create_y_axis("Execution_time","execution time (s)")
#    chart.create_x_axis("x_var",xtitle)
    xdata = xdata_list
    for k,v in ydata_dict.items():
        chart.add_serie(name=k, y=v, x=xdata)

    extra_serie = {"tooltip": {"y_start": "", "y_end": " cal"}}

    
    chart.buildhtml()
    unique_id = str(uuid.uuid4())
    filename = "results_plots_"+unique_id+".html"
    print("Writing results to templates/"+filename)

    output_file = open("templates/"+filename,"w")
    output_file.write(chart.htmlcontent)
    output_file.close()
    return filename



def build_filter(input_dict):
    """
    convert dict of inputs from web form PlotsForm into SQLAlchemy filter.
    """
    field_to_attribute_dict = {
        "context_selections" : BenchmarkMeasurement.context_name,
        "gate_selections" : BenchmarkMeasurement.gate_name,
        "input_type_selections" : BenchmarkMeasurement.input_bitwidth
    }
    and_expr = and_()
    for field, values in input_dict.items():
        if not field in field_to_attribute_dict.keys():
            continue
        or_expr = or_()
        for val in values:
            or_expr += field_to_attribute_dict[field] == val
        and_expr &= or_expr
    return and_expr

    


def generate_plots(input_dict):
    """
    convert input_dict into sqlalchemy query,
    then convert the query output into a 
    list (x-axis vals) and a dict (y-axis category labels and val-lists)
    for input to create_plot
    """
    filt = build_filter(input_dict)
    filtered_rows = session.query(BenchmarkMeasurement).filter(filt).all()
    print("INPUT DICT")
    print(input_dict)
    ### need to organise data so there is a list of unique x-axis vals in xdata,
    ### and a dictionary of { 'category_label' : [y-vals] } 

    ### NOTE the current simple logic below only works if the number of y-vals per selected
    ### category is equal to the number of x-bins..  This would not be the case if e.g. plotting
    ### more than one gate at the same time, with x-axis depth, and category_var HE_lib.
    
    xdata = []    
    ydata = {}
    
    for row in filtered_rows:
        xval = row.__getattribute__(input_dict["x_axis_var"])
        if not xval in xdata:
            xdata.append(xval)
        category = row.__getattribute__(input_dict["category_field"])
        if not category in ydata.keys():
            ydata[category] = []
        execution_time = row.__getattribute__("execution_time")
        ydata[category].append(execution_time)
    return create_plot(xdata, ydata, input_dict["x_axis_var"])

