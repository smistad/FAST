## @example export_visualization_to_video.py
# This example process a stream of ultrasound images with a neural network
# for image segmentation and stores it to a video file using imageio.
# This example requires you to install imageio (pip install imageio imageio-ffmpeg)
# @image html images/examples/python/neural_network_segmentation.jpg width=350px;
import fast
import imageio

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info
fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

# Set up processing pipeline
streamer = fast.ImageFileStreamer.create(
    fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_#.mhd',
    loop=False
)

segmentationNetwork = fast.SegmentationNetwork.create(
    fast.Config.getTestDataPath() + 'NeuralNetworkModels/jugular_vein_segmentation.onnx',
    scaleFactor=1./255.,
    inferenceEngine='OpenVINO'
).connect(streamer)

# Set up rendering
imageRenderer = fast.ImageRenderer.create().connect(streamer)

segmentationRenderer = fast.SegmentationRenderer.create(
    opacity=0.25,
    colors={1: fast.Color.Red(), 2: fast.Color.Blue()},
).connect(segmentationNetwork)

labelRenderer = fast.SegmentationLabelRenderer.create(
        labelNames={1: 'Artery', 2: 'Vein'},
        labelColors={1: fast.Color.Red(), 2: fast.Color.Blue()},
).connect(segmentationNetwork)

# Render to image
renderToImage = fast.RenderToImage.create(bgcolor=fast.Color.Black())\
        .connect([imageRenderer, segmentationRenderer, labelRenderer])

# Collect all image frames
frames = []
for image in fast.DataStream(renderToImage):
    frames.append(image)
    print(len(frames))

# Save frames as video
imageio.mimsave('segmentation_video.mp4', frames, fps=20)
