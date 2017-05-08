#include "yuvParamSet.h"

yuvParamSet::yuvParamSet(QWidget *parent)
	: QDialog(parent),
	mHeight(240),
	mWidth(416),
	formatType(1)
{
	ui.setupUi(this);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowCloseButtonHint;
	setSizeGripEnabled(true);
	setWindowFlags(flags);
	ui.comboBox_fomat->addItem("4:0:0", 0);
	ui.comboBox_fomat->addItem("4:2:0", 1);
	ui.comboBox_fomat->addItem("4:4:4", 2);
	ui.comboBox_fomat->setCurrentIndex(1);
	connect(ui.spinBox_height, SIGNAL(valueChanged(int)), this, SLOT(setheight(int)));
	connect(ui.spinBox_width, SIGNAL(valueChanged(int)), this, SLOT(setwidth(int)));
	connect(ui.comboBox_fomat, SIGNAL(activated(int)), this, SLOT(setformat(int)));
}

yuvParamSet::~yuvParamSet()
{
}

void yuvParamSet::on_p240_clicked()
{
	//mHeight = 240;
	ui.spinBox_height->setValue(240);
}

void yuvParamSet::on_p416_clicked()
{
	//mWidth = 416;
	ui.spinBox_width->setValue(416);
}

void yuvParamSet::on_p480_clicked()
{
	//mHeight = 480;
	ui.spinBox_height->setValue(480);
}

void yuvParamSet::on_p832_clicked()
{
	//mWidth = 832;
	ui.spinBox_width->setValue(832);
}

void yuvParamSet::on_p768_clicked()
{
	//mHeight = 768;
	ui.spinBox_height->setValue(768);
}

void yuvParamSet::on_p1024_clicked()
{
	///mWidth = 1024;
	ui.spinBox_width->setValue(1024);
}

void yuvParamSet::on_p720_clicked()
{
	//mHeight = 720;
	ui.spinBox_height->setValue(720);
}

void yuvParamSet::on_p1280_clicked()
{
	//mWidth = 1280;
	ui.spinBox_width->setValue(1280);
}

void yuvParamSet::on_p1920_clicked()
{
	//mWidth = 1920;
	ui.spinBox_width->setValue(1920);
}

void yuvParamSet::on_p1080_clicked()
{
	//mHeight = 1080;
	ui.spinBox_height->setValue(1080);
}

void yuvParamSet::on_p2560_clicked()
{
	//mWidth = 2560;
	ui.spinBox_width->setValue(2560);
}

void yuvParamSet::on_p1600_clicked()
{
	//mHeight = 1600;
	ui.spinBox_height->setValue(1600);
}

void yuvParamSet::setheight(int rh)
{
	mHeight = rh;
}

void yuvParamSet::setwidth(int rw)
{
	mWidth = rw;
}

void yuvParamSet::setformat(int pformatType)
{
	formatType = pformatType;
}
