#ifndef lmImagmeDraw_h__
#define lmImagmeDraw_h__
#include "lmImageDrawBase.h"
//ʵ����װ����;
//����һ�� pixmap���͵ĳ�Ա;
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

