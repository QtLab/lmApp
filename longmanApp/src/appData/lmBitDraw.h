
#ifndef lmBitDraw_h__
#define lmBitDraw_h__

#include "lmImageDrawFuc.h"
class lmBitDraw :public lmImageDrawFuc
{
public:
	lmBitDraw(lmImageDrawBase * pcDrawBase, int l = 0, int p = 0, int px = 0, int py = 0) :
		lmImageDrawFuc(pcDrawBase, l, p, px, py) {};
	~lmBitDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	void lmDrawFuction(QPainter &cPainter);
};
#endif // lmBitDraw_h__

