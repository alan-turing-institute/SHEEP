"""
WTForms forms for user to enter configuration of an HE test - choose circuit file, context etc.
"""
from wtforms import Form, FloatField, FormField, IntegerField, FileField, \
    SelectField, validators, FieldList, StringField,SelectMultipleField, widgets

class MultiCheckboxField(SelectMultipleField):
    widget = widgets.ListWidget(prefix_label=False)
    option_widget = widgets.CheckboxInput()

class CircuitForm(Form):
    """
    Standard WTForm
    """
    circuit_file = FileField(validators=[validators.InputRequired()],label="Circuit file")
    HE_library = SelectField(choices=[("Clear","Clear"),("HElib_F2","HElib_F2"),("HElib_Fp","HElib_Fp"),("TFHE","TFHE")],label="HE library")
##    parameter_file = FileField(validators=[validators.InputRequired()],label="Parameter file")
    input_type = SelectField(choices=[("bool","bool"),
                                      ("uint8_t","uint8_t"),
                                      ("int8_t","int8_t"),
                                      ("uint16_t","uint16_t"),
                                      ("int16_t","int16_t"),
                                      ("uint32_t","uint32_t"),
                                      ("int32_t","int32_t")]
                             ,label="Input type")


class ResultsForm(Form):
    """
    Let the user choose what results to see
    """
    sql_query = StringField(label="SQL query")
    

class PlotsForm(Form):
    """
    Let the user configure what to plot.
    """
    x_axis_var = SelectField(choices=[
        ("context_name","HE library"),
        ("input_bitwidth","Input bitwidth"),
        ("depth","Depth"),
        ("gate_name","Gate"),        
    ],label="x-axis var")
    category_field = SelectField(choices=[
        ("depth","Depth"),
        ("context_name","HE library"),
        ("input_bitwidth","Input bitwidth"),
        ("gate_name","Gate"),        
    ],label="category var")
    context_selections = MultiCheckboxField('Select contexts', choices=[("HElib_F2","HElib_F2"),("HElib_Fp","HElib_Fp"),("TFHE","TFHE")])
    gate_selections = MultiCheckboxField('Select gate', choices=[("ADD","ADD"),("SUBTRACT","SUBTRACT"),("MULTIPLY","MULTIPLY"),("NEGATE","NEGATE"),("SELECT","SELECT"),("COMPARE","COMPARE")])    
    input_type_selections = MultiCheckboxField('Select input_type', choices=[(1,"bool"),(8,"uint8_t"),(16,"uint16_t"),(32,"uint32_t")])

    
def build_inputs_form(inputs):
    """ 
    return a class of WTForm with custom fields specified by the "inputs" list.
    """
    class InputsForm(Form):
        pass
    for i in inputs:
        field = IntegerField(label=i)
        setattr(InputsForm,i,field)
    return InputsForm
