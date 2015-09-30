#include "VTKLineSetExporter.hpp"
#include "FAST/Data/LineSet.hpp"

#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkDataObject.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

using namespace fast;

vtkStandardNewMacro(VTKLineSetExporter);

VTKLineSetExporter::VTKLineSetExporter() {
    // VTK stuff
    this->SetNumberOfOutputPorts(1);
    this->SetNumberOfInputPorts(0);
    createInputPort<LineSet>(0);
}

int VTKLineSetExporter::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info) {
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

vtkPolyData* VTKLineSetExporter::GetOutput() {
    return GetOutput(0);
}

vtkPolyData* VTKLineSetExporter::GetOutput(int port) {
    return vtkPolyData::SafeDownCast(this->GetOutputDataObject(port));
}


int VTKLineSetExporter::RequestData(
        vtkInformation* vtkNotUsed(request),
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {

    update(); // Run FAST pipeline

    LineSet::pointer input = getStaticInputData<LineSet>();
    LineSetAccess::pointer access = input->getAccess(ACCESS_READ);

    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkSmartPointer<vtkPoints> points = vtkPoints::New();
    points->SetNumberOfPoints(access->getNrOfPoints());

    for(uint i = 0; i < access->getNrOfPoints(); ++i) {
        Vector3f point = access->getPoint(i);
        points->SetPoint(i, point.x(), point.y(), point.z());
    }

    vtkSmartPointer<vtkCellArray> lines = vtkCellArray::New();
    for(uint i = 0; i < access->getNrOfLines(); ++i) {
        Vector2ui line = access->getLine(i);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(line.x());
        lines->InsertCellPoint(line.y());
    }

    output->SetPoints(points);
    output->SetLines(lines);


    return 1;
}

//----------------------------------------------------------------------------
int VTKLineSetExporter::ProcessRequest(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector)
{
  // Create an output object of the correct type.
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
  {
    //return this->RequestDataObject(request, inputVector, outputVector);
  }
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    return this->RequestData(request, inputVector, outputVector);
  }

  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
  {
    //return this->RequestUpdateExtent(request, inputVector, outputVector);
  }

  // execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    //return this->RequestInformation(request, inputVector, outputVector);
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}
