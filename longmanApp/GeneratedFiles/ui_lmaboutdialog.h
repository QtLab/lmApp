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
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_lmAboutDialog
{
public:
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer_2;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_8;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_9;
    QLabel *label_10;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *lmAboutDialog)
    {
        if (lmAboutDialog->objectName().isEmpty())
            lmAboutDialog->setObjectName(QStringLiteral("lmAboutDialog"));
        lmAboutDialog->resize(329, 287);
        verticalLayout = new QVBoxLayout(lmAboutDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalSpacer_2 = new QSpacerItem(292, 26, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        label = new QLabel(lmAboutDialog);
        label->setObjectName(QStringLiteral("label"));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\256\213\344\275\223"));
        font.setPointSize(16);
        font.setBold(true);
        font.setWeight(75);
        label->setFont(font);
        label->setTextFormat(Qt::PlainText);
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        label_2 = new QLabel(lmAboutDialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_2);

        label_3 = new QLabel(lmAboutDialog);
        label_3->setObjectName(QStringLiteral("label_3"));
        QFont font1;
        font1.setPointSize(11);
        label_3->setFont(font1);
        label_3->setAlignment(Qt::AlignCenter);
        label_3->setOpenExternalLinks(true);

        verticalLayout->addWidget(label_3);

        label_8 = new QLabel(lmAboutDialog);
        label_8->setObjectName(QStringLiteral("label_8"));
        QFont font2;
        font2.setPointSize(10);
        label_8->setFont(font2);
        label_8->setAlignment(Qt::AlignCenter);
        label_8->setOpenExternalLinks(true);

        verticalLayout->addWidget(label_8);

        label_4 = new QLabel(lmAboutDialog);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setFont(font2);
        label_4->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_4);

        label_5 = new QLabel(lmAboutDialog);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setFont(font2);
        label_5->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_5);

        label_6 = new QLabel(lmAboutDialog);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setFont(font2);
        label_6->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_6);

        label_7 = new QLabel(lmAboutDialog);
        label_7->setObjectName(QStringLiteral("label_7"));
        QFont font3;
        font3.setFamily(QString::fromUtf8("\345\256\213\344\275\223"));
        font3.setPointSize(20);
        font3.setBold(true);
        font3.setWeight(75);
        label_7->setFont(font3);
        label_7->setTextFormat(Qt::PlainText);
        label_7->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_7);

        label_9 = new QLabel(lmAboutDialog);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setFont(font2);
        label_9->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_9);

        label_10 = new QLabel(lmAboutDialog);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setFont(font2);
        label_10->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label_10);

        verticalSpacer = new QSpacerItem(292, 26, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(lmAboutDialog);

        QMetaObject::connectSlotsByName(lmAboutDialog);
    } // setupUi

    void retranslateUi(QDialog *lmAboutDialog)
    {
        lmAboutDialog->setWindowTitle(QApplication::translate("lmAboutDialog", "Dialog", 0));
        label->setText(QApplication::translate("lmAboutDialog", "LM SHVC bitstream player", 0));
        label_2->setText(QApplication::translate("lmAboutDialog", "Version 0.9.1", 0));
        label_3->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p>Author: Longman Lee (\346\235\216\351\207\221\351\276\231)</p></body></html>", 0));
        label_8->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"651138628@qq.com\">\344\275\234\350\200\205\351\202\256\347\256\261\357\274\232651138628@qq.com</span></a></p></body></html>", 0));
        label_4->setText(QApplication::translate("lmAboutDialog", "Ningbo University, Ningbo", 0));
        label_5->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"https://github.com/LongmanLee/lmApp\"><span style=\" text-decoration: underline; color:#000000;\">\351\241\271\347\233\256\344\270\273\351\241\265(\345\217\263\351\224\256\345\244\215\345\210\266)</span></a></p></body></html>", 0));
        label_6->setText(QApplication::translate("lmAboutDialog", "https://github.com/LongmanLee/lmApp", 0));
        label_7->setText(QApplication::translate("lmAboutDialog", "\346\263\250\346\204\217", 0));
        label_9->setText(QApplication::translate("lmAboutDialog", "\350\213\245\345\256\211\350\243\205\345\234\250C\347\233\230\357\274\214\350\257\267\344\273\245\347\256\241\347\220\206\345\221\230\350\272\253\344\273\275\350\277\220\350\241\214", 0));
        label_10->setText(QApplication::translate("lmAboutDialog", "\346\216\250\350\215\220\345\256\211\350\243\205\345\234\250\351\235\236\347\263\273\347\273\237\347\233\230", 0));
    } // retranslateUi

};

namespace Ui {
    class lmAboutDialog: public Ui_lmAboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMABOUTDIALOG_H
