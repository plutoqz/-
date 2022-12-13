#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ImageProcessing.h"

class ImageProcessing : public QMainWindow
{
	Q_OBJECT

public:
	ImageProcessing(QWidget* parent = nullptr);
	~ImageProcessing();

private:
	Ui::ImageProcessingClass ui;
};
