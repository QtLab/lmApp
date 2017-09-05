#ifndef lmCUSplitDraw_h__
#define lmCUSplitDraw_h__
#include "lmImageDrawFuc.h"
class lmCUSplitDraw :public lmImageDrawFuc
{
public:
	lmCUSplitDraw(lmImageDrawBase * pcDrawBase, int l = 0, int p = 0) :
		lmImageDrawFuc(pcDrawBase,l,p){};
	lmCUSplitDraw(lmImageDrawBase * pcDrawBase, int l = 0, int p = 0,int ppx=0,int ppy=0) :
		lmImageDrawFuc(pcDrawBase,l,p,ppx,ppy){};
	~lmCUSplitDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	void lmDrawFuction(QPainter &cPainter);
};
#endif // lmCUSplitDraw_h__

