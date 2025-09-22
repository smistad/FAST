# Check that user is importing FAST in main thread, which is necessary for Qt
import threading
if threading.current_thread() != threading.main_thread():
    raise ImportError('FAST must be imported in main thread (import fast)!')

# Check that PySide2 is not included before fast
import sys
#if 'fast' not in sys.modules and 'PySide2.QtCore' in sys.modules:
#    raise RuntimeError('You have to import fast before PySide2!\n\n'
#                       'Import FAST before PySide2 in the following way:\n'
#                       'if platform.system() != \'Windows\':\n'
#                       '    import PySide2.QtSvg # Must import this before fast due to conflicting symbols\n'
#                       'import fast # Must import FAST before rest of pyside2\n'
#                       'from PySide2.QtWidgets import *')

import os
is_windows = sys.platform.startswith('win')
# Set base path in config to be two directories up from where the python module is
bin_path = path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'bin'))
if is_windows:
    path = bin_path
else:
    path = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'lib'))
sys.path.append(path) # This is needed to find _fast binary python extension
os.environ['PATH'] = path + os.pathsep + os.environ['PATH'] # This is needed in order for C++ to dynamically load DLLs

from .fast import *
fast.Config.setBasePath(bin_path)
fast.Config.setTerminateHandlerDisabled(True)
if True not in [x in sys.argv[0] for x in ['UFFviewer', 'runPipeline', 'systemCheck']]:
    fast.ImageFileImporter.create('') # Trigger splash, GL context initialization etc.
