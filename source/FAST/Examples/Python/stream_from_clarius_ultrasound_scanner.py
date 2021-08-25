## @example stream_from_clarius_ultrasound_scanner.py
# This example will stream images from a clarius ultrasound scanner, apply a non-local-means filter
# and display it in real-time. Note: Do run this example you have to have:
# * A Clarius ultrasound probe running with the research cast API enabled.
# * Unfreeze the ultrasond probe.
# * Your machine has to be connected to the Clarius probe wifi.
# * If on windows, disable the windows firewall, or add an exception.
import fast

streamer = fast.ClariusStreamer.create()

filter = fast.NonLocalMeans.create().connect(streamer)

renderer = fast.ImageRenderer.create().connect(streamer)

renderer2 = fast.ImageRenderer.create().connect(filter)

fast.DualViewWindow2D.create()\
    .connectLeft(renderer)\
    .connectRight(renderer2)\
    .run()