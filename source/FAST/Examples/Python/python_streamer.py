import fast
import os
import numpy as np
import cv2
use_opencv = True

fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT)

class Streamer(fast.PythonStreamer):
    def __init__(self):
        super().__init__()
        self.createOutputPort(0)
        self.setModified(True)
        print('Constructor ok')

    def generateStream(self):
        print('asd')
        path = fast.Config.getTestDataPath() + '/US/Heart/ApicalFourChamber/US-2D_#.mhd'
        frame = 0
        while(True):
            print(frame)
            filepath = path.replace('#', str(frame))
            if not os.path.exists(filepath):
                print('Stopped')
                break

            # Read frame from disk
            importer = fast.ImageFileImporter.create(filepath)
            image = importer.runAndGetOutputData()

            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                print('Stopped 2')
                break
            frame += 1



""" Make a python process object which simply inverts image with numpy """
class Inverter(fast.PythonProcessObject):
    def __init__(self):
        super().__init__()
        self.createInputPort(0)
        self.createOutputPort(0)

    def execute(self):
        # Get image and invert it with numpy
        image = self.getInputData()
        np_image = np.asarray(image)
        np_image = 255 - np_image # invert

        # If OpenCV is available, add some text using OpenCV
        if use_opencv:
            cv2.putText(np_image, 'OpenCV!', (40, 20), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,0,0), 2)

        # Create new fast image and add as output
        new_output_image = fast.Image.createFromArray(np_image)
        new_output_image.setSpacing(image.getSpacing())
        self.addOutputData(0, new_output_image)

streamer = Streamer.create()

inverter = Inverter.create().connect(streamer)

renderer = fast.ImageRenderer.create().connect(inverter)

fast.SimpleWindow2D.create().connect(renderer).run()