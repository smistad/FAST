import fast
import os
import numpy as np
from time import sleep

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT)
fast.downloadTestDataIfNotExists()


class MyStreamer(fast.PythonRandomAccessStreamer):
    """
    A simple FAST random access streamer which runs in its own thread.
    By random access it is meant that it can move to any given frame index, thus
    facilitating playback with for instance PlaybackWidget.
    """
    def __init__(self):
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
        while True:
            # First, we need to check this streaming is paused
            if self.getPause():
                print('Paused, waiting')
                self.waitForUnpause() # Wait for streamer to be unpaused
                print('Done waiting')
            pause = self.getPause() # Check whether to pause or not
            print('Pause status', pause)
            frame = self.getCurrentFrameIndex()

            print('Streaming', frame)
            filepath = path.replace('#', str(frame))

            # Read frame from disk
            importer = fast.ImageFileImporter.create(filepath)
            image = importer.runAndGetOutputData()

            if not pause:
                if self.getFramerate() > 0:
                    sleep(1.0/self.getFramerate())
                self.getCurrentFrameIndexAndUpdate() # Update the frame index to the next frame
            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                break


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

        # Create new fast image and add as output
        new_output_image = fast.Image.createFromArray(np_image)
        new_output_image.setSpacing(image.getSpacing())
        self.addOutputData(0, new_output_image)


# Setup processing chain and run
streamer = MyStreamer.create()
streamer.setLooping(True)

inverter = Inverter.create().connect(streamer)

widget = fast.PlaybackWidget(streamer)

renderer = fast.ImageRenderer.create().connect(inverter)

window = fast.SimpleWindow2D.create()\
    .connect(renderer)
window.addWidget(widget)
window.run()
