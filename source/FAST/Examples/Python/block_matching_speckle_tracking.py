## @example block_matching_speckle_tracking.py
# This example demonstrates the use of block matching to do speckle tracking on an ultrasound recording.
# The resulting vector/displacement field can be visualized with both matplotlib and FAST.
import fast
import matplotlib.pyplot as plt
import numpy as np

visualize_with_matplotlib = True    # Switch between using FAST and matplotlib for visualization

streamer = fast.ImageFileStreamer.create(fast.Config.getTestDataPath() + '/US/Heart/ApicalFourChamber/US-2D_#.mhd')

blockMatching = fast.BlockMatching.create(
        blockSize=13,
        searchSize=11,
        metric=fast.MatchingMetric_SUM_OF_ABSOLUTE_DIFFERENCES,
        timeLag=1,
        forwardBackwardTracking=False,
).connect(streamer)
blockMatching.setIntensityThreshold(75)

if visualize_with_matplotlib:
    frame_nr = 0
    for fast_image, vectorField in fast.DataStream(streamer, blockMatching):
        spacing = fast_image.getSpacing()
        image = np.asarray(fast_image)
        vectorField = np.asarray(vectorField)

        if frame_nr > 0: # Skip first frame
            plt.imshow(image[..., 0], cmap='gray', aspect=spacing[1]/spacing[0])
            # Show a downsampled vector field
            step = 8
            Y, X = np.mgrid[0:image.shape[0]:step, 0:image.shape[1]:step]
            plt.quiver(X, Y, vectorField[::step, ::step, 0], vectorField[::step, ::step, 1], color='r', scale=step*10)
            plt.show()

        frame_nr += 1

        if fast_image.isLastFrame():
            break

else:
    imageRenderer = fast.ImageRenderer.create().connect(streamer)
    vectorRenderer = fast.VectorFieldColorRenderer.create().connect(blockMatching)
    window = fast.SimpleWindow2D.create()\
        .connect(imageRenderer)\
        .connect(vectorRenderer)\
        .run()