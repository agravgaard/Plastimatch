#ifndef YKTHREADREGI_H
#define YKTHREADREGI_H

#include <QThread>
#include <QString>

using namespace std;
class register_gui;

class YKThreadRegi : public QThread
{ 
public:
	//DPGMTracking* m_pParent;

public:
    //YKThreadRegi(register_gui* pParent, QString& strPathCommand);
    YKThreadRegi(register_gui* pParent, QString& strPathCommand, int iRowIndex);
    ~YKThreadRegi();    
    
    void run();

    int m_iProcessingTime; //ms
    QString m_strCommandFilePath;
    register_gui* m_pParent;
    int m_iIndex;
};

#endif // YKTHREADREGI_H
