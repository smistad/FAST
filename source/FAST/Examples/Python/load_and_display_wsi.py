## @example load_and_display_wsi.py
# An example of loading a whole slide image (WSI) from disk and rendering it
# using an image pyramid renderer.
import fast

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

importer = fast.WholeSlideImageImporter.create(fast.Config.getTestDataPath() + 'WSI/A05.svs')

renderer = fast.ImagePyramidRenderer.create().connect(importer)

fast.SimpleWindow2D.create().connect(renderer).run()
