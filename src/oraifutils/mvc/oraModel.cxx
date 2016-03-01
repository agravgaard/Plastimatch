

#include "oraModel.h"

// Forward declarations
#include "oraObserver.h"

namespace ora 
{


Model
::Model()
{
  m_Observers = new ObserverVectorType;
}

Model
::~Model()
{
  delete m_Observers;
}

void
Model
::Register(Observer *observer)
{
  bool found = false;

  // look if observer has already been added:
  for (unsigned int i = 0; i < m_Observers->size(); ++i)
  {
    if (m_Observers->at(i) == observer)
    {
      found = true;
      break;
    }
  }

  if (!found) // add if not already added
    m_Observers->push_back(observer);
}

void
Model
::Unregister(Observer *observer)
{
  int idx = -1;

  // look if observer has been added:
  for (unsigned int i = 0; i < m_Observers->size(); ++i)
  {
    if (m_Observers->at(i) == observer)
    {
      idx = i;
      break;
    }
  }

  if (idx > -1) // remove if added
    m_Observers->erase(m_Observers->begin() + idx);
}

void
Model
::Notify(int id)
{
  for(unsigned int i = 0; i < m_Observers->size(); ++i)
    m_Observers->at(i)->Update(id); // notify ALL observers
}


}

