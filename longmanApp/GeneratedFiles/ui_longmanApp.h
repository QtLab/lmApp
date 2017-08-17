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
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
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
    QVBoxLayout *verticalLayout_6;
    QGridLayout *gridLayout_8;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuAbout;
    QStatusBar *statusBar;
    QDockWidget *dockWidget_2;
    QWidget *dockWidgetContents_2;
    QHBoxLayout *horizontalLayout_6;
    QTabWidget *tabWidget;
    QWidget *tab_1;
    QGridLayout *gridLayout_11;
    QWidget *tab_5;
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *YUVgroupBox;
    QGridLayout *gridLayout_3;
    QGridLayout *gridLayout;
    QPushButton *beginButton;
    QPushButton *playButton;
    QPushButton *stopButton;
    QPushButton *nextButton;
    QPushButton *backButton;
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
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout_2;
    QPushButton *f2Button;
    QPushButton *f6Button;
    QPushButton *f3Button;
    QPushButton *f1Button;
    QPushButton *f4Button;
    QPushButton *f5Button;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents_3;
    QVBoxLayout *verticalLayout_7;
    QFrame *line_2;
    QGridLayout *gridLayout_6;
    QFrame *line;
    QGridLayout *gridLayout_10;

    void setupUi(QMainWindow *longmanAppClass)
    {
        if (longmanAppClass->objectName().isEmpty())
            longmanAppClass->setObjectName(QStringLiteral("longmanAppClass"));
        longmanAppClass->resize(1113, 853);
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
        verticalLayout_6 = new QVBoxLayout(centralWidget);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        gridLayout_8 = new QGridLayout();
        gridLayout_8->setSpacing(6);
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));

        verticalLayout_6->addLayout(gridLayout_8);

        longmanAppClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(longmanAppClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1113, 23));
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
        dockWidget_2->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidget_2->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::TopDockWidgetArea);
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QStringLiteral("dockWidgetContents_2"));
        horizontalLayout_6 = new QHBoxLayout(dockWidgetContents_2);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        tabWidget = new QTabWidget(dockWidgetContents_2);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setTabShape(QTabWidget::Triangular);
        tab_1 = new QWidget();
        tab_1->setObjectName(QStringLiteral("tab_1"));
        gridLayout_11 = new QGridLayout(tab_1);
        gridLayout_11->setSpacing(6);
        gridLayout_11->setContentsMargins(11, 11, 11, 11);
        gridLayout_11->setObjectName(QStringLiteral("gridLayout_11"));
        tabWidget->addTab(tab_1, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        horizontalLayout_2 = new QHBoxLayout(tab_5);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        YUVgroupBox = new QGroupBox(tab_5);
        YUVgroupBox->setObjectName(QStringLiteral("YUVgroupBox"));
        YUVgroupBox->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(YUVgroupBox->sizePolicy().hasHeightForWidth());
        YUVgroupBox->setSizePolicy(sizePolicy);
        YUVgroupBox->setAlignment(Qt::AlignCenter);
        gridLayout_3 = new QGridLayout(YUVgroupBox);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        beginButton = new QPushButton(YUVgroupBox);
        beginButton->setObjectName(QStringLiteral("beginButton"));

        gridLayout->addWidget(beginButton, 0, 4, 1, 1);

        playButton = new QPushButton(YUVgroupBox);
        playButton->setObjectName(QStringLiteral("playButton"));

        gridLayout->addWidget(playButton, 0, 0, 1, 1);

        stopButton = new QPushButton(YUVgroupBox);
        stopButton->setObjectName(QStringLiteral("stopButton"));

        gridLayout->addWidget(stopButton, 1, 0, 1, 1);

        nextButton = new QPushButton(YUVgroupBox);
        nextButton->setObjectName(QStringLiteral("nextButton"));

        gridLayout->addWidget(nextButton, 0, 1, 1, 1);

        backButton = new QPushButton(YUVgroupBox);
        backButton->setObjectName(QStringLiteral("backButton"));

        gridLayout->addWidget(backButton, 1, 1, 1, 1);

        endButton = new QPushButton(YUVgroupBox);
        endButton->setObjectName(QStringLiteral("endButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(endButton->sizePolicy().hasHeightForWidth());
        endButton->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(endButton, 1, 4, 1, 1);


        gridLayout_3->addLayout(gridLayout, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        FrameIdxSlider = new QSlider(YUVgroupBox);
        FrameIdxSlider->setObjectName(QStringLiteral("FrameIdxSlider"));
        FrameIdxSlider->setPageStep(30);
        FrameIdxSlider->setValue(0);
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
        FrameIdxBox->setMaximum(999999);
        FrameIdxBox->setValue(0);

        horizontalLayout_3->addWidget(FrameIdxBox);

        label1_2 = new QLabel(YUVgroupBox);
        label1_2->setObjectName(QStringLiteral("label1_2"));

        horizontalLayout_3->addWidget(label1_2);

        MaxFraBox = new QSpinBox(YUVgroupBox);
        MaxFraBox->setObjectName(QStringLiteral("MaxFraBox"));
        MaxFraBox->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(MaxFraBox->sizePolicy().hasHeightForWidth());
        MaxFraBox->setSizePolicy(sizePolicy2);
        MaxFraBox->setMaximum(999999);
        MaxFraBox->setValue(0);

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


        horizontalLayout_2->addWidget(YUVgroupBox);

        groupBox_3 = new QGroupBox(tab_5);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setAlignment(Qt::AlignCenter);
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
        label_format->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_format, 1, 2, 1, 1);


        gridLayout_5->addLayout(gridLayout_4, 0, 0, 1, 1);


        horizontalLayout_2->addWidget(groupBox_3);

        groupBox_2 = new QGroupBox(tab_5);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setAlignment(Qt::AlignCenter);
        horizontalLayout = new QHBoxLayout(groupBox_2);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        f2Button = new QPushButton(groupBox_2);
        f2Button->setObjectName(QStringLiteral("f2Button"));

        gridLayout_2->addWidget(f2Button, 0, 0, 1, 1);

        f6Button = new QPushButton(groupBox_2);
        f6Button->setObjectName(QStringLiteral("f6Button"));

        gridLayout_2->addWidget(f6Button, 0, 1, 1, 1);

        f3Button = new QPushButton(groupBox_2);
        f3Button->setObjectName(QStringLiteral("f3Button"));

        gridLayout_2->addWidget(f3Button, 0, 2, 1, 1);

        f1Button = new QPushButton(groupBox_2);
        f1Button->setObjectName(QStringLiteral("f1Button"));

        gridLayout_2->addWidget(f1Button, 1, 1, 1, 1);

        f4Button = new QPushButton(groupBox_2);
        f4Button->setObjectName(QStringLiteral("f4Button"));

        gridLayout_2->addWidget(f4Button, 1, 0, 1, 1);

        f5Button = new QPushButton(groupBox_2);
        f5Button->setObjectName(QStringLiteral("f5Button"));

        gridLayout_2->addWidget(f5Button, 1, 2, 1, 1);


        horizontalLayout->addLayout(gridLayout_2);


        horizontalLayout_2->addWidget(groupBox_2);

        tabWidget->addTab(tab_5, QString());

        horizontalLayout_6->addWidget(tabWidget);

        dockWidget_2->setWidget(dockWidgetContents_2);
        longmanAppClass->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidget_2);
        dockWidget = new QDockWidget(longmanAppClass);
        dockWidget->setObjectName(QStringLiteral("dockWidget"));
        dockWidget->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
        dockWidgetContents_3 = new QWidget();
        dockWidgetContents_3->setObjectName(QStringLiteral("dockWidgetContents_3"));
        verticalLayout_7 = new QVBoxLayout(dockWidgetContents_3);
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        line_2 = new QFrame(dockWidgetContents_3);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_7->addWidget(line_2);

        gridLayout_6 = new QGridLayout();
        gridLayout_6->setSpacing(6);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));

        verticalLayout_7->addLayout(gridLayout_6);

        line = new QFrame(dockWidgetContents_3);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_7->addWidget(line);

        gridLayout_10 = new QGridLayout();
        gridLayout_10->setSpacing(6);
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));

        verticalLayout_7->addLayout(gridLayout_10);

        dockWidget->setWidget(dockWidgetContents_3);
        longmanAppClass->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

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

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(longmanAppClass);
    } // setupUi

    void retranslateUi(QMainWindow *longmanAppClass)
    {
        longmanAppClass->setWindowTitle(QApplication::translate("longmanAppClass", "longmanApp", 0));
        actionOpen->setText(QApplication::translate("longmanAppClass", "\346\211\223\345\274\200yuv", 0));
        actionSave_as_image->setText(QApplication::translate("longmanAppClass", "\344\277\235\345\255\230\344\270\272\345\233\276\347\211\207", 0));
        actionAbout->setText(QApplication::translate("longmanAppClass", "\345\205\263\344\272\216", 0));
        action->setText(QApplication::translate("longmanAppClass", "aboutQT", 0));
        actionAboutQT->setText(QApplication::translate("longmanAppClass", "aboutQT", 0));
        actionOpen_SHVC_bitstream->setText(QApplication::translate("longmanAppClass", "\346\211\223\345\274\200shvc\347\240\201\346\265\201", 0));
        menuFile->setTitle(QApplication::translate("longmanAppClass", "\346\226\207\344\273\266", 0));
        menuEdit->setTitle(QApplication::translate("longmanAppClass", "\347\274\226\350\276\221", 0));
        menuAbout->setTitle(QApplication::translate("longmanAppClass", "\345\205\263\344\272\216", 0));
        dockWidget_2->setWindowTitle(QApplication::translate("longmanAppClass", "\345\212\237\350\203\275\347\252\227\345\217\243", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_1), QApplication::translate("longmanAppClass", "\345\270\247\344\277\241\346\201\257", 0));
        YUVgroupBox->setTitle(QApplication::translate("longmanAppClass", "yuv \346\222\255\346\224\276", 0));
        beginButton->setText(QApplication::translate("longmanAppClass", "\345\244\215\344\275\215", 0));
#ifndef QT_NO_STATUSTIP
        playButton->setStatusTip(QApplication::translate("longmanAppClass", "\345\277\253\346\215\267\351\224\256\342\200\234Space\342\200\235", 0));
#endif // QT_NO_STATUSTIP
        playButton->setText(QApplication::translate("longmanAppClass", "\346\222\255\346\224\276", 0));
        playButton->setShortcut(QApplication::translate("longmanAppClass", "Space", 0));
        stopButton->setText(QApplication::translate("longmanAppClass", "\345\201\234\346\255\242", 0));
        nextButton->setText(QApplication::translate("longmanAppClass", "\346\255\245\350\277\233", 0));
        backButton->setText(QApplication::translate("longmanAppClass", "\346\255\245\351\200\200", 0));
        endButton->setText(QApplication::translate("longmanAppClass", "\346\234\253\345\260\276", 0));
        label->setText(QApplication::translate("longmanAppClass", "\345\275\223\345\211\215\345\270\247", 0));
        label1_2->setText(QApplication::translate("longmanAppClass", "\346\200\273\345\270\247\346\225\260", 0));
        label1->setText(QApplication::translate("longmanAppClass", "\345\270\247\347\216\207", 0));
        groupBox_3->setTitle(QApplication::translate("longmanAppClass", "\344\277\241\346\201\257", 0));
        label_5->setText(QApplication::translate("longmanAppClass", "\351\253\230\357\274\232", 0));
        label_6->setText(QApplication::translate("longmanAppClass", "\345\256\275\357\274\232", 0));
        label_2->setText(QApplication::translate("longmanAppClass", "\351\207\207\346\240\267\346\240\274\345\274\217", 0));
        label_format->setText(QApplication::translate("longmanAppClass", "420", 0));
        groupBox_2->setTitle(QApplication::translate("longmanAppClass", "\345\212\237\350\203\275", 0));
        f2Button->setText(QApplication::translate("longmanAppClass", "\346\211\223\345\274\200yuv", 0));
        f6Button->setText(QApplication::translate("longmanAppClass", "CU\345\210\206\345\211\262", 0));
        f3Button->setText(QApplication::translate("longmanAppClass", "\344\277\235\345\255\230\345\233\276\347\211\207", 0));
        f1Button->setText(QApplication::translate("longmanAppClass", "\346\230\276\347\244\272\345\203\217\347\264\240", 0));
        f4Button->setText(QApplication::translate("longmanAppClass", "\346\211\223\345\274\200\347\240\201\346\265\201", 0));
        f5Button->setText(QApplication::translate("longmanAppClass", "\346\257\224\347\211\271\345\210\206\345\270\203", 0));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QApplication::translate("longmanAppClass", "\344\270\273\350\246\201\345\212\237\350\203\275", 0));
        dockWidget->setWindowTitle(QApplication::translate("longmanAppClass", "\346\237\245\347\234\213\347\252\227\345\217\243", 0));
    } // retranslateUi

};

namespace Ui {
    class longmanAppClass: public Ui_longmanAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LONGMANAPP_H
