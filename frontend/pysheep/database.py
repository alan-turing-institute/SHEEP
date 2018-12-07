"""
SQLAlchemy classes to describe the tables of benchmark measurements.
"""
import os

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, relationship
from sqlalchemy import Column, ForeignKey, Integer, String, Boolean, Float
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import create_engine
from sqlalchemy import and_, or_

import sqlite3

from . import common_utils


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
    benchmark_id = Column(Integer, primary_key=True,nullable=False)
    context = Column(String(250), nullable=False)
    input_bitwidth = Column(Integer, nullable=False)
    input_signed = Column(Boolean, nullable=False)
    circuit_name = Column(String(250), nullable=False)
    num_inputs = Column(Integer, nullable=False)
    num_slots = Column(Integer, nullable=False)
    tbb_enabled = Column(Boolean, nullable=False)
    is_correct = Column(Boolean, nullable=False)
    timings = relationship("Timing", uselist=True)
    parameters = relationship("ParameterSetting",uselist=True) #, back_populates="benchmarks")


class Timing(Base):
    __tablename__ = "timings"
    id = Column(Integer, primary_key=True, autoincrement=True, nullable=False)
    timing_name = Column(String(100), nullable=False)
    timing_value = Column(Float, nullable=False)
    benchmark_id = Column(Integer, ForeignKey("benchmarks.benchmark_id"))


class ParameterSetting(Base):
    __tablename__ = "param"
    id = Column(Integer, primary_key=True, autoincrement=True, nullable=False)
    param_name = Column(String(100), nullable=False)
    param_value = Column(Integer, nullable=False)
    benchmark_id = Column(Integer, ForeignKey("benchmarks.benchmark_id"))


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


def get_last_benchmark_id():
    bms = session.query(BenchmarkMeasurement).all()
    if len(bms) == 0:
        return 0
    return bms[-1].benchmark_id


def upload_benchmark_result(circuit_name,
                            context,
                            input_type,
                            num_inputs,
                            num_slots,
                            tbb_enabled,
                            results_dict,
                            params_dict):
    """
    upload a benchmark result, either from web frontend or notebook.
    """
    bm = BenchmarkMeasurement()
    bm.benchmark_id = get_last_benchmark_id() + 1
    bm.context = context
    bm.input_bitwidth = common_utils.get_bitwidth(input_type)
    bm.input_signed = input_type.startswith("int")
    bm.circuit_name = circuit_name
    bm.is_correct = results_dict['cleartext check']['is_correct']
    bm.num_inputs = num_inputs
    bm.num_slots = num_slots
    bm.tbb_enabled = tbb_enabled
    session.add(bm)

    for k,v in results_dict['timings'].items():
        t = Timing()
        t.timing_name = k
        t.timing_value = float(v)
        t.benchmark_id = bm.benchmark_id
        session.add(t)
    for k,v in params_dict.items():
        p = ParameterSetting()
        p.param_name = k
        p.param_value = v
        p.benchmark_id = bm.benchmark_id
        session.add(p)
    session.commit()
    return True
