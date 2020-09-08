"""
This example loads a video and converts to a stream of image frames and display the
individual frames with matplotlib.
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
    frame_list.append(dataChannel.getNextImage())
    if frame_list[-1].isLastFrame():
        break
    if len(frame_list) == 9:
        # Display the 9 last frames
        f, axes = plt.subplots(3,3, figsize=(10,10))
        for i in range(3):
            for j in range(3):
                axes[i, j].set_title('Frame: ' + str(counter))
                axes[i, j].imshow(frame_list[i + j*3], cmap='gray')
                counter += 1
        plt.show()
        frame_list.clear()
