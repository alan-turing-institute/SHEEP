"""
WTForms forms for user to enter configuration of an HE test - choose circuit file, context etc.
"""
from wtforms import Form, FloatField, FormField, IntegerField, FileField, \
    SelectField, validators, FieldList, StringField,SelectMultipleField, HiddenField, widgets

class MultiCheckboxField(SelectMultipleField):
    """
    field for a multi-checkbox.
    """
    widget = widgets.ListWidget(prefix_label=False)
    option_widget = widgets.CheckboxInput()

class CircuitForm(Form):
    """
    Standard WTForm
    """
    circuit_file = FileField(validators=[validators.InputRequired()],label="Circuit file")

    input_type = SelectField(choices=[]
                             ,label="Input type")
    HE_library = MultiCheckboxField('Select HE libraries:',
                                    choices=[])

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

    context_selections = MultiCheckboxField('Select contexts',
                                            choices=[("HElib_F2","HElib_F2"),
                                                     ("HElib_Fp","HElib_Fp"),
                                                     ("TFHE","TFHE"),
                                                     ("SEAL","SEAL"),
                                                    ("LP","LP")])
    gate_selections = MultiCheckboxField('Select gate(s)',
                                         choices=[("ADD","ADD"),
                                                  ("SUBTRACT","SUBTRACT"),
                                                  ("MULTIPLY","MULTIPLY"),
                                                  ("NEGATE","NEGATE"),
                                                  ("SELECT","SELECT"),
                                                  ("COMPARE","COMPARE")])
    input_type_width = MultiCheckboxField('Select input bitwidth(s)',
                                          choices=[(1,"1"),(8,"8"),(16,"16"),(32,"32")])
    input_type_signed = MultiCheckboxField('Select signed or unsigned input',
                                           choices=[(1,"signed"),(0,"unsigned")])


def build_inputs_form(inputs):
    """
    return a class of WTForm with custom fields specified by the "inputs" list.
    """
    class InputsForm(Form):
        pass
    
    for i in inputs:
        field = StringField(label = i)
        setattr(InputsForm, i, field)

    return InputsForm


def build_param_form(params_dict,eval_strat="serial"):
    """
    return a class of WTForm with custom fields specified by the "parameters" list.
    """
    class ParamsForm(Form):
        pass
    for p,v in params_dict.items():
        if p=="eval_strategy":
            continue
        field = IntegerField(label=p,default=v)
        setattr(ParamsForm,p,field)
    eval_strategy = SelectField(choices=[("serial","serial"),("parallel","parallel")]
                                ,label="Evaluation Strategy",
                                default=eval_strat)
    setattr(ParamsForm,"eval_strategy",eval_strategy)
    return ParamsForm
