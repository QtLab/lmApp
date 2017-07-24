#ifndef lmDataView_h__
#define lmDataView_h__

#include <QDialog>
#include "src\lmView.h"
#include "ui_lmDataView.h"
#include "..\lmTypeDef.h"

class lmDataView : public QDialog, public lmView
{
	Q_OBJECT
		
public:
	lmDataView(QWidget *parent = Q_NULLPTR);
	~lmDataView();
	bool setdataview(longmanEvt &rEvt);
	bool updatedataview(longmanEvt &rEvt);
private:
	Ui::lmDataView ui;
	Pel *yuvDateptr[3] = { nullptr,nullptr,nullptr };
	int yuvHeight = 0;
	int yuvWidth = 0;
	int formatType = 1;
	int drawAreaRange[2] = { 0,0 };
	int lastPositionClikedInDrawArea = 0;
	int mXIn16 = 0;
	int mYIn16 = 0;
protected:
	//void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	//void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	//void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *)Q_DECL_OVERRIDE;
private:
	void drawBackGround(QPainter&);
	void drawclicked(QPainter&);
	void drawinfomation(QPainter&);
	int chInfo(int x,int y);
	void chInfo(int x,int y,int &px,int &py);
	void drawContain(QPainter&);
};
#endif // lmDataView_h__
