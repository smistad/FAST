# Check that user is importing FAST in main thread, which is necessary for Qt
import threading
if threading.current_thread() != threading.main_thread():
    raise ImportError('FAST must be imported in main thread (import fast)!')

import os
import sys
is_windows = sys.platform.startswith('win')
# Set base path in config to be two directories up from where the python module is
bin_path = path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'bin'))
if is_windows:
    path = bin_path
    sys.path.append(bin_path) # This is needed to find _fast binary python extension
else:
    path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'lib'))
    sys.path.append(path) # This is needed to find _fast binary python extension
os.environ['PATH'] = path + os.pathsep + os.environ['PATH'] # This is needed in order for C++ to dynamically load DLLs

from typing import *
from .fast_core import *
from .fast_data import *
from .fast_processobjects import *
from .fast_importers import *
from .fast_exporters import *
from .fast_streamers import *
from .fast_algorithms import *
from .fast_visualization import *
Config.setBasePath(bin_path)
Config.setTerminateHandlerDisabled(True)
if True not in [x in sys.argv[0] for x in ['UFFviewer', 'runPipeline', 'systemCheck']]:
    ImageFileImporter.create('') # Trigger splash, GL context initialization etc.
