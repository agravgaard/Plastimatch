/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <QString>
#include <QFileDialog>
#include <QListView>
#include <QStandardItemModel>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include "YKThreadRegi.h"

#include <QtGlobal>
#include <QDesktopServices>
#include <QUrl>
#include <QClipboard>
#include <QProcess>
#include <QInputDialog>
#include <QMessageBox>

#include "qt_util.h"
#include "register_gui.h"

register_gui::register_gui(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    m_pTableModelMain = NULL;       
    m_pTableModelQue = NULL;

    m_iCurSelRow_Main = -1;
    m_iCurSelCol_Main = -1;

    m_iCurSelRow_Que = -1;
    m_iCurSelCol_Que = -1;
    m_pArrThreadRegi = NULL;

    m_iNumOfThreadAll = DEFAULT_MAXNUM_QUE; 

    InitTableMain(DEFAULT_MAXNUM_MAIN, DEFAULT_NUM_COLUMN_MAIN);
    InitTableQue(DEFAULT_MAXNUM_QUE, DEFAULT_NUM_COLUMN_QUE);

    /*if (m_pArrThreadRegi != NULL)
      {
      DeleteRemainingThreads();
      }*/

    m_pArrThreadRegi = new YKThreadRegi*[m_iNumOfThreadAll]; //200 threads object, double pointer
    for (int i = 0; i < m_iNumOfThreadAll; i++)
    {
        m_pArrThreadRegi[i] = NULL;
    }

    m_timerRunSequential = new QTimer(this);
    m_timerRunMultiThread = new QTimer(this);

    connect(m_timerRunSequential, SIGNAL(timeout()), this, SLOT(SLT_TimerRunSEQ()));
    connect(m_timerRunMultiThread, SIGNAL(timeout()), this, SLOT(SLT_TimerRunMT()));   

    //Set m_strPathCommandTemplateDir in ~/ AppData / Roaming / plastimatch
    InitConfig();

    //Write default templates there
    WriteDefaultTemplateFiles(m_strPathCommandTemplateDir);

    // Read config file.  Save it, to create if the first invokation.
    read_application_settings ();
    write_application_settings ();

    UpdateCommandFileTemplateList(m_strPathCommandTemplateDir); //from the folder
}

//make and set m_strPathCommandTemplateDir in ~/AppData/Roaming/plastimatch
void register_gui::InitConfig()
{
    // Set up application configuration location
    QCoreApplication::setOrganizationName("Plastimatch");
    QCoreApplication::setOrganizationDomain("plastimatch.org");
    QCoreApplication::setApplicationName("register_gui");

    // Find location for command file templates.
    // QT doesn't seem to have an API for getting the
    // user's application data directory.  So we construct
    // a hypothetical ini file name, then grab the directory.
    QSettings tmpSetting(
        QSettings::IniFormat, /* Make sure we get path, not registry */
        QSettings::UserScope, /* Get user directory, not system direcory */
        "Plastimatch",        /* Orginazation name (subfolder within path) */
        "register_gui"        /* Application name (file name with subfolder) */
    );

    QString strPathTemplateBase = QFileInfo(tmpSetting.fileName()).absolutePath();//C:\Users\ykp1\AppData\Roaming/plastimatch. however, the directory was not found
    m_strPathCommandTemplateDir = strPathTemplateBase + "/" + "CommandTemplate";    
    
    QDir plm_setting_dir(m_strPathCommandTemplateDir);

    if (!plm_setting_dir.exists())
    {
        //plm_setting_dir.mkdir(m_strPathCommandTemplateDir);
        if (!plm_setting_dir.mkpath(m_strPathCommandTemplateDir)) //The function will create all parent directories necessary to create the directory.
        {
            cout << "Error! Cannot make a directory for command file templates. You may not able to use templates for command file." << endl;
            m_strPathCommandTemplateDir = "";
        }
        else
        {
            cout << "Command template path: " << m_strPathCommandTemplateDir.toLocal8Bit().constData() << endl;
        }        
    }
}

void register_gui::WriteDefaultTemplateFiles(QString& targetDirPath)
{
    QDir curDir(targetDirPath);
    if (!curDir.exists())
    {
        cout << "Error! Target dir doesn't exist: " << targetDirPath.toLocal8Bit().constData() << endl;
        return;
    }
    CreateDefaultCommandFile(PLAST_RIGID);
    CreateDefaultCommandFile(PLAST_AFFINE);
    CreateDefaultCommandFile(PLAST_BSPLINE);
    CreateDefaultCommandFile(PLAST_GRADIENT);
}



register_gui::~register_gui()
{ 
    if (m_pTableModelMain != NULL)
    {
        delete m_pTableModelMain;
        m_pTableModelMain = NULL;
    }

    if (m_pTableModelQue != NULL)
    {
        delete m_pTableModelQue;
        m_pTableModelQue = NULL;
    }

    m_vRegiQue.clear();

    DeleteRemainingThreads();
}

void register_gui::SLT_SetDefaultDir()
{    
    QString dirPath = QFileDialog::getExistingDirectory (
        this, tr("Open Work Directory"),
        m_strPathDirDefault,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);   

    if (dirPath.length() < 1)
        return;    
    
    dirPath.replace(QString("\\"), QString("/"));
    SetWorkDir(dirPath);
    write_application_settings ();
}

void register_gui::SLT_SetDefaultViewer()
{
    QString strPathExecutable = QFileDialog::getOpenFileName (
        this, "Open executable file", "",
#if WIN32
        "executable file (*.exe);;"
#endif
        "all file (*)",
        0, 0);

    if (strPathExecutable.length() < 1)
        return;

    SetReadImageApp(strPathExecutable);
    write_application_settings ();
}

void register_gui::SetWorkDir(const QString& strPath)
{
    QDir dirDefaultDir = QDir(strPath);
    if (!dirDefaultDir.exists())
        return;
    
    m_strPathDirDefault = strPath;
    ui.lineEdit_WorkingDir->setText(m_strPathDirDefault);
}

void register_gui::SetReadImageApp(const QString& strPath)
{
    QFileInfo ViewerFileInfo(strPath);
    if (!ViewerFileInfo.exists())
        return;

    m_strPathReadImageApp = strPath;
    ui.lineEdit_ImageViewerPath->setText(m_strPathReadImageApp);
}


void register_gui::SetCommandTemplateDir(const QString& strDirPath)
{
    QDir dir(strDirPath);
    
    if (!dir.exists())
        return;

    m_strPathCommandTemplateDir = strDirPath;

    //ui.lineEditDefaultViewerPath->setText(m_strPathReadImageApp);
}

void register_gui::InitTableQue(int rowCnt, int columnCnt)
{
    if (m_pTableModelQue != NULL)
    {
        delete m_pTableModelQue;
        m_pTableModelQue = NULL;
    }

    if (columnCnt < 3)
        return;

    m_pTableModelQue = new QStandardItemModel(rowCnt, columnCnt, this);

    m_pTableModelQue->setHorizontalHeaderItem(0, new QStandardItem(QString("Fixed image file")));
    m_pTableModelQue->setHorizontalHeaderItem(1, new QStandardItem(QString("Moving image file")));
    m_pTableModelQue->setHorizontalHeaderItem(2, new QStandardItem(QString("Command file name")));
    m_pTableModelQue->setHorizontalHeaderItem(3, new QStandardItem(QString("____Status____")));
    m_pTableModelQue->setHorizontalHeaderItem(4, new QStandardItem(QString("Processing time (s)")));
    m_pTableModelQue->setHorizontalHeaderItem(5, new QStandardItem(QString("____Score1____")));
    m_pTableModelQue->setHorizontalHeaderItem(6, new QStandardItem(QString("____Score2____")));

    ui.tableView_que->setModel(m_pTableModelQue);
    ui.tableView_que->resizeColumnsToContents();

    QItemSelectionModel *select = ui.tableView_que->selectionModel();
    //connect(select, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(SLT_SelectionChangedQue(QItemSelection, QItemSelection)));   
}

void register_gui::InitTableMain(int rowCnt, int columnCnt)
{
    if (m_pTableModelMain != NULL)
    {
        delete m_pTableModelMain;
        m_pTableModelMain = NULL;
    }

    if (columnCnt < 3)
        return;

    m_pTableModelMain = new QStandardItemModel(rowCnt, columnCnt, this);

    m_pTableModelMain->setHorizontalHeaderItem(0, new QStandardItem(QString("Fixed image file")));
    m_pTableModelMain->setHorizontalHeaderItem(1, new QStandardItem(QString("Moving image file")));
    m_pTableModelMain->setHorizontalHeaderItem(2, new QStandardItem(QString("Command file name")));
    //m_pTableModelMain->setHorizontalHeaderItem(3, new QStandardItem(QString("       Status      ")));

    ui.tableView_Stage->setModel(m_pTableModelMain);
    ui.tableView_Stage->resizeColumnsToContents();

    QItemSelectionModel *select = ui.tableView_Stage->selectionModel();
    //connect(select, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(SLT_SelectionChangedMain(QItemSelection, QItemSelection)));  
}

void
register_gui::SLT_LoadImages()
{
#if defined (commentout)
    // Display modal dialog
    // TODO, make modeless
    Register_gui_load_dialog *load_dlg = new Register_gui_load_dialog;
    if (load_dlg->exec () == QDialog::Rejected) {
        delete load_dlg;
        return;
    }

    Job_group_type action_pattern = load_dlg->get_action_pattern ();
    QString fixed_pattern = load_dlg->get_fixed_pattern ();
    QString moving_pattern = load_dlg->get_moving_pattern ();
    bool repeat_for_peers = load_dlg->get_repeat_for_peers ();

    printf ("Action: %d\n", action_pattern);
    printf ("Fixed:  %s\n", fixed_pattern.toUtf8().constData());
    printf ("Moving: %s\n", moving_pattern.toUtf8().constData());
    printf ("Repeat: %s\n", repeat_for_peers ? "true" : "false");

    m_actions.append (action_pattern);
    m_strlistPath_Fixed.append (fixed_pattern);
    m_strlistPath_Moving.append (moving_pattern);

    /*for (int i = 0; i < iCntPaths; i++)
    {      
        QFileInfo tmpInfo = QFileInfo(m_strlistPath_Fixed.at(i));
        m_strlistBaseName_Fixed.push_back(tmpInfo.fileName());
    }*/

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI); //When updating table, also do it for combo boxes
    
//    ui.statusBar->showMessage(load_dlg->get_fixed_pattern());
    
    // Process modal dialog
    delete load_dlg;
#endif
}

void register_gui::SLT_LoadFixedFiles()
{
    QFileDialog w;    
    //w.setFileMode(QFileDialog::Directory);//both files and directories are displayed 
    w.setFileMode(QFileDialog::AnyFile);//both files and directories are displayed 
    w.setOption(QFileDialog::DontUseNativeDialog, true);
    QListView *l = w.findChild<QListView*>("listView");
    if (l) {        
        l->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }    
    w.setDirectory(m_strPathDirDefault);
    w.exec();

    QStringList listPath = w.selectedFiles();
    int iCntPaths = listPath.size();
    
    if (iCntPaths < 1)
        return;

    //m_strlistPath_Fixed.clear();
    //m_strlistBaseName_Fixed.clear();
    //ui.comboBox_Fixed->clear();

    m_strlistPath_Fixed = listPath;

    /*for (int i = 0; i < iCntPaths; i++)
    {      
        QFileInfo tmpInfo = QFileInfo(m_strlistPath_Fixed.at(i));
        m_strlistBaseName_Fixed.push_back(tmpInfo.fileName());
    }*/

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI); //When updating table, also do it for combo boxes


 /*   QFileInfo finfo(m_strlistPath_RD_Original_Ref.at(0));
    QDir crntDir = finfo.absoluteDir();
    m_strPathInputDir = crntDir.absolutePath();*/
}

// Used to update Base and ComboList, but now no more combo list
void
register_gui::UpdateBaseAndComboFromFullPath()
{    
    m_strlistBaseName_Fixed.clear();
    m_strlistBaseName_Moving.clear();
    m_strlistBaseName_Command.clear();

    int iCntFixed = m_strlistPath_Fixed.count();
    int iCntMoving = m_strlistPath_Moving.count();
    int iCntCommand = m_strlistPath_Command.count();

    QFileInfo tmpInfo;
    QString fileName;

    for (int i = 0; i < iCntFixed; i++)
    {
        tmpInfo = QFileInfo(m_strlistPath_Fixed.at(i));
        fileName = tmpInfo.fileName();
        m_strlistBaseName_Fixed.push_back(fileName);
    }

    for (int i = 0; i < iCntMoving; i++)
    {
        tmpInfo = QFileInfo(m_strlistPath_Moving.at(i));
        fileName = tmpInfo.fileName();
        m_strlistBaseName_Moving.push_back(fileName);
    }

    for (int i = 0; i < iCntCommand; i++)
    {
        tmpInfo = QFileInfo(m_strlistPath_Command.at(i));
        fileName = tmpInfo.fileName();
        m_strlistBaseName_Command.push_back(fileName);
    }
}

void register_gui::SLT_LoadMovingFiles()
{
    //include DICOM dir as well
    //  QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", m_strPathDirDefault, "image files (*.dcm *.mha *.nrrd)");

    QFileDialog w;
    w.setFileMode(QFileDialog::AnyFile);//both files and directories are displayed 
    w.setOption(QFileDialog::DontUseNativeDialog, true);
    QListView *l = w.findChild<QListView*>("listView");
    if (l) {
        l->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    w.setDirectory(m_strPathDirDefault);
    w.exec();

    QStringList listPath = w.selectedFiles();
    int iCntPaths = listPath.size();

    if (iCntPaths < 1)
        return;

  /*  m_strlistPath_Moving.clear();
    m_strlistBaseName_Moving.clear();
    ui.comboBox_Moving->clear();
*/

    m_strlistPath_Moving = listPath;

 /*   for (int i = 0; i < iCntPaths; i++)
    {
        QFileInfo tmpInfo = QFileInfo(m_strlistPath_Moving.at(i));
        m_strlistBaseName_Moving.push_back(tmpInfo.fileName());
    }*/

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI);
}

void register_gui::SLT_LoadCommandFiles()
{    
     QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select one or more files to open",
         m_strPathDirDefault, "text files (*.txt)");

     int iCntPathList = tmpList.count();

     if (iCntPathList < 1)
         return;

     /*  m_strlistPath_Command.clear();
       m_strlistBaseName_Command.clear();
       ui.comboBox_Command->clear();*/
     m_strlistPath_Command = tmpList;

   /*  for (int i = 0; i < iCntPathList; i++)
     {
         QFileInfo tmpInfo = QFileInfo(m_strlistPath_Command.at(i));
         m_strlistBaseName_Command.push_back(tmpInfo.fileName());
     }*/
     UpdateBaseAndComboFromFullPath();
     UpdateTable_Main(DATA2GUI);    
}

void register_gui::EmptyTableModel(QStandardItemModel* pTableModel)
{
    int iRowCount = pTableModel->rowCount();
    int iColCount = pTableModel->columnCount();

    QString strDummy = "";
    for (int i = 0; i < iRowCount; i++)
    {
        for (int j = 0; j < iColCount; j++)
        {
            pTableModel->setItem(i, j, new QStandardItem(strDummy));
        }
    }
}

//When updating table, also do it for combo boxes
void register_gui::UpdateTable_Main(enUpdateDirection updateDirection)
{   
    /*  if (m_pTableModel != NULL)
        {
        delete m_pTableModel;
        m_pTableModel = NULL;
        }*/

    if (m_pTableModelMain == NULL)
    {
        cout << "error! Initialize table first " << endl;
        return;
    }

    int iCntFixed = m_strlistBaseName_Fixed.count();
    int iCntMoving = m_strlistBaseName_Moving.count();
    int iCntCommand = m_strlistBaseName_Command.count();

    int RowCnt = m_pTableModelMain->rowCount();
    int ColCnt = m_pTableModelMain->columnCount();

    //fixed image
    if (iCntFixed > RowCnt)
    {
        cout << "Data size is larger than table prepared. Enarge the table first to accomodate bigger data" << endl;
        iCntFixed = RowCnt;
    }

    //moving image
    if (iCntMoving > RowCnt)
    {
        cout << "Data size is larger than table prepared. Enarge the table first to accomodate bigger data" << endl;
        iCntMoving = RowCnt;
    }

    //command file
    if (iCntCommand > RowCnt)
    {
        cout << "Data size is larger than table prepared. Enarge the table first to accomodate bigger data" << endl;
        iCntCommand = RowCnt;
    }

    if (updateDirection == DATA2GUI)
    {
        /*   if (iCntFixed > 0)
             ui.comboBox_Fixed->clear();

             if (iCntMoving > 0)
             ui.comboBox_Moving->clear();

             if (iCntCommand > 0)
             ui.comboBox_Command->clear();   */

        //Clear the table
        EmptyTableModel(m_pTableModelMain); //set all text ""

        for (int i = 0; i < iCntFixed; i++)
        {            
            QString strFixed = m_strlistBaseName_Fixed.at(i);
            m_pTableModelMain->setItem(i, 0, new QStandardItem(strFixed));
            //  ui.comboBox_Fixed->addItem(strFixed);
        }

        for (int i = 0; i < iCntMoving; i++)
        {
            QString strMoving = m_strlistBaseName_Moving.at(i);
            m_pTableModelMain->setItem(i, 1, new QStandardItem(strMoving));
            //  ui.comboBox_Moving->addItem(strMoving);
        }    

        for (int i = 0; i < iCntCommand; i++)
        {
            QString strCommand = m_strlistBaseName_Command.at(i);
            m_pTableModelMain->setItem(i, 2, new QStandardItem(strCommand));
            //   ui.comboBox_Command->addItem(strCommand);
        }
    }
    else if (updateDirection == GUI2DATA) //mostly, renaming command files
    {
        QStandardItem* item = NULL;
        for (int i = 0; i < iCntFixed; i++)
        {
            item = m_pTableModelMain->item(i, 0);
            m_strlistBaseName_Fixed[i] = item->text();
        }

        for (int i = 0; i < iCntMoving; i++)
        {
            item = m_pTableModelMain->item(i, 1);
            m_strlistBaseName_Moving[i] = item->text();
        }

        for (int i = 0; i < iCntCommand; i++)
        {
            item = m_pTableModelMain->item(i, 2);
            m_strlistBaseName_Command[i] = item->text();
        }

        //Update strlistPath by renaming files. after it is done, update it with DATA2GUI

        UpdateStrListFromBase(m_strlistBaseName_Fixed, m_strlistPath_Fixed);
        UpdateStrListFromBase(m_strlistBaseName_Moving, m_strlistPath_Moving);
        UpdateStrListFromBase(m_strlistBaseName_Command, m_strlistPath_Command);
        
        //UpdateMainTable(DATA2GUI);
    }  
}

void register_gui::UpdateTable_Que() //only Data2Gui direction
{
    if (m_pTableModelQue == NULL)
    {
        cout << "error! Initialize table first " << endl;
        return;
    }

    int iCntData = m_vRegiQue.size();    

    EmptyTableModel(m_pTableModelQue); //set all text ""

    for (int i = 0; i < iCntData; i++)
    {
        CRegiQueString RegiQue = m_vRegiQue.at(i);

        m_pTableModelQue->setItem(i, 0, new QStandardItem(RegiQue.GetStrFixed()));
        m_pTableModelQue->setItem(i, 1, new QStandardItem(RegiQue.GetStrMoving()));
        m_pTableModelQue->setItem(i, 2, new QStandardItem(RegiQue.GetStrCommand()));
        m_pTableModelQue->setItem(i, 3, new QStandardItem(RegiQue.GetStrStatus()));
        m_pTableModelQue->setItem(i, 4, new QStandardItem(RegiQue.GetStrTime()));
        m_pTableModelQue->setItem(i, 5, new QStandardItem(RegiQue.GetStrScore()));
        //additional data
    }    
}
void register_gui::UpdateStrListFromBase(QStringList& strListBase, QStringList& strListFull)
{
    int iCntBase = strListBase.count();
    int iCntFull = strListFull.count();

    if (iCntBase*iCntFull == 0)
        return;

    if (iCntBase != iCntFull)
    {
        cout << "Error! count is zero or not matched" << endl;
        return;
    }

    QString tmpStrBase;
    QString tmpStrBaseTrimmed;
    QString tmpStrPath;        
    QString strPrevBase;
    QString strNewPath;

    for (int i = 0; i < iCntFull; i++)
    {            
        tmpStrBase = strListBase.at(i);
        tmpStrPath = strListFull.at(i);

        tmpStrBaseTrimmed = tmpStrBase.trimmed();

        QFileInfo tmpInfo(tmpStrPath);
        strPrevBase = tmpInfo.fileName();

        if (strPrevBase != tmpStrBase)
        {
            if (tmpStrBaseTrimmed.length() < 1)//empty -->delete it in the list, not file
            {
                strNewPath = tmpInfo.absolutePath() + "/" + "_invalid_.txt";
                strListFull[i] = strNewPath;
            }
            else
            {
                //rename whatever. Actual file name will be changed
                strNewPath = tmpInfo.absolutePath() + "/" + tmpStrBase;                
                //if (tmpInfo.exists()) //rename it!
                //{
                if (QFile::rename(tmpStrPath, strNewPath))
                    strListFull[i] = strNewPath;
                //}
            }                
        }
    }

    QStringList newListFull;
    //trim the list:        
    for (int i = 0; i < iCntFull; i++)
    {
        tmpStrPath = strListFull.at(i);

        QFileInfo newInfo(tmpStrPath);
        tmpStrBase = newInfo.fileName();

        if (tmpStrBase.contains("_invalid_"))
        {
            //    strListFull.removeAt(i);
        }
        else
            newListFull.push_back(tmpStrPath);
    }

    strListFull.clear();
    strListFull = newListFull;

    //UpdateBase from modified full list
    int iCntNewList = strListFull.count();
    strListBase.clear();

    for (int i = 0; i < iCntNewList; i++)
    {
        tmpStrPath = strListFull.at(i);
        QFileInfo newInfo2(tmpStrPath);
        strListBase.push_back(newInfo2.fileName());
    }
}

void register_gui::AdaptCommandFileToNewPath(QString& strPathCommand,
    QString& strPathFixed,
    QString& strPathMoving)
{    
    QStringList strListOriginal;

    QString strBaseNameFixed;
    QString strBaseNameMoving;
    QString strBaseNameCommand;

    QString strMiddleDirName;
    QString strPathOutputDir;

    strListOriginal = GetStringListFromFile(strPathCommand);
    QFileInfo fInfoCommand(strPathCommand);
    strBaseNameCommand = fInfoCommand.completeBaseName();

    QFileInfo fInfoFixed(strPathFixed);
    strBaseNameFixed = fInfoFixed.completeBaseName();

    QFileInfo fInfoMoving(strPathMoving);
    strBaseNameMoving = fInfoMoving.completeBaseName();

    strMiddleDirName = "/" + strBaseNameFixed + "/" + strBaseNameMoving + "/" + strBaseNameCommand + "/";
    strPathOutputDir = m_strPathDirDefault + strMiddleDirName;

    //cout << strPathOutputDir.toLocal8Bit().constData() << endl;
    QStringList newList;
    newList = ModifyCommandFile(strListOriginal, strPathFixed, strPathMoving, strPathOutputDir);
    //Change fixed and moving according to current path

    //QUTIL::PrintStrList(newList);
    //Save text file
    SaveCommandText(strPathCommand, newList);
}


QStringList register_gui::ModifyCommandFile(QStringList& strlistOriginal,
    QString& strPathFixed,
    QString& strPathMoving,
    QString& strPathOut)
{
    QStringList resultList;

    int cntList = strlistOriginal.count();

    QString modLine;
    QString originalLine;

    int iStage = 0;


    QString strEndFix;
    for (int i = 0; i < cntList; i++)
    {        
        originalLine = strlistOriginal.at(i);
        modLine = originalLine;

        if (originalLine.contains("STAGE"))
        {
            iStage = iStage + 1;
            resultList.push_back(modLine);
            continue;
        }
        if (iStage == 0)
            strEndFix = "_glb";
        else
            strEndFix = QString("%1%2").arg("_s").arg(iStage);       
        

        QStringList listStr = originalLine.split("=");
        QString firstStr = listStr.at(0);

        if (firstStr.contains("fixed"))
        {
            modLine = "fixed=" + strPathFixed;
        }
        else if (firstStr.contains("moving"))
        {
            modLine = "moving=" + strPathMoving;
        }
        else if (firstStr.contains("img_out"))
        {            
            modLine = "img_out=" + strPathOut + "output_img" + strEndFix + ".mha";
        }
        else if (firstStr.contains("xform_out"))
        {
            modLine = "xform_out=" + strPathOut + "output_xform" + strEndFix + ".txt";
        }     


        resultList.push_back(modLine);        
    }

    return resultList;
}

QStringList register_gui::GetStringListFromFile(const QString& strFile)
{
    QStringList resultStrList;

    QFile file(strFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return resultStrList;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        resultStrList.push_back(line);
    }
    file.close();

    return resultStrList;
}


void register_gui::SLT_ReadCommandFile_Main(QModelIndex index)
{
    //cout << "Activation occurred" << endl;
   // cout << index.column() << ", " << index.row() << endl;    
   //works only when command file is clicked

    int row = index.row();
    int col = index.column();

    QStringList listCurCommand;

    if (col != 2 || m_strlistPath_Command.count() <= row)
    {     
        SetCommandViewerText_Main(listCurCommand);
        return;
    }       

    //Get path of command file

    //if it is blank,
    QStandardItem* item = m_pTableModelMain->itemFromIndex(index);    
    QString curStr = item->text();

    if (curStr != m_strlistBaseName_Command.at(row))
    {
        cout << "File name doesn't match" << endl;
        cout << curStr.toLocal8Bit().constData() << " vs ";
        cout << m_strlistBaseName_Command.at(row).toLocal8Bit().constData() << endl;
        return;
    }

    //Read the text file and display it
    QString strPathCommand = m_strlistPath_Command.at(row);
    ReadCommandFile(strPathCommand, listCurCommand);

    //Update command file viewer
    SetCommandViewerText_Main(listCurCommand);
    QString strDisp = curStr + ".txt";
    ui.label_CommandFileCurrent->setText(strDisp);

    //ui.plainTextEdit->setEnabled(true);
    //EnableUIForCommandfile(true);
}

void register_gui::ReadCommandFile(QString& strPathCommandFile, QStringList& strListOutput)
{   
    strListOutput.clear();

    QFile file(strPathCommandFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        strListOutput.push_back(line);
    }
    file.close();
}

void register_gui::SLT_ReadCommandFile_Que(QModelIndex index)
{
    int row = index.row();
    int col = index.column();

    QStringList listCurCommand;
    int iCntQueItem = m_vRegiQue.size();

    if (iCntQueItem <= row)
    {
        SetCommandViewerText_Que(listCurCommand); //empty
        return;
    }

    ////Get path of command file
    

    //Read the text file and display it
    //QString strPathCommand = m_strlistPath_Command.at(row);
    QString strPathCommand = m_vRegiQue.at(row).m_quePathCommand;

    ReadCommandFile(strPathCommand, listCurCommand);

    /*
        QFile file(strPathCommand);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

        QTextStream in(&file);
        while (!in.atEnd())
        {
        QString line = in.readLine();
        listCurCommand.push_back(line);
        }
        file.close();*/

    //Update command file viewer
    SetCommandViewerText_Que(listCurCommand);
}

void register_gui::SLT_UpdateFileList()
{
    UpdateTable_Main(GUI2DATA);//rename and etc...
    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI);
}

//curItem prevItem
void register_gui::SLT_SelectionChangedMain(QItemSelection curItem, QItemSelection prevItem)
{
    //return;
    //only valid if single selection take place
    //UpdateMainTable(GUI2DATA);//if something renamed. sometimes it should not change the name    

    QModelIndexList listModelIndexCur = curItem.indexes();    
    int iCntCur = listModelIndexCur.count();

    if (iCntCur != 1)
        return;

    QModelIndex mIdx = listModelIndexCur.at(0);

    m_iCurSelRow_Main = mIdx.row();
    m_iCurSelCol_Main = mIdx.column();    

    if (mIdx.column() == 2)
        SLT_ReadCommandFile_Main(mIdx);   
}


void register_gui::SLT_ItemClickedMain() //single clicked
{
    QModelIndex mIdx = ui.tableView_Stage->currentIndex();

    m_iCurSelRow_Main = mIdx.row();
    m_iCurSelCol_Main = mIdx.column();

    if (mIdx.column() == 2)
        SLT_ReadCommandFile_Main(mIdx);
}

//void register_gui::SLT_TableItemSelectedMain()

void register_gui::SLT_SelectionChangedQue(QItemSelection curItem, QItemSelection prevItem)
{    
    QModelIndexList listModelIndexCur = curItem.indexes();
    int iCntCur = listModelIndexCur.count(); //7, all cells in a line were selected

    if (iCntCur < 1)
        return;

    QModelIndex mIdx = listModelIndexCur.at(0);//get first cell
    m_iCurSelRow_Que = mIdx.row();
    m_iCurSelCol_Que = mIdx.column();

    SLT_ReadCommandFile_Que(mIdx);
}


void register_gui::SLT_ItemClickedQue() //single clicked
{
    QModelIndex mIdx = ui.tableView_que->currentIndex();
    
    m_iCurSelRow_Que = mIdx.row();
    m_iCurSelCol_Que = mIdx.column();

    SLT_ReadCommandFile_Que(mIdx);
}


void register_gui::SetCommandViewerText_Main(QStringList& strList)
{
    int iCntLines = strList.count();
    ui.plainTextEdit_CommandFile->clear();
    for (int i = 0; i < iCntLines; i++)
    {
        ui.plainTextEdit_CommandFile->appendPlainText(strList.at(i));
    }
}

void register_gui::SetCommandViewerText_Que(QStringList& strList)
{
    int iCntLines = strList.count();
    ui.plainTextEdit_Que->clear();
    for (int i = 0; i < iCntLines; i++)
    {
        ui.plainTextEdit_Que->appendPlainText(strList.at(i));
    }
}


//Read current viewer and save it to stringlist
QStringList register_gui::GetCommandViewerText()
{
    QStringList resultList;

    QString strTmp = ui.plainTextEdit_CommandFile->toPlainText();
    //cout << strTmp.toLocal8Bit().constData() << endl;

    resultList = strTmp.split("\n");
    return resultList;
}

QString
register_gui::get_selected_command_template_name ()
{
    int curIdx = ui.listWidget_CommandLibrary->currentRow();
    if (curIdx < 0) {
        return QString("");
    }
    return ui.listWidget_CommandLibrary->currentItem()->text();
}

void
register_gui::set_selected_command_template_name (
    const QString& template_name)
{
    QList<QListWidgetItem*> existing
        = ui.listWidget_CommandLibrary->findItems (template_name,
            Qt::MatchExactly);
    if (!existing.empty()) {
        existing.first()->setSelected(true);
    }
}

void
register_gui::save_command_template (
    const QString& template_name,
    QStringList& template_contents  // Modified prior to saving
)
{
    if (template_contents.count() < 1) {
        cout << "Error! No text exists." << endl;
        return;
    }

    // Set all the paths to "TBD" to avoid confusion       
    QStringList::iterator it;
    for (it = template_contents.begin(); it != template_contents.end(); ++it)
    {
        QString strTempLine = (*it);

        if (strTempLine.contains("fixed=") || strTempLine.contains("fixed ="))
        {
            strTempLine = "fixed= TBD";
        }
        else if (strTempLine.contains("moving=") || strTempLine.contains("moving ="))
        {
            strTempLine = "moving= TBD";
        }
        else if (strTempLine.contains("fixed_roi=") || strTempLine.contains("fixed_roi ="))
        {
            strTempLine = "fixed_roi= TBD";
        }
        else if (strTempLine.contains("img_out=") || strTempLine.contains("img_out ="))
        {
            strTempLine = "img_out= TBD";
        }
        else if (strTempLine.contains("xform_out=") || strTempLine.contains("xform_out ="))
        {
            strTempLine = "xform_out= TBD";
        }
        (*it) = strTempLine;        
    }

    // Get App Dir Path, m_str...
    QDir curTemplateDir(m_strPathCommandTemplateDir);

    if (!curTemplateDir.exists())
    {
        cout << "Error! No template dir exists" << endl;
        return;
    }    

    QString strPathOut = m_strPathCommandTemplateDir
        + "/" + template_name + ".txt";
    SaveCommandText (strPathOut, template_contents);

    // Update template list
    UpdateCommandFileTemplateList (m_strPathCommandTemplateDir);
    set_selected_command_template_name (template_name);
}

void register_gui::SLT_SaveCommandText()
{    
    QStringList strList = GetCommandViewerText();//from main

    QString strPath;
    if (m_iCurSelRow_Main >= 0 && m_iCurSelRow_Main < m_strlistPath_Command.count() && m_iCurSelCol_Main == 2)
    {
        strPath = m_strlistPath_Command.at(m_iCurSelRow_Main);
        SaveCommandText(strPath, strList);
        SetCommandViewerText_Main(strList); //Check, is this needed??
        cout << strPath.toLocal8Bit().constData() << ": Text file was saved." << endl;
    }
}

void
register_gui::SLT_CommandFileSaveAs()
{
    QString template_name;
    bool got_name = false;
    while (true) {
        // Popup input box to ask the template name.
        // Abort if cancel.
        QInputDialog inputDlg;
        bool ok;
        template_name = QInputDialog::getText (
            this, "Save a new Template",
            "Input Command Template Name", QLineEdit::Normal,
            "template name", &ok);

        if (!ok || template_name.isEmpty()) {
            return;
        }

        // Check to see if item already exists, if so, pop up again
        QList<QListWidgetItem*> existing
            = ui.listWidget_CommandLibrary->findItems (template_name,
                Qt::MatchExactly);
        if (existing.count() > 0) {
            QMessageBox::warning (this, tr("Warning"),
                tr ("Template with that name already exists. Try another name."),
                QMessageBox::Ok);
            continue;
        }
        break;
    }

    QStringList strList = GetCommandViewerText();
    save_command_template (template_name, strList);
}

void
register_gui::SLT_CommandFileSave()
{
    QString template_name = get_selected_command_template_name ();
    if (template_name.isEmpty()) {
        return;
    }
    QStringList template_contents = GetCommandViewerText();
    save_command_template (template_name, template_contents);
}

void
register_gui::SaveCommandText (
    QString& strPathCommand,
    QStringList& strListLines)
{    
    ofstream fout;
    fout.open(strPathCommand.toLocal8Bit().constData());
    
    int cnt = strListLines.count();

    for (int i = 0; i < cnt; i++)
    {
        fout << strListLines.at(i).toLocal8Bit().constData() << endl;
    }

    fout.close();    
}

void register_gui::DeleteRemainingThreads()
{
    cout << "Running into a while loop for deleting remaining threads..." ;
    int cntRunningThread = 1;

    if (m_pArrThreadRegi != NULL)
    {
        while (cntRunningThread != 0)
        {
            cntRunningThread = 0;

            for (int i = 0; i < m_iNumOfThreadAll; i++)
            {
                if (m_pArrThreadRegi[i] != NULL)
                {
                    cntRunningThread++;

                    if (!m_pArrThreadRegi[i]->isRunning())
                    {
                        delete m_pArrThreadRegi[i];
                        m_pArrThreadRegi[i] = NULL;
                        cout << "Thread ID " << i << " has been deleted" << endl;
                    }
                }
            }
        }
    }

    delete[] m_pArrThreadRegi;
    m_pArrThreadRegi = NULL;
    cout << "Done!" << endl;
}

void register_gui::SetTableText(int row, int col, QString& inputStr)
{    
    m_pTableModelMain->setItem(row, col, new QStandardItem(inputStr));
}

void register_gui::SLT_AddSingleToQue()
{
#if defined (commentout)
    QString strFixed, strMoving, strCommand;    

    int curIndex_fixed = ui.comboBox_Fixed->currentIndex();
    int curIndex_moving = ui.comboBox_Moving->currentIndex();
    int curIndex_command = ui.comboBox_Command->currentIndex();

    if (curIndex_fixed < 0 ||
        curIndex_moving < 0 ||
        curIndex_command < 0)
        return;

    int RowCnt = m_pTableModelQue->rowCount();
   // int ColCnt = m_pTableModelQue->columnCount();
    int curDataSize = m_vRegiQue.size();

    if (curDataSize == RowCnt)
    {
        cout << "Error! Que Table is full! Current maximum size = " << RowCnt << " Inquire about it to plastimatch group." << endl;
        return;
    }

    //when only valid
    if (curIndex_fixed < m_strlistPath_Fixed.count() &&
        curIndex_moving < m_strlistPath_Moving.count() &&
        curIndex_command < m_strlistPath_Command.count())
    {
        strFixed = m_strlistPath_Fixed.at(curIndex_fixed);
        strMoving = m_strlistPath_Moving.at(curIndex_moving);
        strCommand = m_strlistPath_Command.at(curIndex_command);        

        if (m_vRegiQue.size() >= DEFAULT_MAXNUM_QUE)
        {
            cout << "Error! Maximum number of que items = " << DEFAULT_MAXNUM_QUE << endl;
            return;
        }

        AddSingleToQue(strFixed, strMoving, strCommand); //data only
    }
    else
    {
        cout << "Error. No data exists" << endl;
    }   

    UpdateTable_Que();
#endif
}

//Read strlistPaths and get the min number of lines to add
void register_gui::SLT_AddMultipleToQueByLine()
{
    int iCntFixed = m_strlistPath_Fixed.count();
    int iCntMoving = m_strlistPath_Moving.count();
    int iCntCommand = m_strlistPath_Command.count();

    int minCnt = 9999;

    if (iCntFixed < minCnt)
        minCnt = iCntFixed;
    if (iCntMoving < minCnt)
        minCnt = iCntMoving;
    if (iCntCommand < minCnt)
        minCnt = iCntCommand;

    cout << "Minimum number of count = " << minCnt << endl;

    QString strFixed, strMoving, strCommand;
    for (int i = 0; i < minCnt; i++)
    {
        strFixed = m_strlistPath_Fixed.at(i);
        strMoving = m_strlistPath_Moving.at(i);
        strCommand = m_strlistPath_Command.at(i);

        if (m_vRegiQue.size() >= DEFAULT_MAXNUM_QUE)
        {
            cout << "Error! Maximum number of que items = " << DEFAULT_MAXNUM_QUE << endl;
            return;
        }

        AddSingleToQue(strFixed, strMoving, strCommand); //data only
    }
    UpdateTable_Que();    
}

void register_gui::SLT_AddMultipleToQueByPermu()
{    
    QString strFixed, strMoving, strCommand;

    int iCntFixed = m_strlistPath_Fixed.count();
    int iCntMoving = m_strlistPath_Moving.count();
    int iCntCommand = m_strlistPath_Command.count();

    int iCopyCnt = 0;

    for (int k = 0; k < iCntFixed; k++)
    {
        for (int i = 0; i < iCntMoving; i++)
        {
            for (int j = 0; j < iCntCommand; j++)
            {
                strFixed = m_strlistPath_Fixed.at(k);
                strMoving = m_strlistPath_Moving.at(i);
                strCommand = m_strlistPath_Command.at(j);

                QString endFix = "_" + QString::number(iCopyCnt);
                QString newStrCommandPath;
                if (iCopyCnt == 0)
                    newStrCommandPath = strCommand;
                else
                {
                    newStrCommandPath = QUTIL::GetPathWithEndFix(strCommand, endFix);
                    //copy
                    QFile::copy(strCommand, newStrCommandPath);
                }

                if (m_vRegiQue.size() >= DEFAULT_MAXNUM_QUE)
                {
                    cout << "Error! Maximum number of que items = " << DEFAULT_MAXNUM_QUE << endl;
                    return;
                }

                AddSingleToQue(strFixed, strMoving, newStrCommandPath); //data only
            }
            iCopyCnt++;
        }
    }
    UpdateTable_Que();
}

Job_group_type
register_gui::get_action_pattern ()
{
    if (ui.buttonGroup_actionPattern->checkedButton() == ui.radioButton_m2f)
    {
        return JOB_GROUP_MOVING_TO_FIXED;
    }
    if (ui.buttonGroup_actionPattern->checkedButton() == ui.radioButton_a2f)
    {
        return JOB_GROUP_ALL_TO_FIXED;
    }
    if (ui.buttonGroup_actionPattern->checkedButton() == ui.radioButton_a2a)
    {
        return JOB_GROUP_ALL_TO_ALL;
    }
    return JOB_GROUP_MOVING_TO_FIXED;
}

QString
register_gui::get_fixed_pattern ()
{
    return ui.lineEdit_fixedPattern->text ();
}

QString
register_gui::get_moving_pattern ()
{
    return ui.lineEdit_movingPattern->text ();
}

bool
register_gui::get_repeat_for_peers ()
{
    return ui.checkBox_repeatForPeers->isChecked ();
}

void
register_gui::SLT_BrowseFixedPattern ()
{
    QString pattern;
    if (get_action_pattern() == JOB_GROUP_ALL_TO_ALL) {
        QFileDialog::Options options =
            QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
        pattern = QFileDialog::getExistingDirectory (
            this, tr("Choose directory"),
            ui.lineEdit_fixedPattern->text (),
            options);
    } else {
        QFileDialog::Options options =
            QFileDialog::DontResolveSymlinks;
        QString filters = tr(
            "Images (*.mha *.mhd *.nii *.nrrd);;All files (*)");
        QString selected_filter = tr("Images (*.mha *.mhd *.nii *.nrrd)");
        pattern = QFileDialog::getOpenFileName (
            this, tr("Choose file"),
            ui.lineEdit_fixedPattern->text (),
            filters, &selected_filter, 
            options);
    }
    ui.lineEdit_fixedPattern->setText (pattern);
}

void
register_gui::SLT_BrowseMovingPattern ()
{
    QString dirPath = QFileDialog::getExistingDirectory (this,
        tr("Open Work Directory"),
        ui.lineEdit_movingPattern->text (),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    ui.lineEdit_movingPattern->setText (dirPath);
}

void
register_gui::get_image_files (
    QStringList& image_list,
    const QString& pattern,
    bool repeat_for_peers)
{
    QFileInfo file_info (pattern);
    if (!file_info.exists ()) {
        return;
    }
    else if (file_info.isFile ()) {
        image_list.append (pattern);
    }
    else if (file_info.isDir ()) {
        QDir dir (pattern);
        QFileInfoList file_list = dir.entryInfoList (QDir::Files, QDir::Name);
        for (QFileInfoList::const_iterator i = file_list.begin ();
             i != file_list.end (); ++i)
        {
            printf ("FIXED: %s\n", QUTIL::c_str(i->fileName()));
        }
    }
}

void
register_gui::SLT_AddImages ()
{
    Job_group_type action_pattern = get_action_pattern ();
    QString fixed_pattern = get_fixed_pattern ();
    QString moving_pattern = get_moving_pattern ();
    bool repeat_for_peers = get_repeat_for_peers ();

    printf ("Action: %d\n", action_pattern);
    printf ("Fixed:  %s\n", fixed_pattern.toUtf8().constData());
    printf ("Moving: %s\n", moving_pattern.toUtf8().constData());
    printf ("Repeat: %s\n", repeat_for_peers ? "true" : "false");

    m_actions.append (action_pattern);
    m_strlistPath_Fixed.append (fixed_pattern);
    m_strlistPath_Moving.append (moving_pattern);

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI);
}

void
register_gui::SLT_QueueJobs ()
{
    int num_actions = std::min (
        m_actions.count(),
        std::min (
            m_strlistPath_Fixed.count(),
            m_strlistPath_Moving.count()));
    int num_commands = m_strlistPath_Command.count();
    printf ("Queuing jobs %d %d\n", num_actions, num_commands);
    

    bool repeat_for_peers = false;
    for (int i = 0; i < num_actions; i++) {
        Job_group_type action_pattern = m_actions.at(i);
        QString fixed_pattern = m_strlistPath_Fixed.at(i);
        QString moving_pattern = m_strlistPath_Moving.at(i);
        if (fixed_pattern == "" || moving_pattern == "")
        {
            printf ("Refused to queue ill-formed job\n");
            continue;
        }

        // Action pattern = f2m
        QStringList fixed_images;
        QStringList moving_images;
        if (action_pattern == JOB_GROUP_MOVING_TO_FIXED) {
            get_image_files (fixed_images, fixed_pattern, repeat_for_peers);
        }

        for (int j = 0; j < num_commands; j++) {
            QString strCommand = m_strlistPath_Command.at(j);

            printf ("Action: %d\n", action_pattern);
            printf ("Fixed:  %s\n", fixed_pattern.toUtf8().constData());
            printf ("Moving: %s\n", moving_pattern.toUtf8().constData());

            // Do something
        }
    }
    UpdateTable_Que();
}

void
register_gui::AddSingleToQue (
    QString& strPathFixed, QString& strPathMoving, QString& strPathCommand)
{
    QFileInfo finfo_fixed, finfo_moving, finfo_command;
    finfo_fixed = QFileInfo(strPathFixed);
    finfo_moving = QFileInfo(strPathMoving);
    finfo_command = QFileInfo(strPathCommand);

    if (finfo_fixed.exists() && finfo_moving.exists() && finfo_command.exists())
    {
        AdaptCommandFileToNewPath(strPathCommand, strPathFixed, strPathMoving);//rewrite the text file according to input paths
        CRegiQueString RegiQue;
        RegiQue.m_quePathFixed = strPathFixed;
        RegiQue.m_quePathMoving = strPathMoving;
        RegiQue.m_quePathCommand = strPathCommand;
        m_vRegiQue.push_back(RegiQue);
    }
    else
    {
        cout << "Error! some files don't exist! ExistFlag: fixed-moving-command= x" << finfo_fixed.exists() << ", " <<
            finfo_moving.exists() << ", " <<
            finfo_command.exists() << endl;
        return;
    }
}
//
//void register_gui::SLT_CopySelectionToAll_Command()
//{
//    int row = m_iCurSelRow_Main;
//    if (m_iCurSelCol_Main == 0) //fixed image: copy only memory, not whole image
//    {
//        QString strPathFixed = m_strlistPath_Fixed.at(row);
//
//        int curFilledCnt = m_strlistPath_Fixed.count();
//
//        for (int i = curFilledCnt; i < m_iNumOfTableRow; i++)
//        {
//            m_strlistPath_Fixed.push_back(strPathFixed);
//
//            QFileInfo tmpInfo = QFileInfo(strPathFixed);
//            m_strlistBaseName_Fixed.push_back(tmpInfo.fileName());
//        }
//    }
//    else if (m_iCurSelCol_Main == 1) //mvoing image: copy only memory, not whole image
//    {
//        QString strPathMoving = m_strlistPath_Moving.at(row);
//
//        int curFilledCnt = m_strlistPath_Moving.count();
//
//        QFileInfo tmpInfo = QFileInfo(strPathMoving);
//
//        for (int i = curFilledCnt; i < m_iNumOfTableRow; i++)
//        {
//            m_strlistPath_Moving.push_back(strPathMoving);
//            m_strlistBaseName_Moving.push_back(tmpInfo.fileName());
//        }
//    }
//    else if (m_iCurSelCol_Main == 2) //command file. copy and rename automatically
//    {
//        QString strPathCommand = m_strlistPath_Command.at(row);
//        int curFilledCnt = m_strlistPath_Command.count();
//        QFileInfo tmpInfo = QFileInfo(strPathCommand);
//
//        for (int i = curFilledCnt; i < m_iNumOfTableRow; i++)
//        {
//            QString endFix = QString::number(i + 1);
//            QString strPathCommandNew = tmpInfo.absolutePath() + "/" + tmpInfo.completeBaseName() + "_" + endFix + "." + tmpInfo.completeSuffix();
//
//            QFile::copy(strPathCommand, strPathCommandNew);
//
//            m_strlistPath_Command.push_back(strPathCommandNew);
//
//            QFileInfo tmpInfo2 = QFileInfo(strPathCommandNew);
//            m_strlistBaseName_Command.push_back(tmpInfo2.fileName());
//        }
//    }
//
//
//    UpdateMainTable(DATA2GUI);
//}

void register_gui::SLT_CopyCommandFile()
{
#if defined (commentout)
    int row = m_iCurSelRow_Main;

    if (m_iCurSelCol_Main != 2)
    {
        cout << "Error! Select a single cell containing a command file." << endl;
        return;
    }     

    if (row >= m_strlistPath_Command.count())
        return;
    
    QString strPathCommand = m_strlistPath_Command.at(row);
    int curFilledCnt = m_strlistPath_Command.count();
    QFileInfo tmpInfo = QFileInfo(strPathCommand);    

    int iNumOfFiles = ui.lineEdit_NumOfCopy->text().toInt();

    int iEndIndex = curFilledCnt + iNumOfFiles;

    for (int i = curFilledCnt; i < iEndIndex; i++)
    {
        QString endFix = QString::number(i + 1);
        QString strPathCommandNew = tmpInfo.absolutePath() + "/" + tmpInfo.completeBaseName() + "_" + endFix + "." + tmpInfo.completeSuffix();

        QFile::copy(strPathCommand, strPathCommandNew);

        m_strlistPath_Command.push_back(strPathCommandNew);

        QFileInfo tmpInfo2 = QFileInfo(strPathCommandNew);
        m_strlistBaseName_Command.push_back(tmpInfo2.fileName());
    }

    UpdateTable_Main(DATA2GUI); //should be called first!
    SLT_UpdateFileList();
#endif
}

void register_gui::SLT_ClearCommandFiles()
{
    m_strlistPath_Command.clear();
    //m_strlistBaseName_Command.clear();

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI);
}

void register_gui::SLT_SortSelectedColumn()
{
    if (m_iCurSelCol_Main == 0) //fixed
    {
        m_strlistPath_Fixed.sort();
    }
    if (m_iCurSelCol_Main == 1) 
    {
        m_strlistPath_Moving.sort();
    }
    if (m_iCurSelCol_Main == 2) 
    {
        m_strlistPath_Command.sort();
    }

    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI);
}

void register_gui::SLT_ClearQueAll()
{
    m_vRegiQue.clear();
    UpdateTable_Que();
}

void register_gui::SLT_RemoveSelectedQue()
{
    int iSelectedRowQue = m_iCurSelRow_Que;
    if (iSelectedRowQue >= m_vRegiQue.size() || iSelectedRowQue < 0)
        return;

    m_vRegiQue.erase(m_vRegiQue.begin() + iSelectedRowQue);
    UpdateTable_Que();

    m_iCurSelRow_Que = -1;
    m_iCurSelCol_Que = -1;
}


//Thread ID = index of Que table (max = 200)
void register_gui::SLT_RunSingleSelected()
{
    int iSelected = m_iCurSelRow_Que; 
    if (!RunRegistrationSingle(iSelected))
    {
        cout << "Error in RunRegistrationSingle!" << endl;
    }
    
}

bool register_gui::RunRegistrationSingle(int index)
{
    if (index >= m_vRegiQue.size() || index < 0)
        return false;

    // if the status is not waiting, do not.
    CRegiQueString RegiQue = m_vRegiQue.at(index);

    if (RegiQue.m_iStatus != ST_NOT_STARTED)
    {
        if (RegiQue.m_iStatus == ST_DONE)
        {
            cout << "Cannot perform the registration. This job is done already." << endl;
            return false;
        }

        else if (RegiQue.m_iStatus == ST_PENDING)
        {
            cout << "Cannot perform the registration. This job is currently pending." << endl;
            return false;
        }
    }
    else
    {
        QString strPathCommmand = RegiQue.m_quePathCommand;
        QString strFileCommmand = RegiQue.GetStrCommand();
        m_pArrThreadRegi[index] = new YKThreadRegi(this, strPathCommmand, index);

        cout << "Starting thread ID= " << index << ". Command file name= " << strFileCommmand.toLocal8Bit().constData() << endl;

        m_pArrThreadRegi[index]->start(QThread::NormalPriority);
        m_pArrThreadRegi[index]->exit();
    }
    return true;
}

void register_gui::SLT_RunBatchSequential()
{
    if (m_timerRunMultiThread->isActive())
    {
        cout << "Error! Finish MT timer first" << endl;
        return;
    }    
    //if there is any pending, don't run

    int iCntPending = GetCountPendingJobs();

    if (iCntPending == 0)
    {        
        m_timerRunSequential->start(500);//Look up every 0.5 s and run next registration        
        m_tTimeSeq = QTime::currentTime();        
        ui.lineEdit_TotalProcTimeSeq->setText("");
    }
        
    else
        cout << "Cannot run! please wait until there is no pending job" << endl;
    
}

void register_gui::SLT_RunBatchMultiThread()
{    
    if (m_vRegiQue.empty())
        return;

    if (m_timerRunSequential->isActive())
    {
        cout << "Error! Finish Sequential timer first" << endl;
        return;
    }

    int iCntPending = GetCountPendingJobs();
    int iCntStandBy = GetCountStandByJobs();

    if (iCntPending == 0 && iCntStandBy > 0)
    {
        m_timerRunMultiThread->start(500);//Look up every 0.5 s and run next registration
        m_tTimeMT = QTime::currentTime();
        ui.lineEdit_TotalProcTimeMT->setText("");
    }        
    else
        cout << "Cannot run due to pending jobs or all-done jobs" << endl;
}
int register_gui::GetCountPendingJobs()
{
    int iCntPending = 0;    
    int iCntQued = m_vRegiQue.size();

    for (int i = 0; i < iCntQued; i++)
    {
        if (m_vRegiQue.at(i).m_iStatus == ST_PENDING)
        {
            iCntPending++;
        }
    }
    return iCntPending;
}

int register_gui::GetCountStandByJobs()
{
    int iCntStandby = 0;
    int iCntQued = m_vRegiQue.size();

    for (int i = 0; i < iCntQued; i++)
    {
        if (m_vRegiQue.at(i).m_iStatus == ST_NOT_STARTED)
        {
            iCntStandby++;
        }
    }
    return iCntStandby;

}

//Continue until there is no "NOt_Started" items.
void register_gui::SLT_TimerRunSEQ() //monitor the value
{
    if (m_vRegiQue.empty())
    {
        m_timerRunSequential->stop();
        QString strNum = QString::number(m_tTimeSeq.elapsed() / 1000.0, 'f', 2);
        ui.lineEdit_TotalProcTimeSeq->setText(strNum);
        cout << "Timer stopped. Empty que" << endl;
        return;
    }   

    int iCntPending = GetCountPendingJobs();

    if (iCntPending > 0)
        return;

    int iCntQued = m_vRegiQue.size();

    //if no pending
    //find the target index
    int targetIdx = -1;
    for (int i = 0; i < iCntQued; i++)
    {
        if (m_vRegiQue.at(i).m_iStatus == ST_NOT_STARTED)
        {
            targetIdx = i;
            break;
        }
    }

    if (targetIdx < 0) //if there is no NOT_STARTED item
    {     
        QString strNum = QString::number(m_tTimeSeq.elapsed() / 1000.0, 'f', 2);
        ui.lineEdit_TotalProcTimeSeq->setText(strNum);
        cout << "Timer stopped. No more items to register." << endl;
        m_timerRunSequential->stop();        
    }
    else
    {
        RunRegistrationSingle(targetIdx);
    }
    return;    
}

void register_gui::SLT_TimerRunMT()
{
    if (m_vRegiQue.empty())
    {
        m_timerRunMultiThread->stop();
        QString strNum = QString::number(m_tTimeMT.elapsed() / 1000.0, 'f', 2);
        ui.lineEdit_TotalProcTimeMT->setText(strNum);
        cout << "Timer stopped. Empty que" << endl;
        return;
    }

    int iMaxNumThread = ui.lineEdit_MaxNumberThread->text().toInt();
    int iCntPending = GetCountPendingJobs();

    int iAvailableSlots = iMaxNumThread - iCntPending;

    if (iAvailableSlots < 1)
        return;    

    int iCntQued = m_vRegiQue.size();
   
    vector<int> vTargetIdx;
    vector<int> vPendingIdx;

    //int targetIdx = -1;
    
    for (int i = 0; i < iCntQued; i++)
    {
        if (m_vRegiQue.at(i).m_iStatus == ST_NOT_STARTED)
            vTargetIdx.push_back(i);                    
        else if (m_vRegiQue.at(i).m_iStatus == ST_PENDING)
            vPendingIdx.push_back(i);        
        
        if (vTargetIdx.size() >= iAvailableSlots)
            break;
    }

    if (vTargetIdx.empty() && vPendingIdx.empty()) //if there is no NOT_STARTED item
    {        
        QString strNum = QString::number(m_tTimeMT.elapsed() / 1000.0, 'f', 2);
        ui.lineEdit_TotalProcTimeMT->setText(strNum);
        cout << "Timer stopped. No more items to register." << endl;
        m_timerRunMultiThread->stop();
    }
    else
    {
        vector<int>::iterator it;
        int curIdx = 0;
        for (it = vTargetIdx.begin(); it != vTargetIdx.end(); ++it)
        {
            curIdx = (*it);
            RunRegistrationSingle(curIdx);
        }        
    }
    return;
}

void register_gui::SLT_OpenSelectedOutputDir()
{
    int iSelected = m_iCurSelRow_Que;

    if (iSelected < 0 || iSelected >= m_vRegiQue.size())
    {
        cout << "Error! Selection is not valid. Try other ones." << endl;
        return;
    }     
    QString strPathCommand = m_vRegiQue.at(iSelected).m_quePathCommand;

    QString strDirPathOutput = GetStrInfoFromCommandFile(PLM_OUTPUT_DIR_PATH, strPathCommand);

    QDir dirOutput(strDirPathOutput);
    if (!dirOutput.exists())
    {
        cout << "Error! The directory cannot found. You should run registration first" << endl;
        return;
    }

    cout << "Found output dir= " << strDirPathOutput.toLocal8Bit().constData() << endl;


    QString path = QDir::toNativeSeparators(strDirPathOutput);// ..(QApplication::applicationDirPath());
    QDesktopServices::openUrl(QUrl("file:///" + path));

// Alternative:
//#if defined   Q_OS_WIN32
//    // start explorer process here. E.g. "explorer.exe C:\windows"
//    QString strCommand = QString("explorer %1").arg(strDirPathOutput); //works in linux as well??
//    //strCommand = explorer H:/CBCT2/CT1/command1 --> 
//    strCommand.replace('/', '\\');    
//    ::system(strCommand.toLocal8Bit().constData());
//#elif defined Q_OS_LINUX
//    QString strCommand = QString("gnome-open %1").arg(strDirPathOutput);
//    ::system(strCommand.toLocal8Bit().constData());
//#elif defined Q_OS_MAC
//    // start WHATEVER filebrowser here
//    QString strCommand = QString("open %1").arg(strDirPathOutput);
//    ::system(strCommand.toLocal8Bit().constData());    
//#endif

}

QString register_gui::GetStrInfoFromCommandFile(enPlmCommandInfo plmInfo, QString& strPathCommandFile)
{
    QString resultStr;
    QStringList tmpStrList = GetStringListFromFile(strPathCommandFile);
    QString strTempLine;

    QString strLineExcerpt;

    int iCntLine = tmpStrList.count();

    for (int i = 0; i < iCntLine; i++)
    {
        strTempLine = tmpStrList.at(i);

        if (plmInfo == PLM_OUTPUT_DIR_PATH &&
            (strTempLine.contains("img_out=") || strTempLine.contains("img_out =")))
        {
            strLineExcerpt = strTempLine;
            break;
        }
        /*else if (plmInfo == enPlmCommandInfo::PLM_OUTPUT_DIR_PATH &&
            (strTempLine.contains("img_out=") || strTempLine.contains("img_out =")))
        {
            strLineImgOut = strTempLine;
            break;
        }*/
    }

    QStringList infoStrList = strLineExcerpt.split("=");

    QString infoStr;
    if (infoStrList.count() > 1)
        infoStr = infoStrList.at(1);

    infoStr = infoStr.trimmed(); //remove only first and last spaces
    if (plmInfo == PLM_OUTPUT_DIR_PATH)
    {
        QFileInfo fInfo(infoStr);
        if (fInfo.exists())
        {
            resultStr = fInfo.absolutePath();
        }
    }
    return resultStr;
}


QStringList register_gui::GetImagePathListFromCommandFile(QString& strPathCommandFile)
{    
    QStringList listImgPath;

    QStringList tmpStrList = GetStringListFromFile(strPathCommandFile);    
    
    QString strLineExcerpt;

    int iCntLine = tmpStrList.count();
    QString strTempLine;

    for (int i = 0; i < iCntLine; i++)
    {
        strTempLine = tmpStrList.at(i);

        if (strTempLine.contains("fixed=") || strTempLine.contains("fixed =") ||
            strTempLine.contains("moving=") || strTempLine.contains("moving =") ||
            strTempLine.contains("img_out=") || strTempLine.contains("img_out =") )
        {
            QString strPath;
            QStringList infoStrList = strTempLine.split("=");
            if (infoStrList.count() > 1)
                strPath = infoStrList.at(1);

            strPath = strPath.trimmed();

            if (strPath.length() > 1)
                listImgPath.push_back(strPath);
            
        }
    }   
    return listImgPath;
}

//Put final command file on 
void register_gui::CopyCommandFileToOutput(QString& strPathOriginCommandFile)
{
    QFileInfo fInfo(strPathOriginCommandFile);
    if (!fInfo.exists())
    {
        cout << "Error! Orinal file doesn't exist" << endl;
        return;
    }     

    QString strOutputDir = GetStrInfoFromCommandFile(PLM_OUTPUT_DIR_PATH, strPathOriginCommandFile);
    QString strNewPath = strOutputDir + "/" + fInfo.fileName();    
    QFile::copy(strPathOriginCommandFile, strNewPath);
}

void register_gui::ExportQueResult(QString& strPathOut)
{
    if (m_vRegiQue.empty())
        return;    

    int iCnt = m_vRegiQue.size();

    ofstream fout;
    fout.open(strPathOut.toLocal8Bit().constData());    
    if (fout.fail())
    {
        cout << "File open failed." << endl;        
        return;
    }

    fout << "Registration report-" << QUTIL::GetTimeStampDirName().toLocal8Bit().constData() << endl;

    fout << "Fixed" << "\t"
        << "Moving" << "\t"
        << "CommandFile" << "\t"
        << "Status" << "\t"
        << "Time(s)" << "\t"
        << "Score1" << "\t"
        << "Score2" << endl;

    for (int i = 0; i < iCnt; i++)
    {
        CRegiQueString curItem = m_vRegiQue.at(i);
        fout << curItem.m_quePathFixed.toLocal8Bit().constData() << "\t"
            << curItem.m_quePathMoving.toLocal8Bit().constData() << "\t"
            << curItem.m_quePathCommand.toLocal8Bit().constData() << "\t"
            << curItem.GetStrStatus().toLocal8Bit().constData() << "\t"
            << curItem.GetStrTime().toLocal8Bit().constData() << "\t"
            << curItem.GetStrScore().toLocal8Bit().constData() << "\t"
            << curItem.GetStrScore().toLocal8Bit().constData() << endl;
    }
    fout.close();


    //QString timeStamp = QUTIL::GetTimeStampDirName();
    //QString strPathDirAnalysis = m_strPathDirWorkDir + "/" + strSubAnalysis
}

void register_gui::SLTM_ExportQueResult()
{
    QString strFilePath = QFileDialog::getSaveFileName(this, "Save registration report file", m_strPathDirDefault, "report (*.txt)", 0, 0);

    if (strFilePath.length() < 1)
        return;

    QFileInfo fInfo(strFilePath);

    if (fInfo.suffix() != "txt" && fInfo.suffix() != "TXT")
    {
        strFilePath = strFilePath + ".txt";
    }

    ExportQueResult(strFilePath);
}


void register_gui::SLT_CopyTableQueToClipboard()
{
    if (m_vRegiQue.empty())
        return;


    qApp->clipboard()->clear();

    QStringList list;

    int rowCnt = m_pTableModelQue->rowCount();
    int columnCnt = m_pTableModelQue->columnCount();
   
    list << "\n";
    list << "Fixed";
    list << "Moving";
    list << "CommandFile";
    list << "Status";
    list << "Processing_Time(s)";
    list << "Score1";
    list << "Score2";
    list << "\n";

    for (int j = 0; j < rowCnt; j++)
    {
        for (int i = 0; i < columnCnt; i++)
        {
            QStandardItem* item = m_pTableModelQue->item(j, i);
            list << item->text();
        }
        list << "\n";
    }

    qApp->clipboard()->setText(list.join("\t"));
}

void register_gui::SLT_ViewSelectedImg()
{
    int iSelected = m_iCurSelRow_Que;

    if (iSelected < 0 || iSelected >= m_vRegiQue.size())
    {
        cout << "Error! Selection is not valid. Try other ones." << endl;
        return;
    }
    QString strPathCommand = m_vRegiQue.at(iSelected).m_quePathCommand;
    QStringList strlistFilePath = GetImagePathListFromCommandFile(strPathCommand);

    //Check available app first

    QFileInfo fInfoApp(m_strPathReadImageApp);

    if (!fInfoApp.exists())
    {
        QUTIL::ShowErrorMessage("Error! Viewer application is not found!");
        return;
    }

    int iCntPath = strlistFilePath.count();

    QString curPath;
    QStringList validFiles;
    for (int i = 0; i < iCntPath; i++)
    {
        curPath = strlistFilePath.at(i);
        QFileInfo fInfoimg(curPath);
        if (fInfoimg.exists())
        {
            validFiles.push_back(curPath);
        }
    }

    //Shell command

    QString strShellCommand = m_strPathReadImageApp;
    strShellCommand = "\"" + strShellCommand + "\""; //IMPORTANT! due to the "Program Files" space issue

    int iCntValidImg = validFiles.count();

    for (int i = 0; i < iCntValidImg; i++)
    {
        strShellCommand = strShellCommand + " " + validFiles.at(i);
    }

//    strShellCommand = "\"" + strShellCommand + "\"";
    if (!QProcess::startDetached(strShellCommand))
        cout << "Failed to run viewer app. Command= " << strShellCommand.toLocal8Bit().constData() << endl;

}

void register_gui::write_application_settings()
{
    QSettings settings;

    //QString test = QFileInfo(settings.fileName()).absolutePath(); //H:HKEY_CURRENTUSER_SOFTWARE/Plastimatch --> no such folder

    settings.setValue ("DEFAULT_WORK_DIR", m_strPathDirDefault);
    settings.setValue ("DEFAULT_VIEWER_PATH", m_strPathReadImageApp);
    //settings.setValue("DEFAULT_TEMPLATE_DIR_PATH", m_strPathCommandTemplateDir);
}

bool register_gui::read_application_settings()
{
    QSettings settings;
    QVariant val = settings.value ("DEFAULT_WORK_DIR");
    if (!val.isNull()) {
        SetWorkDir(val.toString()); 
    }
    else
    {
        // Set workdir to folder of current directory
        QString strPathCurrent = QDir::current().absolutePath();
        SetWorkDir(strPathCurrent);
    }

    val = settings.value ("DEFAULT_VIEWER_PATH");
    if (!val.isNull()) {
        SetReadImageApp(val.toString());
    }

    return true;
}

void
register_gui::CreateDefaultCommandFile(enRegisterOption option)
{
    QString strPathDefaultCommand;

    if (option == PLAST_RIGID) {
        strPathDefaultCommand = m_strPathCommandTemplateDir + "/" + "Default_Rigid.txt";
    }
    else if (option == PLAST_BSPLINE) {
        strPathDefaultCommand = m_strPathCommandTemplateDir + "/" + "Default_B-spline.txt";
    }
    else if (option == PLAST_AFFINE) {
        strPathDefaultCommand = m_strPathCommandTemplateDir + "/" + "Default_Affine.txt";
    }
    else if (option == PLAST_GRADIENT) {
        strPathDefaultCommand = m_strPathCommandTemplateDir + "/" + "Default_Gradient.txt";
    }
    else {
        return;
    }

    QUTIL::GenDefaultCommandFile(strPathDefaultCommand, option);
}

void register_gui::UpdateCommandFileTemplateList (QString& strPathTemplateDir)
{
    //Search in default template directory    
    if (strPathTemplateDir.length() < 1)
        return;

    //Look into the specified directory and load all the text files
    QDir dirCmdTemplate(strPathTemplateDir);

    if (!dirCmdTemplate.exists())
        return;

    ui.listWidget_CommandLibrary->clear();

    QFileInfoList listFile = dirCmdTemplate.entryInfoList(QDir::Files, QDir::Name); //search for DICOM RS file
    if (listFile.size() <= 0)
    {
        cout << "No template file was found." << endl;
        return;
    }
    else
    {
        for (int i = 0; i < listFile.size(); i++)
        {
            if (listFile.at(i).suffix().contains("txt", Qt::CaseInsensitive))
            {
                QString strBaseName = listFile.at(i).baseName();
                ui.listWidget_CommandLibrary->addItem(strBaseName);
            }
        }
    }
}

// This gets called when an item in the command file library listbox
// gets selected
void register_gui::SLT_CommandTemplateSelected()
{
    int curIdx = ui.listWidget_CommandLibrary->currentRow();
    if (curIdx < 0) {
        return;
    }
    
    QString strBase = ui.listWidget_CommandLibrary->currentItem()->text();
    QString curTemplateFilePath = m_strPathCommandTemplateDir + "/" + strBase + ".txt";

    QStringList listCurCommand;
    ReadCommandFile(curTemplateFilePath, listCurCommand);
    SetCommandViewerText_Main(listCurCommand);
    
    QString strDisp = strBase;
    ui.label_CommandFileCurrent->setText(strDisp);
}

void register_gui::EnableUIForCommandfile(bool bEnable)
{    
    ui.groupBox_CommandFile->setEnabled(bEnable);
    //could be individualized for each components such as..
    //ui.plainTextEdit->setEnabled(false);
}

void register_gui::SLT_CopyCommandTemplateToDataPool()
{
    int curIdx = ui.listWidget_CommandLibrary->currentRow();

    if (curIdx < 0)
        return;

    QString curFileBase = ui.listWidget_CommandLibrary->currentItem()->text();
    QString strPathSrc = m_strPathCommandTemplateDir + "/" + curFileBase + ".txt";
    QFileInfo fInfo(strPathSrc);

    if (!fInfo.exists())
        return;

    //Remove "Default"

    QString strNewBase;
    if (curFileBase.contains("Default_", Qt::CaseInsensitive))
        strNewBase = curFileBase.replace("Default_", "Custom_", Qt::CaseInsensitive);
    else
        strNewBase = "Custom_" + curFileBase;

    //working directory
    QString strPathTarget = m_strPathDirDefault + "/" + strNewBase + ".txt";

    QString strPathTargetMod; //final file path to avoid overwritten

    if (!WriteCommandNoOverwriting(strPathSrc, strPathTarget, strPathTargetMod)) //write it with end fix if it is to be overwritten
    {
        cout << "Error in writing." << endl;
        return;
    }
    //    QFile::copy(strPathCommand, strPathCommandNew);

    m_strlistPath_Command.push_back(strPathTargetMod);
    QFileInfo tmpInfo2 = QFileInfo(strPathTargetMod);
    m_strlistBaseName_Command.push_back(tmpInfo2.fileName());
    UpdateTable_Main(DATA2GUI);
    SLT_UpdateFileList();  
}


bool register_gui::WriteCommandNoOverwriting(QString& strPathSrc, QString& strPathTarget, QString& strPathTargetMod)
{
    //strPathSrc exists --> chekced already
   
    //Check target path    
    
    QFileInfo fInfoTargOriginal(strPathTarget);
    QString strBaseName = fInfoTargOriginal.baseName();

    QString curPathOutput = strPathTarget;
    int iEndFix = 0;
    QString strEndFix = "";
    while (true)
    {
        QFileInfo fInfoTarg(curPathOutput);
        if (!fInfoTarg.exists()) //good
        {
            strPathTargetMod = curPathOutput;
            break;
        }
        else
        {
            ++iEndFix;
            strEndFix = "_" + QString::number(iEndFix);
            curPathOutput = fInfoTargOriginal.absolutePath() + "/" + strBaseName + strEndFix + "." + fInfoTargOriginal.suffix();
        }        
    }

    QFile::copy(strPathSrc, strPathTargetMod);

    return true;
}

void register_gui::SLT_BrowseWorkingDir()
{
    QString strPathDir = m_strPathDirDefault;

    QDir dir(strPathDir);
    if (!dir.exists())
    {
        cout << "Error! No dir exists" << endl;
        return;
    }
    QString path = QDir::toNativeSeparators(strPathDir);// ..(QApplication::applicationDirPath());
    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void register_gui::SLT_BrowseTemplateDir()
{
    QString strPathDir = m_strPathCommandTemplateDir;

    QDir dir(strPathDir);
    if (!dir.exists())
    {
        cout << "Error! No dir exists" << endl;
        return;
    }
    QString path = QDir::toNativeSeparators(strPathDir);// ..(QApplication::applicationDirPath());
    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void register_gui::SLT_DeleteSingleTemplate()
{
    int curIdx = ui.listWidget_CommandLibrary->currentRow();

    if (curIdx < 0)
        return;

    QString curFileBase = ui.listWidget_CommandLibrary->currentItem()->text();
    QString strPathSrc = m_strPathCommandTemplateDir + "/" + curFileBase + ".txt";
    QFileInfo fInfo(strPathSrc);

    if (!fInfo.exists())
        return;    

    //Get confirmation

    QMessageBox msgBox;
    QString strMsg = "Command template: " + curFileBase + " will be permanently deleted from the folder. Are you sure?";
    msgBox.setText(strMsg);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int res = msgBox.exec();

    if (res == QMessageBox::Ok)
    {
        //delete the file
        bool bSuccess = QFile::remove(strPathSrc);
        if (bSuccess)
            cout << "Template file: " << strPathSrc.toLocal8Bit().constData() << " was removed. Default templates will be regenerated when restarting the application." << endl;
        else
        {
            cout << "Error! Template file: " << strPathSrc.toLocal8Bit().constData() << " couldn't be removed." << endl;
            return;
        }         
    }
    UpdateCommandFileTemplateList(m_strPathCommandTemplateDir);
}

void register_gui::SLTM_ImportDataPool()
{    
    QString filePath = QFileDialog::getOpenFileName(this, "Open Data pool log file", m_strPathDirDefault, "Data pool log file (*.dpl)", 0, 0);

    /*if (filePath.length() < 1)
        return;*/

    QFileInfo fInfo(filePath);
    if (!fInfo.exists())
        return;  

    ImportDataPool(filePath);
}

void register_gui::ImportDataPool(QString& strPathImportTxt)
{
    QStringList strList = GetStringListFromFile(strPathImportTxt);

    if (strList.count() < 1)
        return;

    m_strlistPath_Fixed.clear();
    m_strlistPath_Moving.clear();
    m_strlistPath_Command.clear();

    m_strlistBaseName_Fixed.clear();
    m_strlistBaseName_Moving.clear();
    m_strlistBaseName_Command.clear();    

    QStringList::const_iterator it = strList.constBegin();
    QString curStr;
    QString curStrSub;
    while (it != strList.constEnd())
    {
        curStr = (*it);
        if (curStr.contains("FIXED_FILE_BEGIN", Qt::CaseSensitive))
        {    
            ++it;
            while (it != strList.end())
            {
                curStrSub = (*it);

                if (curStrSub.contains("FIXED_FILE_END", Qt::CaseSensitive))
                    break;
                else
                {
                    QFileInfo finfo(curStrSub);
                    if (finfo.exists())
                        m_strlistPath_Fixed.push_back(curStrSub);                        
                }
                ++it;
            }
        }
        else if (curStr.contains("MOVING_FILE_BEGIN", Qt::CaseSensitive))
        {
            ++it;
            while (it != strList.end())
            {
                curStrSub = (*it);

                if (curStrSub.contains("MOVING_FILE_END", Qt::CaseSensitive))
                    break;
                else
                {
                    QFileInfo finfo(curStrSub);
                    if (finfo.exists())
                        m_strlistPath_Moving.push_back(curStrSub);
                }
                ++it;
            }
        }
        else if (curStr.contains("COMMAND_FILE_BEGIN", Qt::CaseSensitive))
        {    
            ++it;
            while (it != strList.end())
            {                
                curStrSub = (*it);

                if (curStrSub.contains("COMMAND_FILE_END", Qt::CaseSensitive))
                    break;
                else
                {
                    QFileInfo finfo(curStrSub);
                    if (finfo.exists())
                        m_strlistPath_Command.push_back(curStrSub);
                }
                ++it;
            }
        }
        ++it;
    }//end of while main


    UpdateBaseAndComboFromFullPath();
    UpdateTable_Main(DATA2GUI); //When updating table, also do it for combo boxes
}

void register_gui::SLTM_ExportDataPool()
{
    QString strFilePath = QFileDialog::getSaveFileName(this, "Save Data pool log file", m_strPathDirDefault, "Datafile (*.dpl)", 0, 0);  

    if (strFilePath.length() < 1)
        return;        

    QFileInfo fInfo(strFilePath);

    if (fInfo.suffix() != "dpl" && fInfo.suffix() != "DPL")
    {
        strFilePath = strFilePath + ".dpl";
    }

    ExportDataPool(strFilePath);
}

void register_gui::ExportDataPool(QString& strPathExportTxt)
{
    ofstream fout;
    fout.open(strPathExportTxt.toLocal8Bit().constData());
    if (fout.fail())
    {
        cout << "Writing to file failed" << endl;
        return;
    }
    /*QStringList m_strlistPath_Fixed;
    QStringList m_strlistPath_Moving;
    QStringList m_strlistPath_Command;*/

    fout << "#Data pool log file (*.dpl) for plastimatch register_gui" << endl;
    fout << "%FIXED_FILE_BEGIN%" << endl;

    QStringList::const_iterator it;
    for (it = m_strlistPath_Fixed.begin(); it != m_strlistPath_Fixed.end(); ++it)
    {
        fout << (*it).toLocal8Bit().constData() << endl;
    }
    fout << "%FIXED_FILE_END%" << endl;

    fout << "%MOVING_FILE_BEGIN%" << endl;
    for (it = m_strlistPath_Moving.begin(); it != m_strlistPath_Moving.end(); ++it)
    {
        fout << (*it).toLocal8Bit().constData() << endl;
    }
    fout << "%MOVING_FILE_END%" << endl;
    fout << "%COMMAND_FILE_BEGIN%" << endl;
    for (it = m_strlistPath_Command.begin(); it != m_strlistPath_Command.end(); ++it)
    {
        fout << (*it).toLocal8Bit().constData() << endl;
    }
    fout << "%COMMAND_FILE_END%" << endl;
    fout.close();
}


