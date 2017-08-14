#include "lmDrawManage.h"
#include "lmNormalDraw.h"


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
	QVariant vValue = rEvt.getParam("Image");
	if (vValue != 0)
		curImage = (QImage*)vValue.value<void *>();
	int mtypec = rEvt.getParam("drawTypeCode").toInt();
	if (mtypec > drawType::num)
		return false;
	if (mtypec > 0)
		mdrawtype = drawType( mtypec);
	switch (mdrawtype)
	{
	case lmDrawManage::showImage:
		showimage();
		break;
	case lmDrawManage::yuvdata:
		showyuvdataDraw(rEvt);
		break;
	case lmDrawManage::cudepth:
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
	//װ�κ����Ӧ��ʹ�þֲ�����ķ�ʽ������lmImageDrawBase��������κβ���;
	//�±�������ƴ�С��Ķ���ᱻ�Զ�����,����ֻ���漰��lmImageDrawFuc;
	//��lmImageDrawFuc��ͨ��ָ�����lmImageDrawBase�������Ӧ�ò����
	//�����е�lmImageDrawBase��������κ�Ӱ��;
	lmNormalDraw mfdraw(mImageDraw, yuvdata_xmouse, yuvdata_ymouse);
	QPixmap *mpixmap = mfdraw.lmDraw(*curImage);
	sendSignal(mpixmap);

}

void lmDrawManage::sendSignal(QPixmap * pmap)
{
	longmanEvt lmgraphview(EvtTYPE1);
	lmgraphview.setParam("CommandName", "update_image");
	lmgraphview.setParam("Image", QVariant::fromValue((void*)(pmap)));
	lmgraphview.dispatch();
}
