#include "lmDrawManage.h"
#include "lmNormalDraw.h"
#include "lmCUSplitDraw.h"
#include "lmBitDraw.h"
lmDrawManage::lmDrawManage():
	mImageDraw(new lmImageDraw()),
	mdrawtype(drawType::showImage)
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
	//处理绘制模式;
	int mtypec = rEvt.getParam("drawTypeCode").toInt();
	if (mtypec > drawType::num)
		return false;
	//处理参数;
	//mtypec==0表示没有传输模式信息;
	if (mtypec!=0)
	{
		mdrawtype = drawType(mtypec);
	}
	else
	{
		//来自图片点击的参数,这里考虑改进为索引的方式;
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
	//处理图片指针;
	QVariant vValue = rEvt.getParam("Image");
	if (vValue != 0)
		curImage = (QImage*)vValue.value<void *>();
	if (curImage==nullptr)
		return false;

	//处理绘制标志;
	bool dodraw= rEvt.getParam("do_draw").toBool();
	if (dodraw)
		doDraw();
	return true;
}

void lmDrawManage::showimage()
{
	QPixmap *mpixmap = mImageDraw->lmDraw(*curImage);
	sendSignal(mpixmap);
}

void lmDrawManage::showyuvdataDraw()
{

	//装饰后的类应该使用局部对象的方式，不对lmImageDrawBase对象进行任何操作;
	//下边这个绘制大小框的对象会被自动销毁,析构只会涉及到lmImageDrawFuc;
	//而lmImageDrawFuc是通过指针调用lmImageDrawBase对象，因此应该不会对
	//本类中的lmImageDrawBase对象产生任何影响;
	lmNormalDraw mfdraw(mImageDraw, mousex, mousey);
	QPixmap *mpixmap = mfdraw.lmDraw(*curImage);
	sendSignal(mpixmap);

}

void lmDrawManage::showcuDepthDraw()
{


	lmCUSplitDraw mdraw(mImageDraw,layer,poc,mousex,mousey);
	QPixmap *mpixmap = mdraw.lmDraw(*curImage);
	sendSignal(mpixmap);
}

void lmDrawManage::showBitDraw()
{
	lmBitDraw mdraw(mImageDraw, layer, poc, mousex, mousey);
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

void lmDrawManage::doDraw()
{
	switch (mdrawtype)
	{
	case lmDrawManage::showImage:
		showimage();
		break;
	case lmDrawManage::yuvdata:
		showyuvdataDraw();
		break;
	case lmDrawManage::cudepth:
		showcuDepthDraw();
		break;
	case lmDrawManage::ctubit:
		showBitDraw();
		break;
	default:
		break;
	}
}
