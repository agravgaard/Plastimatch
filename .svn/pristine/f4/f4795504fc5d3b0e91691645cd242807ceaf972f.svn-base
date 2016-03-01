"""
This Python module manages the I/O of pfm files (www.plastimatch.org).
pfm <----> numpy matirx
NOT TESTED ON PYTHON 3
Author: Paolo Zaffino (p dot zaffino at unicz dot it)
"""

import numpy as np

def load_pfm(fn):
    
    if fn.endswith(".pfm"):
        fid = open(fn, "rb")
    else:
        print("No pfm file! \n")
        return
    
    raw_data = fid.readlines()
    fid.close()

    cols = int(raw_data[1].strip().split(" ")[0])
    rows = int(raw_data[1].strip().split(" ")[1])
    
    del raw_data[2]
    del raw_data[1]
    del raw_data[0]
    
    image = np.fromstring("".join(raw_data), dtype=np.float32)
    del raw_data
    image = image.reshape(cols, rows).T

    return image.astype(np.float32)

