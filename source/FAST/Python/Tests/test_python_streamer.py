import fast
import pytest
import numpy as np
import os

from time import sleep


class MyStreamer(fast.PythonStreamer):
    """
    A simple FAST streamer which runs in its own thread.
    """

    def __init__(self):
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
        stop = False
        while not self.isStopped() and not stop:
            print('Streaming', frame)

            # Read frame from disk
            importer = fast.ImageFileImporter.create(path.replace('#', str(frame)))
            image = importer.runAndGetOutputData()

            # Check if this was the last frame
            if not os.path.exists(path.replace('#', str(frame+1))):
                # If last frame, we need to mark it as 'last frame'
                image.setLastFrame('MyStreamer')
                stop = True
            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                # Streaming has been requested to stop. Thus we break the while loop
                break
            frame += 1


""" A python process object which simply inverts an image with numpy """
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


def test_python_streamer():
    streamer = MyStreamer.create()

    inverter = Inverter.create().connect(streamer)

    counter = 0
    previousImage = ''
    stream = fast.DataStream(inverter)
    for image in stream:
        assert previousImage != image
        assert image.getWidth() == 234
        assert image.getHeight() == 591
        previousImage = image
        counter += 1
    print('ok')
    assert counter == 100


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
        self.setFramerate(100)

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
            # First, we need to check this streaming is paused
            if self.getPause():
                self.waitForUnpause() # Wait for streamer to be unpaused
            pause = self.getPause() # Check whether to pause or not
            frame = self.getCurrentFrameIndex()

            print('Streaming', frame)

            # Read frame from disk
            importer = fast.ImageFileImporter.create(path.replace('#', str(frame)))
            image = importer.runAndGetOutputData()
            if frame == self.getNrOfFrames()-1: # If this is last frame, mark it as such
                image.setLastFrame('MyRandomAccessStreamer')

            if not pause:
                if self.getFramerate() > 0:
                    sleep(1.0/self.getFramerate()) # Sleep to give the requested framerate
                self.getCurrentFrameIndexAndUpdate() # Update the frame index to the next frame
            try:
                self.addOutputData(0, image)
                self.frameAdded() # Important to notify any listeners
            except:
                break


def test_python_random_access_streamer():
    streamer = MyRandomAccessStreamer.create()

    inverter = Inverter.create().connect(streamer)

    counter = 0
    previousImage = ''
    stream = fast.DataStream(inverter)
    for image in stream:
        assert previousImage != image
        assert image.getWidth() == 234
        assert image.getHeight() == 591
        previousImage = image
        counter += 1
    print('ok')
    assert counter == 100
