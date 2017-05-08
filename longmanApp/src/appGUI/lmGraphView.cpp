#include "lmGraphView.h"
#include <QKeyEvent>
#include <QPainter>
////�����˵��;
//��ɾ��-1.("CommandName","set_image")+("image",void*)+("image_width",int)+("image_height",int);
//iamgeΪͼƬָ�룬����Ϊ��ߺ͸�ʽ;��������ҪΪ����ǰ�ഫ����Ҫ����,ֻ�ڴ��ļ��ɹ���ִ��һ��;
//2.("CommandName","scale")+("scale",double);
//��δ����;
//3.("CommandName","update_image")+("image",void*);
//������ʾ��ͼƬ;
template <typename T> inline T Clip3(const T minVal, const T maxVal, const T a) { return std::min<T>(std::max<T>(minVal, a), maxVal); }
const double DefaultScale = 0.00403897f;
const double ZoomInFactor = 1.25;
const double Zoommin = 0.25;
const double Zoommax = 4.00;
const double ScrollStep = 0.1;
lmGraphView::lmGraphView(QWidget *parent)
	: QGraphicsView(parent),
	lmView(nullptr),
	mImage(nullptr)

{
	ui.setupUi(this);
	setModelName("Graph_View_Model");
	//
	CallBackFunc pcupdateEvtHandle = std::bind(&lmGraphView::setimage, this, std::placeholders::_1);
	listenParam("set_image", pcupdateEvtHandle);
	CallBackFunc pcscaleEvtHandle = std::bind(&lmGraphView::scaleimage, this, std::placeholders::_1);
	listenParam("scale", pcscaleEvtHandle);
	CallBackFunc pcxupdateHandle = std::bind(&lmGraphView::xupdate, this, std::placeholders::_1);
	listenParam("update_image", pcxupdateHandle);
	//
	imagegroup.setTransformationMode(Qt::FastTransformation);
	msence.addItem(&imagegroup);
	setScene(&msence);
	setAcceptDrops(false);
	setCursor(Qt::CrossCursor);
	//
	curScale = 1;
	imageWidth = 0;
	imageHeight = 0;
	m_iMousePressX = 0;
	m_iMousePressY = 0;
	m_iMousePressImageX = 0;
	m_iMousePressImageY = 0;
	m_controlMouse = false;
}

lmGraphView::~lmGraphView()
{
}

bool lmGraphView::setimage(longmanEvt & upimage)
{
	QVariant vValue = upimage.getParam("image");
	imageWidth = upimage.getParam("width").toInt();
	imageHeight = upimage.getParam("height").toInt();
	int formattyp = upimage.getParam("format").toInt();
	mImage = (QPixmap*)vValue.value<void *>();
	m_controlMouse = true;
	return true;
}

bool lmGraphView::scaleimage(longmanEvt & sacleEvt)
{
	curScale = sacleEvt.getParam("scale").toDouble();
	curScale = (double)Clip3<double>(Zoommin, Zoommax, curScale);
	xscale(curScale);
	return true;
}

void lmGraphView::xscale(double pscalce)
{
	longmanEvt testmsg(EvtTYPE1);
	testmsg.setParam("CommandName", "show_message");
	testmsg.setParam("MsgType", 1);
	testmsg.setParam("info", QStringLiteral("������δ���ƣ�"));
	testmsg.dispatch();
}

bool lmGraphView::xupdate(longmanEvt & rEvt)
{
	 	//int iImgX = imagegroup.scenePos().x();
	 	//int iImgY = imagegroup.scenePos().y();
	QVariant vValue = rEvt.getParam("Image");
	QPixmap *mcImage = (QPixmap*)vValue.value<void *>();
	imagegroup.setPixmap(*mcImage);
	m_controlMouse = true;
	return true;
}

void lmGraphView::wheelEvent(QWheelEvent *event)
{
	//int iMouseX = imagegroup.pos().y();
	//int iMouseY = imagegroup.pos().x();
	//if (iMouseY > yrange || iMouseX > xrange)
	//	return;
	if (!m_controlMouse)return;
	int numDegrees = event->delta() / 120.0f;
	double scaf = numDegrees > 0 ? ScrollStep : -ScrollStep;
	double lastscale = curScale;
	curScale =curScale+ scaf;
	curScale = (double)Clip3<double>(Zoommin, Zoommax, curScale);
	scale(1/lastscale, 1/lastscale);
	scale(curScale, curScale);
	int iTransX = mapToScene(event->pos()).x() - m_iMousePressX;
	int iTransY = mapToScene(event->pos()).y() - m_iMousePressY;
	imagegroup.setPos(m_iMousePressImageX + iTransX,
		m_iMousePressImageY + iTransY);
	//imagegroup.moveBy(iImgX>>1, iImgY>>1);
// 	int iImgX = imagegroup.scenePos().x();
// 	int iImgY = imagegroup.scenePos().y();
// 	imagegroup.moveBy((iImgX - iMouseX)*ScrollStep / curScale,
// 		(iImgY - iMouseY)*ScrollStep / curScale);
}

void lmGraphView::mousePressEvent(QMouseEvent * event)
{
// 	int cx = imagegroup.x();
// 	int cy = imagegroup.y();
// 	if ((cx>=0&&cx<imageWidth)&& (cy >= 0 && cy<imageHeight))
// 	{
		if (!m_controlMouse)return;
		m_iMousePressX = mapToScene(event->pos()).x();
		m_iMousePressY = mapToScene(event->pos()).y();
		m_iMousePressImageX = imagegroup.x();
		m_iMousePressImageY = imagegroup.y();
		//int iMouseX = mapToScene(event->pos()).x();
		//int iMouseY = mapToScene(event->pos()).y();

}

void lmGraphView::mouseMoveEvent(QMouseEvent * event)
{
	if (!m_controlMouse)return;
	int iTransX = mapToScene(event->pos()).x() - m_iMousePressX;
	int iTransY = mapToScene(event->pos()).y() - m_iMousePressY;
	imagegroup.setPos(m_iMousePressImageX + iTransX,
		m_iMousePressImageY + iTransY);
}

