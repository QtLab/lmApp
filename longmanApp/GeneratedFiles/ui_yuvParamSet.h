/********************************************************************************
** Form generated from reading UI file 'yuvParamSet.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_YUVPARAMSET_H
#define UI_YUVPARAMSET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_yuvParamSet
{
public:
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QLabel *label_2;
    QVBoxLayout *verticalLayout_2;
    QSpinBox *spinBox_width;
    QSpinBox *spinBox_height;
    QGridLayout *gridLayout;
    QPushButton *p832;
    QPushButton *p1280;
    QPushButton *p480;
    QPushButton *p416;
    QPushButton *p240;
    QPushButton *p1920;
    QPushButton *p1024;
    QPushButton *p2560;
    QPushButton *p768;
    QPushButton *p1600;
    QPushButton *p720;
    QPushButton *p1080;
    QDialogButtonBox *buttonBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QComboBox *comboBox_fomat;

    void setupUi(QDialog *yuvParamSet)
    {
        if (yuvParamSet->objectName().isEmpty())
            yuvParamSet->setObjectName(QStringLiteral("yuvParamSet"));
        yuvParamSet->resize(600, 120);
        yuvParamSet->setMinimumSize(QSize(600, 120));
        yuvParamSet->setMaximumSize(QSize(619, 120));
        yuvParamSet->setAcceptDrops(false);
        gridLayout_2 = new QGridLayout(yuvParamSet);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        label = new QLabel(yuvParamSet);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label);

        label_2 = new QLabel(yuvParamSet);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label_2);


        horizontalLayout->addLayout(verticalLayout_3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        spinBox_width = new QSpinBox(yuvParamSet);
        spinBox_width->setObjectName(QStringLiteral("spinBox_width"));
        spinBox_width->setMinimum(240);
        spinBox_width->setMaximum(2560);
        spinBox_width->setSingleStep(2);
        spinBox_width->setValue(416);

        verticalLayout_2->addWidget(spinBox_width);

        spinBox_height = new QSpinBox(yuvParamSet);
        spinBox_height->setObjectName(QStringLiteral("spinBox_height"));
        spinBox_height->setMinimum(120);
        spinBox_height->setMaximum(1600);
        spinBox_height->setSingleStep(2);
        spinBox_height->setValue(240);

        verticalLayout_2->addWidget(spinBox_height);


        horizontalLayout->addLayout(verticalLayout_2);


        gridLayout_2->addLayout(horizontalLayout, 0, 0, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        p832 = new QPushButton(yuvParamSet);
        p832->setObjectName(QStringLiteral("p832"));

        gridLayout->addWidget(p832, 0, 1, 1, 1);

        p1280 = new QPushButton(yuvParamSet);
        p1280->setObjectName(QStringLiteral("p1280"));

        gridLayout->addWidget(p1280, 0, 3, 1, 1);

        p480 = new QPushButton(yuvParamSet);
        p480->setObjectName(QStringLiteral("p480"));

        gridLayout->addWidget(p480, 1, 1, 1, 1);

        p416 = new QPushButton(yuvParamSet);
        p416->setObjectName(QStringLiteral("p416"));

        gridLayout->addWidget(p416, 0, 0, 1, 1);

        p240 = new QPushButton(yuvParamSet);
        p240->setObjectName(QStringLiteral("p240"));

        gridLayout->addWidget(p240, 1, 0, 1, 1);

        p1920 = new QPushButton(yuvParamSet);
        p1920->setObjectName(QStringLiteral("p1920"));

        gridLayout->addWidget(p1920, 0, 4, 1, 1);

        p1024 = new QPushButton(yuvParamSet);
        p1024->setObjectName(QStringLiteral("p1024"));

        gridLayout->addWidget(p1024, 0, 2, 1, 1);

        p2560 = new QPushButton(yuvParamSet);
        p2560->setObjectName(QStringLiteral("p2560"));

        gridLayout->addWidget(p2560, 0, 5, 1, 1);

        p768 = new QPushButton(yuvParamSet);
        p768->setObjectName(QStringLiteral("p768"));

        gridLayout->addWidget(p768, 1, 2, 1, 1);

        p1600 = new QPushButton(yuvParamSet);
        p1600->setObjectName(QStringLiteral("p1600"));

        gridLayout->addWidget(p1600, 1, 5, 1, 1);

        p720 = new QPushButton(yuvParamSet);
        p720->setObjectName(QStringLiteral("p720"));

        gridLayout->addWidget(p720, 1, 3, 1, 1);

        p1080 = new QPushButton(yuvParamSet);
        p1080->setObjectName(QStringLiteral("p1080"));

        gridLayout->addWidget(p1080, 1, 4, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 1, 1, 1);

        buttonBox = new QDialogButtonBox(yuvParamSet);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(buttonBox->sizePolicy().hasHeightForWidth());
        buttonBox->setSizePolicy(sizePolicy);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 1, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label_3 = new QLabel(yuvParamSet);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_3);

        comboBox_fomat = new QComboBox(yuvParamSet);
        comboBox_fomat->setObjectName(QStringLiteral("comboBox_fomat"));
        comboBox_fomat->setMaxVisibleItems(3);
        comboBox_fomat->setMaxCount(10);
        comboBox_fomat->setFrame(false);

        horizontalLayout_2->addWidget(comboBox_fomat);


        gridLayout_2->addLayout(horizontalLayout_2, 1, 0, 1, 1);

        QWidget::setTabOrder(spinBox_width, spinBox_height);
        QWidget::setTabOrder(spinBox_height, p416);
        QWidget::setTabOrder(p416, p240);
        QWidget::setTabOrder(p240, p832);
        QWidget::setTabOrder(p832, p480);
        QWidget::setTabOrder(p480, p1024);
        QWidget::setTabOrder(p1024, p768);
        QWidget::setTabOrder(p768, p1280);
        QWidget::setTabOrder(p1280, p720);
        QWidget::setTabOrder(p720, p1920);
        QWidget::setTabOrder(p1920, p1080);
        QWidget::setTabOrder(p1080, p2560);
        QWidget::setTabOrder(p2560, p1600);
        QWidget::setTabOrder(p1600, comboBox_fomat);

        retranslateUi(yuvParamSet);
        QObject::connect(buttonBox, SIGNAL(accepted()), yuvParamSet, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), yuvParamSet, SLOT(reject()));

        comboBox_fomat->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(yuvParamSet);
    } // setupUi

    void retranslateUi(QDialog *yuvParamSet)
    {
        yuvParamSet->setWindowTitle(QApplication::translate("yuvParamSet", "yuvParamSet", 0));
        label->setText(QApplication::translate("yuvParamSet", "\345\256\275", 0));
        label_2->setText(QApplication::translate("yuvParamSet", "\351\253\230", 0));
        p832->setText(QApplication::translate("yuvParamSet", "832", 0));
        p1280->setText(QApplication::translate("yuvParamSet", "1280", 0));
        p480->setText(QApplication::translate("yuvParamSet", "480", 0));
        p416->setText(QApplication::translate("yuvParamSet", "416", 0));
        p240->setText(QApplication::translate("yuvParamSet", "240", 0));
        p1920->setText(QApplication::translate("yuvParamSet", "1920", 0));
        p1024->setText(QApplication::translate("yuvParamSet", "1024", 0));
        p2560->setText(QApplication::translate("yuvParamSet", "2560", 0));
        p768->setText(QApplication::translate("yuvParamSet", "768", 0));
        p1600->setText(QApplication::translate("yuvParamSet", "1600", 0));
        p720->setText(QApplication::translate("yuvParamSet", "720", 0));
        p1080->setText(QApplication::translate("yuvParamSet", "1080", 0));
        label_3->setText(QApplication::translate("yuvParamSet", "\351\207\207\346\240\267\346\240\274\345\274\217", 0));
        comboBox_fomat->setCurrentText(QString());
    } // retranslateUi

};

namespace Ui {
    class yuvParamSet: public Ui_yuvParamSet {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_YUVPARAMSET_H
