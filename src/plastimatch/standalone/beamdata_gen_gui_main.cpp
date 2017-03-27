#include "beamdata_gen_gui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
        beamdata_gen_gui w;
	w.show();
	return a.exec();
}
