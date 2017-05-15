/****************************************************************************
** Meta object code from reading C++ file 'longmanApp.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/appGUI/longmanApp.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'longmanApp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_longmanApp_t {
    QByteArrayData data[16];
    char stringdata0[350];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_longmanApp_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_longmanApp_t qt_meta_stringdata_longmanApp = {
    {
QT_MOC_LITERAL(0, 0, 10), // "longmanApp"
QT_MOC_LITERAL(1, 11, 38), // "on_actionOpen_SHVC_bitstream_..."
QT_MOC_LITERAL(2, 50, 0), // ""
QT_MOC_LITERAL(3, 51, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(4, 75, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(5, 100, 26), // "on_actionAboutQT_triggered"
QT_MOC_LITERAL(6, 127, 21), // "on_nextButton_clicked"
QT_MOC_LITERAL(7, 149, 21), // "on_backButton_clicked"
QT_MOC_LITERAL(8, 171, 30), // "on_FrameIdxSlider_valueChanged"
QT_MOC_LITERAL(9, 202, 22), // "on_beginButton_clicked"
QT_MOC_LITERAL(10, 225, 20), // "on_endButton_clicked"
QT_MOC_LITERAL(11, 246, 6), // "player"
QT_MOC_LITERAL(12, 253, 21), // "on_playButton_clicked"
QT_MOC_LITERAL(13, 275, 21), // "on_stopButton_clicked"
QT_MOC_LITERAL(14, 297, 32), // "on_actionSave_as_image_triggered"
QT_MOC_LITERAL(15, 330, 19) // "on_f1Button_clicked"

    },
    "longmanApp\0on_actionOpen_SHVC_bitstream_triggered\0"
    "\0on_actionOpen_triggered\0"
    "on_actionAbout_triggered\0"
    "on_actionAboutQT_triggered\0"
    "on_nextButton_clicked\0on_backButton_clicked\0"
    "on_FrameIdxSlider_valueChanged\0"
    "on_beginButton_clicked\0on_endButton_clicked\0"
    "player\0on_playButton_clicked\0"
    "on_stopButton_clicked\0"
    "on_actionSave_as_image_triggered\0"
    "on_f1Button_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_longmanApp[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    0,   85,    2, 0x08 /* Private */,
       4,    0,   86,    2, 0x08 /* Private */,
       5,    0,   87,    2, 0x08 /* Private */,
       6,    0,   88,    2, 0x08 /* Private */,
       7,    0,   89,    2, 0x08 /* Private */,
       8,    1,   90,    2, 0x08 /* Private */,
       9,    0,   93,    2, 0x08 /* Private */,
      10,    0,   94,    2, 0x08 /* Private */,
      11,    1,   95,    2, 0x08 /* Private */,
      12,    0,   98,    2, 0x08 /* Private */,
      13,    0,   99,    2, 0x08 /* Private */,
      14,    0,  100,    2, 0x08 /* Private */,
      15,    0,  101,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void longmanApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        longmanApp *_t = static_cast<longmanApp *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_actionOpen_SHVC_bitstream_triggered(); break;
        case 1: _t->on_actionOpen_triggered(); break;
        case 2: _t->on_actionAbout_triggered(); break;
        case 3: _t->on_actionAboutQT_triggered(); break;
        case 4: _t->on_nextButton_clicked(); break;
        case 5: _t->on_backButton_clicked(); break;
        case 6: _t->on_FrameIdxSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->on_beginButton_clicked(); break;
        case 8: _t->on_endButton_clicked(); break;
        case 9: _t->player((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->on_playButton_clicked(); break;
        case 11: _t->on_stopButton_clicked(); break;
        case 12: _t->on_actionSave_as_image_triggered(); break;
        case 13: _t->on_f1Button_clicked(); break;
        default: ;
        }
    }
}

const QMetaObject longmanApp::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_longmanApp.data,
      qt_meta_data_longmanApp,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *longmanApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *longmanApp::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_longmanApp.stringdata0))
        return static_cast<void*>(const_cast< longmanApp*>(this));
    if (!strcmp(_clname, "lmView"))
        return static_cast< lmView*>(const_cast< longmanApp*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int longmanApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
