## @example display_ultrasound_file_format_data_with_matplotlib.py
# This example will stream ultrasound images from an ultrasound file format (UFF) file,
# and display consecutive frames in a 2x2 matplotlib subplot
# @image html images/examples/python/display_uff_data.jpg width=400px;
import fast
import matplotlib.pyplot as plt
import numpy as np

streamer = fast.UFFStreamer.create(
    fast.Config.getTestDataPath() + 'US/UFF/P4_2_A4C.uff',
    gain=0,
    dynamicRange=60,
    #doScanConversion=False # Set to False to get images in beamspace
    #framerate=5,
)

# Create a 2x2 subplot for every set of 4 image frames
image_list = []
for image in fast.DataStream(streamer):
    image_list.append(np.asarray(image))
    if len(image_list) == 4:
        # Display the 4 last images
        f, axes = plt.subplots(2, 2, figsize=(10,10))
        for i in range(2):
            for j in range(2):
                axes[i, j].imshow(image_list[i + j*2][..., 0], cmap='gray')
        plt.show()
        image_list.clear()
        break # Stop loop
