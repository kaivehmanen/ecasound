"Wrapper module which loads pyecasound (python module for Ecasound Control Interface)."

import sys

if sys.version_info[0] >= 2:
    if sys.version_info[1] >= 2:
        import DLFCN
        sys.setdlopenflags(DLFCN.RTLD_LAZY|DLFCN.RTLD_GLOBAL)
        
from pyeca import *
