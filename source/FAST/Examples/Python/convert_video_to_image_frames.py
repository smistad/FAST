## @example convert_video_to_image_frames.py
# This example loads a video and converts to a stream of image frames and display the
# individual frames with matplotlib.
#
# Note that additional dependencies are required to stream videos in FAST:
# Linux: sudo apt install ubuntu-restricted-extras libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev
# Windows: K-lite codec pack https://codecguide.com/download_kl.htm
import fast
import matplotlib.pyplot as plt
import numpy as np

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

streamer = fast.MovieStreamer.create(fast.Config.getTestDataPath() + 'US/sagittal_spine.avi')

frame_list = []
counter = 0
for frame in fast.DataStream(streamer):
    counter += 1

    # Only show every X frame
    if counter % 20 == 0: frame_list.append((np.asarray(frame), counter))

    if len(frame_list) == 9:
        # Display the 9 last frames
        f, axes = plt.subplots(3,3, figsize=(10,10))
        for i in range(3):
            for j in range(3):
                axes[j, i].set_title('Frame: ' + str(frame_list[i + j*3][1]))
                axes[j, i].imshow(frame_list[i + j*3][0][..., 0], cmap='gray')
        plt.show()
        frame_list.clear()
