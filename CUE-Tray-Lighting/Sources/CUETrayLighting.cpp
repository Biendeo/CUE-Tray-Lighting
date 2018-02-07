#include "CUETrayLighting.h"

#include <chrono>
#include <thread>

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

	QAction* creditsAction = new QAction(this);
	creditsAction->setText("By Biendeo");
	creditsAction->setDisabled(true);
	menu->addAction(creditsAction);

	// This option is here if needed explicitly.
	QAction* refreshAction = new QAction(this);
	refreshAction->setText("Refresh");
	connect(refreshAction, SIGNAL(triggered()), this, SLOT(Refresh()));
	menu->addAction(refreshAction);

	QAction* closeAction = new QAction(this);
	closeAction->setText("Close");
	connect(closeAction, SIGNAL(triggered()), trayIcon, SLOT(hide()));
	connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
	menu->addAction(closeAction);
	
	// Set the context menu.
	trayIcon->setContextMenu(menu);
	trayIcon->show();


	// The light controller should already be working and running, but for posterity we refresh
	// after two seconds just to make sure it's up.
	std::this_thread::sleep_for(std::chrono::seconds(2));
	controller.Refresh();
}

void CUETrayLighting::Refresh() {
	controller.Refresh();
}
