#ifndef lmParserBitConfigure_h__
#define lmParserBitConfigure_h__

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths> 
#include "ui_lmParserBitConfigure.h"

class lmParserBitConfigure : public QDialog
{
	Q_OBJECT

public:
	lmParserBitConfigure(QWidget *parent = Q_NULLPTR);
	~lmParserBitConfigure();
	bool getcfg(QString & bitstreampath);
	inline QString getbitstreamPath() { return ui.lineEdit->text(); };
//	inline int getNumLayersToBeDecode() { return ui.spinBox->value();};

private:
	Ui::lmParserBitConfigure ui;
private slots:
	void on_toolButton_clicked();
};
#endif // lmParserBitConfigure_h__
