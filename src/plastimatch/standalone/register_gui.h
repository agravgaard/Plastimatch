/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef REGISTER_GUI_H
#define REGISTER_GUI_H

#include <vector>
#include <QMutex>
#include <QTime>
#include <QStringList>
#include <QtGui/QMainWindow>
#include "ui_register_gui.h"
#include "yk_config.h"
#include "YKThreadRegi.h"

class QStandardItemModel;
class YKThreadRegi;
class QTimer;

class Datapool_item
{
public:
    QString m_group;
    QString m_path;
    QString m_role;
};

enum Job_group_type
{
    JOB_GROUP_MOVING_TO_FIXED,
    JOB_GROUP_ALL_TO_FIXED,
    JOB_GROUP_ALL_TO_ALL
};

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
    void InitConfig();
    void WriteDefaultTemplateFiles(QString& targetDirPath);

    void ReadCommandFile(QString& strPathCommandFile, QStringList& strListOutput);    
    bool WriteCommandNoOverwriting(QString& strPathSrc, QString& strPathTarget, QString& strPathTargetMod);

    void EnableUIForCommandfile(bool bEnable);


    void ImportDataPool(QString& strPathImportTxt);
    void ExportDataPool(QString& strPathImportTxt);    

public slots:                
    void SLT_SetDefaultDir();        
    void SLT_LoadImages();
    void SLT_LoadFixedFiles();
    void SLT_LoadMovingFiles();
    void SLT_LoadCommandFiles();       
    
    void SLT_ReadCommandFile_Main(QModelIndex index); //(QModelIndex& index) didn't work
    void SLT_ReadCommandFile_Que(QModelIndex index); //(QModelIndex& index) didn't work        
    void SLT_SelectionChangedMain(QItemSelection curItem, QItemSelection prevItem); //prevItem is not being used now
    void SLT_ItemClickedMain(); //single clicked
    
    void SLT_SelectionChangedQue(QItemSelection curItem, QItemSelection prevItem);
    void SLT_ItemClickedQue(); //single clicked
    
    void SLT_SaveCommandText();       
    void SLT_CommandFileSaveAs();
    void SLT_CommandFileSave();

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
    //void SLT_CreateSampleRigid();
    //void SLT_CreateSampleDeform();
    void SLT_CommandTemplateSelected();
    void SLT_CopyCommandTemplateToDataPool();    
    void SLT_BrowseWorkingDir();
    void SLT_BrowseTemplateDir();
    void SLT_DeleteSingleTemplate();
    
    void SLTM_ImportDataPool();
    void SLTM_ExportDataPool();

    void SLT_BrowseFixedPattern ();
    void SLT_BrowseMovingPattern ();
    void SLT_AddImages ();
    void SLT_QueueJobs ();

public:
    void UpdateTable_Main(enUpdateDirection updateDirection);
    void UpdateTable_Que(); //only Data2Gui direction

protected:
    // Handle registration pattern, images
    Job_group_type get_action_pattern ();
    QString get_fixed_pattern ();
    QString get_moving_pattern ();
    bool get_repeat_for_peers ();
    void get_image_files (
        QStringList& image_list,
        const QString& pattern,
        bool repeat_for_peers);

    // Handle command library
    QString get_selected_command_template_name ();
    void set_selected_command_template_name (
        const QString& template_name);
    void save_command_template (
        const QString& template_name,
        QStringList& template_contents  // Modified prior to saving
    );
    
    void SetWorkDir(const QString& strPath);
    void SetReadImageApp(const QString& strPath);
    void SetCommandTemplateDir(const QString& strDirPath);

    void InitTableMain(int rowCnt, int columnCnt);
    void InitTableQue(int rowCnt, int columnCnt);

    void SetTableText(int row, int col, QString& inputStr);

    //m_strFileDefaultConfig, m_strPathDirDefault, m_strPathReadImageApp
    void write_application_settings();
    bool read_application_settings();

    void CreateDefaultCommandFile(enRegisterOption option);

    void UpdateCommandFileTemplateList(QString& strPathTemplateDir);

public:
    QList<Job_group_type> m_actions;
    QStringList m_strlistPath_Fixed;
    QStringList m_strlistPath_Moving;
    QStringList m_strlistPath_Command;
    
    QStringList m_strlistBaseName_Fixed;
    QStringList m_strlistBaseName_Moving;
    QStringList m_strlistBaseName_Command;
    
    QStringList m_strlistPathOutputImg;
    QStringList m_strlistPathOutputXf;

    std::list<Datapool_item> m_datapoolItems;

    int m_iCurSelRow_Main;
    int m_iCurSelCol_Main;

    int m_iCurSelRow_Que;
    int m_iCurSelCol_Que;


    QStandardItemModel *m_pTableModelMain;
    QStandardItemModel *m_pTableModelQue;

    //QStandardItemModel *m_pTableModelQue;


    YKThreadRegi** m_pArrThreadRegi; //Thread ID = index of Que table (max = 200)

    int m_iNumOfThreadAll;
    QMutex m_mutex;
    
    vector<CRegiQueString> m_vRegiQue;   

    QTimer* m_timerRunSequential;    
    QTimer* m_timerRunMultiThread;

    QTime m_tTimeSeq;
    QTime m_tTimeMT;
    
    // Application settings
    QString m_strPathDirDefault;
    QString m_strPathReadImageApp;
    QString m_strPathCommandTemplateDir;

private:
    Ui::register_guiClass ui;
};

#endif
