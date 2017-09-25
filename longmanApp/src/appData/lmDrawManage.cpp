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
	//�������ģʽ;
	int mtypec = rEvt.getParam("drawTypeCode").toInt();
	if (mtypec > drawType::num)
		return false;
	//�������;
	//mtypec==0��ʾû�д���ģʽ��Ϣ;
	if (mtypec!=0)
	{
		mdrawtype = drawType(mtypec);
	}
	else
	{
		//����ͼƬ����Ĳ���,���￼�ǸĽ�Ϊ�����ķ�ʽ;
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
	//����ͼƬָ��;
	QVariant vValue = rEvt.getParam("Image");
	if (vValue != 0)
		curImage = (QImage*)vValue.value<void *>();
	if (curImage==nullptr)
		return false;

	//������Ʊ�־;
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

	//װ�κ����Ӧ��ʹ�þֲ�����ķ�ʽ������lmImageDrawBase��������κβ���;
	//�±�������ƴ�С��Ķ���ᱻ�Զ�����,����ֻ���漰��lmImageDrawFuc;
	//��lmImageDrawFuc��ͨ��ָ�����lmImageDrawBase�������Ӧ�ò����
	//�����е�lmImageDrawBase��������κ�Ӱ��;
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
