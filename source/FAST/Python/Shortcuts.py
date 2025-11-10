"""
A few shortcut functions
"""
from typing import Dict, Union

def display2D(
            image:Union[Image, ProcessObject]=None,
            segmentation:Union[Image, ProcessObject]=None,
            lines:Union[Mesh, ProcessObject]=None,
            vertices: Union[Mesh, ProcessObject]=None,
            intensityLevel:float=None,
            intensityWindow:float=None,
            segmentationColors:Dict[int, Color]=None,
            segmentationOpacity:float=0.5,
            segmentationBorderOpacity:float=None,
            segmentationBorderRadius:int=1,
            lineWidth:float=1,
            lineColor:Color=Color.Green(),
            vertexSize:float=10.0,
            vertexSizeIsInPixels:bool=True,
            vertexMinSize:float=1.0,
            vertexColor:Color=Color.Null(),
            vertexOpacity:float=1.0,
            bgcolor:Color=Color.White(),
            width:int=None,
            height:int=None,
            timeout:int=None,
            renderToImage:bool=False,
            returnWindow:bool=False,
        ):
    """
    @brief Shortcut for displaying image, segmentation and meshes using SimpleWindow2D

    :param image: Image to display
    :param segmentation: Segmentation to display
    :param lines: Lines to display
    :param vertices: Vertices to display
    :param intensityLevel: Intensity level for image rendering
    :param intensityWindow: Intensity window for image rendering
    :param segmentationColors: Colors to use for segmentation
    :param segmentationOpacity: Opacity of segmentation
    :param segmentationBorderOpacity: Border opacity of segmentation
    :param segmentationBorderRadius: Size of segmentation border
    :param lineWidth: Width of line
    :param lineColor: Color of line
    :param vertexSize Vertex point size (can be in pixels or millimeters, see sizeIsInPixels parameter)
    :param vertexSizeIsInPixels Whether size is given in pixels or millimeters
    :param vertexMinSize Minimum size in pixels, used when sizeInPixels = false
    :param vertexColor Override color stored for each vertex
    :param vertexOpacity Opacity of vertices: 1 = no transparency, 0 = fully transparent
    :param bgcolor: Background color
    :param width: Width of window
    :param height: Height of window
    :param timeout: If set to a number, the window will auto close after this many milliseconds
    :param renderToImage: Use RenderToImage instead of SimpleWindow and return the resulting image
    :param returnWindow: Whether to return the window object, or to run it
    :return:
    """

    if image is None and segmentation is None and lines is None and vertices is None:
        raise ValueError('No data was given to display2D')

    def set_default_value(x, default):
        return default if x is None else x

    width = set_default_value(width, 0)
    height = set_default_value(height, 0)
    intensityLevel = set_default_value(intensityLevel, -1)
    intensityWindow = set_default_value(intensityWindow, -1)
    segmentationColors = set_default_value(segmentationColors, LabelColors())
    segmentationBorderOpacity = set_default_value(segmentationBorderOpacity, -1)

    renderers = []

    renderer = ImageRenderer.create(
        level=intensityLevel,
        window=intensityWindow
    ).connect(image)
    renderers.append(renderer)

    if segmentation is not None:
        renderer = SegmentationRenderer.create(
            colors=segmentationColors,
            opacity=segmentationOpacity,
            borderOpacity=segmentationBorderOpacity,
            borderRadius=segmentationBorderRadius
        ).connect(segmentation)
        renderers.append(renderer)

    if lines is not None:
        renderer = LineRenderer.create(
            lineWidth=lineWidth,
            color=lineColor
        ).connect(lines)
        renderers.append(renderer)

    if vertices is not None:
        renderer = VertexRenderer.create(
            size=vertexSize,
            sizeIsInPixels=vertexSizeIsInPixels,
            minSize=vertexMinSize,
            color=vertexColor,
            opacity=vertexOpacity,
        ).connect(vertices)
        renderers.append(renderer)

    if renderToImage:
        render = RenderToImage.create(
            bgcolor=bgcolor,
            width=width,
            height=height
        ).connect(renderers)
        return render.runAndGetOutputData()
    else:
        window = SimpleWindow2D.create(
            bgcolor=bgcolor,
            width=width,
            height=height
        ).connect(renderers)
        if timeout:
            window.setTimeout(timeout)
        if returnWindow:
            return window
        else:
            window.run()
