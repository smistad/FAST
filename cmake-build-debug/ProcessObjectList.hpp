#ifndef FAST_PROCESS_OBJECT_LIST_HPP_
#define FAST_PROCESS_OBJECT_LIST_HPP_

#include "FAST/ProcessObjectRegistry.hpp"

#include "FAST/Streamers/ImageFileStreamer.hpp"
#include "FAST/Streamers/OpenIGTLinkStreamer.hpp"
#include "FAST/Streamers/MovieStreamer.hpp"
#include "FAST/Streamers/CameraStreamer.hpp"
#include "FAST/Streamers/RealSenseStreamer.hpp"
#include "FAST/Streamers/ClariusStreamer.hpp"
#include "FAST/Algorithms/AddTransformation/AddTransformation.hpp"
#include "FAST/Algorithms/AddTransformation/SetTransformation.hpp"
#include "FAST/Algorithms/AirwaySegmentation/AirwaySegmentation.hpp"
#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"
#include "FAST/Algorithms/BlockMatching/BlockMatching.hpp"
#include "FAST/Algorithms/CenterlineExtraction/CenterlineExtraction.hpp"
#include "FAST/Algorithms/GradientVectorFlow/EulerGradientVectorFlow.hpp"
#include "FAST/Algorithms/GradientVectorFlow/MultigridGradientVectorFlow.hpp"
#include "FAST/Algorithms/HounsefieldConverter/HounsefieldConverter.hpp"
#include "FAST/Algorithms/ImageChannelConverter/ImageChannelConverter.hpp"
#include "FAST/Algorithms/ImageCropper/ImageCropper.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Algorithms/ImageInverter/ImageInverter.hpp"
#include "FAST/Algorithms/ImageMultiply/ImageMultiply.hpp"
#include "FAST/Algorithms/ImagePatch/PatchGenerator.hpp"
#include "FAST/Algorithms/ImagePatch/PatchStitcher.hpp"
#include "FAST/Algorithms/ImagePatch/ImageToBatchGenerator.hpp"
#include "FAST/Algorithms/ImageResampler/ImageResampler.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"
#include "FAST/Algorithms/ImageSlicer/ImageSlicer.hpp"
#include "FAST/Algorithms/IterativeClosestPoint/IterativeClosestPoint.hpp"
#include "FAST/Algorithms/LaplacianOfGaussian/LaplacianOfGaussian.hpp"
#include "FAST/Algorithms/LevelSet/LevelSetSegmentation.hpp"
#include "FAST/Algorithms/LungSegmentation/LungSegmentation.hpp"
#include "FAST/Algorithms/MeshToSegmentation/MeshToSegmentation.hpp"
#include "FAST/Algorithms/Morphology/Dilation.hpp"
#include "FAST/Algorithms/Morphology/Erosion.hpp"
#include "FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/ImageClassificationNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/ImageClassificationNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/SegmentationNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/ImageToImageNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/BoundingBoxNetwork.hpp"
#include "FAST/Algorithms/NeuralNetwork/TensorToSegmentation.hpp"
#include "FAST/Algorithms/NonLocalMeans/NonLocalMeans.hpp"
#include "FAST/Algorithms/NonMaximumSuppression/NonMaximumSuppression.hpp"
#include "FAST/Algorithms/RegionProperties/RegionProperties.hpp"
#include "FAST/Algorithms/ScaleImage/ScaleImage.hpp"
#include "FAST/Algorithms/SeededRegionGrowing/SeededRegionGrowing.hpp"
#include "FAST/Algorithms/SegmentationVolumeReconstructor/SegmentationVolumeReconstructor.hpp"
#include "FAST/Algorithms/Skeletonization/Skeletonization.hpp"
#include "FAST/Algorithms/SurfaceExtraction/SurfaceExtraction.hpp"
#include "FAST/Algorithms/TemplateMatching/TemplateMatching.hpp"
#include "FAST/Algorithms/TemporalSmoothing/ImageWeightedMovingAverage.hpp"
#include "FAST/Algorithms/TemporalSmoothing/ImageMovingAverage.hpp"
#include "FAST/Algorithms/TissueSegmentation/TissueSegmentation.hpp"
#include "FAST/Algorithms/TubeSegmentationAndCenterlineExtraction/TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Algorithms/UltrasoundImageCropper/UltrasoundImageCropper.hpp"
#include "FAST/Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.hpp"
#include "FAST/Algorithms/VectorMedianFilter/VectorMedianFilter.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Importers/MetaImageImporter.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Importers/DICOMFileImporter.hpp"
#include "FAST/Importers/WholeSlideImageImporter.hpp"
#include "FAST/Exporters/ImageExporter.hpp"
#include "FAST/Exporters/MetaImageExporter.hpp"
#include "FAST/Exporters/StreamExporter.hpp"
#include "FAST/Exporters/ImageFileExporter.hpp"
#include "FAST/Exporters/StreamToFileExporter.hpp"
#include "FAST/Exporters/VTKMeshFileExporter.hpp"
#include "FAST/Visualization/BoundingBoxRenderer/BoundingBoxRenderer.hpp"
#include "FAST/Visualization/HeatmapRenderer/HeatmapRenderer.hpp"
#include "FAST/Visualization/ImagePyramidRenderer/ImagePyramidRenderer.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.hpp"
#include "FAST/Visualization/SegmentationPyramidRenderer/SegmentationPyramidRenderer.hpp"
#include "FAST/Visualization/SegmentationRenderer/SegmentationRenderer.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/TextRenderer/TextRenderer.hpp"
#include "FAST/Visualization/VectorFieldRenderer/VectorFieldRenderer.hpp"
#include "FAST/Visualization/VectorFieldRenderer/VectorFieldColorRenderer.hpp"
#include "FAST/Visualization/VolumeRenderer/AlphaBlendingVolumeRenderer.hpp"


namespace fast {
FAST_REGISTER_PO(ImageFileStreamer)
FAST_REGISTER_PO(OpenIGTLinkStreamer)
FAST_REGISTER_PO(MovieStreamer)
FAST_REGISTER_PO(CameraStreamer)
FAST_REGISTER_PO(RealSenseStreamer)
FAST_REGISTER_PO(ClariusStreamer)
FAST_REGISTER_PO(AddTransformation)
FAST_REGISTER_PO(SetTransformation)
FAST_REGISTER_PO(AirwaySegmentation)
FAST_REGISTER_PO(BinaryThresholding)
FAST_REGISTER_PO(BlockMatching)
FAST_REGISTER_PO(CenterlineExtraction)
FAST_REGISTER_PO(EulerGradientVectorFlow)
FAST_REGISTER_PO(MultigridGradientVectorFlow)
FAST_REGISTER_PO(HounsefieldConverter)
FAST_REGISTER_PO(ImageChannelConverter)
FAST_REGISTER_PO(ImageCropper)
FAST_REGISTER_PO(ImageGradient)
FAST_REGISTER_PO(ImageInverter)
FAST_REGISTER_PO(ImageMultiply)
FAST_REGISTER_PO(PatchGenerator)
FAST_REGISTER_PO(PatchStitcher)
FAST_REGISTER_PO(ImageToBatchGenerator)
FAST_REGISTER_PO(ImageResampler)
FAST_REGISTER_PO(ImageResizer)
FAST_REGISTER_PO(ImageSlicer)
FAST_REGISTER_PO(IterativeClosestPoint)
FAST_REGISTER_PO(LaplacianOfGaussian)
FAST_REGISTER_PO(LevelSetSegmentation)
FAST_REGISTER_PO(LungSegmentation)
FAST_REGISTER_PO(MeshToSegmentation)
FAST_REGISTER_PO(Dilation)
FAST_REGISTER_PO(Erosion)
FAST_REGISTER_PO(NeuralNetwork)
FAST_REGISTER_PO(ImageClassificationNetwork)
FAST_REGISTER_PO(ClassificationToText)
FAST_REGISTER_PO(SegmentationNetwork)
FAST_REGISTER_PO(ImageToImageNetwork)
FAST_REGISTER_PO(BoundingBoxNetwork)
FAST_REGISTER_PO(TensorToSegmentation)
FAST_REGISTER_PO(NonLocalMeans)
FAST_REGISTER_PO(NonMaximumSuppression)
FAST_REGISTER_PO(RegionProperties)
FAST_REGISTER_PO(ScaleImage)
FAST_REGISTER_PO(SeededRegionGrowing)
FAST_REGISTER_PO(SegmentationVolumeReconstructor)
FAST_REGISTER_PO(Skeletonization)
FAST_REGISTER_PO(SurfaceExtraction)
FAST_REGISTER_PO(TemplateMatching)
FAST_REGISTER_PO(ImageWeightedMovingAverage)
FAST_REGISTER_PO(ImageMovingAverage)
FAST_REGISTER_PO(TissueSegmentation)
FAST_REGISTER_PO(TubeSegmentationAndCenterlineExtraction)
FAST_REGISTER_PO(UltrasoundImageCropper)
FAST_REGISTER_PO(UltrasoundImageEnhancement)
FAST_REGISTER_PO(VectorMedianFilter)
FAST_REGISTER_PO(BoundingBoxSetAccumulator)
FAST_REGISTER_PO(ImageFileImporter)
FAST_REGISTER_PO(MetaImageImporter)
FAST_REGISTER_PO(VTKMeshFileImporter)
FAST_REGISTER_PO(DICOMFileImporter)
FAST_REGISTER_PO(WholeSlideImageImporter)
FAST_REGISTER_PO(ImageExporter)
FAST_REGISTER_PO(MetaImageExporter)
FAST_REGISTER_PO(StreamExporter)
FAST_REGISTER_PO(ImageFileExporter)
FAST_REGISTER_PO(StreamToFileExporter)
FAST_REGISTER_PO(VTKMeshFileExporter)
FAST_REGISTER_PO(BoundingBoxRenderer)
FAST_REGISTER_PO(HeatmapRenderer)
FAST_REGISTER_PO(ImagePyramidRenderer)
FAST_REGISTER_PO(ImageRenderer)
FAST_REGISTER_PO(SegmentationLabelRenderer)
FAST_REGISTER_PO(SegmentationPyramidRenderer)
FAST_REGISTER_PO(SegmentationRenderer)
FAST_REGISTER_PO(SliceRenderer)
FAST_REGISTER_PO(TextRenderer)
FAST_REGISTER_PO(VectorFieldRenderer)
FAST_REGISTER_PO(VectorFieldColorRenderer)
FAST_REGISTER_PO(AlphaBlendingVolumeRenderer)

}

#endif
