#pragma once
#ifndef lmNormalDraw_h__
#define lmNormalDraw_h__
//˵����ʵ��<�鿴yuv����ʱ����ͼƬ�ϻ��Ƶ�һ������һ��С��>�Ĺ���;
//�̳�lmImageDrawFucģ�飬��Ϊ���ܵľ���ʵ��ģ��
#include "lmImageDrawFuc.h"
class lmNormalDraw:public lmImageDrawFuc
{
public:
	lmNormalDraw(lmImageDrawBase * pcDrawBase,int ,int,int,int);
	lmNormalDraw(lmImageDrawBase * pcDrawBase);
	~lmNormalDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	void lmDrawFuction(QPixmap *mPixMap);
	int xmouseclick = 0;
	int ymouseclick = 0;
	int imageHeight = 0;
	int imageWidth = 0;
};
#endif // lmNormalDraw_h__

