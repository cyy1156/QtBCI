/****************************************************************************
** Meta object code from reading C++ file 'eeg_frame_assembler.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../device/serial_eeg/eeg_frame_assembler.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'eeg_frame_assembler.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN23SerialEegFrameAssemblerE_t {};
} // unnamed namespace

template <> constexpr inline auto SerialEegFrameAssembler::qt_create_metaobjectdata<qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SerialEegFrameAssembler",
        "frameReady",
        "",
        "frame",
        "checksumFailureOccurred",
        "totalSoFar",
        "lengthResyncOccurred",
        "rxBufferOverflowed",
        "previousSize",
        "onBytes"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'frameReady'
        QtMocHelpers::SignalData<void(const QByteArray &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 3 },
        }}),
        // Signal 'checksumFailureOccurred'
        QtMocHelpers::SignalData<void(quint64)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 5 },
        }}),
        // Signal 'lengthResyncOccurred'
        QtMocHelpers::SignalData<void(quint64)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::ULongLong, 5 },
        }}),
        // Signal 'rxBufferOverflowed'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Slot 'onBytes'
        QtMocHelpers::SlotData<void(const QByteArray &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QByteArray, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SerialEegFrameAssembler, qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SerialEegFrameAssembler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>.metaTypes,
    nullptr
} };

void SerialEegFrameAssembler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialEegFrameAssembler *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->frameReady((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 1: _t->checksumFailureOccurred((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1]))); break;
        case 2: _t->lengthResyncOccurred((*reinterpret_cast<std::add_pointer_t<quint64>>(_a[1]))); break;
        case 3: _t->rxBufferOverflowed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onBytes((*reinterpret_cast<std::add_pointer_t<QByteArray>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SerialEegFrameAssembler::*)(const QByteArray & )>(_a, &SerialEegFrameAssembler::frameReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegFrameAssembler::*)(quint64 )>(_a, &SerialEegFrameAssembler::checksumFailureOccurred, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegFrameAssembler::*)(quint64 )>(_a, &SerialEegFrameAssembler::lengthResyncOccurred, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SerialEegFrameAssembler::*)(int )>(_a, &SerialEegFrameAssembler::rxBufferOverflowed, 3))
            return;
    }
}

const QMetaObject *SerialEegFrameAssembler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialEegFrameAssembler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN23SerialEegFrameAssemblerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialEegFrameAssembler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SerialEegFrameAssembler::frameReady(const QByteArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void SerialEegFrameAssembler::checksumFailureOccurred(quint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void SerialEegFrameAssembler::lengthResyncOccurred(quint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SerialEegFrameAssembler::rxBufferOverflowed(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
