## @example stream_from_clarius_ultrasound_scanner.py
# This example will stream images from a clarius ultrasound scanner, apply a non-local-means filter
# and display it in real-time. Note: Do run this example you have to have:
# * A Clarius ultrasound probe running with the research cast API enabled.
# * Unfreeze the ultrasond probe.
# * Your machine has to be connected to the Clarius probe wifi.
# * If on windows, disable the windows firewall, or add an exception.
import fast

streamer = fast.ClariusStreamer.New()

filter = fast.NonLocalMeans.New()
filter.setInputConnection(streamer.getOutputPort())

renderer = fast.ImageRenderer.New()
renderer.addInputConnection(streamer.getOutputPort())

renderer2 = fast.ImageRenderer.New()
renderer2.addInputConnection(filter.getOutputPort())

window = fast.DualViewWindow.New()
window.addRendererToTopLeftView(renderer)
window.addRendererToBottomRightView(renderer2)
window.getTopLeftView().set2DMode()
window.getBottomRightView().set2DMode()
window.start()
