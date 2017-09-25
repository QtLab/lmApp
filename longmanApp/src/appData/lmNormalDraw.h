#ifndef lmNormalDraw_h__
#define lmNormalDraw_h__
//说明：实现<查看yuv数据时，在图片上绘制的一个大框和一个小框>的功能;
//继承lmImageDrawFuc模块，作为功能的具体实现模块
#include "lmImageDrawFuc.h"
class lmNormalDraw:public lmImageDrawFuc
{
public:
	lmNormalDraw(lmImageDrawBase * pcDrawBase,int px,int py);
	~lmNormalDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	void lmDrawFuction(QPainter &cPainter);

};
#endif // lmNormalDraw_h__

