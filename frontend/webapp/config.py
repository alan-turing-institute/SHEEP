"""
Configuration for the sheep flask app
"""

import os

class SheepConfig:
    if "SHEEP_HOME" in os.environ.keys():
        SHEEP_HOME = os.environ["SHEEP_HOME"]
    else:
        SHEEP_HOME = os.path.join(os.environ["HOME"],
                                  "SHEEP","pysheep")
    UPLOAD_FOLDER = os.path.join(SHEEP_HOME, "webapp","uploads")
