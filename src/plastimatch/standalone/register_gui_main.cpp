#include "register_gui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
        register_gui w;
	w.show();
	return a.exec();
}
