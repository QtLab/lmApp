/********************************************************************************
** Form generated from reading UI file 'lmaboutdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMABOUTDIALOG_H
#define UI_LMABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_lmAboutDialog
{
public:
    QGridLayout *gridLayout;
    QFrame *frame_2;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_2;
    QSpacerItem *verticalSpacer;
    QLabel *label_10;
    QLabel *label_9;
    QSpacerItem *verticalSpacer_2;
    QFrame *frame;
    QGridLayout *gridLayout_2;
    QLabel *label_7;
    QLabel *label_11;
    QFrame *frame_3;
    QGridLayout *gridLayout_4;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_8;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;

    void setupUi(QDialog *lmAboutDialog)
    {
        if (lmAboutDialog->objectName().isEmpty())
            lmAboutDialog->setObjectName(QStringLiteral("lmAboutDialog"));
        lmAboutDialog->resize(450, 280);
        lmAboutDialog->setMinimumSize(QSize(450, 280));
        lmAboutDialog->setMaximumSize(QSize(450, 280));
        gridLayout = new QGridLayout(lmAboutDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        frame_2 = new QFrame(lmAboutDialog);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        gridLayout_3 = new QGridLayout(frame_2);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        label_10 = new QLabel(frame_2);
        label_10->setObjectName(QStringLiteral("label_10"));
        QFont font;
        font.setPointSize(10);
        label_10->setFont(font);
        label_10->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_10);

        label_9 = new QLabel(frame_2);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setFont(font);
        label_9->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(label_9);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer_2);


        gridLayout_3->addLayout(verticalLayout_2, 0, 0, 1, 1);


        gridLayout->addWidget(frame_2, 1, 1, 1, 1);

        frame = new QFrame(lmAboutDialog);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout_2 = new QGridLayout(frame);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_7 = new QLabel(frame);
        label_7->setObjectName(QStringLiteral("label_7"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\256\213\344\275\223"));
        font1.setPointSize(20);
        font1.setBold(true);
        font1.setWeight(75);
        label_7->setFont(font1);
        label_7->setTextFormat(Qt::PlainText);
        label_7->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_7, 0, 0, 1, 1);


        gridLayout->addWidget(frame, 1, 0, 1, 1);

        label_11 = new QLabel(lmAboutDialog);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setFrameShape(QFrame::Box);
        label_11->setFrameShadow(QFrame::Raised);
        label_11->setLineWidth(4);
        label_11->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_11, 0, 0, 1, 1);

        frame_3 = new QFrame(lmAboutDialog);
        frame_3->setObjectName(QStringLiteral("frame_3"));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        gridLayout_4 = new QGridLayout(frame_3);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(frame_3);
        label->setObjectName(QStringLiteral("label"));
        QFont font2;
        font2.setFamily(QString::fromUtf8("\345\256\213\344\275\223"));
        font2.setPointSize(12);
        font2.setBold(true);
        font2.setWeight(75);
        label->setFont(font2);
        label->setTextFormat(Qt::PlainText);
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        label_2 = new QLabel(frame_3);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_2);

        label_3 = new QLabel(frame_3);
        label_3->setObjectName(QStringLiteral("label_3"));
        QFont font3;
        font3.setPointSize(11);
        label_3->setFont(font3);
        label_3->setAlignment(Qt::AlignCenter);
        label_3->setOpenExternalLinks(true);

        verticalLayout->addWidget(label_3);

        label_8 = new QLabel(frame_3);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setFont(font);
        label_8->setAlignment(Qt::AlignCenter);
        label_8->setOpenExternalLinks(true);

        verticalLayout->addWidget(label_8);

        label_4 = new QLabel(frame_3);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_4);

        label_5 = new QLabel(frame_3);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setFont(font);
        label_5->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_5);

        label_6 = new QLabel(frame_3);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setFont(font);
        label_6->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_6);


        gridLayout_4->addLayout(verticalLayout, 0, 0, 1, 1);


        gridLayout->addWidget(frame_3, 0, 1, 1, 1);

        label_11->raise();
        frame->raise();
        frame_2->raise();
        frame_3->raise();

        retranslateUi(lmAboutDialog);

        QMetaObject::connectSlotsByName(lmAboutDialog);
    } // setupUi

    void retranslateUi(QDialog *lmAboutDialog)
    {
        lmAboutDialog->setWindowTitle(QApplication::translate("lmAboutDialog", "Dialog", 0));
        label_10->setText(QApplication::translate("lmAboutDialog", "\346\216\250\350\215\220\345\256\211\350\243\205\345\234\250\351\235\236\347\263\273\347\273\237\347\233\230", 0));
        label_9->setText(QApplication::translate("lmAboutDialog", "\350\213\245\345\256\211\350\243\205\345\234\250C\347\233\230\357\274\214\350\257\267\344\273\245\347\256\241\347\220\206\345\221\230\350\272\253\344\273\275\350\277\220\350\241\214", 0));
        label_7->setText(QApplication::translate("lmAboutDialog", "\346\263\250\346\204\217", 0));
        label_11->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><img src=\":/appicon/app.ico\"/></p></body></html>", 0));
        label->setText(QApplication::translate("lmAboutDialog", "LM SHVC bitstream player", 0));
        label_2->setText(QApplication::translate("lmAboutDialog", "Version 0.9.1", 0));
        label_3->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p>\344\275\234\350\200\205: Longman Lee (\346\235\216\351\207\221\351\276\231)</p></body></html>", 0));
        label_8->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"651138628@qq.com\">\344\275\234\350\200\205\351\202\256\347\256\261\357\274\232651138628@qq.com</span></a></p></body></html>", 0));
        label_4->setText(QApplication::translate("lmAboutDialog", "Ningbo University, Ningbo", 0));
        label_5->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"https://github.com/LongmanLee/lmApp\"><span style=\" text-decoration: underline; color:#000000;\">\351\241\271\347\233\256\344\270\273\351\241\265(\345\217\263\351\224\256\345\244\215\345\210\266)</span></a></p></body></html>", 0));
        label_6->setText(QApplication::translate("lmAboutDialog", "https://github.com/LongmanLee/lmApp", 0));
    } // retranslateUi

};

namespace Ui {
    class lmAboutDialog: public Ui_lmAboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMABOUTDIALOG_H
