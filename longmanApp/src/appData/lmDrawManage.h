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
	void showyuvdataDraw(longmanEvt& rEvt);
	void sendSignal(QPixmap * pmap);
private:
	lmImageDrawBase *mImageDraw;
	QImage* curImage=nullptr;
	drawType mdrawtype;
	
// 	int yuvdata_xmouse = 0;
// 	int yuvdata_ymouse = 0;
};
#endif // lmDrawManage_h__

