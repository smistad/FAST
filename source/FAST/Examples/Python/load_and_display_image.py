## @example load_and_display_image.py
# This example simply loads a metaimage (.mhd) and displays it on screen
# @image html images/examples/python/left_ventricle.jpg
import fast

# This will download the test data needed to run the example
fast.downloadTestDataIfNotExists()

importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_0.mhd')

renderer = fast.ImageRenderer.create().connect(importer)

window = fast.SimpleWindow2D.create(bgcolor=fast.Color.Black())\
    .connect(renderer)\
    .run()
