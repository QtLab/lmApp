#include "lmDataView.h"
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include <string>
const unsigned int sqSize[3] = {32,32,24};
const unsigned int ViewSize[3][2] = 
{
	{16,16},
	{16,24},
	{32,32},
};
bool drawclick = false;
bool drawcontain = false;
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
	yuvWidth = rEvt.getParam("width").toInt();
	yuvHeight = rEvt.getParam("height").toInt();
	formatType = rEvt.getParam("format").toInt();
	drawAreaRange[0] = ViewSize[formatType][0] * sqSize[formatType] +  sqSize[formatType];//height
	drawAreaRange[1] = ViewSize[formatType][1] * sqSize[formatType] +  sqSize[formatType];//width
	setMinimumSize(drawAreaRange[1] + sqSize[formatType], drawAreaRange[0] + sqSize[formatType]);
	setMaximumSize(drawAreaRange[1] + sqSize[formatType], drawAreaRange[0] + sqSize[formatType]);
	update();
	std::cout << "inti data view" << std::endl;
	return true;
}

bool lmDataView::updatedataview(longmanEvt & rEvt)
{

	int xmouseclick = rEvt.getParam("xIn16").toInt();
	int ymouseclick = rEvt.getParam("yIn16").toInt();
	mXIn16 = (int)(xmouseclick / 16.0) * 16;
	mYIn16 = (int)(ymouseclick / 16.0) * 16;
	return true;
}


void lmDataView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		bool respond = event->y() <= drawAreaRange[0] && event->x() <= drawAreaRange[1];
		respond= respond&& event->y() > sqSize[formatType] && event->x() > sqSize[formatType];
		if (respond)
			lastPositionClikedInDrawArea = (event->y() / sqSize[formatType] - 1)*(ViewSize[formatType][1])+ event->x() / sqSize[formatType]- 1;
		else
			lastPositionClikedInDrawArea = -1;
		update();
		drawclick = true;
	}
	else
		QWidget::mousePressEvent(event);
}

void lmDataView::paintEvent(QPaintEvent * event)
{
	//对于widget对象，Qpainter只能在该函数,或该函数调用的函数内使用;
	QPainter mcpainter(this);
	drawBackGround(mcpainter);
	if (drawclick)
	{
		drawclicked(mcpainter);
		drawclick = false;
	}
	
}
//绘制黑框;
void lmDataView::drawBackGround(QPainter& cpainter)
{
	QFont displayFont;
	cpainter.setFont(displayFont);
	QFontMetrics fontMetrics(displayFont);
	
	for (int row = 0; row < ViewSize[formatType][0] + 1; ++row)
	{
		for (int column = 0; column < ViewSize[formatType][1] + 1; ++column)
		{
			cpainter.setPen(QPen(Qt::black));
			if (!row)
			{
				if (column)
				{
					QString V = QString::fromStdString(std::to_string(column));
					cpainter.drawText(column*sqSize[formatType] + (sqSize[formatType] / 2) - fontMetrics.width(V) / 2, row*sqSize[formatType] + sqSize[formatType] / 4 + fontMetrics.ascent(), V);
				}
			}
			else if (!column)
			{
				QString V = QString::fromStdString(std::to_string(row));
				cpainter.drawText(column*sqSize[formatType] + (sqSize[formatType] / 2) - fontMetrics.width(V) / 2, row*sqSize[formatType] + sqSize[formatType] / 4 + fontMetrics.ascent(), V);
			}
			else
			{
				if (row < 17/* * sqSize[formatType]*/ && column < 17/* * sqSize[formatType]*/)
					cpainter.setPen(QPen(Qt::black));
				if (row < 17/* * sqSize[formatType]*/ && column >= 17/* * sqSize[formatType]*/)
					cpainter.setPen(QPen(Qt::yellow));
				if (row >= 17/* * sqSize[formatType]*/ && column < 17/* * sqSize[formatType]*/)
					cpainter.setPen(QPen(Qt::blue));
				if (row >= 17 /* * sqSize[formatType]*/ && column >= 17/* * sqSize[formatType]*/)
					cpainter.setPen(QPen(Qt::red));
				cpainter.drawRect(column*sqSize[formatType], row*sqSize[formatType], sqSize[formatType], sqSize[formatType]);
			}

		}
	}

}
//标记选中的元素;
void lmDataView::drawclicked(QPainter& cpainter)
{

	if (lastPositionClikedInDrawArea != -1)
	{
		int y = (lastPositionClikedInDrawArea / ViewSize[formatType][1])*sqSize[formatType] + sqSize[formatType] + 1;
		int x = (lastPositionClikedInDrawArea % ViewSize[formatType][1])*sqSize[formatType] + sqSize[formatType] + 1;
		cpainter.fillRect(x, y, sqSize[formatType] - 1, sqSize[formatType] - 1, QBrush(Qt::red));
		drawinfomation(cpainter);
	}
}
//绘制选中元素的信息;
void lmDataView::drawinfomation(QPainter& cpainter)
{
	QFont displayFont;
	cpainter.setFont(displayFont);
	QFontMetrics fontMetrics(displayFont);
	int iwidthInDrawArea = lastPositionClikedInDrawArea % ViewSize[formatType][1];
	int iheightInDrawArea = lastPositionClikedInDrawArea / ViewSize[formatType][1];
	QString ch;
	if (iwidthInDrawArea<16&& iheightInDrawArea<16)
	{
		ch = "Y";
	}
	else if(iheightInDrawArea<16)
	{
		ch = "U";
	}
	else
	{
		ch = "V";
	}
	QString V = "(" + ch + "," + QString::fromStdString(std::to_string(iwidthInDrawArea + mXIn16))+","+ QString::fromStdString(std::to_string(iheightInDrawArea + mYIn16))+")";
	cpainter.drawText(ViewSize[formatType][1] * (sqSize[formatType]>>1) + (sqSize[formatType] / 2) - fontMetrics.width(V) / 2, (ViewSize[formatType][0]+1) *sqSize[formatType] + sqSize[formatType] / 4 + fontMetrics.ascent(), V);
}
