## @example python_random_access_streamer.py
# An example showing how to make a FAST random access streamer in python.
# A FAST streamer is a special process object which generates data asynchronously.
# This can for instance be streaming data from an ultrasound scanner in real-time or from disk.
# A random access streamer can move to any given frame index at any time, thus enabling
# controllable playback with for instance the PlaybackWidget as shown in this example.
# @image html images/examples/python/python_random_access_streamer.jpg width=400px;
import fast
import os
import numpy as np
from time import sleep

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT)
fast.downloadTestDataIfNotExists()


class MyRandomAccessStreamer(fast.PythonRandomAccessStreamer):
    """
    A simple FAST random access streamer which runs in its own thread.
    By random access it is meant that it can move to any given frame index, thus
    facilitating playback with for instance PlaybackWidget.
    This streamer reads a series of MHD images on disk.
    This can be done easily with the ImageFileStreamer, but this is just an example.
    """
    def __init__(self):
        """
        Constructor, remember to create the output port here 
        """
        super().__init__()
        self.createOutputPort(0)
        self.setFramerate(30)

    def getNrOfFrames(self):
        """
        This function must return how many frames the streamer has.
        :return: nr of frames
        """
        return 100

    def generateStream(self):
        """
        This method runs in its own thread.
        Run you streaming loop here.
        Remember to call self.addOutputData and self.frameAdded for each frame.
        If these calls return and exception, it means the streaming should stop, thus you need to exit
        your streaming loop.
        """
        path = fast.Config.getTestDataPath() + '/US/Heart/ApicalFourChamber/US-2D_#.mhd'
        while not self.isStopped():
            # First, we need to check if this streaming is paused
            if self.getPause():
                self.waitForUnpause() # Wait for streamer to be unpaused
            pause = self.getPause() # Check whether to pause or not
            frame = self.getCurrentFrameIndex()

            print('Streaming', frame)

            # Read frame from disk
            importer = fast.ImageFileImporter.create(path.replace('#', str(frame)))
            image = importer.runAndGetOutputData()
            if frame == self.getNrOfFrames()-1: # If this is last frame, mark it as such
                image.setLastFrame('MyStreamer')

            if not pause:
                if self.getFramerate() > 0:
                    sleep(1.0/self.getFramerate()) # Sleep to give the requested framerate
                self.getCurrentFrameIndexAndUpdate() # Update the frame index to the next frame
            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                break


# Setup processing chain and run
streamer = MyRandomAccessStreamer.create()
streamer.setLooping(True)

renderer = fast.ImageRenderer.create().connect(streamer)

window = fast.SimpleWindow2D.create()\
    .connect(renderer)
widget = fast.PlaybackWidget(streamer) # GUI widget for controlling playback
window.addWidget(widget)
window.run()
