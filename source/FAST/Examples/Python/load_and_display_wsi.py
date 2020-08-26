import fast

importer = fast.WholeSlideImageImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'WSI/A05.svs')

renderer = fast.ImagePyramidRenderer.New()
renderer.setInputConnection(importer.getOutputPort())

window = fast.SimpleWindow.New()
window.set2DMode()
window.addRenderer(renderer)
window.start()
