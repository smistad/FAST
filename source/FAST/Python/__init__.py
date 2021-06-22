import os
import sys
is_windows = sys.platform.startswith('win')
# Set base path in config to be two directories up from where the python module is
bin_path = path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'bin'))
if is_windows:
    path = bin_path
else:
    path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'lib'))
sys.path.append(bin_path) # This is needed to find _fast binary python extension
os.environ['PATH'] = path + os.pathsep + os.environ['PATH'] # This is needed in order for C++ to dynamically load DLLs

from .fast import *
fast.Config.setBasePath(bin_path)
fast.Config.setTerminateHandlerDisabled(True)
fast.Object() # Trigger splash
