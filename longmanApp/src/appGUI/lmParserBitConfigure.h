#ifndef lmParserBitConfigure_h__
#define lmParserBitConfigure_h__

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths> 
#include "ui_lmParserBitConfigure.h"
#include "src/lmView.h"
//作用:码流信息选择;
class lmParserBitConfigure : public QDialog,public lmView
{
	Q_OBJECT

public:
	lmParserBitConfigure(QWidget *parent = Q_NULLPTR);
	~lmParserBitConfigure();
	bool getcfg(QString & bitstreampath);
	inline QString getbitstreamPath() { return ui.lineEdit->text(); };
//	inline int getNumLayersToBeDecode() { return ui.spinBox->value();};
	void resetState(bool bitpath=true);
private:
	Ui::lmParserBitConfigure ui;
	bool handleEvt(longmanEvt&);
	bool getLayertoDec();
	int mMaxLayer = -1;
	void proCheckBox();
private slots:
	void on_toolButton_clicked();
};
#endif // lmParserBitConfigure_h__
