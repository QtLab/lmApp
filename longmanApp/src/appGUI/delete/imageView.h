#ifndef imageView_h__
#define imageView_h__

#include <QGraphicsView>
#include "src\lmView.h"
class imageView : public QGraphicsView,public lmView
{
	Q_OBJECT

public:
	imageView();
	~imageView();
};
#endif // imageView_h__
