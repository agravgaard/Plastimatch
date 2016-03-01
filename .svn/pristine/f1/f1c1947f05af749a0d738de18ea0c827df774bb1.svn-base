#include "YKThreadRegi.h"
#include "register_gui.h"

#include "registration.h"
#include "registration_data.h"
#include "registration_parms.h"

#include "plm_exception.h"

#include <QTime>

using namespace std;

YKThreadRegi::YKThreadRegi(register_gui* pParent, QString& strPathCommand, int iRowIndex)
{     
    m_strCommandFilePath = strPathCommand;
    m_iProcessingTime = 0;  
    m_iIndex = iRowIndex;

    m_pParent = pParent;
}

YKThreadRegi::~YKThreadRegi()
{
  
}

void YKThreadRegi::run()//called by thread.start()
{    
    if (m_pParent == NULL)
    {
        exec();
        return;
    }        

    m_pParent->m_mutex.lock(); //maybe not needed just for copying
    if (m_iIndex >= m_pParent->m_vRegiQue.size())
    {
        m_pParent->m_mutex.unlock(); //maybe not needed just for copying    
        exec();
        return;
    }

    m_pParent->m_vRegiQue.at(m_iIndex).m_iStatus = ST_PENDING;
    m_pParent->UpdateTable_Que();

    m_pParent->m_mutex.unlock(); //maybe not needed just for copying        

    std::string strPath = m_strCommandFilePath.toLocal8Bit().constData();
    Registration reg;
    if (reg.set_command_file(strPath) < 0) {
        printf("Error.  could not load %s as command file.\n",
            strPath.c_str());
    }

    std::string strFixed = reg.get_registration_parms()->get_fixed_fn(); //  return d_ptr->rparms;
    std::string strMoving = reg.get_registration_parms()->get_moving_fn();   


    QTime time;
    time.start();
    try {
        reg.do_registration();
    }
    catch (Plm_exception e) {
        printf("Your error was %s", e.what());

        m_pParent->m_mutex.lock();        
        m_iProcessingTime = time.elapsed();
        m_pParent->m_vRegiQue.at(m_iIndex).m_iStatus = ST_ERROR;
        m_pParent->m_vRegiQue.at(m_iIndex).m_fProcessingTime = m_iProcessingTime / 1000.0;
        m_pParent->m_mutex.unlock();
        exec();
        return;
    }

    m_iProcessingTime = time.elapsed();

    /*QString strDisp = "Done: " + QString::number(m_iProcessingTime/1000, 'f', 1) + " s";
    m_pParent->SetTableText(m_iIndex, DEFAULT_NUM_COLUMN_MAIN - 1, strDisp);*/

    m_pParent->m_mutex.lock(); //maybe not needed just for copying
    m_pParent->m_vRegiQue.at(m_iIndex).m_iStatus = ST_DONE;
    m_pParent->m_vRegiQue.at(m_iIndex).m_fProcessingTime = m_iProcessingTime / 1000.0;
    m_pParent->UpdateTable_Que();
    //m_pParent->m_vRegiQue.at(m_iIndex).m_fScore = 999.0;"not yet implemented"
    m_pParent->CopyCommandFileToOutput(this->m_strCommandFilePath);

    m_pParent->m_mutex.unlock(); //maybe not needed just for copying    
    exec();//event roop Thread is still alive To quit thread, call exit()
    //quit thread     
}

