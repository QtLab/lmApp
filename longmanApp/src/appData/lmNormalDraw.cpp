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
	if (xmouseclick<0 || xmouseclick>imageWidth || ymouseclick<0 || ymouseclick>imageHeight)
		return;
	int bigx = (int)(xmouseclick / 64.0) * 64;
	int bigy = (int)(ymouseclick / 64.0) * 64;
	cPainter.drawRect(bigx, bigy, 64, 64);
}
