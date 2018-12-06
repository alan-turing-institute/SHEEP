"""
SQLAlchemy classes to describe the tables of benchmark measurements.
"""

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import Column, ForeignKey, Integer, String, Boolean, Float
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import create_engine
from sqlalchemy import and_, or_

import sqlite3

import os

### location of the sqlite file holding the DB
### (at some point may replace with e.g. postgres DB, but
### this will only require minimal changes, thanks to sqlalchemy).

if "SHEEP_HOME" in os.environ.keys():
    if "pysheep" in os.environ["SHEEP_HOME"]:
        DB_LOCATION = os.path.join(os.environ["SHEEP_HOME"],"sheep.db")
    else:
        DB_LOCATION = os.path.join(os.environ["SHEEP_HOME"],"pysheep","sheep.db")
else:
    DB_LOCATION = os.path.join(os.environ["HOME"],"SHEEP","sheep.db")

Base = declarative_base()
engine = create_engine("sqlite:///"+DB_LOCATION)


class BenchmarkMeasurement(Base):
    __tablename__ = "benchmarks"
    id = Column(Integer, primary_key=True, autoincrement=True,nullable=False)
    context_name = Column(String(250), nullable=False)
    input_bitwidth = Column(Integer, nullable=False)
    input_signed = Column(Boolean, nullable=False)
    circuit_name = Column(String(250), nullable=True)
    num_inputs = Column(Integer, nullable=True)
    num_slots = Column(Integer, nullable=True)
    tbb_enabled = Column(Boolean, nullable=True)
    setup_time = Column(Float, nullable=True)
    encryption_time = Column(Float, nullable=True)
    execution_time = Column(Float, nullable=False)
    is_correct = Column(Boolean, nullable=False)
    parameter_set = Column(Integer, nullable=False)


class ParameterSetting(Base):
    __tablename__ = "paramsets"
    id = Column(Integer, primary_key=True, autoincrement=True, nullable=False)
    paramset_id = Column(Integer, nullable=False)
    param_name = Column(String(100), nullable=False)
    param_value = Column(Integer, nullable=False)


Base.metadata.create_all(engine)

Base.metadata.bind = engine
DBSession = sessionmaker(bind=engine)
session = DBSession()

def get_table_and_columns(query):
    """
    parse the query to extract table name and columns
    """
    table_name = ""
    columns = []
    if table_regex.search(query):
        table_name = table_regex.search(query).groups()[1]
    if column_regex.search(query):
        columns = column_regex.search(query).groups()[1].split(",")
    return table_name, columns



def execute_query_sqlalchemy(filt):
    """
    Perform a query on the db
    """
    session.query(BenchmarkMeasurement).filter(filt).all()



def build_filter(input_dict):
    """
    convert dict of inputs from web form PlotsForm into SQLAlchemy filter.
    """
    field_to_attribute_dict = {
        "context_selections" : BenchmarkMeasurement.context_name,
        "gate_selections" : BenchmarkMeasurement.gate_name,
        "input_type_width" : BenchmarkMeasurement.input_bitwidth,
        "input_type_signed" : BenchmarkMeasurement.input_signed
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


def upload_benchmark_result(results_dict):
    """
    upload a benchmark result, either from web frontend or notebook.
    """
    bm = BenchmarkMeasurement()
    bm.context_name = results_dict['context_name']
    bm.input_bitwidth = results_dict['input_bitwidth']
    bm.input_signed = results_dict['input_signed']
    bm.circuit_name = results_dict['circuit_name']
    bm.execution_time = results_dict['execution_time']
    bm.is_correct = results_dict['is_correct']
    session.add(bm)
    session.commit()
    return True
