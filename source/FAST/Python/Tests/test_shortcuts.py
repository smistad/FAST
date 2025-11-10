import fast
import pytest

timeout = 1000 # Set to None for debugging

def test_display2D():
    importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_100.mhd')
    segmentation = fast.BinaryThresholding.create(100).connect(importer)

    fast.display2D(importer, segmentation, timeout=timeout, bgcolor=fast.Color.Black())

    fast.display2D(
        importer,
        segmentation,
        timeout=timeout,
        bgcolor=fast.Color.Black(),
        segmentationColors={1: fast.Color.Red()},
        width=1024,
        height=512,
        intensityLevel=10,
        intensityWindow=100
    )


def test_display2D_render_to_image():
    importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_100.mhd')
    segmentation = fast.BinaryThresholding.create(100).connect(importer)

    image = fast.display2D(
        importer,
        segmentation,
        timeout=timeout,
        bgcolor=fast.Color.Black(),
        segmentationColors={1: fast.Color.Red()},
        width=1024,
        height=512,
        intensityLevel=10,
        intensityWindow=100,
        renderToImage=True,
    )
    assert image.getWidth() == 1024
    assert image.getHeight() == 512
    assert image.getNrOfChannels() == 3


def test_display2D_return_window():
    importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_100.mhd')
    segmentation = fast.BinaryThresholding.create(100).connect(importer)

    window = fast.display2D(importer, returnWindow=True)
    renderer = fast.SegmentationRenderer.create().connect(segmentation)
    window.connect(renderer)
    if timeout:
        window.setTimeout(timeout)
    window.run()


def test_display2D_mesh():
    importer = fast.ImageFileImporter.create(fast.Config.getTestDataPath() + 'US/JugularVein/US-2D_100.mhd')
    image = importer.runAndGetOutputData()
    spacing = image.getSpacing()

    vertices = [
        fast.MeshVertex([10*spacing[0], 10*spacing[1], 0]),
        fast.MeshVertex([10*spacing[0], 50*spacing[1], 0]),
        fast.MeshVertex([200*spacing[0], 200*spacing[1], 0]),
    ]
    lines = [
        fast.MeshLine(0, 1),
        fast.MeshLine(1, 2),
    ]

    mesh = fast.Mesh.create(vertices, lines)

    fast.display2D(
        importer,
        lines=mesh,
        lineColor=fast.Color.Red(),
        lineWidth=1,
        timeout=timeout,
    )

    fast.display2D(
        importer,
        vertices=mesh,
        vertexSize=2,
        vertexSizeIsInPixels=False,
        vertexMinSize=1,
        vertexOpacity=0.5,
        timeout=timeout,
    )


def test_display2D_no_input():
    with pytest.raises(ValueError):
        fast.display2D()
