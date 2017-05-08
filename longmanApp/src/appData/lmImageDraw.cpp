#include "lmImageDraw.h"
lmImageDraw::lmImageDraw()
{

}


lmImageDraw::~lmImageDraw()
{
}

//void lmImageDraw::lmImageDrawInit(QPixmap *imagePtr, int iWidth, int iHeight, int iForamt)
//{
////	mImagePtr = imagePtr;
//	mWidth = iWidth;
//	mHeight = iHeight;
//	mForamt = iForamt;
//}

QPixmap *lmImageDraw::lmDraw(QImage &iDrawMap)
{
	mPixMap = QPixmap::fromImage(iDrawMap);
	QPainter cPainter(&mPixMap);
	cPainter.setBrush(Qt::SolidPattern);
	cPainter.setPen(Qt::blue);
	cPainter.drawRect(25,25,125,125);
	return &mPixMap;
}
