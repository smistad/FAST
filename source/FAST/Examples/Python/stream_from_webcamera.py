## @example stream_from_webcamera.py
# This example will stream images from your webcamera and run it through a simple edge detection filter (LaplacianOfGaussian)
# and display it in real-time.
import fast

streamer = fast.CameraStreamer.create()

filter = fast.LaplacianOfGaussian.create().connect(streamer)

renderer = fast.ImageRenderer.create().connect(filter)

window = fast.SimpleWindow2D.create().connect(renderer).run()
