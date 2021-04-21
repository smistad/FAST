## @example generate_tissue_patches_from_wsi.py
# This example loads a whole slide image (WSI), and generates a stream of
# patches of tissue in the WSI, and finally displays it using matplotlib
import fast
import matplotlib.pyplot as plt

#fast.Reporter.setGlobalReportMethod(fast.Reporter.COUT) # Uncomment to show debug info

fast.downloadTestDataIfNotExists() # This will download the test data needed to run the example

importer = fast.WholeSlideImageImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + 'WSI/A05.svs')

tissueSegmentation = fast.TissueSegmentation.New()
tissueSegmentation.setInputConnection(importer.getOutputPort())

patchGenerator = fast.PatchGenerator.New()
patchGenerator.setInputConnection(0, importer.getOutputPort())
patchGenerator.setInputConnection(1, tissueSegmentation.getOutputPort())
patchGenerator.setPatchSize(512, 512)

dataChannel = patchGenerator.getOutputPort()
patchGenerator.update() # Start patch generator pipeline

patch_list = []
while True:
    patch_list.append(dataChannel.getNextImage())
    if patch_list[-1].isLastFrame():
        break
    if len(patch_list) == 9:
        # Display the 9 last patches
        f, axes = plt.subplots(3,3, figsize=(10,10))
        for i in range(3):
            for j in range(3):
                axes[i, j].imshow(patch_list[i + j*3])
        plt.show()
        patch_list.clear()
