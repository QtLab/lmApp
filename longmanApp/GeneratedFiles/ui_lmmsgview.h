/********************************************************************************
** Form generated from reading UI file 'lmmsgview.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMMSGVIEW_H
#define UI_LMMSGVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_lmMsgView
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QTextBrowser *textBrowser;

    void setupUi(QWidget *lmMsgView)
    {
        if (lmMsgView->objectName().isEmpty())
            lmMsgView->setObjectName(QStringLiteral("lmMsgView"));
        lmMsgView->resize(203, 138);
        verticalLayout = new QVBoxLayout(lmMsgView);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(lmMsgView);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(lmMsgView);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);

        textBrowser = new QTextBrowser(lmMsgView);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));

        verticalLayout->addWidget(textBrowser);


        retranslateUi(lmMsgView);
        QObject::connect(pushButton, SIGNAL(clicked()), textBrowser, SLOT(clear()));

        QMetaObject::connectSlotsByName(lmMsgView);
    } // setupUi

    void retranslateUi(QWidget *lmMsgView)
    {
        lmMsgView->setWindowTitle(QApplication::translate("lmMsgView", "Form", 0));
        label->setText(QApplication::translate("lmMsgView", "MessageOuput", 0));
        pushButton->setText(QApplication::translate("lmMsgView", "clear", 0));
    } // retranslateUi

};

namespace Ui {
    class lmMsgView: public Ui_lmMsgView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMMSGVIEW_H
