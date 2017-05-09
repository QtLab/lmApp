#ifndef lmDataView_h__
#define lmDataView_h__

#include <QWidget>
#include "src\lmView.h"
#include "ui_lmDataView.h"
#include "..\lmTypeDef.h"

class lmDataView : public QWidget, public lmView
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
	int yuvHeight;
	int yuvWidth;
	int formatType;
protected:
	//void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	//void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	//void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *)Q_DECL_OVERRIDE;
};
#endif // lmDataView_h__
