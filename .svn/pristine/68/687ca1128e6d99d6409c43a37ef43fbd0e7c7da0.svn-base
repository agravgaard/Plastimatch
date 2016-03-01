//
#include "vtkORAStructureReader.h"

#include "vtkContoursToSurfaceFilter.h"
// ORAIFTools
#include "oraStringTools.h"
#include "oraIniAccess.h"
#include "oraSimpleMacros.h"

#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkPolyDataNormals.h>
#include <vtkMath.h>


namespace ora
{

/**
 * Helper class maintaining some temporary 2D slice/contours information.
 *
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */
class ContourReconstructionInfo
{
public:
  /** 'Nominal' slice position **/
  double Position;
  /** Slice plane origin **/
  double SliceOrigin[3];
  /** Slice plane row vector **/
  double SliceHorizontal[3];
  /** Slice plane column vector **/
  double SliceVertical[3];
  /** Slice plane inverted column vector **/
  double SliceVerticalInv[3];
  /** Slice plane slicing vector **/
  double SliceNormal[3];
  /** Contour's section name in INI file **/
  std::string SectionName;
  /** Contour's number of points **/
  int NumberOfPoints;

  /** Default constructor **/
  ContourReconstructionInfo()
  {
    SectionName = "";
    Position = 0.;
    SliceOrigin[0] = SliceOrigin[1] = SliceOrigin[2] = 0.;
    SliceHorizontal[0] = 1.;
    SliceHorizontal[1] = SliceHorizontal[2] = 0.;
    SliceVertical[0] = SliceVertical[2] = 0.;
    SliceVertical[1] = 1.;
    SliceVerticalInv[0] = SliceVerticalInv[2] = 0.;
    SliceVerticalInv[1] = -1.;
    SliceNormal[0] = SliceNormal[1] = 0.;
    SliceNormal[2] = 1.;
    NumberOfPoints = 0;
  }
};

}

vtkCxxRevisionMacro(vtkORAStructureReader, "1.0")
;

vtkStandardNewMacro(vtkORAStructureReader)
;

int vtkORAStructureReader::CanReadFile(const char *fileName)
{
  Ini->SetFileName(std::string(fileName));
  Ini->LoadValues();

  // file format must have at least 2.3 version:
  bool canRead = false;
  std::vector<int> version;
  ora::ParseVersionString(Ini->ReadString("File", "FileFormat", ""), version);
  if (version.size() >= 2 && (version[0] > 2 || (version[0] == 2 && version[1]
      >= 3)))
    canRead = true;

  return canRead;
}

void vtkORAStructureReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "FileName: " << (FileName ? FileName : "")  << std::endl;
  os << indent << "IniAccess: " << Ini << std::endl;
  os << indent << "ReadColorsAsWell: " << ReadColorsAsWell << std::endl;
  os << indent << "GenerateSurfaceNormals: " << GenerateSurfaceNormals
      << std::endl;
  os << indent << "SurfaceNormalsFeatureAngle: " << SurfaceNormalsFeatureAngle
      << std::endl;
  os << indent << "Generate3DSurfaceFrom2DContoursIfApplicable: "
      << Generate3DSurfaceFrom2DContoursIfApplicable << std::endl;
  os << indent << "ContoursToSurfaceFilter: " << ContoursToSurfaceFilter
      << std::endl;
}

vtkORAStructureReader::vtkORAStructureReader()
{
  Generate3DSurfaceFrom2DContoursIfApplicable = false;
  GenerateSurfaceNormals = true;
  SurfaceNormalsFeatureAngle = 135.;
  ReadColorsAsWell = true;
  FileName = NULL;
  Ini = new ora::IniAccess();
  this->SetNumberOfInputPorts(0);
  ContoursToSurfaceFilter = vtkContoursToSurfaceFilter::New();
}

vtkORAStructureReader::~vtkORAStructureReader()
{
  if (FileName)
    delete[] FileName;
  delete Ini;
  ContoursToSurfaceFilter->Delete();
}

bool vtkORAStructureReader::Read3DStructureRawData(vtkPolyData *output)
{
  if (!output)
    return false;
  VSPNEW(pd, vtkPolyData)
  int j = 0;
  bool pieceOK = true;
  do
  {
    j++;
    // access the section maps directly (better performance):
    std::vector<ora::IniAccessSectionEntry *> *sectionMaps =
        Ini->GetSectionMaps();
    // - search for current 'vertices'- and 'tris'-sections:
    std::string srch = "Vertices" + ora::StreamConvert(j);
    ora::IniAccessSectionEntry *vmap = NULL;
    for (std::size_t i = 0; i < sectionMaps->size(); i++)
    {
      if (srch == (*sectionMaps)[i]->GetSectionName())
      {
        vmap = (*sectionMaps)[i];
        break;
      }
    }
    srch = "Tris" + ora::StreamConvert(j);
    ora::IniAccessSectionEntry *tmap = NULL;
    for (std::size_t i = 0; i < sectionMaps->size(); i++)
    {
      if (srch == (*sectionMaps)[i]->GetSectionName())
      {
        tmap = (*sectionMaps)[i];
        break;
      }
    }
    if (vmap && tmap && vmap->GetIndents()->size()
        && tmap->GetIndents()->size())
    {
      VSPNEW(points, vtkPoints)
      points->SetDataTypeToFloat();
      points->Allocate(vmap->GetIndents()->size()); // allocate for fast access
      VSPNEW(polys, vtkCellArray)
      VSPNEW(colors, vtkFloatArray)
      if (ReadColorsAsWell)
      {
        colors->SetNumberOfComponents(4); // RGBA
        colors->SetNumberOfTuples(vmap->GetIndents()->size()); // each vertex
        colors->SetName("color");
      }

      ora::IniAccessSectionEntry::IndentMapType::iterator it;
      ora::IniAccessSectionEntry::IndentMapType *vinds = vmap->GetIndents();
      for (it = vinds->begin(); it != vinds->end(); it++)
      {
        int n = 0;
        float x, y, z, r, g, b, a;
        sscanf(it->first.c_str(), "V%d", &n);
        sscanf(it->second.c_str(), "%f,%f,%f,%f,%f,%f,%f", &x, &y, &z, &r, &g,
            &b, &a);
        if (n > 0)
        {
          x *= 10.f;
          y *= 10.f;
          z *= 10.f;
          points->InsertPoint(n - 1, x, y, z);
          if (ReadColorsAsWell)
            colors->SetTuple4(n - 1, r, g, b, a);
        }
      }

      // generate triangulation (cell) list of structure
      polys->Allocate(polys->EstimateSize(tmap->GetIndents()->size(), 3));
      ora::IniAccessSectionEntry::IndentMapType *tinds = tmap->GetIndents();
      vtkIdType pts[3];
      for (it = tinds->begin(); it != tinds->end(); it++)
      {
        long v0, v1, v2;
        sscanf(it->second.c_str(), "%ld,%ld,%ld", &v0, &v1, &v2);
        pts[0] = v0 - 1;
        pts[1] = v1 - 1;
        pts[2] = v2 - 1;
        if (v0 > 0)
          polys->InsertNextCell(3, pts);
      }

      pd->SetPoints(points);
      pd->SetPolys(polys);
      if (ReadColorsAsWell)
        pd->GetPointData()->SetScalars(colors);
      if (GenerateSurfaceNormals)
      {
        VSPNEW(ngen, vtkPolyDataNormals)
        ngen->SetInput(pd);
        ngen->SetFeatureAngle(SurfaceNormalsFeatureAngle);
        ngen->SetSplitting(true);
        ngen->SetConsistency(false);
        ngen->SetAutoOrientNormals(false);
        ngen->SetComputePointNormals(true);
        ngen->SetComputeCellNormals(true);
        ngen->SetFlipNormals(false);
        ngen->SetNonManifoldTraversal(true);
        ngen->Update();
        pd = ngen->GetOutput();
      }
    }
    else
    {
      pieceOK = false;
    }
  }
  while (pieceOK);

  output->ShallowCopy(pd); // take over

  return true;
}

bool vtkORAStructureReader::Read2DContourRawData(vtkPolyData *output)
{
  if (!output)
    return false;

  output->SetPoints(NULL);
  output->SetPolys(NULL);
  output->SetLines(NULL);

  // NOTE: we need Polys if we want to produce a 3D surface, but we need
  // Lines if we do not.

  // extract contours reconstruction info:
  int nocont = -1;
  int nopts = -1;
  int i = 1, j = 0;
  std::string sect, sect2base, sect2, s;
  std::vector<ora::ContourReconstructionInfo *> cis;
  double curPos;
  double curSlcOrg[3];
  double curSlcHorz[3];
  double curSlcVert[3];
  float x, y, z;
  ora::ContourReconstructionInfo *cri;
  vtkIdType totalPoints = 0;
  do
  {
    sect = "Slice" + ora::StreamConvert(i);
    sect2base = "Contour" + ora::StreamConvert(i);
    nocont = Ini->ReadValue<int> (sect, "NoOfContours", -1);
    if (nocont > -1)
    {
      curPos = Ini->ReadValue<double> (sect, "Position", 0.) * 10.;
      s = Ini->ReadString(sect, "SlcOrg", "");
      sscanf(s.c_str(), "%f,%f,%f", &x, &y, &z);
      curSlcOrg[0] = x * 10.;
      curSlcOrg[1] = y * 10.;
      curSlcOrg[2] = z * 10.;
      s = Ini->ReadString(sect, "SlcHorz", "");
      sscanf(s.c_str(), "%f,%f,%f", &x, &y, &z);
      curSlcHorz[0] = x;
      curSlcHorz[1] = y;
      curSlcHorz[2] = z;
      s = Ini->ReadString(sect, "SlcVert", "");
      sscanf(s.c_str(), "%f,%f,%f", &x, &y, &z);
      curSlcVert[0] = x;
      curSlcVert[1] = y;
      curSlcVert[2] = z;
      j = 1;
      while (j <= nocont)
      {
        sect2 = sect2base + "." + ora::StreamConvert(j);
        nopts = Ini->ReadValue<int> (sect2, "NoOfPoints", -1);
        if (nopts > 0)
        {
          cri = new ora::ContourReconstructionInfo();
          cri->NumberOfPoints = nopts;
          cri->Position = curPos;
          cri->SectionName = sect2;
          cri->SliceOrigin[0] = curSlcOrg[0];
          cri->SliceOrigin[1] = curSlcOrg[1];
          cri->SliceOrigin[2] = curSlcOrg[2];
          cri->SliceHorizontal[0] = curSlcHorz[0];
          cri->SliceHorizontal[1] = curSlcHorz[1];
          cri->SliceHorizontal[2] = curSlcHorz[2];
          cri->SliceVertical[0] = curSlcVert[0];
          cri->SliceVertical[1] = curSlcVert[1];
          cri->SliceVertical[2] = curSlcVert[2];
          cri->SliceVerticalInv[0] = -curSlcVert[0];
          cri->SliceVerticalInv[1] = -curSlcVert[1];
          cri->SliceVerticalInv[2] = -curSlcVert[2];
          vtkMath::Cross(cri->SliceHorizontal, cri->SliceVertical,
              cri->SliceNormal);
          cis.push_back(cri);
          totalPoints += nopts;
        }
        j++;
      }
    }
    i++;
  }
  while (nocont > -1);
  if (totalPoints <= 0)
    return false;

  // prepare contours:
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(totalPoints);
  cells->Allocate(1, totalPoints);
  float xa[4];
  float ya[4];
  std::string key, ln;
  vtkIdType pt = 0;
  double orig[3];
  double cs[9];
  for (std::size_t n = 0; n < cis.size(); n++)
  {
    // store some meta-info for faster transformation afterwards:
    orig[0] = cis[n]->SliceOrigin[0];
    orig[1] = cis[n]->SliceOrigin[1];
    orig[2] = cis[n]->SliceOrigin[2];
    cs[0] = cis[n]->SliceHorizontal[0]; // dot(slcHorz,(1,0,0))
    cs[1] = cis[n]->SliceNormal[0]; // dot(slcNormal,(1,0,0))
    cs[2] = cis[n]->SliceVerticalInv[0]; // dot(-slcVert,(1,0,0))
    cs[3] = cis[n]->SliceHorizontal[1]; // dot(slcHorz,(0,1,0))
    cs[4] = cis[n]->SliceNormal[1]; // dot(slcNormal,(0,1,0))
    cs[5] = cis[n]->SliceVerticalInv[1]; // dot(-slcVert,(0,1,0))
    cs[6] = cis[n]->SliceHorizontal[2]; // dot(slcHorz,(0,0,1))
    cs[7] = cis[n]->SliceNormal[2]; // dot(slcNormal,(0,0,1))
    cs[8] = cis[n]->SliceVerticalInv[2]; // dot(-slcVert,(0,0,1))

    // process cell:
    cells->InsertNextCell(cis[n]->NumberOfPoints);
    int maxID = cis[n]->NumberOfPoints / 4;
    if (cis[n]->NumberOfPoints % 4 != 0)
      maxID++;
    for (int k = 1; k <= maxID; k++)
    {
      key = "Pts" + ora::StreamConvert(k);
      ln = Ini->ReadString(cis[n]->SectionName, key, "");
      int npts = sscanf(ln.c_str(), "%f,%f,%f,%f,%f,%f,%f,%f", &xa[0], &ya[0],
          &xa[1], &ya[1], &xa[2], &ya[2], &xa[3], &ya[3]) / 2;
      for (int xx = 0; xx < npts; xx++)
      {
        x = (double) xa[xx] * 10.;
        z = (double) ya[xx] * 10.;
        y = 0.; // cis[n]->Position ... see ORA
        // transform point into PCS according to ORA implementation:
        x = orig[0] + x * cs[0] + y * cs[1] + z * cs[2];
        y = orig[1] + x * cs[3] + y * cs[4] + z * cs[5];
        z = orig[2] + x * cs[6] + y * cs[7] + z * cs[8];

        points->SetPoint(pt, x, y, z);
        cells->InsertCellPoint(pt);
        pt++;
      }
    }
  }

  // clean
  for (std::size_t n = 0; n < cis.size(); n++)
    delete cis[n];
  cis.clear();

  if (!Generate3DSurfaceFrom2DContoursIfApplicable)
  {
    // we're ready if we do not want to derive a surface
    output->SetLines(cells);
    output->SetPoints(points);
    return true;
  }

  // prepare a new poly data object for 3D reconstruction input
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  pd->SetPolys(cells);
  pd->SetPoints(points);

  // this filter is presumed to be configured:
  ContoursToSurfaceFilter->SetInput(pd);
  ContoursToSurfaceFilter->Update();

  if (GenerateSurfaceNormals)
  {
    VSPNEW(ngen, vtkPolyDataNormals)
    ngen->SetInput(ContoursToSurfaceFilter->GetOutput());
    ngen->SetFeatureAngle(SurfaceNormalsFeatureAngle);
    ngen->SetSplitting(true);
    ngen->SetConsistency(false);
    ngen->SetAutoOrientNormals(false);
    ngen->SetComputePointNormals(true);
    ngen->SetComputeCellNormals(true);
    ngen->SetFlipNormals(false);
    ngen->SetNonManifoldTraversal(true);
    ngen->Update();
    output->ShallowCopy(ngen->GetOutput()); // take over
  }
  else
  {
    output->ShallowCopy(ContoursToSurfaceFilter->GetOutput()); // take over
  }

  return true;
}

int vtkORAStructureReader::RequestData(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(
      vtkDataObject::DATA_OBJECT()));

  if (!CanReadFile(FileName))
  {
    vtkErrorMacro(<< "The file '" << FileName << "' cannot be read by this reader!")
    return 0;
  }

  int complexity = Ini->ReadValue<int> ("Structure", "StComplexity", -1);
  if (complexity == 1) // 3D structure
  {
    if (!Read3DStructureRawData(output))
    {
      vtkErrorMacro(<< "Could not read 3D raw data.")
      return 0;
    }
  }
  else if (complexity == 0) // contours
  {
    if (!Read2DContourRawData(output))
    {
      vtkErrorMacro(<< "Could not read 2D contour raw data.")
      return 0;
    }
  }
  else
  {
    vtkErrorMacro(<< "Do not support the complexity defined in the file.")
    return 0;
  }

  return 1;
}

int vtkORAStructureReader::RequestInformation(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **vtkNotUsed(inputVector), vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(), -1);

  return 1;
}
