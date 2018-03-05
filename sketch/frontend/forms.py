"""
WTForms forms for user to enter configuration of an HE test - choose circuit file, context etc.
"""
from wtforms import Form, FloatField, FormField, IntegerField, FileField, SelectField, validators, FieldList

class CircuitForm(Form):
    """
    Standard WTForm
    """
    circuit_file = FileField(validators=[validators.InputRequired()],label="Circuit file")
    HE_library = SelectField(choices=[("Clear","Clear"),("HElib","HElib"),("TFHE","TFHE")],label="HE library")
    parameter_file = FileField(validators=[validators.InputRequired()],label="Parameter file")
    input_type = SelectField(choices=[("bool","bool"),("uint8_t","uint8_t"),("int8_t","int8_t")],label="Input type")
##    inputs_file = FileField(validators=[validators.InputRequired()],label="")    
    


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
