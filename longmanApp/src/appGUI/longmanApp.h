#ifndef longmanApp_h__
#define longmanApp_h__

#include <QMainWindow>
#include <QTimeLine>
#include "ui_longmanApp.h"
#include "lmGraphView.h"
#include "src\lmView.h"
#include "yuvParamSet.h"
#include "lmDataView.h"
#include "lmParserBitConfigure.h"
#include "lmlayerlist.h"
#include "lmmsgview.h"
#include "lmaboutdialog.h"
//主界面，额外继承了lmView，通过listenParam接口设定响应函数;
class lmAbout;
class longmanApp : public QMainWindow ,public lmView
{
	Q_OBJECT

public:
	longmanApp(QWidget *parent = 0);
	bool updatemainwindow(longmanEvt&);
	bool openyuvFailed(longmanEvt&);
	bool saveCurImage(longmanEvt&);
public:
	//static;
	static void xMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &strMsg);
private:
	Ui::longmanAppClass ui;
	lmGraphView *m_imageView;
	//lmMessageBox *msgBox;
	lmDataView* m_DataView;
	QTimeLine timeLine;
	QImage* imageSave;
	QPixmap* curImage;
	lmParserBitConfigure *mBitParseCFG;
	lmLayerList *mlayerList;
	lmMsgView *mMsgOutput;
	lmAboutDialog* mAbout;
	void sendEvttoChnagePOC(int ppoc);
private slots:
	void on_actionOpen_SHVC_bitstream_triggered();
	void on_actionOpen_triggered();
	void on_actionAbout_triggered();
	void on_actionAboutQT_triggered();
	void on_nextButton_clicked();
	void on_backButton_clicked();
	void on_FrameIdxSlider_valueChanged(int);
	void on_beginButton_clicked();
	void on_endButton_clicked();
	void player(int);
	void on_playButton_clicked();
	void on_stopButton_clicked();
	void on_actionSave_as_image_triggered();
	void on_f1Button_clicked();
	void on_f2Button_clicked();
	void on_f3Button_clicked();
	void on_f4Button_clicked();
	void on_f5Button_clicked();
	void on_f6Button_clicked();
protected:
	virtual void dropEvent(QDropEvent *event);
};
#endif // longmanApp_h__
