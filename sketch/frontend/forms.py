"""
WTForms forms for user to enter configuration of an HE test - choose circuit file, context etc.
"""
from wtforms import Form, FloatField, FormField, IntegerField, FileField, SelectField, validators, FieldList, StringField

class CircuitForm(Form):
    """
    Standard WTForm
    """
    circuit_file = FileField(validators=[validators.InputRequired()],label="Circuit file")
    HE_library = SelectField(choices=[("Clear","Clear"),("HElib","HElib"),("TFHE","TFHE")],label="HE library")
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
