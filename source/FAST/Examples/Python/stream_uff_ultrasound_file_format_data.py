## @example stream_uff_ultrasound_file_format_data.py
# This example will stream ultrasound images from an ultrasound file format (UFF) file,
# apply a non-local-means filter and display it in real-time.
import fast

streamer = fast.UFFStreamer.create(
    'path/to/file.uff',
    framerate=5,
    loop=True
)

filter = fast.NonLocalMeans.create().connect(streamer)

renderer = fast.ImageRenderer.create().connect(streamer)

renderer2 = fast.ImageRenderer.create().connect(filter)

fast.DualViewWindow2D.create(bgcolor=fast.Color.Black())\
    .connectLeft(renderer)\
    .connectRight(renderer2)\
    .run()