/********************************************************************************
** Form generated from reading UI file 'lmParserBitConfigure.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LMPARSERBITCONFIGURE_H
#define UI_LMPARSERBITCONFIGURE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_lmParserBitConfigure
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QToolButton *toolButton;
    QLabel *label_4;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *lmParserBitConfigure)
    {
        if (lmParserBitConfigure->objectName().isEmpty())
            lmParserBitConfigure->setObjectName(QStringLiteral("lmParserBitConfigure"));
        lmParserBitConfigure->resize(400, 128);
        lmParserBitConfigure->setMinimumSize(QSize(400, 128));
        lmParserBitConfigure->setMaximumSize(QSize(400, 128));
        verticalLayout = new QVBoxLayout(lmParserBitConfigure);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(lmParserBitConfigure);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(lmParserBitConfigure);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        horizontalLayout->addWidget(lineEdit);

        toolButton = new QToolButton(lmParserBitConfigure);
        toolButton->setObjectName(QStringLiteral("toolButton"));

        horizontalLayout->addWidget(toolButton);

        label_4 = new QLabel(lmParserBitConfigure);
        label_4->setObjectName(QStringLiteral("label_4"));

        horizontalLayout->addWidget(label_4);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(lmParserBitConfigure);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(lmParserBitConfigure);
        QObject::connect(buttonBox, SIGNAL(accepted()), lmParserBitConfigure, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), lmParserBitConfigure, SLOT(reject()));

        QMetaObject::connectSlotsByName(lmParserBitConfigure);
    } // setupUi

    void retranslateUi(QDialog *lmParserBitConfigure)
    {
        lmParserBitConfigure->setWindowTitle(QApplication::translate("lmParserBitConfigure", "lmParserBitConfigure", 0));
        label->setText(QApplication::translate("lmParserBitConfigure", "bitstreampath", 0));
        toolButton->setText(QApplication::translate("lmParserBitConfigure", "...", 0));
        label_4->setText(QApplication::translate("lmParserBitConfigure", "-b x", 0));
    } // retranslateUi

};

namespace Ui {
    class lmParserBitConfigure: public Ui_lmParserBitConfigure {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LMPARSERBITCONFIGURE_H
