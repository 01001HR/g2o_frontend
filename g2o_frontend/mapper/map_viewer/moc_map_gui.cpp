/****************************************************************************
** Meta object code from reading C++ file 'map_gui.h'
**
** Created: Fri Mar 8 18:32:26 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "map_gui.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'map_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MapGUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,
      29,    7,    7,    7, 0x0a,
      48,    7,    7,    7, 0x0a,
      70,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MapGUI[] = {
    "MapGUI\0\0loadReferenceGraph()\0"
    "loadCurrentGraph()\0clearReferenceGraph()\0"
    "clearCurrentGraph()\0"
};

void MapGUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MapGUI *_t = static_cast<MapGUI *>(_o);
        switch (_id) {
        case 0: _t->loadReferenceGraph(); break;
        case 1: _t->loadCurrentGraph(); break;
        case 2: _t->clearReferenceGraph(); break;
        case 3: _t->clearCurrentGraph(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MapGUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MapGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MapGUI,
      qt_meta_data_MapGUI, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MapGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MapGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MapGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MapGUI))
        return static_cast<void*>(const_cast< MapGUI*>(this));
    if (!strcmp(_clname, "Ui::MainWindow"))
        return static_cast< Ui::MainWindow*>(const_cast< MapGUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MapGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
