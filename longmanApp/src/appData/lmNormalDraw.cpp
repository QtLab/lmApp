#include "lmNormalDraw.h"



lmNormalDraw::lmNormalDraw(lmImageDrawBase * pcDrawBase):
	lmImageDrawFuc(pcDrawBase)
{
}


lmNormalDraw::~lmNormalDraw()
{
}

QPixmap * lmNormalDraw::lmDraw(QImage &iDrawMap)
{
	QPixmap *mPixMap = c_lmImageDB->lmDraw(iDrawMap);
	lmDrawFuction(mPixMap);
	return mPixMap;
}

void lmNormalDraw::lmDrawFuction(QPixmap *mPixMap)
{
	QPainter cPainter(mPixMap);
	cPainter.setPen(Qt::blue);
	cPainter.drawRect(25, 25, 125, 125);
}
