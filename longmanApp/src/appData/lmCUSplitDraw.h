#ifndef lmCUSplitDraw_h__
#define lmCUSplitDraw_h__
#include "lmImageDrawFuc.h"
class lmCUSplitDraw :public lmImageDrawFuc
{
public:
	lmCUSplitDraw(lmImageDrawBase * pcDrawBase, int l = 0, int p = 0) :
		lmImageDrawFuc(pcDrawBase), mLayer(l), mPoc(p) {};
	lmCUSplitDraw(lmImageDrawBase * pcDrawBase, int l = 0, int p = 0,int ppx=0,int ppy=0) :
		lmImageDrawFuc(pcDrawBase), mLayer(l), mPoc(p), px(ppx), py(ppy) {};
	~lmCUSplitDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	int mLayer;
	int mPoc;
	int imageWidth = 0;
	int imageHeight = 0;
	int px;
	int py;
	void lmDrawFuction(QPainter &cPainter);
};
#endif // lmCUSplitDraw_h__

