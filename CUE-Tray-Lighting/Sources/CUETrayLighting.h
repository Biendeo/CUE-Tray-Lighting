#pragma once

#include <QtWidgets/QDialog>
#include "ui_CUETrayLighting.h"

#include "LightController.h"

class CUETrayLighting : public QDialog {
	Q_OBJECT

	public:
	CUETrayLighting(QWidget* parent = Q_NULLPTR);

	private:
	Ui::CUETrayLightingClass ui;
	LightController controller;

	private slots:
	void Refresh();
};
