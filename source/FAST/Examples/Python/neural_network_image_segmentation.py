## @example neural_network_image_segmentation.py
# This example process a stream of ultrasound images with a neural network
# for image segmentation and displays the results on screen.
# @image html images/examples/python/neural_network_segmentation.jpg width=350px;
import fast

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info
fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

streamer = fast.ImageFileStreamer.create(
    fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_#.mhd',
    loop=True
)

segmentationNetwork = fast.SegmentationNetwork.create(
    fast.Config.getTestDataPath() + 'NeuralNetworkModels/jugular_vein_segmentation.onnx',
    scaleFactor=1./255.
).connect(streamer)

imageRenderer = fast.ImageRenderer.create().connect(streamer)

segmentationRenderer = fast.SegmentationRenderer.create(
    opacity=0.25,
    colors={1: fast.Color.Red(), 2: fast.Color.Blue()},
).connect(segmentationNetwork)

labelRenderer = fast.SegmentationLabelRenderer.create(
        labelNames={1: 'Artery', 2: 'Vein'},
        labelColors={1: fast.Color.Red(), 2: fast.Color.Blue()},
).connect(segmentationNetwork)

widget = fast.PlaybackWidget(streamer)

window = fast.SimpleWindow2D.create(bgcolor=fast.Color.Black())\
    .connect([imageRenderer, segmentationRenderer, labelRenderer])\
    .connect(widget)\
    .run()
