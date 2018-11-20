"""
test that we can upload files where we expect to.
"""

import os

from werkzeug.datastructures import FileStorage

from pysheep.frontend import frontend_utils

if "SHEEP_HOME" in os.environ.keys():
    SHEEP_HOME = os.environ["SHEEP_HOME"]
else:
    SHEEP_HOME = os.path.join(os.environ["HOME"],"SHEEP")

UPLOAD_FOLDER = os.path.join(SHEEP_HOME,"pysheep","pysheep","tests","tmp")
RESOURCE_FOLDER = os.path.join(SHEEP_HOME,"pysheep","pysheep","tests","testfiles")


def test_clean_upload_folder():
    """
    test that we get an empty directory
    """
    frontend_utils.cleanup_upload_dir({"UPLOAD_FOLDER": UPLOAD_FOLDER})
    assert(os.path.exists(UPLOAD_FOLDER))
    assert(len(os.listdir(UPLOAD_FOLDER)) == 0)



def test_file_upload():
    """
    test the frontend_utils upload_files function
    """

    with open(os.path.join(RESOURCE_FOLDER, "simple_add.sheep"), 'rb') as fp:
        fs = FileStorage(fp)

        testdict = {"test_add" : fs}
        uploaded_filenames = frontend_utils.upload_files(testdict, UPLOAD_FOLDER)
        assert(os.path.exists(uploaded_filenames["test_add"]))
