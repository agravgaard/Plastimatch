#include "gamma_gui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
        gamma_gui w;
	w.show();
	return a.exec();
}
