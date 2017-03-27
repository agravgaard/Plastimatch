

#include "vtkORAMemoryStructureSource.h"

// ORAIFTools
#include "oraSimpleMacros.h"
#include "SimpleDebugger.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkObjectFactory.h>


vtkCxxRevisionMacro(vtkORAMemoryStructureSource, "1.0");
vtkStandardNewMacro(vtkORAMemoryStructureSource);

void
vtkORAMemoryStructureSource
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "--- ORA shared memory ---" << std::endl;
  os << indent << "Structure UID: " << StructureUID << std::endl;
  os << indent << "Structure Name: " << StructureName << std::endl;
  os << indent << "Structure Set UID: " << StructureSetUID << std::endl;
  os << indent << "Number of Polygons: " << NumberOfPolys << std::endl;
  os << indent << "Vertex Coordinates Pointer: " << VertexCoords << std::endl;
  os << indent << "Textures Coordinates Pointer: " << TextureCoords << std::endl;
  os << indent << "Vertex Color Pointer: " << VertexColor << std::endl;
  os << indent << "Vertex Normal Pointer: " << VertexNormal << std::endl;
  os << indent << "ORA Modification (ms): " << ORAModifiedTimeStamp << std::endl;
}

vtkORAMemoryStructureSource
::vtkORAMemoryStructureSource()
{
  NumberOfPolys = 0;
  VertexCoords = NULL;
  TextureCoords = NULL;
  VertexColor = NULL;
  VertexNormal = NULL;
  StructureUID = "";
  StructureName = "";
  StructureSetUID = "";
  StructureType = ST_UNDEFINED;
  DFAlpha = 0;
  for (int i = 0; i < 3; i++)
  {
    DFColor[i] = 0;
    AFColor[i] = 0;
    SFColor[i] = 0;
    ContourColor[i] = 0;
  }
  ContourWidth = 1;
  ORAModifiedTimeStamp = 86400 * 1000 + 5; // impossible
  this->SetNumberOfInputPorts(0);
}

vtkORAMemoryStructureSource
::~vtkORAMemoryStructureSource()
{

}

bool
vtkORAMemoryStructureSource
::CanCreateStructure()
{
  return (NumberOfPolys > 0 && // at least 1 triangle
      VertexCoords && // vertex coordinates are mandatory
      StructureUID.length() > 0); // need UID
}

int
vtkORAMemoryStructureSource
::RequestData(vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!CanCreateStructure())
    return 0;

  // get the info objects
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  VSPNEW(points, vtkPoints)
  VSPNEW(polys, vtkCellArray)
  vtkIdType i, pi, numvert;
  vtkIdType pts[3];
  float p[3];

  // generate vertex (point) list of structure
  numvert = 3 * NumberOfPolys; // number of total vertices
  points->SetDataTypeToFloat();
  points->Allocate(numvert); // allocate for fast access
  for(i = 0, pi = 0; i < numvert; i++)
  {
    p[0] = VertexCoords[pi++];
    p[1] = VertexCoords[pi++];
    p[2] = VertexCoords[pi++];
    points->InsertPoint(i, p);
  }
  // generate triangulation (cell) list of structure
  polys->Allocate(polys->EstimateSize(NumberOfPolys, 3)); // exact estimation
  for(i = 0, pi = 0; i < NumberOfPolys; i++)
  {
    pts[0] = pi++;
    pts[1] = pi++;
    pts[2] = pi++;
    polys->InsertNextCell(3, pts);
  }
  // setup primary polydata
  output->SetPoints(points);
  output->SetPolys(polys);
  // set the normals for all vertices if applicable (contained in memory)
  if (VertexNormal)
  {
    VSPNEW(normals, vtkDoubleArray)
    double n[3];
    normals->SetNumberOfComponents(3); // 3D-vectors
    normals->SetNumberOfTuples(numvert); // for each vertex

    // set the normals
    for (i = 0, pi = 0; i < numvert; i++)
    {
      n[0] = VertexNormal[pi++];
      n[1] = VertexNormal[pi++];
      n[2] = VertexNormal[pi++];
      normals->SetTuple(i, n);
    }

    // pack into poly data
    output->GetPointData()->SetNormals(normals);
  }
   // set the colors for all vertices if applicable (contained in memory)
  if (VertexColor)
  {
    VSPNEW(colors, vtkUnsignedCharArray)
    unsigned char rgba[4];
    colors->SetNumberOfComponents(4); // RGBA
    colors->SetNumberOfTuples(numvert); // for each vertex

    // set the normals
    for (i = 0, pi = 0; i < numvert; i++)
    {
      rgba[0] = VertexColor[pi++];
      rgba[1] = VertexColor[pi++];
      rgba[2] = VertexColor[pi++];
      rgba[3] = VertexColor[pi++];
      colors->SetTupleValue(i, rgba);
    }

    // pack into poly data
    output->GetPointData()->SetScalars(colors);
  }

  return 1;
}

int
vtkORAMemoryStructureSource
::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
               -1);

  return 1;
}

