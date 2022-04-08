## @example render_to_image.py
# This example renders an image with FAST, then converts it to an image which is displayed by matplotlib

import fast
import matplotlib.pyplot as plt

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/US-2D.jpg')

segmentation = fast.BinaryThresholding.create(50).connect(importer)

renderer = fast.ImageRenderer.create().connect(importer)

segmentationRenderer = fast.SegmentationRenderer.create(opacity=0.1, borderOpacity=0.9) \
    .connect(segmentation)

toImage = fast.RenderToImage.create().connect([renderer, segmentationRenderer])
image = toImage.runAndGetOutputData()

plt.title('Rendered by FAST!')
plt.imshow(image)
plt.show()