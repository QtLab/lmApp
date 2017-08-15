#include "lmDrawManage.h"
#include "lmNormalDraw.h"
#include "lmCUSplitDraw.h"

lmDrawManage::lmDrawManage():
	mImageDraw(new lmImageDraw()),
	mdrawtype(drawType::num)
{
}


lmDrawManage::~lmDrawManage()
{
	delete mImageDraw;
}

// QPixmap * lmDrawManage::lmDraw(QImage &iDrawMap)
// {
// 	QPixmap * tmap = nullptr;
// 	switch (mdrawtype)
// 	{
// 	case lmDrawManage::showImage:
// 	{
// 		tmap = mImageDraw->lmDraw(iDrawMap);
// 		break;
// 	}
// 	case lmDrawManage::yuvdata:
// 		{
// 			lmNormalDraw mfdraw(mImageDraw, yuvdata_xmouse, yuvdata_ymouse);
// 			tmap = mfdraw.lmDraw(iDrawMap);
// 			break;
// 		}
// 	case lmDrawManage::cudepth:
// 		break;
// 	case lmDrawManage::ctubit:
// 		break;
// 	default:
// 		break;
// 	}
// 	return tmap;
// }

bool lmDrawManage::handleEvt(longmanEvt& rEvt)
{
	//设置绘制模式时，不传递其他参数;
	//处理绘制模式;
	int mtypec = rEvt.getParam("drawTypeCode").toInt();
	if (mtypec > drawType::num)
		return false;
	//若改变绘制模式,则应该保护参数;
	if (mtypec!=0)
	{
		mdrawtype = drawType(mtypec);
	}
	else
	{
		//是否是点击图片产生的参数变化;
		if (rEvt.getParam("from_picture_clicked").toBool())
		{
			mousex = rEvt.getParam("yuvdata_xmouse").toInt();
			mousey = rEvt.getParam("yuvdata_ymouse").toInt();
		}
		else
		{
			layer = rEvt.getParam("layer").toInt();
			poc = rEvt.getParam("poc").toInt();
		}

	}
	//然后单独处理图片指针;
	QVariant vValue = rEvt.getParam("Image");
	if (vValue != 0)
		curImage = (QImage*)vValue.value<void *>();
	if (curImage==nullptr)
		return false;


	switch (mdrawtype)
	{
	case lmDrawManage::showImage:
		showimage();
		break;
	case lmDrawManage::yuvdata:
		showyuvdataDraw(rEvt);
		break;
	case lmDrawManage::cudepth:
		showcuDepthDraw(rEvt);
		break;
	case lmDrawManage::ctubit:
		break;
	default:
		break;
	}
	return true;
}

void lmDrawManage::showimage()
{
	QPixmap *mpixmap = mImageDraw->lmDraw(*curImage);
	sendSignal(mpixmap);
}

void lmDrawManage::showyuvdataDraw(longmanEvt& rEvt)
{

	int yuvdata_xmouse = rEvt.getParam("yuvdata_xmouse").toInt();
	int yuvdata_ymouse = rEvt.getParam("yuvdata_ymouse").toInt();
	//装饰后的类应该使用局部对象的方式，不对lmImageDrawBase对象进行任何操作;
	//下边这个绘制大小框的对象会被自动销毁,析构只会涉及到lmImageDrawFuc;
	//而lmImageDrawFuc是通过指针调用lmImageDrawBase对象，因此应该不会对
	//本类中的lmImageDrawBase对象产生任何影响;
	lmNormalDraw mfdraw(mImageDraw, yuvdata_xmouse, yuvdata_ymouse);
	QPixmap *mpixmap = mfdraw.lmDraw(*curImage);
	sendSignal(mpixmap);

}

void lmDrawManage::showcuDepthDraw(longmanEvt& rEvt)
{


	lmCUSplitDraw mdraw(mImageDraw,layer,poc,mousex,mousey);
	QPixmap *mpixmap = mdraw.lmDraw(*curImage);
	sendSignal(mpixmap);
}

void lmDrawManage::sendSignal(QPixmap * pmap)
{
	longmanEvt lmgraphview(EvtTYPE1);
	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(pmap)));
	lmgraphview.dispatch();
}
