## @example slicer_window_CT.py
# This example shows how the SlicerWindow can be used to visualize 3 slice planes of a 3D CT volume along with a segmentation.
# @image html images/examples/python/slicer_window_CT.jpg
import fast

importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + '/CT/CT-Thorax.mhd')

segmentation = fast.BinaryThresholding.create(150)\
    .connect(importer)

fast.SlicerWindow.create()\
    .connectImage(importer)\
    .connectSegmentation(segmentation, opacity=0.25, borderOpacity=0.75)\
    .run()