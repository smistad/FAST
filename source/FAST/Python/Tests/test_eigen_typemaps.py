import fast
import numpy as np
import pytest


def test_vector3f():
    image = fast.Image.create(512, 512, fast.TYPE_UINT8, 3)
    s = image.getSpacing()
    assert s == pytest.approx((1,1,1))
    # Test tuple
    image.setSpacing((1.2, 3, 2))
    s = image.getSpacing()
    assert s == pytest.approx((1.2, 3, 2))
    # Test list
    image.setSpacing([1.1, 4, 1])
    s = image.getSpacing()
    assert s == pytest.approx((1.1, 4, 1))
    # Numpy array
    image.setSpacing(np.array([1., 2.2, 10]))
    s = image.getSpacing()
    assert s == pytest.approx((1, 2.2, 10))

    with pytest.raises(TypeError):
        image.setSpacing((2, '3', 1))

    with pytest.raises(ValueError):
        image.setSpacing((2, 1))

    with pytest.raises(TypeError):
        image.setSpacing(1)


def test_vector3i():
    pass


def test_vector3ui():
    image = fast.Image.create(512, 512, 32, fast.TYPE_UINT8, 1)
    size = image.getSize()
    assert size == (512, 512, 32)


def test_vector2f():
    region = fast.Region()

    region.centroid = (2.2, 1.1)
    c = region.centroid
    assert c == pytest.approx((2.2, 1.1))

    with pytest.raises(ValueError):
        region.centroid = (2,)


def test_vector2i():
    region = fast.Region()

    region.minPixelPosition = (128, 20)
    p = region.minPixelPosition
    assert p == (128, 20)


def test_vectorXi():
    image = fast.Image.create(512, 512, fast.TYPE_UINT8, 3)
    image.fill(0)

    cropped_image = image.crop((0, 0), (128, 128))

    access = image.getImageAccess(fast.ACCESS_READ_WRITE)
    access.setVector((32, 20), (4, 5, 3, 1))
    v0 = access.getScalar((32, 20), 0)
    v1 = access.getScalar((32, 20), 1)
    v2 = access.getScalar((32, 20), 2)
    assert v0 == pytest.approx(4)
    assert v1 == pytest.approx(5)
    assert v2 == pytest.approx(3)


def test_vector_of_eigen_vectors():
    rg = fast.SeededRegionGrowing.create(0, 1, [(23, 2, 1), (23, 43, 1)])
    points = rg.getSeedPoints()
    assert points[0] == (23, 2, 1)
    assert points[1] == (23, 43, 1)

    region = fast.Region()
    region.pixels = [(20, 1), (32, 32), (128, 128)]
    p = region.pixels
    assert len(p) == 3
    assert p[0] == (20, 1)
    assert p[1] == (32, 32)
    assert p[2] == (128, 128)


def test_vector_of_vector_of_eigen_vectors():
    bbn = fast.TensorToBoundingBoxSet.create(anchors=[
        [(1.1, 2.2), (1.1, 2.2)],
        [(3.1, 3.2)],
        [(3.1, 3.2), (23, 23)],
    ])

    anchors = bbn.getAnchors()
    assert anchors[0][1] == pytest.approx((1.1, 2.2))
    assert anchors[1][0] == pytest.approx((3.1, 3.2))
    assert anchors[2][1] == pytest.approx((23, 23))


def test_eigen_matrix_typemaps():
    transform = fast.Transform.create()

    # Matrix4f:
    m = [
        [1.1, 0, 0, 10],
        [0, 2.2, 0, 20],
        [0, 0, 3.3, 30],
        [0, 0, 0, 1]
    ]
    transform.setMatrix(m)
    m2 = transform.getMatrix()
    for i in range(4):
        assert m2[i] == pytest.approx(m[i])

    with pytest.raises(ValueError):
        m3 = [
            [1.1, 0, 0, 10],
            [0, 2.2, 0, 20],
            [0, 0, 3.3, 30],
        ]
        transform.setMatrix(m3)

    with pytest.raises(ValueError):
        m3 = [
            [1.1, 0, 0, 10],
            [0, 2.2, 0],
            [0, 0, 3.3, 30],
            [0, 0, 0, 1],
        ]
        transform.setMatrix(m3)

    with pytest.raises(TypeError):
        m3 = [
            [1.1, 0, 0, 10],
            [0, 2.2, 0, 20],
            [0, 0, 3.3, 30],
            [0, 'err', 0, 1],
        ]
        transform.setMatrix(m3)