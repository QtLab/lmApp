#include "lmaboutdialog.h"
#include "ui_lmaboutdialog.h"

lmAboutDialog::lmAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lmAboutDialog)
{
    ui->setupUi(this);
	setWindowTitle(QStringLiteral("���ڳ���"));
}

lmAboutDialog::~lmAboutDialog()
{
    delete ui;
}
