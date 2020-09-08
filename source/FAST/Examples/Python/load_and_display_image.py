"""
This example simply loads an metaimage (.mhd) and displays it on screen
"""
import fast

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_0.mhd')

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
