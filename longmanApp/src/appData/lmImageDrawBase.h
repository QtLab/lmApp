#ifndef lmImageDrawBase_h__
#define lmImageDrawBase_h__
#include <QTGui\QPixmap>
#include <QTGui\QImage>
#include <QTGui\QPainter>
#include "..\lmTypeDef.h"
//抽象被装饰者;
class lmImageDrawBase
{
public:
	lmImageDrawBase() {};
	virtual ~lmImageDrawBase() = 0;
	virtual QPixmap *lmDraw(QImage &iDrawMap) = 0;
};
#endif // lmImageDrawBase_h__

