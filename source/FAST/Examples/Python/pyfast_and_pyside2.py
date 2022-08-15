## @example pyfast_and_pyside2.py
# This example demonstrates how to use FAST together with Qt Python GUI with PySide2
#
# @m_class{m-block m-warning} @par PySide2 Qt Version
# @parblock
#     For this example you <b>must</b> use the same Qt version of PySide2 as used in FAST (5.15.2)
#     Do this with: <b>pip install pyside2==5.15.2.1</b>
# @endparblock
#
# @image html images/examples/python/pyfast_and_pyside.jpg width=400px;

import platform
if platform.system() != 'Windows':
    import PySide2.QtSvg # Must import this before fast due to conflicting symbols
import fast # Must import FAST before rest of pyside2
from PySide2.QtWidgets import *
from shiboken2 import wrapInstance
from random import random

# Setup a FAST pipeline
importer = fast.ImageFileImporter\
    .create(fast.Config.getDocumentationPath() + '/images/FAST_logo_square.png')

renderer = fast.ImageRenderer.create()\
    .connect(importer)

window = fast.SimpleWindow2D.create()\
    .connect(renderer)
window.setTitle('pyFAST+PySide2=TRUE')

# Add Qt button to the FAST window
button = QPushButton("Press here!")
# Convert FAST QWidget pointer to a PySide2 QWidget
mainWidget = wrapInstance(int(window.getWidget()), QWidget)
layout = mainWidget.layout()
layout.addWidget(button)

# Add event: When button is clicked, change background color of the FAST view randomly
button.clicked.connect(lambda: window.getView().setBackgroundColor(fast.Color(random(), random(), random())))

# Run everything!
window.run()
