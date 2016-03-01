#include "nki2mha_converter.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	nki2mha_converter w;
	w.show();
	return a.exec();
}
