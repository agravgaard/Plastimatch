#ifndef REGISTER_GUI_H
#define REGISTER_GUI_H

#include <QtGui/QMainWindow>
#include "ui_register_gui.h"
#include <QStringList>
#include <vector>
#include "yk_config.h"
#include "YKThreadRegi.h"
#include <QMutex>

#include <QTime>

using namespace std;

class QStandardItemModel;
class YKThreadRegi;
class QTimer;

class register_gui : public QMainWindow
{
    Q_OBJECT

public:
    register_gui(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~register_gui();        

    void SetCommandViewerText_Main(QStringList& strList);

    void SetCommandViewerText_Que(QStringList& strList);

    QStringList GetCommandViewerText();    
    QStringList GetStringListFromFile(const QString& strFile);

    QStringList ModifyCommandFile(QStringList& strlistOriginal, QString& strPathFixed, QString& strPathMoving, QString& strPathOut);

    void SaveCommandText(QString& strPathCommand, QStringList& strListLines);

    void UpdateStrListFromBase(QStringList& strListBase, QStringList& strListFull);

    void DeleteRemainingThreads();

    void EmptyTableModel(QStandardItemModel* pTableModel);

    void UpdateBaseAndComboFromFullPath();//Base and ComboList

    void AdaptCommandFileToNewPath(QString& strPathCommand,
        QString& strPathFixed,
        QString& strPathMoving);

    void AddSingleToQue(QString& strPathFixed, QString& strPathMoving, QString& strPathCommand);

    bool RunRegistrationSingle(int index);    
    int GetCountPendingJobs();
    int GetCountStandByJobs();

    QString GetStrInfoFromCommandFile(enPlmCommandInfo plmInfo, QString& strPathCommandFile);
    QStringList GetImagePathListFromCommandFile(QString& strPathCommandFile);


    void CopyCommandFileToOutput(QString& strPathOriginCommandFile);

    void ExportQueResult(QString& strPathOut);

    public slots:                
        void SLT_SetDefaultDir();        
        void SLT_LoadFixedFiles();
        void SLT_LoadMovingFiles();
        void SLT_LoadCommandFiles();       
        void SLT_ReadCommandFile_Main(QModelIndex index); //(QModelIndex& index) didn't work
        void SLT_ReadCommandFile_Que(QModelIndex index); //(QModelIndex& index) didn't work        
        void SLT_SelectionChangedMain(QItemSelection curItem, QItemSelection prevItem);
        void SLT_SelectionChangedQue(QItemSelection curItem, QItemSelection prevItem);
        void SLT_SaveCommandText();        

        void SLT_AddSingleToQue();
        void SLT_AddMultipleToQueByLine();
        void SLT_AddMultipleToQueByPermu();
        //void SLT_CopySelectionToAll_Command();
        void SLT_CopyCommandFile();
        void SLT_ClearCommandFiles();

        void SLT_UpdateFileList();//renaming, etc
        void SLT_SortSelectedColumn(); //main only

        void SLT_ClearQueAll();
        void SLT_RemoveSelectedQue();

        void SLT_RunSingleSelected();
        void SLT_RunBatchSequential();
        void SLT_RunBatchMultiThread();

        void SLT_TimerRunSEQ();
        void SLT_TimerRunMT();
        
        void SLTM_ExportQueResult();
        void SLT_CopyTableQueToClipboard();
        void SLT_SetDefaultViewer();

        void SLT_OpenSelectedOutputDir();
        void SLT_ViewSelectedImg();
        void SLT_CreateSampleRigid();
        void SLT_CreateSampleDeform();

public:    
    QStringList m_strlistPath_Fixed;
    QStringList m_strlistPath_Moving;
    QStringList m_strlistPath_Command;

    QStringList m_strlistBaseName_Fixed;
    QStringList m_strlistBaseName_Moving;
    QStringList m_strlistBaseName_Command;

    QStringList m_strlistPathOutputImg;
    QStringList m_strlistPathOutputXf;

    QString m_strPathDirDefault;
    QString m_strPathReadImageApp;
    QString m_strPathCommandTemplateDir;

    void SetWorkDir(const QString& strPath);
    void SetReadImageApp(const QString& strPath);
    void InitTableMain(int rowCnt, int columnCnt);
    void InitTableQue(int rowCnt, int columnCnt);
    void UpdateTable_Main(enUpdateDirection updateDirection);
    void UpdateTable_Que(); //only Data2Gui direction

    void SetTableText(int row, int col, QString& inputStr);

    void WriteDefaultConfig(); //m_strFileDefaultConfig, m_strPathDirDefault, m_strPathReadImageApp
    bool ReadDefaultConfig();

    void CreateSampleCommand(enRegisterOption option);

    void SetTemplateNameFromSample (const char *name);

    int m_iCurSelRow_Main;
    int m_iCurSelCol_Main;

    int m_iCurSelRow_Que;
    int m_iCurSelCol_Que;


    QStandardItemModel *m_pTableModelMain;
    QStandardItemModel *m_pTableModelQue;
    YKThreadRegi** m_pArrThreadRegi; //Thread ID = index of Que table (max = 200)

    int m_iNumOfThreadAll;
    QMutex m_mutex;
    
    vector<CRegiQueString> m_vRegiQue;   

    QTimer* m_timerRunSequential;    
    QTimer* m_timerRunMultiThread;

    QTime m_tTimeSeq;
    QTime m_tTimeMT;
    

private:
    Ui::register_guiClass ui;
};

#endif
