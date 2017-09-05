#include "lmNormalDraw.h"



//lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase, int pl, int pp, int px, int py):
//	lmImageDrawFuc(pcDrawBase, pl, pp,px, py)
//{
//}


//lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase):
//	lmImageDrawFuc(pcDrawBase,0,0)
//{
//
//}

lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase, int px, int py):
	lmImageDrawFuc(pcDrawBase, 0, 0, px, py)
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
void lmNormalDraw::lmDrawFuction(QPainter &cPainter)
{
	if (px<0 || px>imageWidth || py<0 || py>imageHeight)
		return;
	cPainter.setPen(Qt::blue);
	int bigx = (int)(px / 64.0) * 64;
	int bigy = (int)(py / 64.0) * 64;
	cPainter.drawRect(bigx, bigy, 64, 64);
	cPainter.setPen(Qt::red);
	int smallx = (int)(px / 16.0) * 16;
	int smally = (int)(py / 16.0) * 16;
	cPainter.drawRect(smallx, smally, 16, 16);
}
