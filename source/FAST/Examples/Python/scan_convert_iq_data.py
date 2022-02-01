## @example scan_convert_iq_data.py
# This example will create some fake IQ data and perform envelope detection, log compression and scan conversion.
# @image html images/examples/python/scan_convert_iq_data.jpg
import fast
import numpy as np
import matplotlib.pyplot as plt

# Create some fake IQ beamspace data
iq_data = fast.Image.createFromArray(np.random.normal(size=(512,512,2)).astype(np.float32))

# Add some geometric data used for scan conversion
iq_data.setFrameData('isPolar', 'true')
iq_data.setFrameData('startRadius', '0')
iq_data.setFrameData('stopRadius', '0.12')
iq_data.setFrameData('startTheta', '-0.785398') # Start and stop angle in radians
iq_data.setFrameData('stopTheta', '0.785398')
iq_data.setFrameData('depthSpacing', '0.000235')
iq_data.setFrameData('azimuthSpacing', '0.003074')

# Create processing & visualization chain

envelope = fast.EnvelopeAndLogCompressor.create()\
        .connect(iq_data)

scan_convert = fast.ScanConverter.create()\
        .connect(envelope)

renderer = fast.ImageRenderer.create()\
        .connect(scan_convert)

fast.SimpleWindow2D.create()\
        .connect(renderer)\
        .run()

# Visualize output of scan converter with matplotlib instead of FAST:
plt.imshow(np.asarray(scan_convert.runAndGetOutputData())[..., 0], cmap='gray')
plt.show()
