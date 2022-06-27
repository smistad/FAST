import fast
import numpy as np
import pytest

def test_data_stream_single():
    streamer = fast.ImageFileStreamer\
        .create(fast.Config.getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd")

    dataStream = fast.DataStream(streamer)
    previousImage = ''
    counter = 0
    for image in dataStream:
        assert previousImage != image
        assert image.getWidth() == 234
        assert image.getHeight() == 591
        previousImage = image
        counter += 1
    assert counter == 100

def test_data_stream_multiple():
    streamer1 = fast.ImageFileStreamer \
        .create(fast.Config.getTestDataPath() + "/US/Heart/ApicalFourChamber/US-2D_#.mhd")
    streamer2 = fast.ImageFileStreamer \
        .create(fast.Config.getTestDataPath() + "/US/Heart/ApicalTwoChamber/US-2D_#.mhd")
    streamer3 = fast.ImageFileStreamer \
        .create(fast.Config.getTestDataPath() + "/US/Heart/ApicalLongAxis/US-2D_#.mhd")

    dataStream = fast.DataStream(streamer1, streamer2, streamer3)
    previousImage1 = ''
    previousImage2 = ''
    previousImage3 = ''
    counter = 0
    for image1, image2, image3 in dataStream:
        assert previousImage1 != image1
        assert previousImage2 != image2
        assert previousImage3 != image3
        assert image1.getWidth() == 234
        assert image1.getHeight() == 591
        assert image2.getWidth() == 234
        assert image2.getHeight() == 591
        assert image3.getWidth() == 234
        assert image3.getHeight() == 591
        previousImage1 = image1
        previousImage2 = image2
        previousImage3 = image3
        counter += 1
    assert counter == 100
