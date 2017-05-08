#ifndef lmImagmeDraw_h__
#define lmImagmeDraw_h__
#include "lmImageDrawBase.h"
//实例被装饰者;
//
class lmImageDraw:public lmImageDrawBase
{
public:
	lmImageDraw();
	~lmImageDraw();
	virtual QPixmap *lmDraw(QImage &iDrawMap);
	void setDrawParam() {};
private:
	QPixmap  mPixMap;
// 	int mWidth;
// 	int mHeight;
// 	int mForamt;
};
#endif // lmImagmeDraw_h__

