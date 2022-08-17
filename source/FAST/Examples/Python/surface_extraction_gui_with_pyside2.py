## @example surface_extraction_gui_with_pyside2.py
# This example demonstrates how to use FAST together with Qt Python GUI with PySide2.
# The GUI application performs Gaussian smoothing and marching cubes surface extraction on a CT thorax volume.
#
# @m_class{m-block m-warning} @par PySide2 Qt Version
# @parblock
#     For this example you <b>must</b> use the same Qt version of PySide2 as used in FAST (5.15.2)
#     Do this with: <b>pip install pyside2==5.15.2.1</b>
# @endparblock
#
# @image html images/examples/python/pyside_surface_extraction.jpg

import platform
if platform.system() != 'Windows':
    import PySide2.QtSvg # Must import this before fast due to conflicting symbols
import fast # Must import FAST before rest of pyside2
from PySide2.QtWidgets import *
from PySide2.QtCore import Qt
from shiboken2 import wrapInstance
from random import random

fast.downloadTestDataIfNotExists()

# Create FAST Pipeline and window
importer = fast.ImageFileImporter\
        .create(fast.Config.getTestDataPath() + 'CT/CT-Abdomen.mhd')

smoothing = fast.GaussianSmoothing\
        .create(stdDev=1.0)\
        .connect(importer)

surfaceExtraction = fast.SurfaceExtraction\
        .create(threshold=300)\
        .connect(smoothing)

renderer = fast.TriangleRenderer.create()\
    .connect(surfaceExtraction)

window = fast.SimpleWindow3D.create(width=1024, height=512)\
    .connect(renderer)

 # Get the underlying QtWidget of the FAST window and convert it to pyside2
mainWidget = wrapInstance(int(window.getWidget()), QWidget)

# Create GUI in Qt
layout = mainWidget.layout()
menuWidget = QWidget()
layout.addWidget(menuWidget)
menuLayout = QVBoxLayout()
menuWidget.setLayout(menuLayout)
menuLayout.setAlignment(Qt.AlignTop)
title = QLabel('<h3>Python GUI Example</h3>')
menuWidget.setFixedWidth(400)
menuLayout.addWidget(title)

# Threshold GUI
menuLayout.addWidget(QLabel('Threshold:'))
threshold_slider = QSlider(Qt.Horizontal)
threshold_slider.setRange(100, 500)
threshold_slider.setValue(300)
threshold_slider.setSingleStep(10)
# Connect slider to FAST
threshold_slider.valueChanged.connect(lambda x: surfaceExtraction.setThreshold(x))
menuLayout.addWidget(threshold_slider)

# Smoothing GUI
menuLayout.addWidget(QLabel('Smoothing:'))
smoothing_slider = QSlider(Qt.Horizontal)
smoothing_slider.setValue(1)
smoothing_slider.setRange(1, 3)
smoothing_slider.setSingleStep(1)
# Connect slider to FAST
smoothing_slider.valueChanged.connect(lambda x: smoothing.setStandardDeviation(x))
menuLayout.addWidget(smoothing_slider)

# Run everything!
window.run()

