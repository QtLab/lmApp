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
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *lmAboutDialog)
    {
        if (lmAboutDialog->objectName().isEmpty())
            lmAboutDialog->setObjectName(QStringLiteral("lmAboutDialog"));
        lmAboutDialog->resize(329, 211);
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
        label_8->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"651138628@qq.com\"><span style=\" text-decoration: underline; color:#00007f;\">\344\275\234\350\200\205\351\202\256\347\256\261</span></a></p></body></html>", 0));
        label_4->setText(QApplication::translate("lmAboutDialog", "Ningbo University, Ningbo", 0));
        label_5->setText(QApplication::translate("lmAboutDialog", "<html><head/><body><p><a href=\"https://github.com/LongmanLee/lmApp\"><span style=\" text-decoration: underline; color:#0000ff;\">\351\241\271\347\233\256\344\270\273\351\241\265</span></a></p></body></html>", 0));
    } // retranslateUi

};

namespace Ui {
    class lmAboutDialog: public Ui_lmAboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMABOUTDIALOG_H
