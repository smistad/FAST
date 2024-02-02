import fast
from vbeam.util.download import cached_download
from vbeam_streamer import VBeamStreamer

# UFF dataset to download and run
data_url = "http://www.ustb.no/datasets/Verasonics_P2-4_parasternal_long_small.uff"
#data_url = "http://www.ustb.no/datasets/PICMUS_carotid_cross.uff"
#data_url = "http://www.ustb.no/datasets/PICMUS_carotid_long.uff"

# Setup vbeam streamer
streamer = VBeamStreamer.create(cached_download(data_url), is_sector_scan=True)

# Setup processing chain
logCompress = fast.EnvelopeAndLogCompressor.create().connect(streamer)

scanConvert = fast.ScanConverter.create(1024, 1024).connect(logCompress)

filter = fast.NonLocalMeans.create(filterSize=3, searchSize=11,
                                   inputMultiplicationWeight=0.25,
                                   smoothingAmount=0.1).connect(scanConvert)

# Setup widgets
widget = fast.PlaybackWidget(streamer)
gainSlider = fast.SliderWidget('Gain', 10, 0, 50, 5, fast.SliderCallback(lambda x: (scanConvert.setGain(x), streamer.refresh())))
dynamicRangeSlider = fast.SliderWidget('Dynamic Range', 60, 10, 100, 5, fast.SliderCallback(lambda x: (scanConvert.setDynamicRange(x), streamer.refresh())))
smoothingSlider = fast.SliderWidget('Smoothing', 0.10, 0.05, 0.4, 0.05, fast.SliderCallback(lambda x: (filter.setSmoothingAmount(x), streamer.refresh())))

# Setup rendering, window and run
renderer = fast.ImageRenderer.create().connect(filter)

fast.SimpleWindow2D.create(bgcolor=fast.Color.Black()) \
    .connect(renderer) \
    .connect(widget) \
    .connect([gainSlider, dynamicRangeSlider, smoothingSlider], fast.WidgetPosition_RIGHT) \
    .run()
