#ifndef lmParserBitConfigure_h__
#define lmParserBitConfigure_h__

#include <QDialog>
#include "ui_lmParserBitConfigure.h"

class lmParserBitConfigure : public QDialog
{
	Q_OBJECT

public:
	lmParserBitConfigure(QWidget *parent = Q_NULLPTR);
	~lmParserBitConfigure();

private:
	Ui::lmParserBitConfigure ui;
};
#endif // lmParserBitConfigure_h__
