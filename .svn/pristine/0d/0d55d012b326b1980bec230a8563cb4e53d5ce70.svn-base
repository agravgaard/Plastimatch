

#include "oraFrameOfReference.h"

#include <utility>

// ORAIFTools
#include "oraStringTools.h"
#include "oraIniAccess.h"
#include "oraFileTools.h"
#include "SimpleDebugger.h"


namespace ora 
{


FrameOfReference
::FrameOfReference()
{
  m_Transform = TransformType::New(); // pure identity
  m_VTKTransform = vtkSmartPointer<vtkTransform>::New();
  m_UID = "";
  m_Descriptor = "";
  m_Translation.Fill(0.);
  m_Rotation.SetIdentity();
  m_Origin.Fill(0.);
  m_Scaling.Fill(1.);
  m_FORID = "";
}

FrameOfReference
::~FrameOfReference()
{
  m_Transform = NULL;
  m_VTKTransform = NULL;
}

void
FrameOfReference
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Transformation:" << std::endl;
  if (m_Transform)
    m_Transform->Print(os, indent.GetNextIndent());
  else
    os << indent.GetNextIndent() << "not set" << std::endl;

  os << indent << "VTK-Transformation:" << std::endl;
  if (m_VTKTransform)
    m_VTKTransform->Print(os);
  else
    os << indent.GetNextIndent() << "not set" << std::endl;

  os << indent << "FOR UID:" << std::endl;
  os << indent.GetNextIndent() << "'" << m_UID << "'" << std::endl;

  os << indent << "Descriptor:" << std::endl;
    os << indent.GetNextIndent() << "'" << m_Descriptor << "'" << std::endl;

  os << indent << "Translation:" << std::endl;
  os << indent.GetNextIndent() << m_Translation << std::endl;

  os << indent << "Rotation:" << std::endl;
  for (int i = 0; i < 3; i++)
    os << indent.GetNextIndent() << m_Rotation[i][0] <<
      " " << m_Rotation[i][1] << " " << m_Rotation[i][2] <<
      std::endl;

  os << indent << "Origin:" << std::endl;
  os << indent.GetNextIndent() << m_Origin << std::endl;

  os << indent << "Scaling:" << std::endl;
  os << indent.GetNextIndent() << m_Scaling << std::endl;

  os << indent << "FOR-ID:" << std::endl;
  os << indent.GetNextIndent() << m_FORID << std::endl;
}

bool
FrameOfReference
::LoadFromORAString(std::string oraString)
{
  // FORMAT:
  // (older)
  // <FOR-UID>=
  //   <descriptor>,<transl-x>,<transl-y>,<transl-z>|
  //   <r11>,<r12>,<r13>,<r21>,<r22>,<r23>,<r31>,<r32>,<r33>|
  //   <orig-x>,<orig-y>,<orig-z>|
  //   <scale-x>,<scale-y>,<scale-z>
  //   (|<FOR-ID>)
  // (newer)
  // <FOR-UID>=
  //   <descriptor>|
  //   <transl-x>,<transl-y>,<transl-z>|
  //   <r11>,<r12>,<r13>,<r21>,<r22>,<r23>,<r31>,<r32>,<r33>|
  //   <orig-x>,<orig-y>,<orig-z>|
  //   <scale-x>,<scale-y>,<scale-z>
  //   (|<FOR-ID>)
  // (newer: variation)
  // <FOR-UID>=
  //   <descriptor>|
  //   <transl-x>,<transl-y>,<transl-z>|
  //   <r11>,<r12>,<r13>,<r21>,<r22>,<r23>,<r31>,<r32>,<r33>|
  //   <orig-x>,<orig-y>,<orig-z>,<scale-x>,<scale-y>,<scale-z>
  //   (|<FOR-ID>)
  //
  // NOTE: due to some old open radART releases and DICOM converter releases,
  // there are FOR entries which contain scaling factors of 0.0 - we simply
  // do not allow them and replace them by 1.0!

  // -> check basic criteria for both formats:
  std::vector<std::string> tok0;
  TokenizeIncludingEmptySpaces(oraString, tok0, "=");
  if (tok0.size() < 2)
    return false;

  this->m_UID = tok0[0];

  std::vector<std::string> tok1;

  TokenizeIncludingEmptySpaces(tok0[1], tok1, "|");
  if (tok1.size() < 4) // at least 4 pipe-components required
    return false;

  // now distinguish the format-type:
  std::vector<std::string> tok2;
  tok2.clear();
  TokenizeIncludingEmptySpaces(tok1[0], tok2, ",");

  if (tok1[0].length() > 0 &&
      tok1[0].find(",") == std::string::npos) // newer format
  {
    if (tok1.size() < 5) // at least 5 pipe-components required
      return false;

    // descriptor:
    this->m_Descriptor = tok1[0];

    // translation:
    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[1], tok2, ",");
    if (tok2.size() < 3)
      return false;
    for (int i = 0; i < 3; ++i)
      this->m_Translation[i] = atof(tok2[i].c_str()) * 10.; // [cm] -> [mm]

    // rotational matrix coefficients:
    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[2], tok2, ",");
    if (tok2.size() < 9)
      return false;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        this->m_Rotation[i][j] = atof(tok2[i * 3 + j].c_str());

    // -> auto-correct an ORA bug (0-matrix --> identity):
    bool allZero = true;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (fabs(this->m_Rotation[i][j]) > 1e-6)
          allZero = false;
      }
    }
    if (allZero) // -> auto-correction
    {
      this->m_Rotation.SetIdentity();
    }

    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[3], tok2, ",");
    if (tok2.size() <= 3) // newer format
    {
      // FOR origin:
      if (tok2.size() < 3)
        return false;
      for (int i = 0; i < 3; ++i)
        this->m_Origin[i] = atof(tok2[i].c_str()) * 10.; // [cm] -> [mm]

      // scaling:
      tok2.clear();
      TokenizeIncludingEmptySpaces(tok1[4], tok2, ",");
      if (tok2.size() < 3)
        return false;
      for (int i = 0; i < 3; ++i)
      {
        this->m_Scaling[i] = atof(tok2[i].c_str());
        if (fabs(this->m_Scaling[i]) < 1e-6)
          this->m_Scaling[i] = 1.0; // auto-replace invalid scaling factor
      }

      // (optional in case of old FORs) FORID
      if (tok1.size() > 5) // included (seems to be a newer FOR)
        this->m_FORID = TrimF(tok1[5]);
      else
        this->m_FORID = ""; // undefined
    }
    else // newer format: variation
    {
      // FOR origin:
      if (tok2.size() < 6)
        return false;
      for (int i = 0; i < 3; ++i)
        this->m_Origin[i] = atof(tok2[i].c_str()) * 10.; // [cm] -> [mm]

      // scaling:
      for (int i = 0; i < 3; ++i)
      {
        this->m_Scaling[i] = atof(tok2[3 + i].c_str());
        if (this->m_Scaling[i] == 0)
          this->m_Scaling[i] = 1.0; // auto-replace invalid scaling factor
      }

      // (optional in case of old FORs) FORID
      if (tok1.size() > 4) // included (seems to be a newer FOR)
        this->m_FORID = TrimF(tok1[4]);
      else
        this->m_FORID = ""; // undefined
    }
  }
  else if (tok2.size() == 4)  // older format
  {
    // descriptor, translation:
    this->m_Descriptor = tok2[0];
    for (int i = 0; i < 3; ++i)
      this->m_Translation[i] = atof(tok2[i + 1].c_str()) * 10.; // [cm] -> [mm]

    // rotational matrix coefficients:
    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[1], tok2, ",");
    if (tok2.size() < 9)
      return false;
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        this->m_Rotation[i][j] = atof(tok2[i * 3 + j].c_str());

    // -> auto-correct an ORA bug (0-matrix --> identity):
    bool allZero = true;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        if (fabs(this->m_Rotation[i][j]) > 1e-6)
          allZero = false;
      }
    }
    if (allZero) // -> auto-correction
    {
      this->m_Rotation.SetIdentity();
    }

    // FOR origin:
    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[2], tok2, ",");
    if (tok2.size() < 3)
      return false;
    for (int i = 0; i < 3; ++i)
      this->m_Origin[i] = atof(tok2[i].c_str()) * 10.; // [cm] -> [mm]

    // scaling:
    tok2.clear();
    TokenizeIncludingEmptySpaces(tok1[3], tok2, ",");
    if (tok2.size() < 3)
      return false;
    for (int i = 0; i < 3; ++i)
    {
      this->m_Scaling[i] = atof(tok2[i].c_str());
      if (this->m_Scaling[i] == 0)
        this->m_Scaling[i] = 1.0; // auto-replace invalid scaling factor
    }

    // (optional in case of old FORs) FORID
    if (tok1.size() > 4) // included (seems to be a newer FOR)
      this->m_FORID = TrimF(tok1[4]);
    else
      this->m_FORID = ""; // undefined
  }
  else // unknown
    return false;

  this->UpdateInternalTransformations(); // update FOR

  return true;
}

void
FrameOfReference
::UpdateInternalTransformations()
{
  // VTK

  // set back:
  m_VTKTransform->Identity();
  m_VTKTransform->PostMultiply();

  // retrieve basic matrix from patient position:
  vtkSmartPointer<vtkMatrix4x4> patientPositionMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  patientPositionMatrix->Zero();

  // BUG-FIX (2010-06-22): the FOR descriptor (patient position) is irrelevant
  // for image the pure FOR-transformation, we simply have to apply the
  // pure LPS-to-IEC transformation. However, the name has not been changed
  // (patientPositionMatrix) as it describes the transformation from the
  // patient-based LPS-system to the machine-oriented IEC-system.
  patientPositionMatrix->SetElement(0, 0, 1.);
  patientPositionMatrix->SetElement(1, 2, -1.);
  patientPositionMatrix->SetElement(2, 1, 1.);
  patientPositionMatrix->SetElement(3, 3, 1.);

//  if (m_Descriptor == "HFP")
//  {
//    patientPositionMatrix->SetElement(0, 0, -1.);
//    patientPositionMatrix->SetElement(1, 2, 1.);
//    patientPositionMatrix->SetElement(2, 1, 1.);
//  }
//  else if (m_Descriptor == "HFDR")
//  {
//    patientPositionMatrix->SetElement(0, 2, 1.);
//    patientPositionMatrix->SetElement(1, 0, 1.);
//    patientPositionMatrix->SetElement(2, 1, 1.);
//  }
//  else if (m_Descriptor == "FFDR")
//  {
//    patientPositionMatrix->SetElement(0, 2, 1.);
//    patientPositionMatrix->SetElement(1, 0, -1.);
//    patientPositionMatrix->SetElement(2, 1, -1.);
//  }
//  else if (m_Descriptor == "FFP")
//  {
//    patientPositionMatrix->SetElement(0, 0, 1.);
//    patientPositionMatrix->SetElement(1, 2, 1.);
//    patientPositionMatrix->SetElement(2, 1, -1.);
//  }
//  else if (m_Descriptor == "HFDL")
//  {
//    patientPositionMatrix->SetElement(0, 2, -1.);
//    patientPositionMatrix->SetElement(1, 0, -1.);
//    patientPositionMatrix->SetElement(2, 1, 1.);
//  }
//  else if (m_Descriptor == "FFDL")
//  {
//    patientPositionMatrix->SetElement(0, 2, -1.);
//    patientPositionMatrix->SetElement(1, 0, 1.);
//    patientPositionMatrix->SetElement(2, 1, -1.);
//  }
//  else if (m_Descriptor == "FFS")
//  {
//    patientPositionMatrix->SetElement(0, 0, -1.);
//    patientPositionMatrix->SetElement(1, 2, -1.);
//    patientPositionMatrix->SetElement(2, 1, -1.);
//  }
//  else // HFS or other (treated as HFS)
//  {
//    patientPositionMatrix->SetElement(0, 0, 1.);
//    patientPositionMatrix->SetElement(1, 2, -1.);
//    patientPositionMatrix->SetElement(2, 1, 1.);
//  }
//  patientPositionMatrix->SetElement(3, 3, 1.);

  // convert rotation to 4x4 homogeneous matrix
  vtkSmartPointer<vtkMatrix4x4> rotMatrix =
    vtkSmartPointer<vtkMatrix4x4>::New();
  rotMatrix->Identity();
  for (unsigned int i = 0; i < 3; i++) // override the inner 3x3-matrix
    for (unsigned int j = 0; j < 3; j++)
      rotMatrix->SetElement(i, j, m_Rotation[i][j]);

  // patient position correction (HFS, HFP ...) - around WCS-orgin!
  patientPositionMatrix->Invert();
  m_VTKTransform->Concatenate(patientPositionMatrix);
  // affine FOR transformations
  // - FOR back scaling (from FOR origin)
  m_VTKTransform->Translate(-m_Origin[0], -m_Origin[1], -m_Origin[2]);
  m_VTKTransform->Scale(m_Scaling[0], m_Scaling[1], m_Scaling[2]);
  // - FOR back rotation (around FOR origin)
  rotMatrix->Invert();
  m_VTKTransform->Concatenate(rotMatrix);
  m_VTKTransform->Translate(m_Origin[0], m_Origin[1], m_Origin[2]);
  // - FOR shifting
  m_VTKTransform->Translate(-m_Translation[0], -m_Translation[1],
      -m_Translation[2]);

  rotMatrix = NULL;
  patientPositionMatrix = NULL;

  // NOTE: this transform represents the pure FOR-transform; computation of the
  // resultant image transformation in WCS requires preceding incorporation
  // of image origin and image orientation (direction cosines)!


  // VTK -> ITK

  vtkSmartPointer<vtkMatrix4x4> vmatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vmatrix->DeepCopy(this->m_VTKTransform->GetMatrix());
//  vmatrix->Invert();
  TransformType::MatrixType imatrix;
  TransformType::OffsetType ioffset;
  for (int i = 0; i < 3; i++)
  {
    // matrix
    for (int j = 0; j < 3; j++)
      imatrix[i][j] = vmatrix->GetElement(i, j);
    // offset
    ioffset[i] = vmatrix->GetElement(i, 3);
  }
  this->m_Transform->SetMatrix(imatrix);
  this->m_Transform->SetOffset(ioffset);
}


// static member initialization
FrameOfReference::Pointer FrameOfReferenceCollection::m_DefaultFOR = NULL;

FrameOfReference::Pointer
FrameOfReferenceCollection
::GetDefaultFOR()
{
  if (!FrameOfReferenceCollection::m_DefaultFOR)
  {
    FrameOfReferenceCollection::m_DefaultFOR = FrameOfReference::New();
    std::string fakestr;
    fakestr = "=HFS|0,0,0|1,0,0,0,1,0,0,0,1|0,0,0|1,1,1|";
    FrameOfReferenceCollection::m_DefaultFOR->LoadFromORAString(fakestr);
  }

  return FrameOfReferenceCollection::m_DefaultFOR;
}

FrameOfReferenceCollection
::FrameOfReferenceCollection()
{
  m_FORMap.clear();
  m_PatientUID = "";
  m_PatientName = "";
}

FrameOfReferenceCollection
::~FrameOfReferenceCollection()
{
  m_FORMap.clear();
}

void
FrameOfReferenceCollection
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Frames of Reference (n=" << m_FORMap.size() << "):" <<
    std::endl;
  if (m_FORMap.size() > 0)
  {
    FORMapType::const_iterator it;
    for (it = m_FORMap.begin(); it != m_FORMap.end(); ++it)
      os << indent.GetNextIndent() << it->first << " (" <<
        it->second.GetPointer() << ")" << std::endl;
  }
  else
    os << indent.GetNextIndent() << "<no frames>" << std::endl;
  os << indent << "Patient UID: " << m_PatientUID << std::endl;
  os << indent << "Patient Name: " << m_PatientName << std::endl;
}

FrameOfReferenceCollection::Pointer
FrameOfReferenceCollection
::CreateFromFile(std::string framesFile)
{
  framesFile = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
    framesFile); // UNC-compatibility!

  IniAccess ini(framesFile);

  if (!ini.IsSectionExisting("FramesOfReference"))
    return NULL;

  std::vector<std::string> *indents = ini.GetIndents("FramesOfReference");
  std::vector<std::string>::iterator it;

  FrameOfReferenceCollection::Pointer forColl =
    FrameOfReferenceCollection::New();

  for (it = indents->begin(); it != indents->end(); ++it)
  {
    FrameOfReference::Pointer FOR = FrameOfReference::New();
    bool loaded = FOR->LoadFromORAString(*it + "=" +
        ini.ReadString("FramesOfReference", *it, "", false));
    if (loaded)
      forColl->AddFOR(FOR);
  }
  delete indents;

  if (forColl->GetFORUIDs().size() <= 0) // no valid frames found
    forColl = NULL;

  return forColl;
}

FrameOfReference::Pointer
FrameOfReferenceCollection
::FindFOR(const std::string UID)
{
  FrameOfReference::Pointer ret = NULL;
  FORMapType::iterator it;
  for (it = m_FORMap.begin(); it != m_FORMap.end(); ++it)
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
FrameOfReferenceCollection
::AddFOR(FrameOfReference::Pointer FOR)
{
  if (FOR)
  {
    std::pair<FORMapType::iterator, bool> ret =
      m_FORMap.insert(std::make_pair(FOR->GetUID(), FOR));

    return ret.second;
  }

  return false;
}

bool
FrameOfReferenceCollection
::RemoveFOR(FrameOfReference::Pointer FOR)
{
  if (FOR)
    return this->RemoveFOR(FOR->GetUID());

  return false;
}

bool
FrameOfReferenceCollection
::RemoveFOR(std::string UID)
{
  if (UID.length() > 0)
  {
    FORMapType::iterator it;

    for (it = m_FORMap.begin(); it != m_FORMap.end(); ++it)
    {
      if (it->first == UID)
      {
        m_FORMap.erase(it); //, it);
        return true;
      }
    }
  }

  return false;
}

void
FrameOfReferenceCollection
::Clear()
{
  m_FORMap.clear();
}

std::vector<std::string>
FrameOfReferenceCollection
::GetFORUIDs()
{
  std::vector<std::string> ret;
  FORMapType::iterator it;

  for (it = m_FORMap.begin(); it != m_FORMap.end(); ++it)
  {
    ret.push_back(it->first);
  }

  return ret;
}

unsigned int
FrameOfReferenceCollection
::GetNumberOfFORs()
{
  return m_FORMap.size();
}


}

