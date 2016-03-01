//
#include <QApplication>
#include <QMessageBox>
#include <QRect>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include "oraREG23ControlWindow.h" // derives from Qt

#include <itksys/SystemTools.hxx>

#include <vtkObject.h>

#include "reg23info.h"
#include "oraREG23Model.h"
// ORAIFModel
#include <oraTaskManager.h>
// ORAIFTools
#include <oraIniAccess.h>
#include <oraStringTools.h>

#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
#include <X11/Xlib.h>
#endif

#include <stdlib.h>
#include <vector>
#include <sstream>

/** Utility-method for printing information on program usage. **/
void PrintUsage(const char *binname, std::ostream &os)
{
  std::string progname = "<binary-name>";

  if (binname)
    progname = std::string(binname);

  os << "\n";
  os
  << "   *** REG-2-3   U S A G E   I N F O R M A T I O N ***\n";
  os
  << "         " << ora::REG23GlobalInformation::GetCombinedApplicationName() << "\n";
  os << "\n";
  os << progname << " [options] [<volume-image> [fixed-image1 [fixed-image2 [...]]]]\n";
  os << "\n";
  os << "  -h or --help ... print this short help\n";
  os << "  -c or --config ... (REQUIRED!) specification of configuration file: <config-file>\n";
  os << "  -ng or --no-gui ... flag indicating that the main GUI should not be displayed (application terminates immediately after registration)\n";
  os << "  -cwg or --control-window-geometry ... position and size of the control window (main GUI) - otherwise it will have a standard position: <pos-x> <pos-y> <width> <height>\n";
  os << "  -rvg{x} or --render-view-geometry{x} ... position and size of x-th render view (x is 1-based according to fixed image) - otherwise the render view won't be shown: <pos-x> <pos-y> <width> <height>\n";
  os << "  -ncl or --no-config-load ... flag indicating that there is no 'load config' button on the main GUI\n";
  os << "  -wg or --window-geometry ... configuration file with position and size of the GUI\n";
  os << "  -wgwb or --window-geometry-writeback ... write window geometry on program close if -wg is set.\n";
  os << "  -sot or --stay-on-top ... windows will stay on top of other applications\n";
  os << "  -gwd or --global-warning-display ... display global ITK/VTK warnings in output window (by default OFF)\n";
  os << "  -fl or --force-language ... specify a language (e.g. en or de) that will be used for application presentation instead of the language that was automatically detected (default behavior)\n";
  os << "  -wlss or --window-level-storage-strategy ... 0..do not store window/level in fixed image sub-folders; 1..create files (reg23-wl.inf) in fixed image sub-folders that store the window/level settings on application end; default: 0 (do not create these files)\n";
  os << "  -wlrs or --window-level-recovery-strategy ... 0..do not load window/level settings from fixed image sub-folders; 1..try to load the last window/level settings from reg23-wl.inf files in fixed image sub-folders (if not found, window/level is set as defined in config file); default: 0 (do not recover window/level settings)\n";
  os << "  -s or --science ... if specified, the science mode is switch to ON (only effective if the loaded config file does not involve the ScientificMode-key in Science-section)\n";
  os << "  -imm or --intelligent-mask-mode ... 1..ON (default); 0..OFF; if this mode is ON, image masks that have once been generated are stored in the respective fixed image folder; at the next startup, this mask is loaded instead of being expensively generated again, but only if certain criteria are met (structures, rules ... have not been changed!)\n";
  os << "\n";
  os << "  NOTE: optional arguments are case-sensitive!\n";
  os << "\n";
  os << "\n";
  os << ora::REG23GlobalInformation::GetCombinedApplicationName() << "\n";
  os << ora::REG23GlobalInformation::GetCombinedVersionString() << "\n\n";
  os << ora::REG23GlobalInformation::GetCombinedCompanyName() << "\n";
  os << ora::REG23GlobalInformation::Copyright << "\n\n";
  std::vector<ora::REG23GlobalInformation::AuthorInformation> authors =
      ora::REG23GlobalInformation::GetAuthorsInformation();
  os << "Authors:\n";
  for (std::size_t i = 0; i < authors.size(); i++)
  {
    os << "\n * " << authors[i].Name << "\n";
    os << "   (" << authors[i].Contribution << ")\n";
    os << "   contact: " << authors[i].Mail << "\n";
  }
  os << "\n";
}


/** Loads a predefined layout for the GUI windows (position and size) based on
 * the number of fixed images.
 * @param configFile The reg23 configuration file (INI-like) which contains
 *   information on the registration setup, and - optionally - on the images to
 *   be registered.
 * @param configFileGeometry The reg23 layout configuration file (INI-like)
 *   which contains information on the window layouts.
 * @param lastCommandLineOptionIndex Index of the last option entry in the
 *    command line arguments (in order to be able to parse the file names after
 *    the options).
 * @param commandLineArguments Optional application command line arguments
 *    (to support "FROM-COMMAND-LINE" entries in image file specifications).
 * @param [out] cwgConfig Window geometry of control window.
 * @param [out] rvgConfig List of render view geometries.
 * @param [out] visibilities List of render view visibilities.
 *    FALSE means not visible and TRUE visible (default). If not specified
 *    in configuration file TRUE is returned.
 * @param errorSection return the section where an error occured.
 * @param errorKey return the key where an error occured.
 * @param errorMessage return the error message.
 * @return TRUE if the configuration is sufficient.
 */
bool LoadWindowGeometryConfiguration(std::string configFile,
    std::string configFileGeometry, int lastCommandLineOptionIndex,
    std::vector<std::string> commandLineArguments, QRect &cwgConfig,
    std::vector<QRect> &rvgConfig, std::vector<bool> &visibilities, std::string &errorSection,
    std::string &errorKey, std::string &errorMessage)
{
  bool m_ValidConfiguration = false;
  errorSection = "";
  errorKey = "";
  errorMessage = "";

  std::vector<std::string> fixedImageFileNames;
  std::vector<std::string> v;

  if (!itksys::SystemTools::FileExists(configFile.c_str()))
  {
    errorMessage = "Config file (" + configFile + ") does not exist.";
    return false;
  }
  ora::IniAccess config(configFile);

  if (!itksys::SystemTools::FileExists(configFileGeometry.c_str()))
  {
    errorMessage = "Geometry config file (" + configFileGeometry
        + ") does not exist.";
    return false;
  }
  ora::IniAccess configGeometry(configFileGeometry);

  int i = 1;
  std::string s = "";
  std::vector<std::string> sa;

  // Get number of fixed images
  int last = lastCommandLineOptionIndex;
  errorSection = "Images";
  bool expectFixedImagesFromCommandLine = false;
  i = 1;
  do
  {
    errorKey = "FixedImage" + ora::StreamConvert(i);
    s = ora::TrimF(config.ReadString(errorSection, errorKey, "", true));
    if (ora::ToUpperCaseF(s) == "FROM-COMMAND-LINE")
    {
      expectFixedImagesFromCommandLine = true;
      break; // rest is ignored
    }
    i++;
  } while (s.length() > 0);
  if (!expectFixedImagesFromCommandLine)
  {
    i = 1;
    do
    {
      errorKey = "FixedImage" + ora::StreamConvert(i);
      s = ora::TrimF(config.ReadString(errorSection, errorKey, "", true));
      if (s.length() > 0)
        fixedImageFileNames.push_back(s);
      i++;
    } while (s.length() > 0);
  }
  else
  {
    std::vector<std::string> fixedFiles;
    if (lastCommandLineOptionIndex > 0)
    {
      i = 1;
      while (++last < (int) commandLineArguments.size())
      {
        errorKey = "FixedImage" + ora::StreamConvert(i);
        s = commandLineArguments[last];
        if (s.length() > 0)
          fixedImageFileNames.push_back(s);
        i++;
      }
    }
  }

  // Get geometry parameters
  // - control window geometry
  errorSection = "Layout" + ora::StreamConvert(fixedImageFileNames.size());
  if (!configGeometry.IsSectionExisting(errorSection))
  {
    errorMessage = "No section for " + fixedImageFileNames.size();
    errorMessage += " fixed images available.";
    return false;
  }

  errorKey = "ControlWindowGeometry";
  s = ora::TrimF(configGeometry.ReadString(errorSection, errorKey, ""));
  if (s.length() <= 0)
  {
    errorMessage = "A ControlWindowGeometry entry is required.";
    return false;
  }
  std::vector<std::string> tokens;
  ora::Tokenize(s, tokens, " ");
  if (tokens.size() < 4)
  {
    errorMessage
        = "The geometry requires at least 4 values (<pos-x> <pos-y> <width> <height>)!\n"
            + s;
    return false;
  }
  cwgConfig.setLeft(atoi(tokens[0].c_str()));
  cwgConfig.setTop(atoi(tokens[1].c_str()));
  cwgConfig.setWidth(atoi(tokens[2].c_str()));
  cwgConfig.setHeight(atoi(tokens[3].c_str()));

  // - render view geometries
  rvgConfig.clear();
  for (std::size_t k = 1; k <= fixedImageFileNames.size(); k++)
  {
    errorKey = "RenderViewGeometry" + ora::StreamConvert(k);
    s = ora::TrimF(configGeometry.ReadString(errorSection, errorKey, ""));
    if (s.length() <= 0)
    {
      errorMessage
          = "A RenderViewGeometry entry for each fixed image is required.";
      return false;
    }

    tokens.clear();
    ora::Tokenize(s, tokens, " ");
    if (tokens.size() < 4)
    {
      errorMessage
          = "The geometry requires at least 4 values (<pos-x> <pos-y> <width> <height>)!\n"
              + s;
      return false;
    }

    QRect r;
    r.setLeft(atoi(tokens[0].c_str()));
    r.setTop(atoi(tokens[1].c_str()));
    r.setWidth(atoi(tokens[2].c_str()));
    r.setHeight(atoi(tokens[3].c_str()));
    rvgConfig.push_back(r);

    bool vis = true;  // default visibility
    // If window visibly flag exists, set visibility according to flag
    if (tokens.size() >= 5 && atoi(tokens[4].c_str()) == 0)
    {
        vis = false;
    }
    visibilities.push_back(vis);
  }

  errorSection = "";
  errorKey = "";
  errorMessage = "";
  m_ValidConfiguration = true;
  return m_ValidConfiguration;
}


/**
 * N-way 2D/3D Registration (REG23). This simple tool
 * enables interactive intensity-based 2D/3D registration of N projective images
 * (X-rays) with a volume image (CT). It is mainly controlled by a configuration
 * file, and enables some additional options in the GUI.
 *
 * @author phil
 * @author Markus
 * @version 1.1
 */
int main(int argc, char *argv[])
{
  std::string configFile = ""; // configuration file
  bool noGUI = false; // do not display GUI flag
  QRect *cwg = NULL; // control window geometry
  bool noConfigLoad = false;  // do not display config load button
  std::string configFileGeometry = ""; // configuration file
  bool geometryWriteBack = false;  // write window geometry on close
  bool stayOnTop = false; // application windows stay on top
  bool globalWarnings = false; // global ITK/VTK warnings
  QString language = ""; // language abbreviation
  int wlStorageStrategy = 0; // window/level storage strategy
  int wlRecoveryStrategy = 0; // window/level storage strategy
  bool science = false; // science-mode overload
  int intelliMasks = 1; // intelligent masks mode

  // basic command line pre-processing:
  std::string progName = "";
  if (argc > 0)
    progName = std::string(argv[0]);
  int last = 0;
  for (int i = 1; i < argc; i++)
  {
    if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
    {
      QApplication app(argc, argv);
      QString title = QString::fromStdString(
          ora::REG23GlobalInformation::ApplicationShortName) + "-Help";
      QString text = "";
      std::ostringstream os;
      PrintUsage(progName.length() > 0 ? progName.c_str() : NULL, os);
      text = QString::fromStdString(os.str());
      QMessageBox::information(NULL, title, text);
      return EXIT_SUCCESS;
    }
    if (std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config")
    {
      last = i + 1;
      i++;
      configFile = std::string(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-ng" || std::string(argv[i]) == "--no-gui")
    {
      last = i;
      noGUI = true;
      continue;
    }
    if (std::string(argv[i]) == "-cwg" || std::string(argv[i]) == "--control-window-geometry")
    {
      last = i + 4;
      cwg = new QRect();
      i++;
      cwg->setLeft(atoi(argv[i]));
      i++;
      cwg->setTop(atoi(argv[i]));
      i++;
      cwg->setWidth(atoi(argv[i]));
      i++;
      cwg->setHeight(atoi(argv[i]));
      continue;
    }
    if (std::string(argv[i]) == "-ncl" || std::string(argv[i]) == "--no-config-load")
    {
      last = i;
      noConfigLoad = true;
      continue;
    }
    if (std::string(argv[i]).substr(0, 4) == "-rvg" ||
        std::string(argv[i]).substr(0, 22) == "--render-view-geometry")
    {
      last = i + 4;
      i += 4;
      continue;
    }
    if (std::string(argv[i]) == "-wg" || std::string(argv[i]) == "--window-geometry")
    {
      last = i + 1;
      i++;
      configFileGeometry = std::string(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-wgwb" || std::string(argv[i]) == "--window-geometry-writeback")
    {
      last = i;
      geometryWriteBack = true;
      continue;
    }
    if (std::string(argv[i]) == "-sot" || std::string(argv[i]) == "--stay-on-top")
    {
      last = i;
      stayOnTop = true;
      continue;
    }
    if (std::string(argv[i]) == "-gwd" || std::string(argv[i]) == "--global-warning-display")
    {
      last = i;
      globalWarnings = true;
      continue;
    }
    if (std::string(argv[i]) == "-fl" || std::string(argv[i]) == "--force-language")
    {
      last = i + 1;
      i++;
      language = QString::fromStdString(std::string(argv[i]));
      continue;
    }
    if (std::string(argv[i]) == "-wlss" || std::string(argv[i]) == "--window-level-storage-strategy")
    {
      last = i + 1;
      i++;
      wlStorageStrategy = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-wlrs" || std::string(argv[i]) == "--window-level-recovery-strategy")
    {
      last = i + 1;
      i++;
      wlRecoveryStrategy = atoi(argv[i]);
      continue;
    }
    if (std::string(argv[i]) == "-s" || std::string(argv[i]) == "--science")
    {
      last = i;
      science = true;
      continue;
    }
    if (std::string(argv[i]) == "-imm" || std::string(argv[i]) == "--intelligent-mask-mode")
    {
      last = i + 1;
      i++;
      intelliMasks = atoi(argv[i]);
      continue;
    }
  }

  // create and connect basic components:
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  // initialize the Xlib module support for concurrent threads
  // NOTE: must be the first Xlib-call of an application!
  XInitThreads();
#endif

  // ITK/VTK warning display:
  vtkObject::SetGlobalWarningDisplay(globalWarnings);
  itk::Object::SetGlobalWarningDisplay(globalWarnings);

  QApplication app(argc, argv);

    // Qt-based internationalization:
  // auto-detection of system language and/or country codes:
  if (language.length() <= 0)
    language = QLocale::system().name(); // <language-code>_<country-code>
  // <language-code> code only (country-code may be too specific ...)
  QChar qc('_');
  int p;
  if ((p = language.indexOf(qc)) > -1)
    language = language.mid(0, p);
  QLocale::setDefault(QLocale(language)); // set system locale
  QTranslator qtTranslator;
  QDir qttranslSearchDir(QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  if (qttranslSearchDir.absolutePath().length() <= 0 ||
      !qttranslSearchDir.exists())
  {
    // -> set to binary's path:
    qttranslSearchDir = QDir(QString::fromStdString(
        itksys::SystemTools::GetFilenamePath(progName)));
  }
  bool tsucc = qtTranslator.load("qt_" + language, qttranslSearchDir.absolutePath());
  if (language.left(2) == QString("en"))
    tsucc = true; // special case: Qt is programmed in English
  if (tsucc)
    app.installTranslator(&qtTranslator);
  QTranslator appTranslator;
  std::string tp = itksys::SystemTools::GetFilenamePath(std::string(argv[0]));
#if !( ( defined( _WIN32 ) || defined ( _WIN64 ) ) && !defined( __CYGWIN__ ) )
  std::string SEP = "/";
#else
  std::string SEP = "\\";
#endif
  if (tp.length() > 0)
    ora::EnsureStringEndsWith(tp, SEP);
  tp += "reg23_";
  tp += language.toStdString();
  tsucc = appTranslator.load(QString::fromStdString(tp));
  if (tsucc)
    app.installTranslator(&appTranslator);

  ora::REG23Model *model = new ora::REG23Model();
  model->SetWindowLevelStorageStrategy(wlStorageStrategy);
  model->SetWindowLevelRecoveryStrategy(wlRecoveryStrategy);
  model->SetScientificMode(science);
  model->SetIntelligentMaskMode(intelliMasks);
  model->SetTaskManager(ora::TaskManager::GetInstance("reg23"));
  ora::REG23ControlWindow *cwin = new ora::REG23ControlWindow();
  cwin->SetModel(model);
  Q_INIT_RESOURCE(oraREG23ControlWindow);
  Q_INIT_RESOURCE(oraREG23TaskPresentationWidget);
  Q_INIT_RESOURCE(oraREG23RenderViewDialog);
  Q_INIT_RESOURCE(oraXYPlotWidget);
  Q_INIT_RESOURCE(oraREG23AboutDialog);

  // provide configuration file data:
  std::vector<std::string> cmdLn;
  for (int x = 0; x < argc; x++)
    cmdLn.push_back(argv[x]);
  model->SetCommandLineArguments(cmdLn); // for "FROM-COMMAND-LINE" argument
  model->SetLastCommandLineOptionIndex(last);
  model->SetConfigFile(configFile);
  model->Register(cwin);

  // Get number of fixed images to set up predefined GUI layouts
  bool useConfigWindowGeometry = false;
  QRect cwgConfig;
  std::vector<QRect> rvgConfig;
  std::vector<bool> visibilities;
  if (configFile.size() != 0 && configFileGeometry.size() != 0)
  {
    std::string errSect, errKey, errMsg;
    bool result = LoadWindowGeometryConfiguration(configFile,
        configFileGeometry, last, cmdLn, cwgConfig, rvgConfig, visibilities,
        errSect, errKey, errMsg);
    if (!result)
    {
      // std::cerr << "Configuration Error\n" <<
      //   "The window geometry configuration appears to be invalid!\n" <<
      //   "Error occurred here: "<< errSect << ", " << errKey << "\n" <<
      //   "Error description:" << errMsg << "\n";
      // return EXIT_FAILURE;
      // -> use default window geometry
      useConfigWindowGeometry = false;
    }
    else
    {
      useConfigWindowGeometry = true;
    }
  }
  if (geometryWriteBack)
  {
    cwin->SetLayoutConfigurationFile(QString::fromStdString(configFileGeometry));
    app.connect(&app, SIGNAL(lastWindowClosed()), cwin, SLOT(OnLastWindowClosed()));
  }
  cwin->SetWindowsStayOnTop(stayOnTop);

  // optional render view geometries:
  if (useConfigWindowGeometry)
  {
    for (unsigned int i = 0; i < rvgConfig.size(); ++i)
    {
      cwin->AddRenderViewGeometry(i /* 0-based */, rvgConfig[i].left(),
          rvgConfig[i].top(), rvgConfig[i].width(), rvgConfig[i].height(), visibilities[i]);
    }
  }
  else
  {
    for (int i = 1; i < argc; i++)
    {
      int idx = -1;
      if (std::string(argv[i]).substr(0, 4) == "-rvg")
        idx = atoi(std::string(argv[i]).substr(4, 10).c_str());
      if (std::string(argv[i]).substr(0, 22) == "--render-view-geometry")
        idx = atoi(std::string(argv[i]).substr(22, 10).c_str());
      if (idx > 0)
      {
        int g[4];
        i++;
        g[0] = atoi(argv[i]);
        i++;
        g[1] = atoi(argv[i]);
        i++;
        g[2] = atoi(argv[i]);
        i++;
        g[3] = atoi(argv[i]);
        cwin->AddRenderViewGeometry(idx - 1 /* 0-based */, g[0], g[1], g[2], g[3], true);
      }
    }
  }

  // control window config:
  cwin->Initialize();
  if (useConfigWindowGeometry)
  {
    cwin->setGeometry(cwgConfig);
    // TODO: Make clean function to handle this.
    QRect corr;
    corr.setX(cwgConfig.x() + cwgConfig.x() - cwin->pos().x());
    corr.setY(cwgConfig.y() + cwgConfig.y() - cwin->pos().y());
    corr.setWidth(cwgConfig.width());
    corr.setHeight(cwgConfig.height());
    cwin->setGeometry(corr);
  }
  else if (cwg)
  {
    cwin->setGeometry(*cwg);
    // TODO: Make clean function to handle this.
    QRect corr;
    corr.setX(cwg->x() + cwg->x() - cwin->pos().x());
    corr.setY(cwg->y() + cwg->y() - cwin->pos().y());
    corr.setWidth(cwg->width());
    corr.setHeight(cwg->height());
    cwin->setGeometry(corr);
    delete cwg;
  }
  cwin->SetLoadConfigVisible(!(noGUI || noConfigLoad)); // user cannot select!
  cwin->SetNoGUI(noGUI);
  cwin->setVisible(true); // always true, no-GUI-mode is handled internally

  // execute the GUI-based application:
  int ret = app.exec();

  if (!ora::TaskManager::DestroyInstance("reg23"))
  {
    std::cerr << "Could not reliably finish the task manager." << std::endl;
    return EXIT_FAILURE;
  }

  return ret;
}
