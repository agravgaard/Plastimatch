
#include "oraQProgressEmitter.h"


namespace ora
{


QProgressEmitter
::QProgressEmitter()
  : QVTKProgressEventAdaptor()
{

}

QProgressEmitter
::~QProgressEmitter()
{

}

void
QProgressEmitter
::SetProgress(double progress)
{
  this->EmitProgressSignal(progress);
}


}
