#include "lmNormalDraw.h"



lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase, int px, int py, int pw, int ph):
	lmImageDrawFuc(pcDrawBase),
	xmouseclick(px),
	ymouseclick(py),
	imageWidth(pw),
	imageHeight(ph)
{
}


lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase):
	lmImageDrawFuc(pcDrawBase)
{

}

lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase, int px, int py):
	lmImageDrawFuc(pcDrawBase),
	xmouseclick(px),
	ymouseclick(py)
{

}

lmNormalDraw::~lmNormalDraw()
{
	
}

QPixmap * lmNormalDraw::lmDraw(QImage &iDrawMap)
{
	QPixmap *mPixMap = c_lmImageDB->lmDraw(iDrawMap);
	imageWidth = mPixMap->width();
	imageHeight = mPixMap->height();
	QPainter cmPainter(mPixMap);
	lmDrawFuction(cmPainter);
	return mPixMap;
}

// lmNormalDraw& lmNormalDraw::setMouseClickedPos(int px, int py)
// {
// 	xmouseclick = px;
// 	ymouseclick = py;
// 	return *this;
// }

void lmNormalDraw::lmDrawFuction(QPixmap *mPixMap)
{
	
	if (xmouseclick<0 || xmouseclick>imageWidth || ymouseclick<0 || ymouseclick>imageHeight)
		return;
	QPainter cPainter(mPixMap);
	cPainter.setPen(Qt::blue);
	int bigx = (int)(xmouseclick / 64.0) * 64;
	int bigy = (int)(ymouseclick / 64.0) * 64;
	cPainter.drawRect(bigx, bigy, 64, 64);
	cPainter.setPen(Qt::red);
	int smallx = (int)(xmouseclick / 16.0) * 16;
	int smally = (int)(ymouseclick / 16.0) * 16;
	cPainter.drawRect(smallx, smally, 16, 16);
}

void lmNormalDraw::lmDrawFuction(QPainter &cPainter)
{
	if (xmouseclick<0 || xmouseclick>imageWidth || ymouseclick<0 || ymouseclick>imageHeight)
		return;
	cPainter.setPen(Qt::blue);
	int bigx = (int)(xmouseclick / 64.0) * 64;
	int bigy = (int)(ymouseclick / 64.0) * 64;
	cPainter.drawRect(bigx, bigy, 64, 64);
	cPainter.setPen(Qt::red);
	int smallx = (int)(xmouseclick / 16.0) * 16;
	int smally = (int)(ymouseclick / 16.0) * 16;
	cPainter.drawRect(smallx, smally, 16, 16);
}
