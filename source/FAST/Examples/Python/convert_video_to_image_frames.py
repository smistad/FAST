"""
This example loads a video and converts to a stream of image frames and display the
individual frames with matplotlib.

Note that additional depedencies are required to stream videos in FAST:
Linux: sudo apt install libgstreamer1.0-dev libgstreamer-plugins-bad1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-good1.0-dev
Windows: K-lite codec pack https://codecguide.com/download_kl.htm
"""
import fast
import matplotlib.pyplot as plt

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

streamer = fast.MovieStreamer.New()
streamer.setFilename(fast.Config.getTestDataPath() + 'US/sagittal_spine.avi')

dataChannel = streamer.getOutputPort()
streamer.update() # Start pipeline

frame_list = []
counter = 0
while True:
    frame = dataChannel.getNextImage()
    counter += 1
    if frame.isLastFrame():
        break

    # Only show every X frame
    if counter % 20 == 0: frame_list.append((frame, counter))

    if len(frame_list) == 9:
        # Display the 9 last frames
        f, axes = plt.subplots(3,3, figsize=(10,10))
        for i in range(3):
            for j in range(3):
                axes[j, i].set_title('Frame: ' + str(frame_list[i + j*3][1]))
                axes[j, i].imshow(frame_list[i + j*3][0], cmap='gray')
        plt.show()
        frame_list.clear()
