#ifndef lmGraphView_h__
#define lmGraphView_h__

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include "src\lmView.h"
#include "ui_lmGraphView.h"
class lmGraphView : public QGraphicsView, public lmView
{
	Q_OBJECT

public:
	lmGraphView(QWidget *parent = Q_NULLPTR);
	~lmGraphView();
public:
	bool setimage(longmanEvt &);
	bool scaleimage(longmanEvt &);
	bool xupdate(longmanEvt &);
private:
	Ui::lmGraphView ui;
	QGraphicsScene msence;
	QGraphicsPixmapItem imagegroup;
	double pixmapScale;
	double curScale;
	void xscale(double);
	int imageWidth;
	int imageHeight;

	QPixmap  *mImage;//Щїжи
protected:
	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	//void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	//void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	//void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
	int m_iMousePressX;
	int m_iMousePressY;
	int m_iMousePressImageX;
	int m_iMousePressImageY;
	bool m_controlMouse;
};
#endif // lmGraphView_h__
