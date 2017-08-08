#include "lmImageDraw.h"
lmImageDraw::lmImageDraw()
{

}


lmImageDraw::lmImageDraw(const QImage &iDrawMap)
{
	mPixMap = QPixmap::fromImage(iDrawMap);
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
	//应该仅存在复制;
	mPixMap = QPixmap::fromImage(iDrawMap);
	return &mPixMap;
}
