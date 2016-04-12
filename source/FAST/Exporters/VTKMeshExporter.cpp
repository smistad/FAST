#include "VTKMeshExporter.hpp"
#include "FAST/Data/Mesh.hpp"

#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

using namespace fast;

vtkStandardNewMacro(VTKMeshExporter);

VTKMeshExporter::VTKMeshExporter() {
    // VTK stuff
    this->SetNumberOfOutputPorts(1);
    this->SetNumberOfInputPorts(0);
    createInputPort<Mesh>(0);
}

int VTKMeshExporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {

    update(); // Run FAST pipeline

    Mesh::pointer input = getStaticInputData<Mesh>();
    MeshAccess::pointer access = input->getMeshAccess(ACCESS_READ);

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkPolyData *output = this->GetOutput();

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(input->getNrOfVertices());

    for(int i = 0; i < input->getNrOfVertices(); i++) {
    	MeshVertex v = access->getVertex(i);
    	VectorXf position = v.getPosition();
		if(input->getDimensions() == 2) {
			points->SetPoint(i, position.x(), position.y(), 0);
		} else {
			points->SetPoint(i, position.x(), position.y(), position.z());
		}
	}
	output->SetPoints(points);

    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    if(input->getDimensions() == 2) {
		for(int i = 0; i < input->getNrOfLines(); i++) {
			VectorXui line = access->getLine(i);
			polys->InsertNextCell(2);
			polys->InsertCellPoint(line.x());
			polys->InsertCellPoint(line.y());
		}
		output->SetLines(polys);
    } else {
    	for(int i = 0; i < input->getNrOfTriangles(); i++) {
			VectorXui triangle = access->getTriangle(i);
			polys->InsertNextCell(3);
			polys->InsertCellPoint(triangle.x());
			polys->InsertCellPoint(triangle.y());
			polys->InsertCellPoint(triangle.z());
		}
		output->SetPolys(polys);
    }
    // TODO if 3D, also export normals

    return 1;
}
