import fast

streamer = fast.CameraStreamer.New()

renderer = fast.ImageRenderer.New()
renderer.addInputConnection(streamer.getOutputPort())

window = fast.SimpleWindow.New()
window.addRenderer(renderer)
window.set2DMode()
window.start()
