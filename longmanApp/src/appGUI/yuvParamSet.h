#ifndef yuvParamSet_h__
#define yuvParamSet_h__

#include <QDialog>
#include "ui_yuvParamSet.h"
//目前可选参数:w.h.format;
class yuvParamSet : public QDialog
{
	Q_OBJECT

public:
	yuvParamSet(QWidget *parent/* = Q_NULLPTR*/);
	~yuvParamSet();
	const int getyuvwidth()const {return mWidth;};
	const int getyuvheight()const { return mHeight; };
	const int getformattype()const { return formatType; };
private:
	Ui::yuvParamSet ui;
	int mWidth;
	int mHeight;
	int formatType;//0:400,1:420,2:444;
	private slots:
	void on_p240_clicked();
	void on_p416_clicked();
	void on_p480_clicked();
	void on_p832_clicked();
	void on_p768_clicked();
	void on_p1024_clicked();
	void on_p720_clicked();
	void on_p1280_clicked();
	void on_p1920_clicked();
	void on_p1080_clicked();
	void on_p2560_clicked();
	void on_p1600_clicked();
	void setheight(int);
	void setwidth(int);
	void setformat(int);
};
#endif // yuvParamSet_h__
