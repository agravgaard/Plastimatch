

#include "oraVTKStructure.h"

// ORAIFTools
#include "SimpleDebugger.h"

#include <vtkTransform.h>
#include <vtkCell.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>

// Forward declarations
#include <vtkImplicitFunction.h>
#include <vtkStripper.h>

namespace ora
{

// static member initialization
VTKStructureSet *VTKStructureSet::DefaultStructureSet = NULL;


VTKStructureSet
::VTKStructureSet()
{
  STSMap.clear();
  StructureSetUID = "";
}

VTKStructureSet
::~VTKStructureSet()
{
  STSMap.clear();
}

VTKStructureSet *
VTKStructureSet
::GetDefaultStructureSet()
{
  if (!VTKStructureSet::DefaultStructureSet)
  {
    DefaultStructureSet = new VTKStructureSet();
    DefaultStructureSet->SetStructureSetUID(""); // has no UID
  }

  return VTKStructureSet::DefaultStructureSet;
}

VTKStructure *
VTKStructureSet
::FindStructure(const std::string UID)
{
  VTKStructure *ret = NULL;
  STSMapType::iterator it;
  for (it = STSMap.begin(); it != STSMap.end(); ++it)
  {
    if (it->first == UID)
    {
      ret = it->second;
      if (ret) // only exit if the entry is NOT NULL!
        break;
    }
  }
  return ret;
}

bool
VTKStructureSet
::AddStructure(VTKStructure *structure)
{
  if (structure)
  {
    std::pair<STSMapType::iterator, bool> ret =
      STSMap.insert(std::make_pair(structure->GetStructureUID(), structure));

    if (ret.second) // auto-referencing
    {
      structure->SetStructureSet(this);
      structure->SetStructureSetUID(this->StructureSetUID);
    }

    return ret.second;
  }

  return false;
}

bool
VTKStructureSet
::RemoveStructure(VTKStructure *structure)
{
  if (structure)
    return this->RemoveStructure(structure->GetStructureUID());

  return false;
}

bool
VTKStructureSet
::RemoveStructure(std::string UID)
{
  if (UID.length() > 0)
  {
    STSMapType::iterator it;

    for (it = STSMap.begin(); it != STSMap.end(); ++it)
    {
      if (it->first == UID)
      {
        if (it->second) // auto-dereferencing
        {
          it->second->SetStructureSet(NULL);
          it->second->SetStructureSetUID("");
        }
        STSMap.erase(it); //, it);
        return true;
      }
    }
  }

  return false;
}

void
VTKStructureSet
::Clear()
{
  STSMap.clear();
}

std::vector<std::string>
VTKStructureSet
::GetStructureUIDs()
{
  std::vector<std::string> ret;
  STSMapType::iterator it;

  for (it = STSMap.begin(); it != STSMap.end(); ++it)
  {
    ret.push_back(it->first);
  }

  return ret;
}

std::vector<VTKStructure *>
VTKStructureSet
::GetStructures()
{
  std::vector<VTKStructure *> ret;
  STSMapType::iterator it;

  for (it = STSMap.begin(); it != STSMap.end(); ++it)
  {
    ret.push_back(it->second);
  }

  return ret;
}

unsigned int
VTKStructureSet
::GetNumberOfStructures()
{
  return STSMap.size();
}


VTKStructure *
VTKStructure
::LoadFromORAMemory(VSP(structSource, vtkORAMemoryStructureSource))
{
  // structure-source must adequately be pre-configured
  if (!structSource || !structSource->CanCreateStructure())
    return NULL;

  VTKStructure *structure = new VTKStructure();
  structSource->Update(); // update here to avoid delays later
  structure->SetStructurePolyData(structSource->GetOutput());
  structure->SetStructureUID(structSource->GetStructureUID());
  structure->SetStructureName(structSource->GetStructureName());
  structure->SetStructureSetUID(structSource->GetStructureSetUID());
  structure->SetORAModificationTimeStamp(
      structSource->GetORAModifiedTimeStamp());
  // create actor representation
  structure->CreateNewStructureActor(structSource);
  // create contour property
  structure->CreateNewContourProperty(structSource);

  return structure;
}

VTKStructure
::VTKStructure()
{
  StructurePolyData = NULL;
  StructureActor = NULL;
  StructureMapper = NULL;
  ContourProperty = NULL;
  StructureLODActor = NULL;
  Cutter = NULL; // created on demand
  Stripper = NULL; // created on demand

  // FIXME: meta-data later!
  StructureSet = NULL;
  StructureSetUID = "";
  StructureUID = "";
  StructureName = "";
  ORAModificationTimeStamp = 86400 * 1000 + 5; // impossible!
  FileName = "";
}

VTKStructure
::~VTKStructure()
{
  StructurePolyData = NULL;
  StructureActor = NULL;
  StructureLODActor = NULL;
  Cutter = NULL;
  Stripper = NULL;

  // FIXME: meta-data later!
  StructureSet = NULL;
}

bool
VTKStructure
::CreateNewStructureActor(VSP(structSource, vtkORAMemoryStructureSource))
{
  if (!structSource || !structSource->GetOutput())
    return false;

  StructureActor = NULL;
  StructureMapper = NULL;

  // generate mapper / actor pipeline
  StructureMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  StructureActor = vtkSmartPointer<vtkActor>::New();
  StructureMapper->SetInput(structSource->GetOutput());
  StructureActor->SetMapper(StructureMapper);

  vtkORAMemoryStructureSource::StructureTypeType stType =
    structSource->GetStructureType();
  if (stType != vtkORAMemoryStructureSource::ST_3D_SCAN &&
      stType != vtkORAMemoryStructureSource::ST_BEAM_VISUALIZATION)
    StructureMapper->ScalarVisibilityOff(); // global color (actor-wide)
  else
    StructureMapper->ScalarVisibilityOn(); // vertex colors!

  vtkProperty *prop = StructureActor->GetProperty();
  float c[3]; // material props
  structSource->GetDFColor(c);
  prop->SetDiffuseColor(c[0], c[1], c[2]);
  structSource->GetAFColor(c);
  prop->SetAmbientColor(c[0], c[1], c[2]);
  structSource->GetSFColor(c);
  prop->SetSpecularColor(c[0], c[1], c[2]);
  prop->SetDiffuse(1);
  prop->SetAmbient(0);//0.2);
  prop->SetSpecular(0);//0.2);
  double alpha = structSource->GetDFAlpha() + 0.1 /** offset **/;
  alpha = (alpha > 1.) ? 1. : alpha;
  prop->SetOpacity(alpha);

  prop->SetRepresentationToSurface();

//  VSPNEW(tt, vtkTransform)
//  tt->Translate(-0.5, 0, -1.5);
//  StructureActor->SetUserTransform(tt);

  return true;
}

bool
VTKStructure
::CreateNewContourProperty(VSP(structSource, vtkORAMemoryStructureSource))
{
  if (!structSource)
    return false;

  ContourProperty = NULL;

  ContourProperty = vtkSmartPointer<vtkProperty>::New();

  float c[3]; // material props
  structSource->GetContourColor(c);
  ContourProperty->SetColor(c[0], c[1], c[2]);
  ContourProperty->SetOpacity(1);
  ContourProperty->SetLineWidth(structSource->GetContourWidth());
  ContourProperty->SetInterpolationToFlat();
  ContourProperty->SetRepresentationToWireframe();

  return true;
}

vtkPolyData *
VTKStructure
::CutByImplicitFunction(vtkImplicitFunction *ifunc, bool copy, bool simplify,
    bool &didSimplification)
{
  if (!ifunc || !StructurePolyData)
    return NULL;

  didSimplification = false;
  if (!Cutter)
    Cutter = vtkSmartPointer<vtkCutter>::New();

  // pure cutting
  Cutter->SetInput(StructurePolyData);
  Cutter->SetCutFunction(ifunc);
  Cutter->Update();

  // memory management
  vtkPolyData *contour = NULL;
  if (copy)
  {
    // create an effective copy in order to be independent of this structure
    // object and its internal cutter:
    // (NOTE: this poly data object must be freed externally!)
    vtkPolyData *cut = vtkPolyData::New();
    cut->DeepCopy(Cutter->GetOutput());
    contour = cut;
  }
  else
  {
    contour = Cutter->GetOutput();
  }

  // optional contours simplification
  if (simplify && contour)
  {
    // clean the generated contours:
    if (!Stripper)
      Stripper = vtkSmartPointer<vtkStripper>::New();
    Stripper->SetInput(contour);
    Stripper->SetMaximumLength(100000); // more or less infinite max. length
    Stripper->Update();
    contour = Stripper->GetOutput();

    // post-process open contour fragments: sometimes the stripper isn't able to
    // strip the whole contour; try to connect the fragments manually:
    vtkIdType pi, li, li2, nids, nids2;
    vtkCell *cell;
    vtkIdList *ptlist;
    vtkIdType *pptlist;
    int pc;
    if (contour->GetNumberOfCells() > 1)
    {
      std::vector<vtkIdType> *lines = new std::vector<vtkIdType>[contour->GetNumberOfCells()];
      for (li = 0; li < contour->GetNumberOfCells(); li++)
      {
        cell = contour->GetCell(li);
        ptlist = cell->GetPointIds();
        pptlist = ptlist->GetPointer(0);
        nids = ptlist->GetNumberOfIds();
        for (pc = 0; pc < nids; pc++)
          lines[li].push_back(pptlist[pc]);
      }

      bool connectedSomething;
      vtkIdType first, last, first2, last2;
      do // have to do checks iteratively
      {
        li = 0;
        connectedSomething = false;
        while (li < contour->GetNumberOfCells())
        {
          // first and last point ID of first and last point of curr. cont.
          if (lines[li].size() > 1)
          {
            first = lines[li][0];
            nids = lines[li].size();
            last = lines[li][nids - 1];
            // now search for connected other contours that share a point ID
            li2 = 0;
            while (li2 < contour->GetNumberOfCells())
            {
              if (li != li2)
              {
                // first and last point ID of first and last point of inv. cont.
                if (lines[li2].size() > 1)
                {
                  first2 = lines[li2][0];
                  nids2 = lines[li2].size();
                  last2 = lines[li2][nids2 - 1];
                  // check shared points at line borders
                  if (first == last2)
                  {
                    // copy first cell points list to the end of second cell
                    for (pi = 1; pi < nids; pi++)
                      lines[li2].push_back(lines[li][pi]);
                    // delete first cell
                    lines[li].clear();
                    connectedSomething = true;
                  }
                  else if (last == last2) // clock-change (more or less theory)
                  {
                    // -> reverse order: copy first cell points to end of second
                    for (pi = (nids - 2); pi >= 0; pi--)
                      lines[li2].push_back(lines[li][pi]);
                    // delete first cell
                    lines[li].clear();
                    connectedSomething = true;
                  }
                  else if (first == first2) // clock-change (more or less theory)
                  {
                    // -> reverse elements of second cell points
                    std::vector<vtkIdType> tmp;
                    for (pi = 0; pi < nids2; pi++)
                      tmp.push_back(lines[li2][pi]);
                    for (pi = 0; pi < nids2; pi++)
                      lines[li2][pi] = tmp[nids2 - 1 - pi];
                    tmp.clear();
                    // copy first cell points list to the end of new second cell
                    for (pi = 1; pi < nids; pi++)
                      lines[li2].push_back(lines[li][pi]);
                    // delete first cell
                    lines[li].clear();
                    connectedSomething = true;
                  }
                  if (connectedSomething) // new check necessary
                    break;
                }
              }
              li2++;
            }
          }
          if (connectedSomething) // new check necessary
            break;

          li++;
        }
      } while (connectedSomething);

      didSimplification = true; // mark this
      vtkPolyData *newContour = vtkPolyData::New();
      vtkPoints *pts = vtkPoints::New();
      pts->ShallowCopy(contour->GetPoints());
      newContour->SetPoints(pts);
      vtkCellArray *lns = vtkCellArray::New();

      for (li = 0; li < contour->GetNumberOfCells(); li++)
      {
        if (lines[li].size() <= 0) // eliminated lines
          continue;
        vtkIdList *idl = vtkIdList::New();
        for (unsigned int xx = 0; xx < lines[li].size(); xx++)
          idl->InsertNextId(lines[li][xx]);
        lns->InsertNextCell(idl);
        idl->Delete();
        lines[li].clear();
      }
      newContour->SetLines(lns);
      pts->Delete();
      lns->Delete();
      if (copy)
        contour->Delete();
      contour = newContour;
      delete [] lines;
    }

    return contour;
  }
  else
  {
    return contour;
  }
}


}
