## @example filter_image_and_display_with_matplotlib.py
# This example show how a FAST image can be converted to a numpy ndarray
# and displayed using matplotlib in python.
import fast
import numpy as np
import matplotlib.pyplot as plt

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

# Set up FAST pipeline
importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/Heart/ApicalFourChamber/US-2D_0.mhd')

filter = fast.NonLocalMeans.create().connect(importer)

# Execute pipeline and convert images to numpy arrays
input_image = importer.runAndGetOutputData()
pixel_spacing = input_image.getSpacing()
input_image = np.asarray(input_image)
filtered_image = np.asarray(filter.runAndGetOutputData())

# Display using matplotlib
f, axes = plt.subplots(1,2)
aspect = pixel_spacing[1] / pixel_spacing[0] # Compensate for anisotropic pixel spacing
axes[0].imshow(input_image[..., 0], cmap='gray', aspect=aspect)
axes[1].imshow(filtered_image[..., 0], cmap='gray', aspect=aspect)
plt.show()
