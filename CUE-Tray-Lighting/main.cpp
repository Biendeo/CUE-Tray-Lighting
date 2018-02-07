#include "CUETrayLighting.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CUETrayLighting w;
	w.show();
	return a.exec();
}
