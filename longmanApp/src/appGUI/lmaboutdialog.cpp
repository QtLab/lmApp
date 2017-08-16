#include "lmaboutdialog.h"
#include "ui_lmaboutdialog.h"

lmAboutDialog::lmAboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lmAboutDialog)
{
    ui->setupUi(this);
	setWindowTitle(QStringLiteral("¹ØÓÚ³ÌÐò"));
}

lmAboutDialog::~lmAboutDialog()
{
    delete ui;
}
