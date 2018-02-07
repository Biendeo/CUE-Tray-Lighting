#include "CUETrayLighting.h"

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>

CUETrayLighting::CUETrayLighting(QWidget* parent) : QDialog(parent) {
	ui.setupUi(this);

	// Make the tray icon.
	QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
	//? A different icon may be ideal at some point.
	trayIcon->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	trayIcon->setToolTip("CUE Tray Lighting");

	// The right-click context menu.
	QMenu* menu = new QMenu(this);

	QAction* aboutAction = new QAction(this);
	aboutAction->setText("CUE Tray Lighting");
	aboutAction->setDisabled(true);
	menu->addAction(aboutAction);
	
	QAction* closeAction = new QAction(this);
	closeAction->setText("Close");
	connect(closeAction, SIGNAL(triggered()), trayIcon, SLOT(hide()));
	connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
	menu->addAction(closeAction);
	
	// Set the context menu.
	trayIcon->setContextMenu(menu);
	trayIcon->show();
}
