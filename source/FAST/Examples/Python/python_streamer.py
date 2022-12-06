## @example python_streamer.py
# An example showing how to make a FAST streamer in python.
# A FAST streamer is a special process object which inherits from
# Streamer, and generates data asynchronously.
# This can for instance be streaming data from an ultrasound scanner in real-time or from disk.
# @image html images/examples/python/left_ventricle.jpg width=400px;
import fast
import os
import numpy as np
from time import sleep

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT)
fast.downloadTestDataIfNotExists()

class MyStreamer(fast.PythonStreamer):
    """
    A simple FAST streamer which runs in its own thread.
    This streamer reads a series of MHD images on disk.
    This can be done easily with the ImageFileStreamer, but this is just an example.
    """

    def __init__(self):
        """
        Constructor, remember to create the output port here
        """
        super().__init__()
        self.createOutputPort(0)

    def generateStream(self):
        """
        This method runs in its own thread.
        Run you streaming loop here.
        Remember to call self.addOutputData and self.frameAdded for each frame.
        If these calls return and exception, it means the streaming should stop, thus you need to exit
        your streaming loop
        """
        path = fast.Config.getTestDataPath() + '/US/Heart/ApicalFourChamber/US-2D_#.mhd'
        frame = 0
        while True:
            print('Streaming', frame)
            filepath = path.replace('#', str(frame))
            if not os.path.exists(filepath):
                # No more frames on disk, restart streaming loop
                frame = 0
                continue

            # Read frame from disk
            importer = fast.ImageFileImporter.create(filepath)
            image = importer.runAndGetOutputData()

            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
                sleep(0.02)
            except:
                # Streaming has been requested to stop. Thus we break the while loop
                break
            frame += 1


# Setup processing chain and run
streamer = MyStreamer.create()

renderer = fast.ImageRenderer.create().connect(streamer)

fast.SimpleWindow2D.create().connect(renderer).run()
