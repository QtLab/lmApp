#ifndef lmNormalDraw_h__
#define lmNormalDraw_h__
//˵����ʵ��<�鿴yuv����ʱ����ͼƬ�ϻ��Ƶ�һ������һ��С��>�Ĺ���;
//�̳�lmImageDrawFucģ�飬��Ϊ���ܵľ���ʵ��ģ��
#include "lmImageDrawFuc.h"
class lmNormalDraw:public lmImageDrawFuc
{
public:
	lmNormalDraw(lmImageDrawBase * pcDrawBase,int px,int py);
	~lmNormalDraw();
	QPixmap *lmDraw(QImage &iDrawMap);
private:
	void lmDrawFuction(QPainter &cPainter);

};
#endif // lmNormalDraw_h__

