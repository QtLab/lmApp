#ifndef LMMSGVIEW_H
#define LMMSGVIEW_H

#include <QWidget>
#include <QMessageBox>
#include "src/lmView.h"
namespace Ui {
class lmMsgView;
}

class lmMsgView : public QWidget,public lmView
{
    Q_OBJECT

public:
    explicit lmMsgView(QWidget *parent = 0);
    ~lmMsgView();
	
private:
    Ui::lmMsgView *ui;
	bool handleMsg(longmanEvt&);
	QMessageBox mWarningbox;
};

#endif // LMMSGVIEW_H
