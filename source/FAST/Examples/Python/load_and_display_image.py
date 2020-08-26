import fast

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'US/US-2D.bmp')

renderer = fast.ImageRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
