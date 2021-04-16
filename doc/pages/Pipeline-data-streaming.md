FAST supports streaming of data in multiple pipelines. Three different streaming modes are implemented:
* **PROCESS_ALL_FRAMES** - In this mode, all data frames are processed.
* **NEWEST_FRAME_ONLY** - Only the newest frame is kept. When a new frame is received, it replaces the old one. This mode is useful for live data streaming. E.g. streaming data from an ultrasound scanner.
* **STORE_ALL_FRAMES** - All frames are stored. This mode is useful if you want to implemented some playback feature, where you can scroll back and forth in time. However, this may use a lot of memory.

Data flows between process objects using a DataPort object which implements each of the three modes described above. To make sure everything is synchronized properly in a complex pipeline a timestep variable is also used to indicate which frame is currently being processed.

The main methods of the DataPort object are:
* **DataObject::pointer getNextFrame()** - This method will always return the next frame for the current timestep. If that frame has not arrived yet, the method will block until it arrives.
* **void addFrame(DataObject::pointer frame)** - This method will add the next data frame. If the PROCESS_ALL_FRAMES mode is used this method may block if the frame buffer is full. The size of the frame buffer can be modified, the default is 50 frames.

Both of these methods are protected with a mutex to ensure thread safety.

## Process all frames mode

In this mode, all data frames are processed in the correct order. In this mode, we also want to avoid overflowing the memory, this can happen if a streamer object produce frames faster than the pipeline is able to process. In computer science this is often referred to as the [producer-consumer problem](https://en.wikipedia.org/wiki/Producer–consumer_problem). The solution is to use a semaphore in the DataPort object.

## Newest frame only mode

In this mode, new data is always added to slot (current_timestep + 1), even if it replaces some other data. The DataPort will thus keep a maximum of two frames (timestep) and (timestep + 1). getNextFrame() will block if no frame has been received yet, this is implemented using a condition variable which will notify the blocking thread when the data arrives.

## Store all frames mode

In this mode, all frames are stored for all DataPorts. Warning: This may exhaust your memory. getNextFrame() will block if the frame for the current timestep has not arrived, this is implemented using a condition variable which will notify the blocking thread when the data arrives.

## Static data

In the case of static data, data which does not change from one timestep to the next, the data is simply moved to the next timestep. A data port is marked as having static data in the update call of the producing process object if the following conditions are met:
1. The process object did not execute. A process object executed if it is either modified or one if its parents has new data for it.
2. The process object is not a streamer.

## Concurrent compute and visualization

When rendering, FAST uses two main threads, one thread for rendering and one thread for computation. When you call start() on window the computation thread is created, while the main thread is used for visualization by Qt. The computation thread is implemented in a class called ComputationThread, by default it increments the timestep by 1 for each iteration. However, it can be paused, and the timestep may also be set manually. The computation thread does the following in order:
1. Update timestep
2. For each renderer: Call update(currentTimestep) on each of the renders input connections.
3. Call execute() on each renderer
4. If stop signal is set: Stop, else: Repeat from 1.

To make sure the rendering is synchronous, the renders will not accept new input data until the current frame has been rendered.
