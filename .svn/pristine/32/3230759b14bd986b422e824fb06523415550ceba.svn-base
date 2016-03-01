

#include "oraSetupError.h"

// ORAIFTools
#include "oraStringTools.h"
#include "oraFileTools.h"
#include "oraIniAccess.h"

#include <itksys/SystemTools.hxx>

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>


namespace ora
{


PlanarViewIdentifier
::PlanarViewIdentifier()
{
  FileName = "";
  AcquisitionDateTimeStr = "";
  ViewUID = "";
}

PlanarViewIdentifier
::PlanarViewIdentifier(std::string fileName, std::string acquDT)
{
  FileName = fileName;
  AcquisitionDateTimeStr = acquDT;
  ora::ORAStringToCDateTime(acquDT, AcquisitionDateTime);
  ViewUID = "";
}


void
SetupError
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "Plan UID: " << m_PlanUID << "\n";
  os << indent << "Method: " << m_Method << "\n";
  os << indent << "Operator Sign: " << m_OperatorSign << "\n";
  os << indent << "Date Time: " << m_DateTime << "\n";
  os << indent << "C Date Time: " << asctime(&m_CDateTime) << "\n";
  os << indent << "Total Translation: " << m_TotalTranslation << "\n";
  os << indent << "Rotation: " << m_Rotation << "\n";
  os << indent << "Dilation: " << m_Dilation << "\n";
  os << indent << "Average IGRT Translation: " << m_AverageIGRTTranslation <<
      "\n";
  os << indent << "Center Of Rotation: " << m_CenterOfRotation << "\n";
  os << indent << "Center Of Rotation Is Up To Date: " <<
      m_CenterOfRotationIsUpToDate << "\n";
  os << indent << "Corrected Center Of Rotation: " <<
      m_CorrectedCenterOfRotation << "\n";
  os << indent << "Rotation Angles: " << m_RotationAngles << "\n";
  os << indent << "Real Translation: " << m_RealTranslation << "\n";
  os << indent << "Transform: " << m_Transform << "\n";
}

SetupError
::SetupError()
{
  m_AllowRotation = true; // default: yes!
  m_ReferenceImageFilename = "";
  m_PlanUID = "";
  m_Method = ST_UNKNOWN;
  m_OperatorSign = "";
  m_DateTime = "";
  m_CDateTime.tm_sec = 0; // init structure
  m_CDateTime.tm_min = 0;
  m_CDateTime.tm_hour = 0;
  m_CDateTime.tm_mday = 0;
  m_CDateTime.tm_mon = 0;
  m_CDateTime.tm_year = 0;
  m_CDateTime.tm_wday = 0;
  m_CDateTime.tm_yday = 0;
  m_CDateTime.tm_isdst = 0;
  m_TotalTranslation.Fill(0);
  m_Rotation.SetIdentity();
  m_Dilation.Fill(1);
  m_AverageIGRTTranslation.Fill(0);
  m_CenterOfRotation.Fill(0);
  m_CorrectedCenterOfRotation.Fill(0);
  m_CenterOfRotationIsUpToDate = false;
  m_RotationAngles.Fill(0);
  m_RealTranslation.Fill(0);
  m_Transform = TransformType::New();
}

SetupError
::~SetupError()
{
  m_Transform = NULL;
}

bool
SetupError
::Equals(SetupError::Pointer other)
{
  if (other)
    return ((other->GetMethod() == this->m_Method) &&
            (other->GetOperatorSign() == this->m_OperatorSign) &&
            (other->GetDateTime() == this->m_DateTime));
  else
    return false;
}

bool
SetupError
::LoadFromORAString(std::string oraString)
{
  // NOTE: plan UID is not contained in this entry!
  std::vector<std::string> toks;
  Tokenize(oraString, toks, "=");
  if (toks.size() == 2)
  {
    // left part
    std::vector<std::string> entries;
    std::string s;
    Tokenize(toks[0], entries, " ");
    if (entries.size() >= 4)
    {
      // NOTE: could also be an entry from study setup errors: -> assume that
      // the last 4 parameters are standard, and the first parameters form a
      // file specification:
      std::size_t n = entries.size();
      // method
      s = ToLowerCaseF(TrimF(entries[n - 4]));
      if (s == "rigidtransformation")
        m_Method = ST_RIGID_TRANSFORM;
      else if (s.substr(0, 15) == "markerdetection")
        m_Method = ST_MARKER;
      else
        m_Method = ST_UNKNOWN;
      // operator
      m_OperatorSign = TrimF(entries[n - 3]);
      // date / time
      this->SetDateTime(TrimF(entries[n - 2]) + " " + TrimF(entries[n - 1]));
      // (only for study setup errors) reference image file:
      m_ReferenceImageFilename = "";
      for (std::size_t k = 0; k < (n - 4); k++)
      {
        if (k > 0)
          m_ReferenceImageFilename += " ";
        m_ReferenceImageFilename += entries[k];
      }
    }
    else
    {
      return false;
    }

    // right part
    entries.clear();
    Tokenize(toks[1], entries, "|");
    if (entries.size() == 7)
    {
      // total transform
      std::vector<std::string> comp;
      Tokenize(entries[0], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_TotalTranslation[i] = atof(comp[i].c_str()) * 10.;
      }
      // rotation
      comp.clear();
      Tokenize(entries[1], comp, ",");
      if (comp.size() == 9)
      {
        for (int i = 0; i < 3; i++)
          for (int j = 0; j < 3; j++)
            m_Rotation[i][j] = atof(comp[i * 3 + j].c_str());
        // -> auto-correct an ORA bug (0-matrix --> identity):
        bool allZero = true;
        for (int i = 0; i < 3; i++)
        {
          for (int j = 0; j < 3; j++)
          {
            if (fabs(m_Rotation[i][j]) > 1e-6)
              allZero = false;
          }
        }
        if (allZero) // -> auto-correction
        {
          m_Rotation.SetIdentity();
        }
      }
      // dilation
      comp.clear();
      Tokenize(entries[2], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_Dilation[i] = atof(comp[i].c_str());
        // -> auto-correct an ORA bug (0-vector --> 1-vector):
        bool allZero = true;
        for (int i = 0; i < 3; i++)
        {
          if (fabs(m_Dilation[i]) > 1e-6)
            allZero = false;
        }
        if (allZero) // -> auto-correction
        {
          m_Dilation.Fill(1.0);
        }
      }
      // average IGRT translation
      comp.clear();
      Tokenize(entries[3], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_AverageIGRTTranslation[i] = atof(comp[i].c_str()) * 10.;
      }
      // center of rotation
      comp.clear();
      Tokenize(entries[4], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_CenterOfRotation[i] = atof(comp[i].c_str()) * 10.;
        m_CenterOfRotationIsUpToDate = false; // set back!
      }
      // rotation angles
      comp.clear();
      Tokenize(entries[5], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_RotationAngles[i] = atof(comp[i].c_str()); // deg
      }
      // real translation
      comp.clear();
      Tokenize(entries[6], comp, ",");
      if (comp.size() == 3)
      {
        for (int i = 0; i < 3; i++)
          m_RealTranslation[i] = atof(comp[i].c_str()) * 10.;
      }

      // finally update the internal transform representation
      this->UpdateInternalTransform();

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

void
SetupError
::SetDateTime(std::string dateTime)
{
  m_DateTime = dateTime;
  ORAStringToCDateTime(m_DateTime, m_CDateTime);
}

void
SetupError
::SetCenterOfRotation(Point3DType cor)
{
  itkDebugMacro("setting m_CenterOfRotation to " << cor);
  if (this->m_CenterOfRotation != cor)
  {
    this->m_CenterOfRotation = cor;
    this->m_CenterOfRotationIsUpToDate = false; // set back!
    this->UpdateInternalTransform();
    this->Modified();
  }
}

void
SetupError
::CorrectCenterOfRotation(Point3DType isoCenter)
{
  for (int d = 0; d < 3; d++) // just a simple subtraction!
    m_CorrectedCenterOfRotation[d] = m_CenterOfRotation[d] - isoCenter[d];
  m_CenterOfRotationIsUpToDate = true;
  this->UpdateInternalTransform(); // in order to take over the center!
}

SetupError::Point3DType
SetupError
::GetCorrectedCenterOfRotation(bool &isValid)
{
  isValid = m_CenterOfRotationIsUpToDate;
  return m_CorrectedCenterOfRotation;
}

void
SetupError
::UpdateInternalTransform()
{
  Point3DType cor;
  if (this->m_CenterOfRotationIsUpToDate) // corrected version
    cor = m_CorrectedCenterOfRotation;
  else // uncorrected version
    cor = this->m_CenterOfRotation;

  // implement it according to ORA implementation:
  vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
  t->PostMultiply();
  double x[3];
  double shift[3];
  for (int d = 0; d < 3; d++)
  {
    // ORA-comment on this:
    // "if average values were corrected before by couch-movements"
    x[d] = this->m_TotalTranslation[d] - this->m_AverageIGRTTranslation[d];
    shift[d] = -1. * (cor[d] - x[d]);
  }
  // first, a shift to the corrected center of rotation:
  t->Translate(shift);
  // apply rotation:
  if (m_AllowRotation) // OK, apply rotation
  {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        m->SetElement(i, j, this->m_Rotation[i][j]);
  }
  else // no rotation!
  {
    m->Identity();
  }
  t->Concatenate(m);
  // back-shift + offset:
  for (int d = 0; d < 3; d++)
    shift[d] = cor[d];
  t->Translate(shift);

  Matrix3x3Type mi;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      mi[i][j] = t->GetMatrix()->GetElement(i, j);
  this->m_Transform->SetMatrix(mi);
  Vector3DType oi;
  for (int i = 0; i < 3; i++)
    oi[i] = t->GetMatrix()->GetElement(i, 3);
  this->m_Transform->SetOffset(oi);
  this->m_Transform->GetParameters(); // update internal m_Parameters ...

  m = NULL;
  t = NULL;
}

int
SetupError
::FindBestMatchingPlanarView(std::vector<PlanarViewIdentifier *> *planarViews,
    unsigned int maxSec)
{
  if (!planarViews || planarViews->size() <= 0)
    return -1;

  int minIdx = -1;
  double minDiff = static_cast<double>(maxSec) + 1e-6;
  time_t tref = mktime(&this->m_CDateTime);
  for (std::size_t i = 0; i < planarViews->size(); i++)
  {
    if (!planarViews)
      continue;
    time_t tview = mktime(&(*planarViews)[i]->AcquisitionDateTime);
    double diff = fabs(difftime(tref, tview));
    if (diff < minDiff)
    {
      minIdx = i;
      minDiff = diff;
    }
  }

  return minIdx;
}



// static member initialization
SetupError::Pointer SetupErrorCollection::m_DefaultSetupError = NULL;

SetupError::Pointer
SetupErrorCollection
::GetDefaultSetupError()
{
  if (!SetupErrorCollection::m_DefaultSetupError)
  {
    SetupErrorCollection::m_DefaultSetupError = SetupError::New();
    std::ostringstream os;
    // -> apply a fake string: RIGID IDENTITY (NO ERROR)
    std::string timestamp;
    CurrentORATimeString(timestamp);
    os << "RigidTransformation NN " << timestamp << "=0.0,0.0,0.0|" <<
        "1.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,1.0|1.0,1.0,1.0|0.0,0.0,0.0|" <<
        "0.0,0.0,0.0|0.0,0.0,0.0|0.0,0.0,0.0";
    SetupErrorCollection::m_DefaultSetupError->LoadFromORAString(os.str());
  }

  return SetupErrorCollection::m_DefaultSetupError;
}

SetupErrorCollection::Pointer
SetupErrorCollection
::CreateFromFile(std::string setupErrorsFile)
{
  setupErrorsFile = UnixUNCConverter::GetInstance()->EnsureOSCompatibility(
    setupErrorsFile); // UNC-compatibility!

  if (!itksys::SystemTools::FileExists(setupErrorsFile.c_str(), true))
    return NULL;

  IniAccess ini(setupErrorsFile);
  SetupErrorCollection::Pointer coll = SetupErrorCollection::New();

  // parse sections (all except the [File] section)
  std::vector<std::string> *sections = ini.GetSections();

  for (std::size_t i = 0; i < sections->size(); i++)
  {
    std::string section = (*sections)[i];
    if (TrimF(section) != "File") // seems like a plan UID ...
    {
      std::vector<std::string> *errs = ini.GetIndents(section);

      for (std::size_t j = 0; j < errs->size(); j++)
      {
        std::string err = (*errs)[j];
        std::string val = ini.ReadString(section, err, "");
        std::string combined = err + "=" + val;

        SetupError::Pointer se = SetupError::New();
        if (se->LoadFromORAString(combined))
        {
          se->SetPlanUID(section); // not covered by ORA string
          coll->AddSetupError(se);
        }
      }

      delete errs;
    }
  }

  delete sections;

  return coll;
}

SetupErrorCollection
::SetupErrorCollection()
{
  m_SetupErrors.clear();
}

SetupErrorCollection
::~SetupErrorCollection()
{
  Clear();
}

void
SetupErrorCollection
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << "Setup Errors: N=" << m_SetupErrors.size() << "\n";
}

bool
SetupErrorCollection
::AddSetupError(SetupError::Pointer se)
{
  if (!se)
    return false;

  for (std::size_t i = 0; i < m_SetupErrors.size(); i++)
  {
    if (m_SetupErrors[i]->Equals(se))
      return false;
  }
  m_SetupErrors.push_back(se);

  return true;
}

unsigned int
SetupErrorCollection
::GetNumberOfSetupErrors()
{
  return m_SetupErrors.size();
}

void
SetupErrorCollection
::Clear()
{
  for (std::size_t i = 0; i < m_SetupErrors.size(); ++i)
    m_SetupErrors[i] = NULL;
  m_SetupErrors.clear();
}

const std::vector<SetupError::Pointer> *
SetupErrorCollection
::GetSetupErrors()
{
  return &m_SetupErrors;
}

bool
SetupErrorCollection
::RemoveSetupError(SetupError::Pointer se)
{
  if (!se)
    return false;

  for (std::size_t i = 0; i < m_SetupErrors.size(); ++i)
  {
    if (m_SetupErrors[i]->Equals(se))
    {
      m_SetupErrors[i] = NULL;
      m_SetupErrors.erase(m_SetupErrors.begin() + i); // erase
      return true;
    }
  }

  return false;
}

SetupError::Pointer
SetupErrorCollection
::FindSetupError(std::string dateTime, SetupError::SetupType method,
  std::string operatorSign)
{
  if (dateTime.length() <= 0)
    return NULL;

  for (std::size_t i = 0; i < m_SetupErrors.size(); ++i)
  {
    if (dateTime == m_SetupErrors[i]->GetDateTime()) // must match
    {
      bool match = true;
      if (method != SetupError::ST_UNKNOWN)
      {
        if (m_SetupErrors[i]->GetMethod() != method)
          match = false;
      }
      if (operatorSign.length() > 0)
      {
        if (m_SetupErrors[i]->GetOperatorSign() != operatorSign)
          match = false;
      }
      if (match)
        return m_SetupErrors[i];
    }
  }

  return NULL;
}

SetupError::Pointer
SetupErrorCollection
::FindBestMatchingSetupError(std::string dateTime, bool mustBeSameDay,
    std::string planUID, SetupError::SetupType method, std::string operatorSign)
{
  Trim(dateTime);
  if (dateTime.length() <= 0) // need at least this one
    return NULL;

  // reference date/times
  tm dt;
  ORAStringToCDateTime(dateTime, dt);
  time_t thisDT = mktime(&dt);
  tm cdt = dt;
  cdt.tm_sec = 0; // clear time-part
  cdt.tm_min = 0;
  cdt.tm_hour = 0;
  cdt.tm_isdst = 0;
  time_t thisD = mktime(&cdt);
  int minIdx = -1;
  double minDiff = 9e18;

  for (std::size_t i = 0; i < m_SetupErrors.size(); ++i)
  {
    tm refDT = m_SetupErrors[i]->GetCDateTime();
    time_t othDT = mktime(&refDT);
    tm odt = m_SetupErrors[i]->GetCDateTime();
    odt.tm_sec = 0; // clear time-part
    odt.tm_min = 0;
    odt.tm_hour = 0;
    odt.tm_isdst = 0;
    time_t othD = mktime(&odt);


    bool consider = !(mustBeSameDay && fabs(difftime(thisD, othD)) > 1e-3);
    if (consider)
    {
      bool match = true;
      if (planUID.length() > 0)
      {
        if (m_SetupErrors[i]->GetPlanUID() != planUID)
          match = false;
      }
      if (method != SetupError::ST_UNKNOWN)
      {
        if (m_SetupErrors[i]->GetMethod() != method)
          match = false;
      }
      if (operatorSign.length() > 0)
      {
        if (m_SetupErrors[i]->GetOperatorSign() != operatorSign)
          match = false;
      }
      if (match)
      {
        double diff = fabs(difftime(thisDT, othDT));
        if (diff < minDiff)
        {
          minDiff = diff;
          minIdx = i;
        }
      }
    }
  }

  if (minIdx > -1)
     return m_SetupErrors[(std::size_t)minIdx];
  else
    return NULL;
}

unsigned int
SetupErrorCollection
::RemoveSetupErrors(std::string dateTime, SetupError::SetupType method,
  std::string operatorSign)
{
  unsigned int removed = 0;

  SetupError::Pointer se = NULL;
  do
  {
    se = FindSetupError(dateTime, method, operatorSign);
    if (se)
    {
      if (RemoveSetupError(se))
      {
        removed++;
      }
    }
  } while (se);

  return removed;
}


}
