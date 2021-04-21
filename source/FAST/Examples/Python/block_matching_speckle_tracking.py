## @example block_matching_speckle_tracking.py
# This example demonstrates the use of block matching to do speckle tracking on an ultrasound recording.
# The resulting vector/displacement field can be visualized with both matplotlib and FAST.
import fast
import matplotlib.pyplot as plt
import numpy as np

visualize_with_matplotlib = True    # Switch between using FAST and matplotlib for visualization

streamer = fast.ImageFileStreamer.New()
streamer.setFilenameFormat(fast.Config.getTestDataPath() + '/US/Heart/ApicalFourChamber/US-2D_#.mhd')

blockMatching = fast.BlockMatching.New()
blockMatching.setInputConnection(streamer.getOutputPort())
blockMatching.setMatchingMetric(fast.BlockMatching.MatchingMetric_SUM_OF_ABSOLUTE_DIFFERENCES)
blockMatching.setBlockSize(13)
blockMatching.setIntensityThreshold(75)
blockMatching.setTimeLag(1)
blockMatching.setForwardBackwardTracking(False)

if visualize_with_matplotlib:
    imageChannel = streamer.getOutputPort()
    vectorChannel = blockMatching.getOutputPort()

    frame_nr = 0
    while True:
        blockMatching.update()
        fast_image = imageChannel.getNextImage()
        spacing = fast_image.getSpacing()
        image = np.asarray(fast_image)
        vectorField = np.asarray(vectorChannel.getNextImage())

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
    imageRenderer = fast.ImageRenderer.New()
    imageRenderer.addInputConnection(streamer.getOutputPort())

    vectorRenderer = fast.VectorFieldColorRenderer.New()
    vectorRenderer.addInputConnection(blockMatching.getOutputPort())

    window = fast.SimpleWindow.New()
    window.set2DMode()
    window.addRenderer(imageRenderer)
    window.addRenderer(vectorRenderer)
    window.start()
