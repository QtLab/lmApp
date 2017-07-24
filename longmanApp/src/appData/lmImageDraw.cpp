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
	return &mPixMap;
}
