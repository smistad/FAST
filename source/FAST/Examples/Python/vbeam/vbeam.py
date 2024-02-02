## @example vbeam.py
# This example shows how to use [vbeam](https://github.com/magnusdk/vbeam) by Magnus Kvalevåg to beamform ultrasound channel data
# stored in the ultrasound file format (UFF) in python and then process the resulting IQ data to end up with a scanconverted filtered image on screen.
# Note that this example requires you to [install vbeam and jax](https://github.com/magnusdk/vbeam#installation).
# It has been tested with version vbeam==1.05 and jax==0.4.16
# @image html images/examples/python/vbeam_example.jpg width=350px;
#
# To do this we first create a custom PythonRandomAccessStreamer with FAST where we use vbeam to load
# and beamform some UFF data as shown below.
#
# vbeam_streamer.py:
# @include vbeam_streamer.py
#
# We can now use this streamer in a FAST pipeline were we apply some simple envelope and log compression to the IQ data
# and then scan convert and filter the final image using Non-Local Means.
# We can also add some slider widgets to the window to let the user control the gain, dynamic range and smoothing amount
# as well as playback widget to easily play/stop and scroll in the recording.
# @include vbeam_streamer_pipeline.py
