## @example extract_surface_mesh.py
# This examples extract a surface mesh with over 3 million triangles from a CT volume
# using the Marching Cubes algorithm.
# It also shows how you can access the vertex and triangle data directly.
import fast

importer = fast.ImageFileImporter.New()
importer.setFilename(fast.Config.getTestDataPath() + "/CT/CT-Abdomen.mhd")

extraction = fast.SurfaceExtraction.New()
extraction.setInputConnection(importer.getOutputPort())
extraction.setThreshold(300)

mesh = extraction.updateAndGetOutputMesh()
access = mesh.getMeshAccess(fast.ACCESS_READ)

# Get size of mesh:
print('Mesh size (vertices, triangles)', mesh.getNrOfVertices(), mesh.getNrOfTriangles())
# Get the position of first vertex
print(access.getVertex(0).getPosition())
# Get endpoints (vertex indices) of triangle 0
print(access.getTriangle(0).getEndpoint1(), access.getTriangle(0).getEndpoint2(), access.getTriangle(0).getEndpoint3())

# Get all vertices and triangles as lists, this is slow if it is a big surface:
#vertices = access.getVertices()
#triangles = access.getTriangles()

renderer = fast.TriangleRenderer.New()
renderer.setInputData(mesh)

window = fast.SimpleWindow.New()
window.addRenderer(renderer)
window.start()
