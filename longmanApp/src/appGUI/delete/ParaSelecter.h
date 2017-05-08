#pragma once

#include <QDialog>
#include "ui_ParaSelecter.h"

class ParaSelecter : public QDialog
{
	Q_OBJECT

public:
	ParaSelecter(QWidget *parent = Q_NULLPTR);
	~ParaSelecter();

private:
	Ui::ParaSelecter ui;
};
