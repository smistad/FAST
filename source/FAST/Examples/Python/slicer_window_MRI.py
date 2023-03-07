## @example slicer_window_MRI.py
# This example shows how the SlicerWindow can be used to visualize 3 slice planes of a 3D MRI volume along with a segmentation.
# @image html images/examples/python/slicer_window_MRI.jpg
import fast

importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + '/MRI/MR-Abdomen.mhd')

segmentation = fast.BinaryThresholding.create(600)\
    .connect(importer)

fast.SlicerWindow.create()\
    .connectImage(importer)\
    .connectSegmentation(segmentation, opacity=0.25, borderOpacity=0.75, colors={1: fast.Color.Red()})\
    .run()