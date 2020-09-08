"""
An example of loading a whole slide image (WSI) from disk and rendering it
using an image pyramid renderer.
"""
import fast

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

importer = fast.WholeSlideImageImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'WSI/A05.svs')

renderer = fast.ImagePyramidRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
