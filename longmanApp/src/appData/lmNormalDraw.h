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
	lmNormalDraw(lmImageDrawBase * pcDrawBase,int px,int py);
	~lmNormalDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
	//lmNormalDraw& setMouseClickedPos(int px,int py);
private:
	void lmDrawFuction(QPixmap *mPixMap);
	void lmDrawFuction(QPainter &cPainter);
	int xmouseclick = 0;
	int ymouseclick = 0;
	int imageHeight = 0;
	int imageWidth = 0;
};
#endif // lmNormalDraw_h__

