

#include "oraQFileTools.h"

#include <math.h>

#include "oraFileTools.h"

#include "itksys/SystemTools.hxx"

#include <QDir>
#include <QFSFileEngine>
#include <QDateTime>


namespace ora
{


std::vector<std::string>
GeneralFileTools
::FindFilesInDirectory(std::string directory, std::string filePattern,
    bool recursive, std::string dirPattern)
{
  std::vector<std::string> result;
  // QDir uses UNIX-like slashes generally!
  std::string absolutePath;
  QStringList fileFilters;
  QStringList dirFilters;
  QDir dir;
  QStringList dirsToParse;

  fileFilters.append(QString::fromStdString(filePattern));
  dirFilters.append(QString::fromStdString(dirPattern));
  if (directory.substr(0, 2) == "\\\\")
    dirsToParse.push_back(QString::fromStdString(UnixUNCConverter::GetInstance()->
      EnsureUNCCompatibility(directory)));
  else
    dirsToParse.push_back(QString::fromStdString(UnixUNCConverter::GetInstance()->
      EnsureUNIXCompatibility(directory)));
  while (dirsToParse.count() > 0)
  {
    dir.setPath(dirsToParse.front()); // set current directory
    dirsToParse.pop_front();
    dirsToParse.size();

    if (dir.exists())
    {
      // configure for file filtering:
      dir.setFilter(QDir::Files | QDir::NoSymLinks);
      dir.setNameFilters(fileFilters);
      absolutePath = (dir.absolutePath() + QDir::separator()).toStdString();

      QStringListIterator it(dir.entryList()); // append to result-vector
      while (it.hasNext())
        result.push_back(absolutePath + it.next().toStdString());

      if (recursive) // find sub-directories if requested
      {
        dir.setFilter(QDir::Dirs | QDir::NoSymLinks);
        dir.setNameFilters(dirFilters);
        QStringList dirList(dir.entryList());
        for(QStringList::iterator i = dirList.begin(); i!= dirList.end(); ++i)
        {
          if(*i != "." && *i != "..")
            dirsToParse.push_back(QString::fromStdString(absolutePath) + *i);
        }
      }
    }
  }

  return result;
}

int
GeneralFileTools
::DeleteOlderFiles(std::string path, std::string pattern, double maxAge)
{
  if (path.length() <= 0)
    return 0;

  // search for possible files:
  std::vector<std::string> files = GeneralFileTools::FindFilesInDirectory(
      path, pattern, false, "");

  int deleted = 0;
  QFSFileEngine fe;
  QDateTime mt; // modified time
  QDateTime minTime = QDateTime::currentDateTime(); // initialized with now
  minTime = minTime.addSecs(-(int)(maxAge * 86400.0));
  // now check the modified times and delete files on demand (if older)
  for (unsigned int x = 0; x < files.size(); ++x)
  {
    fe.setFileName(QString::fromStdString(files[x]));
    mt = fe.fileTime(QFSFileEngine::ModificationTime);
    if (mt < minTime)
    {
      if (itksys::SystemTools::RemoveFile(files[x].c_str()))
        deleted++;
    }
  }

  return deleted;
}


}

