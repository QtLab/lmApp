#ifndef lmImagmeDraw_h__
#define lmImagmeDraw_h__
#include "..\lmTypeDef.h"
#include <QTGui\QPixmap>
#include <QTGui\QImage>
#include <QTGui\QPainter>
class lmImageDraw
{
public:
	lmImageDraw();
	~lmImageDraw();
	//void lmImageDrawInit(QPixmap * imagePtr,int iWidth,int iHeight,int iForamt);
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	QPixmap  mPixMap;
	int mWidth;
	int mHeight;
	int mForamt;
};
#endif // lmImagmeDraw_h__

