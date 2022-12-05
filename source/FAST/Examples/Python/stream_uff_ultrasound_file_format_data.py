## @example stream_uff_ultrasound_file_format_data.py
# This example will stream ultrasound images from an ultrasound file format (UFF) file,
# apply a non-local-means filter and display it in real-time.
# @image html images/examples/python/stream_uff_data_and_filter.jpg width=400px;
import fast

streamer = fast.UFFStreamer.create(
    fast.Config.getTestDataPath() + "US/UFF/P4_2_PLAX.uff",
    framerate=5,
    loop=True,
)

filter = fast.NonLocalMeans.create(
    smoothingAmount=0.4,
    inputMultiplicationWeight=0.8
).connect(streamer)

renderer = fast.ImageRenderer.create()\
    .connect(streamer)

renderer2 = fast.ImageRenderer.create()\
    .connect(filter)

widget = fast.PlaybackWidget(streamer)
fast.DualViewWindow2D.create(
    bgcolor=fast.Color.Black(),
    width=1024,
    height=512
    ).connectLeft(renderer)\
    .connectRight(renderer2)\
    .connect(widget)\
    .run()