#include "lmDataView.h"
#include <QMouseEvent>
#include <QPainter>
#include <iostream>
#include <string>
#include<time.h>
//��ͬ��ʽ�ı��ߴ�;
const unsigned int sqSize[3] = {32,32,24};
//��������ߴ�;
const unsigned int ViewSize[3][2] =
{
	{16,16},
	{16,24},
	{32,32},
};
bool drawclick = false;
bool drawcontain = false;
const char *cha[4] = { "Y","U","V","NULL" };
const QColor color[4] = {Qt::black,QColor(182,64,128,200),QColor(64,64,180,200),QColor(164,192,25,200) };
const int lRect = 16;
const int sRect = 8;
bool isInPaintArea=true;
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
	std::cout << "inti data view\n";
	return true;
}

bool lmDataView::updatedataview(longmanEvt & rEvt)
{

	int xmouseclick = rEvt.getParam("xIn16").toInt();
	int ymouseclick = rEvt.getParam("yIn16").toInt();
	mXIn16 = (int)(xmouseclick / 16.0) * 16;
	mYIn16 = (int)(ymouseclick / 16.0) * 16;
	drawcontain = true;
	update();
	return true;
}


void lmDataView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		isInPaintArea = event->y() <= drawAreaRange[0] && event->x() <= drawAreaRange[1];
		
		bool res= isInPaintArea&& event->y() > sqSize[formatType] && event->x() > sqSize[formatType];
		if (res)
			{
				lastPositionClikedInDrawArea = (event->y() / sqSize[formatType] - 1)*(ViewSize[formatType][1])+ event->x() / sqSize[formatType]- 1;
				update();
				drawclick = true;
			}
		else
			lastPositionClikedInDrawArea = -1;

	}
	else
		QWidget::mousePressEvent(event);
}

void lmDataView::paintEvent(QPaintEvent * event)
{
	//if (!isInPaintArea)
	//	return;
	//����widget����Qpainterֻ���ڸú���,��ú������õĺ�����ʹ��;
	clock_t lBefore = clock();
	QPainter mcpainter(this);
	drawBackGround(mcpainter);
	if (drawclick)
	{
		drawclicked(mcpainter);
		drawContain(mcpainter);
		drawclick = false;
	}
	if (drawcontain)
	{
		drawContain(mcpainter);
		//drawcontain = false;
	}
	double dResult = (double)(clock() - lBefore) / CLOCKS_PER_SEC;
	std::cout << "dataview���ƴ���ʱ�䣺" << dResult << "s" << std::endl;
}
//���ƺڿ�;
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
				cpainter.setPen(QPen(color[chInfo(column-1, row-1)]));
				cpainter.drawRect(column*sqSize[formatType], row*sqSize[formatType], sqSize[formatType], sqSize[formatType]);
			}

		}
	}

}
//���ѡ�е�Ԫ��;
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
//����ѡ��Ԫ�ص���Ϣ;
void lmDataView::drawinfomation(QPainter& cpainter)
{
	QFont displayFont;
	cpainter.setFont(displayFont);
	QFontMetrics fontMetrics(displayFont);
	cpainter.setPen(QPen(Qt::black));
	int iwidthInDrawArea = lastPositionClikedInDrawArea % ViewSize[formatType][1];
	int iheightInDrawArea = lastPositionClikedInDrawArea / ViewSize[formatType][1];
	QString ch;
	ch = cha[chInfo(iwidthInDrawArea, iheightInDrawArea)];
	int iwInImage = 0;
	int ihInImage = 0;
	chInfo(iwidthInDrawArea, iheightInDrawArea, iwInImage, ihInImage);
	QString V = "(channel:" + ch + ",postionInImage:" + QString::fromStdString(std::to_string(iwInImage + mXIn16))+","+ QString::fromStdString(std::to_string(ihInImage + mYIn16))+")";
	//����������Ϣ;
	cpainter.drawText(ViewSize[formatType][1] * (sqSize[formatType]>>1) + (sqSize[formatType] / 2) - fontMetrics.width(V) / 2, (ViewSize[formatType][0]+1) *sqSize[formatType] + sqSize[formatType] / 4 + fontMetrics.ascent(), V);
	//��������ָʾ��;
	cpainter.setPen(QPen(QColor(192,32,32)));
	cpainter.drawLine((iwidthInDrawArea+1)*sqSize[formatType]+ (sqSize[formatType] >> 1),0, (iwidthInDrawArea + 1)*sqSize[formatType] + (sqSize[formatType] >> 1), sqSize[formatType]* (2+ViewSize[formatType][0]));
	cpainter.drawLine(0, (iheightInDrawArea + 1)*sqSize[formatType] + (sqSize[formatType] >> 1), sqSize[formatType] * (2 + ViewSize[formatType][1]), (iheightInDrawArea + 1)*sqSize[formatType] + (sqSize[formatType] >> 1));
}
//�ж�ͨ����Ϣ;ret=0:Y;ret=1:U;ret=2:V;
int lmDataView::chInfo(int x, int y)
{
	//400��444��ʽ;
	if (formatType == 0 || formatType == 2)
	{
		if (y < lRect && x < lRect)
			return 0;
		if (y < lRect && x >= lRect)
			return 2;
		if (y >= lRect&& x < lRect)
			return 1;
	}
	else
	{
		if (y < lRect && x < lRect)
			return 0;
		if (y < sRect && x >= lRect)
			return 1;
		if (y >= sRect && x >= lRect)
			return 2;
	}
/*	if (y >= 17 && x >= 17)*/
		return 3;
}
//����VUͨ����ͼƬ�е�����;
//����Ϊix��iy��Ϊĳ�����ڻ��Ʊ���е�����λ��;
//���Ϊpx��py��Ϊ��������ͼƬ�������16��С���ƫ��λ��;
void lmDataView::chInfo(int ix, int iy, int &px, int &py)
{
	int chType = chInfo(ix, iy);
	//��VUͨ����ֱ�Ӹ�ֵ;
	if (!chType)
	{
		px = ix;
		py = iy;
		return;
	}
	//UVͨ��,444��ʽ;
	if ( formatType == 2)
		//��������ֲ���ʽ;
		//YV
		//UN
	{
		if (chType==1)//U
		{
			px = ix;
			py = iy - lRect;
			return;
		}
		if (chType == 2)//V
		{
			px = ix - lRect;
			py = iy;
			return;
		}
		if (chType == 3)//NULL
		{
			px = -1;
			py = -1;
			return;
		}
	}
	if (formatType == 1)
		//��������ֲ���ʽ;
		//YU
		//YV
	{
		if (chType == 1)//U
		{
			px = 2 * (ix - lRect);
			py = 2 * iy;
			return;
		}
		if (chType == 2)//V
		{
			px = 2 * (ix - lRect);
			py = 2 * (iy - sRect);
			return;
		}
	}
}

void lmDataView::drawContain(QPainter& cpainter)
{
	QFont displayFont;
	cpainter.setFont(displayFont);
	QFontMetrics fontMetrics(displayFont);
	cpainter.setPen(QPen(Qt::black));
	std::cout << "������������" << std::endl;
	for (int row = 0; row < ViewSize[formatType][0] + 1; ++row)
	{
		for (int column = 0; column < ViewSize[formatType][1] + 1; ++column)
		{
			//���ص�ʵ��λ��;
			int ixInTable = column - 1;
			int iyInTable = row - 1;
			if (!row)
			{
				continue;
			}
			else if (!column)
			{
				continue;
			}
			else
			{
				int pxInimage = 0;
				int pyInimage = 0;
				chInfo(ixInTable, iyInTable, pxInimage, pyInimage);
				if (pxInimage < 0 || pyInimage < 0)
					continue;
				int ch = chInfo(ixInTable, iyInTable);
				int strideIamge = ch ? yuvWidth >> 1 : yuvWidth;
				int ix = ch ? pxInimage >> 1 : pxInimage;
				int iy = ch ? pyInimage >> 1 : pyInimage;
				int mmyIn16 = ch ? mYIn16 >> 1 : mYIn16;
				int mmxIn16 = ch ? mXIn16 >> 1 : mXIn16;
				long long offsetImage= (iy + mmyIn16)*strideIamge+ mmxIn16 + ix;
				int ipelVlaue = yuvDateptr[ch][offsetImage];
				QString V = QString::fromStdString(std::to_string(ipelVlaue));
				cpainter.drawText(column*sqSize[formatType] + (sqSize[formatType] / 2) - fontMetrics.width(V) / 2, row*sqSize[formatType] + sqSize[formatType] / 2 + fontMetrics.ascent()/2, V);
				cpainter.fillRect(column*sqSize[formatType], row*sqSize[formatType], sqSize[formatType], sqSize[formatType], QBrush(QColor(ipelVlaue, ipelVlaue, ipelVlaue,128)));
			}
		}
	}
}
