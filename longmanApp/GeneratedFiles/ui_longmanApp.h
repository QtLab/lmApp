/********************************************************************************
** Form generated from reading UI file 'longmanApp.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LONGMANAPP_H
#define UI_LONGMANAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_longmanAppClass
{
public:
    QAction *actionOpen;
    QAction *actionSave_as_image;
    QAction *actionAbout;
    QAction *action;
    QAction *actionAboutQT;
    QAction *actionOpen_SHVC_bitstream;
    QWidget *centralWidget;
    QGridLayout *gridLayout_7;
    QGridLayout *gridLayout_8;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout;
    QMdiArea *mdiArea;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuAbout;
    QStatusBar *statusBar;
    QDockWidget *dockWidget_2;
    QWidget *dockWidgetContents_2;
    QHBoxLayout *horizontalLayout_6;
    QGroupBox *YUVgroupBox;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *playButton;
    QPushButton *stopButton;
    QPushButton *nextButton;
    QPushButton *backButton;
    QPushButton *beginButton;
    QPushButton *endButton;
    QVBoxLayout *verticalLayout;
    QSlider *FrameIdxSlider;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSpinBox *FrameIdxBox;
    QLabel *label1_2;
    QSpinBox *MaxFraBox;
    QLabel *label1;
    QSpinBox *FrameRateBox;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout_4;
    QLabel *label_5;
    QSpinBox *spinBoxwid;
    QLabel *label_6;
    QSpinBox *spinBoxhei;
    QLabel *label_2;
    QLabel *label_format;
    QDockWidget *dockWidget_1;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *FucAeraBox;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout;
    QFormLayout *formLayout;
    QPushButton *f1Button;
    QPushButton *f2Button;
    QPushButton *f3Button;
    QPushButton *f4Button;
    QPushButton *f5Button;
    QPushButton *f6Button;
    QPushButton *f7Button;
    QPushButton *f8Button;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout_6;
    QFrame *frame;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_3;
    QLCDNumber *lcdNumber;
    QFrame *line;

    void setupUi(QMainWindow *longmanAppClass)
    {
        if (longmanAppClass->objectName().isEmpty())
            longmanAppClass->setObjectName(QStringLiteral("longmanAppClass"));
        longmanAppClass->resize(1024, 768);
        longmanAppClass->setMinimumSize(QSize(1024, 768));
        actionOpen = new QAction(longmanAppClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSave_as_image = new QAction(longmanAppClass);
        actionSave_as_image->setObjectName(QStringLiteral("actionSave_as_image"));
        actionAbout = new QAction(longmanAppClass);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        action = new QAction(longmanAppClass);
        action->setObjectName(QStringLiteral("action"));
        actionAboutQT = new QAction(longmanAppClass);
        actionAboutQT->setObjectName(QStringLiteral("actionAboutQT"));
        actionOpen_SHVC_bitstream = new QAction(longmanAppClass);
        actionOpen_SHVC_bitstream->setObjectName(QStringLiteral("actionOpen_SHVC_bitstream"));
        centralWidget = new QWidget(longmanAppClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setMinimumSize(QSize(768, 640));
        gridLayout_7 = new QGridLayout(centralWidget);
        gridLayout_7->setSpacing(6);
        gridLayout_7->setContentsMargins(11, 11, 11, 11);
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        gridLayout_8 = new QGridLayout();
        gridLayout_8->setSpacing(6);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));

        gridLayout_7->addLayout(gridLayout_8, 1, 0, 1, 1);

        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout_7->addWidget(line_2, 1, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        mdiArea = new QMdiArea(centralWidget);
        mdiArea->setObjectName(QStringLiteral("mdiArea"));

        horizontalLayout->addWidget(mdiArea);


        gridLayout_7->addLayout(horizontalLayout, 0, 0, 1, 1);

        longmanAppClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(longmanAppClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1024, 23));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QStringLiteral("menuAbout"));
        longmanAppClass->setMenuBar(menuBar);
        statusBar = new QStatusBar(longmanAppClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        longmanAppClass->setStatusBar(statusBar);
        dockWidget_2 = new QDockWidget(longmanAppClass);
        dockWidget_2->setObjectName(QStringLiteral("dockWidget_2"));
        dockWidget_2->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
        dockWidget_2->setAllowedAreas(Qt::NoDockWidgetArea);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        horizontalLayout_6 = new QHBoxLayout(dockWidgetContents_2);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        YUVgroupBox = new QGroupBox(dockWidgetContents_2);
        YUVgroupBox->setObjectName(QStringLiteral("YUVgroupBox"));
        YUVgroupBox->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(YUVgroupBox->sizePolicy().hasHeightForWidth());
        YUVgroupBox->setSizePolicy(sizePolicy);
        gridLayout_3 = new QGridLayout(YUVgroupBox);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        playButton = new QPushButton(YUVgroupBox);
        playButton->setObjectName(QStringLiteral("playButton"));

        horizontalLayout_2->addWidget(playButton);

        stopButton = new QPushButton(YUVgroupBox);
        stopButton->setObjectName(QStringLiteral("stopButton"));

        horizontalLayout_2->addWidget(stopButton);

        nextButton = new QPushButton(YUVgroupBox);
        nextButton->setObjectName(QStringLiteral("nextButton"));

        horizontalLayout_2->addWidget(nextButton);

        backButton = new QPushButton(YUVgroupBox);
        backButton->setObjectName(QStringLiteral("backButton"));

        horizontalLayout_2->addWidget(backButton);

        beginButton = new QPushButton(YUVgroupBox);
        beginButton->setObjectName(QStringLiteral("beginButton"));

        horizontalLayout_2->addWidget(beginButton);

        endButton = new QPushButton(YUVgroupBox);
        endButton->setObjectName(QStringLiteral("endButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(endButton->sizePolicy().hasHeightForWidth());
        endButton->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(endButton);


        gridLayout_3->addLayout(horizontalLayout_2, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        FrameIdxSlider = new QSlider(YUVgroupBox);
        FrameIdxSlider->setObjectName(QStringLiteral("FrameIdxSlider"));
        FrameIdxSlider->setMinimum(-1);
        FrameIdxSlider->setPageStep(30);
        FrameIdxSlider->setValue(-1);
        FrameIdxSlider->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(FrameIdxSlider);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label = new QLabel(YUVgroupBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_3->addWidget(label);

        FrameIdxBox = new QSpinBox(YUVgroupBox);
        FrameIdxBox->setObjectName(QStringLiteral("FrameIdxBox"));
        FrameIdxBox->setEnabled(false);
        FrameIdxBox->setMinimum(-1);
        FrameIdxBox->setMaximum(999999);
        FrameIdxBox->setValue(-1);

        horizontalLayout_3->addWidget(FrameIdxBox);

        label1_2 = new QLabel(YUVgroupBox);
        label1_2->setObjectName(QStringLiteral("label1_2"));

        horizontalLayout_3->addWidget(label1_2);

        MaxFraBox = new QSpinBox(YUVgroupBox);
        MaxFraBox->setObjectName(QStringLiteral("MaxFraBox"));
        MaxFraBox->setEnabled(false);
        MaxFraBox->setMinimum(-1);
        MaxFraBox->setMaximum(999999);
        MaxFraBox->setValue(-1);

        horizontalLayout_3->addWidget(MaxFraBox);

        label1 = new QLabel(YUVgroupBox);
        label1->setObjectName(QStringLiteral("label1"));

        horizontalLayout_3->addWidget(label1);

        FrameRateBox = new QSpinBox(YUVgroupBox);
        FrameRateBox->setObjectName(QStringLiteral("FrameRateBox"));
        FrameRateBox->setEnabled(true);
        FrameRateBox->setMinimum(1);
        FrameRateBox->setMaximum(50);
        FrameRateBox->setValue(30);

        horizontalLayout_3->addWidget(FrameRateBox);


        verticalLayout->addLayout(horizontalLayout_3);


        gridLayout_3->addLayout(verticalLayout, 0, 1, 1, 1);


        horizontalLayout_6->addWidget(YUVgroupBox);

        groupBox_3 = new QGroupBox(dockWidgetContents_2);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        gridLayout_5 = new QGridLayout(groupBox_3);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setSpacing(6);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout_4->addWidget(label_5, 1, 0, 1, 1);

        spinBoxwid = new QSpinBox(groupBox_3);
        spinBoxwid->setObjectName(QStringLiteral("spinBoxwid"));
        spinBoxwid->setEnabled(false);
        spinBoxwid->setMaximum(2560);

        gridLayout_4->addWidget(spinBoxwid, 0, 1, 1, 1);

        label_6 = new QLabel(groupBox_3);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout_4->addWidget(label_6, 0, 0, 1, 1);

        spinBoxhei = new QSpinBox(groupBox_3);
        spinBoxhei->setObjectName(QStringLiteral("spinBoxhei"));
        spinBoxhei->setEnabled(false);
        spinBoxhei->setMaximum(1600);

        gridLayout_4->addWidget(spinBoxhei, 1, 1, 1, 1);

        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_4->addWidget(label_2, 0, 2, 1, 1);

        label_format = new QLabel(groupBox_3);
        label_format->setObjectName(QStringLiteral("label_format"));

        gridLayout_4->addWidget(label_format, 0, 3, 1, 1);


        gridLayout_5->addLayout(gridLayout_4, 0, 0, 1, 1);


        horizontalLayout_6->addWidget(groupBox_3);

        dockWidget_2->setWidget(dockWidgetContents_2);
        longmanAppClass->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidget_2);
        dockWidget_1 = new QDockWidget(longmanAppClass);
        dockWidget_1->setObjectName(QStringLiteral("dockWidget_1"));
        dockWidget_1->setFeatures(QDockWidget::DockWidgetVerticalTitleBar);
        dockWidget_1->setAllowedAreas(Qt::NoDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        FucAeraBox = new QGroupBox(dockWidgetContents);
        FucAeraBox->setObjectName(QStringLiteral("FucAeraBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(FucAeraBox->sizePolicy().hasHeightForWidth());
        FucAeraBox->setSizePolicy(sizePolicy2);
        FucAeraBox->setAlignment(Qt::AlignCenter);
        FucAeraBox->setFlat(true);
        verticalLayout_3 = new QVBoxLayout(FucAeraBox);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        groupBox_2 = new QGroupBox(FucAeraBox);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        gridLayout = new QGridLayout(groupBox_2);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        f1Button = new QPushButton(groupBox_2);
        f1Button->setObjectName(QStringLiteral("f1Button"));

        formLayout->setWidget(0, QFormLayout::LabelRole, f1Button);

        f2Button = new QPushButton(groupBox_2);
        f2Button->setObjectName(QStringLiteral("f2Button"));

        formLayout->setWidget(0, QFormLayout::FieldRole, f2Button);

        f3Button = new QPushButton(groupBox_2);
        f3Button->setObjectName(QStringLiteral("f3Button"));

        formLayout->setWidget(1, QFormLayout::LabelRole, f3Button);

        f4Button = new QPushButton(groupBox_2);
        f4Button->setObjectName(QStringLiteral("f4Button"));

        formLayout->setWidget(1, QFormLayout::FieldRole, f4Button);

        f5Button = new QPushButton(groupBox_2);
        f5Button->setObjectName(QStringLiteral("f5Button"));

        formLayout->setWidget(2, QFormLayout::LabelRole, f5Button);

        f6Button = new QPushButton(groupBox_2);
        f6Button->setObjectName(QStringLiteral("f6Button"));

        formLayout->setWidget(2, QFormLayout::FieldRole, f6Button);

        f7Button = new QPushButton(groupBox_2);
        f7Button->setObjectName(QStringLiteral("f7Button"));

        formLayout->setWidget(3, QFormLayout::LabelRole, f7Button);

        f8Button = new QPushButton(groupBox_2);
        f8Button->setObjectName(QStringLiteral("f8Button"));

        formLayout->setWidget(3, QFormLayout::FieldRole, f8Button);


        gridLayout->addLayout(formLayout, 0, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox_2);

        groupBox = new QGroupBox(FucAeraBox);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_6 = new QGridLayout();
        gridLayout_6->setSpacing(6);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));

        gridLayout_2->addLayout(gridLayout_6, 0, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox);

        frame = new QFrame(FucAeraBox);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_4 = new QVBoxLayout(frame);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label_3 = new QLabel(frame);
        label_3->setObjectName(QStringLiteral("label_3"));

        verticalLayout_4->addWidget(label_3);

        lcdNumber = new QLCDNumber(frame);
        lcdNumber->setObjectName(QStringLiteral("lcdNumber"));

        verticalLayout_4->addWidget(lcdNumber);


        verticalLayout_3->addWidget(frame);

        line = new QFrame(FucAeraBox);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_3->addWidget(line);


        verticalLayout_2->addWidget(FucAeraBox);

        dockWidget_1->setWidget(dockWidgetContents);
        longmanAppClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget_1);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave_as_image);
        menuFile->addAction(actionOpen_SHVC_bitstream);
        menuAbout->addAction(actionAbout);
        menuAbout->addAction(actionAboutQT);

        retranslateUi(longmanAppClass);
        QObject::connect(FrameIdxSlider, SIGNAL(valueChanged(int)), FrameIdxBox, SLOT(setValue(int)));

        QMetaObject::connectSlotsByName(longmanAppClass);
    } // setupUi

    void retranslateUi(QMainWindow *longmanAppClass)
    {
        longmanAppClass->setWindowTitle(QApplication::translate("longmanAppClass", "longmanApp", 0));
        actionOpen->setText(QApplication::translate("longmanAppClass", "open", 0));
        actionSave_as_image->setText(QApplication::translate("longmanAppClass", "save as image", 0));
        actionAbout->setText(QApplication::translate("longmanAppClass", "about", 0));
        action->setText(QApplication::translate("longmanAppClass", "aboutQT", 0));
        actionAboutQT->setText(QApplication::translate("longmanAppClass", "aboutQT", 0));
        actionOpen_SHVC_bitstream->setText(QApplication::translate("longmanAppClass", "open SHVC bitstream", 0));
        menuFile->setTitle(QApplication::translate("longmanAppClass", "file", 0));
        menuEdit->setTitle(QApplication::translate("longmanAppClass", "edit", 0));
        menuAbout->setTitle(QApplication::translate("longmanAppClass", "about", 0));
        dockWidget_2->setWindowTitle(QApplication::translate("longmanAppClass", "Player Area", 0));
        YUVgroupBox->setTitle(QApplication::translate("longmanAppClass", "Player Control", 0));
#ifndef QT_NO_STATUSTIP
        playButton->setStatusTip(QApplication::translate("longmanAppClass", "\345\277\253\346\215\267\351\224\256\342\200\234Space\342\200\235", 0));
#endif // QT_NO_STATUSTIP
        playButton->setText(QApplication::translate("longmanAppClass", "Paly", 0));
        playButton->setShortcut(QApplication::translate("longmanAppClass", "Space", 0));
        stopButton->setText(QApplication::translate("longmanAppClass", "Stop", 0));
        nextButton->setText(QApplication::translate("longmanAppClass", "Next", 0));
        backButton->setText(QApplication::translate("longmanAppClass", "Back", 0));
        beginButton->setText(QApplication::translate("longmanAppClass", "Home", 0));
        endButton->setText(QApplication::translate("longmanAppClass", "End", 0));
        label->setText(QApplication::translate("longmanAppClass", "Current Frame:", 0));
        label1_2->setText(QApplication::translate("longmanAppClass", "Total Frame:", 0));
        label1->setText(QApplication::translate("longmanAppClass", "Frame Rate:", 0));
        groupBox_3->setTitle(QApplication::translate("longmanAppClass", "Image Info", 0));
        label_5->setText(QApplication::translate("longmanAppClass", "height", 0));
        label_6->setText(QApplication::translate("longmanAppClass", "width", 0));
        label_2->setText(QApplication::translate("longmanAppClass", "format", 0));
        label_format->setText(QApplication::translate("longmanAppClass", "420", 0));
        dockWidget_1->setWindowTitle(QApplication::translate("longmanAppClass", "Function Area", 0));
        groupBox_2->setTitle(QApplication::translate("longmanAppClass", "Function Area", 0));
        f1Button->setText(QApplication::translate("longmanAppClass", "show Pixel", 0));
        f2Button->setText(QApplication::translate("longmanAppClass", "open yuv", 0));
        f3Button->setText(QApplication::translate("longmanAppClass", "save image", 0));
        f4Button->setText(QApplication::translate("longmanAppClass", "open bitstr", 0));
        f5Button->setText(QApplication::translate("longmanAppClass", "f5", 0));
        f6Button->setText(QApplication::translate("longmanAppClass", "f6", 0));
        f7Button->setText(QApplication::translate("longmanAppClass", "f7", 0));
        f8Button->setText(QApplication::translate("longmanAppClass", "f8", 0));
        groupBox->setTitle(QApplication::translate("longmanAppClass", "List Area", 0));
        label_3->setText(QApplication::translate("longmanAppClass", "wait dev...", 0));
    } // retranslateUi

};

namespace Ui {
    class longmanAppClass: public Ui_longmanAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LONGMANAPP_H
