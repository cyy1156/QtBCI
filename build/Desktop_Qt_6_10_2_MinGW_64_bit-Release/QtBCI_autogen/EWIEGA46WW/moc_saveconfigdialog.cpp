/****************************************************************************
** Meta object code from reading C++ file 'saveconfigdialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../saveconfigdialog.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'saveconfigdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16SaveConfigDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto SaveConfigDialog::qt_create_metaobjectdata<qt_meta_tag_ZN16SaveConfigDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SaveConfigDialog",
        "newEegClicked",
        "",
        "openEegClicked",
        "newTxtClicked",
        "openTxtClicked",
        "newPsdClicked",
        "openPsdClicked",
        "newFftClicked",
        "openFftClicked",
        "serialConfigClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'newEegClicked'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'openEegClicked'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'newTxtClicked'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'openTxtClicked'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'newPsdClicked'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'openPsdClicked'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'newFftClicked'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'openFftClicked'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'serialConfigClicked'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SaveConfigDialog, qt_meta_tag_ZN16SaveConfigDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SaveConfigDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SaveConfigDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SaveConfigDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16SaveConfigDialogE_t>.metaTypes,
    nullptr
} };

void SaveConfigDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SaveConfigDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->newEegClicked(); break;
        case 1: _t->openEegClicked(); break;
        case 2: _t->newTxtClicked(); break;
        case 3: _t->openTxtClicked(); break;
        case 4: _t->newPsdClicked(); break;
        case 5: _t->openPsdClicked(); break;
        case 6: _t->newFftClicked(); break;
        case 7: _t->openFftClicked(); break;
        case 8: _t->serialConfigClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::newEegClicked, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::openEegClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::newTxtClicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::openTxtClicked, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::newPsdClicked, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::openPsdClicked, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::newFftClicked, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::openFftClicked, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (SaveConfigDialog::*)()>(_a, &SaveConfigDialog::serialConfigClicked, 8))
            return;
    }
}

const QMetaObject *SaveConfigDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SaveConfigDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SaveConfigDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int SaveConfigDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void SaveConfigDialog::newEegClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SaveConfigDialog::openEegClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SaveConfigDialog::newTxtClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SaveConfigDialog::openTxtClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SaveConfigDialog::newPsdClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SaveConfigDialog::openPsdClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SaveConfigDialog::newFftClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SaveConfigDialog::openFftClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SaveConfigDialog::serialConfigClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}
QT_WARNING_POP
