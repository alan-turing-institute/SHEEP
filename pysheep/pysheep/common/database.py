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

#### the following regexes are only necessary for when we allow the user to
#### enter their own SQL query - shouldn't be there long-term!
import re
table_regex = re.compile("(FROM|from) ([\w]+)")
column_regex = re.compile("(SELECT|select) ([\*\w\,\s]+) (FROM|from)")

### location of the sqlite file holding the DB
### (at some point may replace with e.g. postgres DB, but
### this will only require minimal changes, thanks to sqlalchemy).

if "SHEEP_HOME" in os.environ.keys():
    DB_LOCATION = os.environ["SHEEP_HOME"]+"/pysheep/sheep.db"
else:
    DB_LOCATION = os.environ["HOME"]+"/SHEEP/pysheep/sheep.db"

Base = declarative_base()
engine = create_engine("sqlite:///"+DB_LOCATION)


class BenchmarkMeasurement(Base):
    __tablename__ = "benchmarks"
    id = Column(Integer, primary_key=True, autoincrement=True,nullable=False)
    context_name = Column(String(250), nullable=False)
    input_bitwidth = Column(Integer, nullable=False)
    input_signed = Column(Boolean, nullable=False)    
    gate_name = Column(String(250), nullable=True)
    circuit_name = Column(String(250), nullable=True)
    depth = Column(Integer, nullable=True)
    num_inputs = Column(Integer, nullable=True)    
    num_slots = Column(Integer, nullable=True)
    tbb_enabled = Column(Boolean, nullable=True)
    setup_time = Column(Float, nullable=True)
    encryption_time = Column(Float, nullable=True)    
    execution_time = Column(Float, nullable=False)
    is_correct = Column(Boolean, nullable=False)
    ciphertext_size = Column(Integer, nullable=True)
    private_key_size = Column(Integer, nullable=True)
    public_key_size = Column(Integer, nullable=True)        
##### add all the parameters for all the contexts
    HElib_BaseParamSet = Column(Integer, nullable=True)
    HElib_BitsPerLevel = Column(Integer, nullable=True)
    HElib_HamingWeight = Column(Integer, nullable=True)
    HElib_Bootstrap = Column(Integer, nullable=True)
    HElib_c = Column(Integer, nullable=True)
    HElib_d = Column(Integer, nullable=True)
    HElib_g1 = Column(Integer, nullable=True)
    HElib_g2 = Column(Integer, nullable=True)
    HElib_g3 = Column(Integer, nullable=True)
    HElib_Levels = Column(Integer, nullable=True)
    HElib_m = Column(Integer, nullable=True)
    HElib_m1 = Column(Integer, nullable=True)
    HElib_m2 = Column(Integer, nullable=True)
    HElib_m3 = Column(Integer, nullable=True)
    HElib_ord1 = Column(Integer, nullable=True)
    HElib_ord2 = Column(Integer, nullable=True)
    HElib_ord3 = Column(Integer, nullable=True)
    HElib_phim = Column(Integer, nullable=True)    
    TFHE_MinimumLambda = Column(Integer, nullable=True)
    SEAL_PlaintextModulus = Column(Integer, nullable=True)
    SEAL_N = Column(Integer, nullable=True)    
    SEAL_Security = Column(Integer, nullable=True)


    
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
        
def execute_query_sqlite3(query):
    """
    raw sql query
    """
    table,columns = get_table_and_columns(query)
    db = sqlite3.connect(DB_LOCATION)
    cursor = db.cursor()
    ### get the column headings, if e.g. '*' was used in the query
    if table and (len(columns) == 0 or columns[0] == "*"):
        cursor.execute("PRAGMA table_info("+table+");")
        columns_raw = cursor.fetchall()
        columns = []
        for c in columns_raw:
            columns.append(c[1])
    ### now execute the query    
    cursor.execute(query)
    output = cursor.fetchall()
    return columns, output

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
