#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include "CUETrayLighting.h"
#include "SystemInfo.h"

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	// If there is no system tray available, the program shouldn't run.
	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(0, QObject::tr("CUE Tray Lighting"), QObject::tr("I couldn't detect any tray on this system."));
		return 1;
	}

	SetupCPUReadings();

	CUETrayLighting w;

	// I only want the tray icon, so we won't show the dialog from here.
	//w.show();

	return a.exec();
}