## @example neural_network_image_segmentation.py
# This example process a stream of ultrasound images with a neural network
# for image segmentation and displays the results on screen.
import fast

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info
fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

streamer = fast.ImageFileStreamer.New()
streamer.setFilenameFormat(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_#.mhd')
streamer.enableLooping()

segmentationNetwork = fast.SegmentationNetwork.New()
segmentationNetwork.setInputConnection(streamer.getOutputPort())
segmentationNetwork.setInferenceEngine('OpenVINO')
segmentationNetwork.setScaleFactor(1/255)
segmentationNetwork.load(fast.Config.getTestDataPath() +
    'NeuralNetworkModels/jugular_vein_segmentation.xml') 

imageRenderer = fast.ImageRenderer.New()
imageRenderer.addInputConnection(streamer.getOutputPort())

segmentationRenderer = fast.SegmentationRenderer.New()
segmentationRenderer.addInputConnection(segmentationNetwork.getOutputPort())
segmentationRenderer.setOpacity(0.25)
segmentationRenderer.setColor(1, fast.Color.Red())
segmentationRenderer.setColor(2, fast.Color.Blue())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(imageRenderer)
window.addRenderer(segmentationRenderer)
window.getView().setBackgroundColor(fast.Color.Black())
window.start()
