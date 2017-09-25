#ifndef lmDrawManage_h__
#define lmDrawManage_h__
#include "src/lmmodel.h"
//#include "lmImageDrawFuc.h"
#include "lmImageDraw.h"
class lmDrawManage
{
public:
	enum drawType
	{
		num = 4,
		showImage = 1,
		yuvdata = 2,
		cudepth = 3,
		ctubit = 4
	};
	lmDrawManage();
	~lmDrawManage();
	//QPixmap *lmDraw(QImage &iDrawMap);
	void setDraw(drawType pty) { mdrawtype = pty; };
	//主要传递构造绘制方法的参数;
	bool handleEvt(longmanEvt&);
private:
	void showimage();
	void showyuvdataDraw();
	void showcuDepthDraw();
	void showBitDraw();
	void sendSignal(QPixmap * pmap);
	void doDraw();
private:
	lmImageDrawBase *mImageDraw;
	QImage* curImage=nullptr;
	drawType mdrawtype;
	int layer = 0;
	int poc = 0;
	int mousex = 0;
	int mousey = 0;
};
#endif // lmDrawManage_h__

