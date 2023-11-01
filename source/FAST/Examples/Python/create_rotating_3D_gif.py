## @example create_rotating_3D_gif.py
# This example shows how you can create a rotating 3D GIF using FAST and imageio.
# Remember to install imageio (pip install imageio).
# A custom python process objects add a rotation around its own axis for every iteration.
# RenderToImage renders the results to a FAST image, which is convert to a ndarray and then given
# to imageio to save it as a GIF.
# @image html images/examples/python/rotating_3D.gif width=400px;
import fast
import numpy as np
import imageio
import math

iterations = 28 # Number of frames in GIF
size = 384 # Output size

class Rotate(fast.PythonProcessObject):
    def __init__(self, iterations):
        super().__init__()
        self.createInputPort(0)
        self.createOutputPort(0)
        self.angle = 0
        self.iterations = iterations

    def execute(self):
        input = self.getInputData()

        spacing = input.getSpacing()
        transform = fast.Transform.create()
        # Rotate volume the right way first
        transform.addRotation(3.14/2, np.array([1, 0, 0], dtype=np.float32))
        transform.addRotation(3.14, np.array([0, 1, 0], dtype=np.float32))
        # Do the rotation:
        transform.addRotation(self.angle, np.array([0, 0, 1], dtype=np.float32))
        # To rotate around the center, we have to move to volume so its center is at origo:
        transform.addTranslation(np.array([-input.getWidth()*spacing[0,0]/2, -input.getHeight()*spacing[1,0]/2, -input.getDepth()*spacing[2,0]/2], dtype=np.float32))
        input.setTransform(transform)

        self.angle += 2*math.pi/iterations # Increment angle to have it rotate over time
        self.setModified(True) # Do this to make sure this PO is called repeatadly
        self.addOutputData(0, input)

importer = fast.ImageFileImporter \
    .create(fast.Config.getTestDataPath() + "/CT/CT-Thorax.mhd")

rotation = Rotate.create(iterations)\
    .connect(importer)

renderer = fast.MaximumIntensityProjection.create() \
    .connect(rotation)

renderToImage = fast.RenderToImage.create(fast.Color.White(), size)\
    .connect(renderer)
frames = []
for i in range(iterations):
    print(i)
    frames.append(np.asarray(renderToImage.runAndGetOutputData()))
imageio.mimsave('rotating_3D.gif', frames, fps=5)
print('Done')