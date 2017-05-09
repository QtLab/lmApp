#include "lmDataView.h"
#include <QMouseEvent>
#include <QPainter>
lmDataView::lmDataView(QWidget *parent)
	: QDialog(parent, Qt::CustomizeWindowHint|Qt::WindowTitleHint),
	lmView(nullptr)
{
	ui.setupUi(this); 
	setModelName("Data_View_Model");
	CallBackFunc pcsetEvtHandle = std::bind(&lmDataView::setdataview, this, std::placeholders::_1);
	listenParam("set_dataview", pcsetEvtHandle);
	CallBackFunc pcupdateEvtHandle = std::bind(&lmDataView::updatedataview, this, std::placeholders::_1);
	listenParam("update_dataview", pcupdateEvtHandle);
	setMouseTracking(true);
}

lmDataView::~lmDataView()
{
}

bool lmDataView::setdataview(longmanEvt & rEvt)
{
	QVariant yValue = rEvt.getParam("y_ptr");
	QVariant uValue = rEvt.getParam("u_ptr");
	QVariant vValue = rEvt.getParam("v_ptr");
	yuvDateptr[0] = (Pel*)yValue.value<void *>();
	yuvDateptr[1] = (Pel*)uValue.value<void *>();
	yuvDateptr[2] = (Pel*)vValue.value<void *>();
	return true;
}

bool lmDataView::updatedataview(longmanEvt & rEvt)
{
	return true;
}

void lmDataView::mouseReleaseEvent(QMouseEvent *event)
{

}

void lmDataView::paintEvent(QPaintEvent * event)
{
	//对于widget对象，Qpainter只能在该函数,或该函数调用的函数内使用;
	QPainter cpainter(this);
	cpainter.drawRect(0, 0, 128, 128);
}
