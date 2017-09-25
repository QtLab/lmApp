#ifndef lmImagmeDraw_h__
#define lmImagmeDraw_h__
#include "lmImageDrawBase.h"
//实例被装饰者;
//包含一个 pixmap类型的成员;
class lmImageDraw:public lmImageDrawBase
{
public:
	lmImageDraw();
	lmImageDraw(const QImage &iDrawMap);
	~lmImageDraw();
	/*virtual */QPixmap *lmDraw(QImage &iDrawMap);
	void setDrawParam() {};
private:
	QPixmap  mPixMap;
// 	int mWidth;
// 	int mHeight;
// 	int mForamt;
};
#endif // lmImagmeDraw_h__

