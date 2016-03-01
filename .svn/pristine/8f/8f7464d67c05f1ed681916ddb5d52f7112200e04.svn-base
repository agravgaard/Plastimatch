/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <iostream>
#include <QApplication>
#include <QLabel>

int
main (int argc, char **argv)
{
    // Display designer path
    QStringList paths = QCoreApplication::libraryPaths(); for (QStringList::iterator it = paths.begin(); it!=paths.end(); it++) { std::cout << "Looking for plugins at path: " << it->toStdString() << std::endl; } 

    QApplication app(argc, argv);
    QLabel *label = new QLabel("Hello World!");
  
    label->show();

    return app.exec();
}
