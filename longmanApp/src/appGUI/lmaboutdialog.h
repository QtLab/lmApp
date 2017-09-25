#ifndef LMABOUTDIALOG_H
#define LMABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class lmAboutDialog;
}

class lmAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit lmAboutDialog(QWidget *parent = 0);
    ~lmAboutDialog();

private:
    Ui::lmAboutDialog *ui;
};

#endif // LMABOUTDIALOG_H
