"""
SQLAlchemy classes to describe the tables of benchmark measurements
"""

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy import Column, ForeignKey, Integer, String, Boolean, Float
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import create_engine

Base = declarative_base()
engine = create_engine("sqlite:///sheep.db")

class BenchmarkMeasurement(Base):
    __tablename__ = "benchmarks"
    id = Column(Integer, primary_key=True, autoincrement=True)
    context_name = Column(String(250), nullable=False)
    input_bitwidth = Column(Integer, nullable=False)
    gate_name = Column(String(250), nullable=False)
    depth = Column(Integer, nullable=False)
    num_slots = Column(Integer, nullable=False)
    execution_time = Column(Float, nullable=False)
    is_correct = Column(Boolean, nullable=False)

Base.metadata.create_all(engine)

Base.metadata.bind = engine
DBSession = sessionmaker(bind=engine)
session = DBSession()

