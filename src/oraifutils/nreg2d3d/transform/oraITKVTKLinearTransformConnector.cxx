//

#include "oraITKVTKLinearTransformConnector.h"

namespace ora
{

VTKTransformModifiedObserver *
VTKTransformModifiedObserver::New()
{
  return (new VTKTransformModifiedObserver());
}

void VTKTransformModifiedObserver::Execute(vtkObject *caller,
    unsigned long event, void *callData)
{
  if (this->m_Host && event == vtkCommand::ModifiedEvent)
    this->m_Host->UpdateTransform(false); // initiate the update (VTK -> ITK)
}

void VTKTransformModifiedObserver::SetHost(HostPointer host)
{
  this->m_Host = host;
}
VTKTransformModifiedObserver::HostPointer VTKTransformModifiedObserver::GetHost()
{
  return this->m_Host;
}

VTKTransformModifiedObserver::VTKTransformModifiedObserver() :
  vtkCommand()
{
  this->m_Host = NULL;
}

VTKTransformModifiedObserver::~VTKTransformModifiedObserver()
{
  this->m_Host = NULL;
}

void ITKTransformModifiedObserver::Execute(itk::Object *caller,
    const itk::EventObject &event)
{
  this->Execute((const itk::Object *) caller, event);
}

void ITKTransformModifiedObserver::Execute(const itk::Object *object,
    const itk::EventObject &event)
{
  if (this->m_Host && typeid(event) == typeid(itk::ModifiedEvent))
    this->m_Host->UpdateTransform(true); // initiate the update (ITK -> VTK)
}

void ITKTransformModifiedObserver::SetHost(HostPointer host)
{
  this->m_Host = host;
}

ITKTransformModifiedObserver::HostPointer ITKTransformModifiedObserver::GetHost()
{
  return this->m_Host;
}

ITKTransformModifiedObserver::ITKTransformModifiedObserver() :
  itk::Command()
{
  this->m_Host = NULL;
}

ITKTransformModifiedObserver::~ITKTransformModifiedObserver()
{
  this->m_Host = NULL;
}

ITKVTKLinearTransformConnector::ITKVTKLinearTransformConnector() :
  itk::Object()
{
  this->m_RelativeVTKITKMatrix = NULL;
  this->m_RelativeITKVTKMatrix = NULL;
  this->m_ITKTransform = NULL;
  this->m_VTKTransform = NULL;
  this->m_IsUpdating = false;
  this->m_ITKObserver = ITKTransformModifiedObserver::New();
  this->m_ITKObserver->SetHost(this);
  this->m_ITKObserverTag = 0;
  this->m_VTKObserver = VTKTransformModifiedObserver::New();
  this->m_VTKObserver->SetHost(this);
  this->m_VTKObserverTag = 0;
}

ITKVTKLinearTransformConnector::~ITKVTKLinearTransformConnector()
{
  this->m_RelativeVTKITKMatrix = NULL;
  this->m_RelativeITKVTKMatrix = NULL;
  this->m_ITKTransform = NULL;
  this->m_VTKTransform = NULL;
  this->SetITKTransform(NULL); // unregister observer
  this->m_ITKObserver = NULL;
  this->SetVTKTransform(NULL); // unregister observer
  this->m_VTKObserver = NULL;
}

void ITKVTKLinearTransformConnector::PrintSelf(std::ostream& os,
    itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Relative VTK to ITK matrix: " << std::endl;
  if (this->m_RelativeVTKITKMatrix)
    this->m_RelativeVTKITKMatrix->Print(os);
  os << indent << "VTK transform: " << std::endl;
  if (this->m_VTKTransform)
    this->m_VTKTransform->Print(os);
  os << indent << "VTK observer:" << std::endl;
  if (this->m_VTKObserver)
    this->m_VTKObserver->Print(os);

  os << indent << "Relative ITK to VTK matrix: " << std::endl;
  if (this->m_RelativeITKVTKMatrix)
    this->m_RelativeITKVTKMatrix->Print(os);
  os << indent << "ITK transform:\n" << this->m_ITKTransform << std::endl;
  os << indent << "ITK observer:\n" << this->m_ITKObserver << std::endl;
}

void ITKVTKLinearTransformConnector::SetITKTransform(
    ITKTransformType *itkTransf)
{
  if (itkTransf != this->m_ITKTransform)
  {
    if (this->m_ITKTransform) // unregister observer
    {
      this->m_ITKTransform->RemoveObserver(this->m_ITKObserverTag);
      this->m_ITKObserverTag = 0;
    }

    this->m_ITKTransform = itkTransf; // new transform

    if (this->m_ITKTransform)
    {
      this->m_ITKObserverTag = this->m_ITKTransform->AddObserver(
          itk::ModifiedEvent(), this->m_ITKObserver); // register observer
      this->m_ITKTransform->Modified(); // force immediate update
    }
    this->Modified();
  }
}

void ITKVTKLinearTransformConnector::SetVTKTransform(
    VTKTransformPointer vtkTransf)
{
  if (vtkTransf != this->m_VTKTransform)
  {
    if (this->m_VTKTransform) // unregister observer
    {
      this->m_VTKTransform->RemoveObserver(this->m_VTKObserverTag);
      this->m_VTKObserverTag = 0;
    }

    this->m_VTKTransform = vtkTransf; // new transform

    if (this->m_VTKTransform)
    {
      this->m_VTKObserverTag = this->m_VTKTransform->AddObserver(
          vtkCommand::ModifiedEvent, this->m_VTKObserver); // register observer
      this->m_VTKTransform->Modified(); // force immediate update
    }
    this->Modified();
  }
}

void ITKVTKLinearTransformConnector::SetRelativeITKVTKMatrix(
    RelativeMatrixPointer matrix)
{
  if (matrix != this->m_RelativeITKVTKMatrix)
  {
    this->m_RelativeITKVTKMatrix = matrix;
    this->UpdateTransform(true); // force immediate update
    this->Modified();
  }
}

void ITKVTKLinearTransformConnector::SetRelativeVTKITKMatrix(
    RelativeMatrixPointer matrix)
{
  if (matrix != this->m_RelativeVTKITKMatrix)
  {
    this->m_RelativeVTKITKMatrix = matrix;
    this->UpdateTransform(false); // force immediate update
    this->Modified();
  }
}

void ITKVTKLinearTransformConnector::UpdateTransform(bool itkToVTK)
{
  if (this->m_IsUpdating) // reentrancy
    return;

  // for mutual transformation update both transformations are required
  if (!this->m_ITKTransform || !this->m_VTKTransform)
    return;

  this->m_IsUpdating = true;

  unsigned int i = 0, j = 0;
  if (itkToVTK) // ITK -> VTK
  {
    vtkSmartPointer<vtkMatrix4x4> vmatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    ITKTransformType::MatrixType imatrix = this->m_ITKTransform->GetMatrix();
    ITKTransformType::OffsetType ioffset = this->m_ITKTransform->GetOffset();

    for (i = 0; i < 3; i++)
    {
      // matrix
      for (j = 0; j < 3; j++)
        vmatrix->SetElement(i, j, imatrix[i][j]);
      // offset
      vmatrix->SetElement(i, 3, ioffset[i]);
    }

    if (this->m_RelativeITKVTKMatrix) // optional relative ITK -> VTK transform
      vtkMatrix4x4::Multiply4x4(vmatrix, this->m_RelativeITKVTKMatrix, vmatrix);

    this->m_VTKTransform->SetMatrix(vmatrix); // Modified called internally!
  }
  else // VTK -> ITK
  {
    vtkSmartPointer<vtkMatrix4x4> vmatrix = this->m_VTKTransform->GetMatrix();
    ITKTransformType::MatrixType imatrix;
    ITKTransformType::OffsetType ioffset;

    if (!this->m_RelativeVTKITKMatrix) // no relative VTK -> ITK transform
    {
      for (i = 0; i < 3; i++)
      {
        // matrix
        for (j = 0; j < 3; j++)
          imatrix[i][j] = vmatrix->GetElement(i, j);
        // offset
        ioffset[i] = vmatrix->GetElement(i, 3);
      }
    }
    else // relative VTK -> ITK transform
    {
      vtkSmartPointer<vtkMatrix4x4> temp = vtkSmartPointer<vtkMatrix4x4>::New();
      vtkMatrix4x4::Multiply4x4(vmatrix, this->m_RelativeVTKITKMatrix, temp);
      for (i = 0; i < 3; i++)
      {
        // matrix
        for (j = 0; j < 3; j++)
          imatrix[i][j] = temp->GetElement(i, j);
        // offset
        ioffset[i] = temp->GetElement(i, 3);
      }
    }
    this->m_ITKTransform->SetMatrix(imatrix);
    this->m_ITKTransform->SetOffset(ioffset);
  }
  this->m_IsUpdating = false;
}

}
