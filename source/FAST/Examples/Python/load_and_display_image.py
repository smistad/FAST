## @example load_and_display_image.py
# This example simply loads a metaimage (.mhd) and displays it on screen
# @image html images/examples/python/left_ventricle.jpg
import fast

# This will download the test data needed to run the example
fast.downloadTestDataIfNotExists()

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_0.mhd')

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
