/****************************************************************************
** Meta object code from reading C++ file 'acquisitionengine.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../core/acquisitionengine.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'acquisitionengine.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17AcquisitionEngineE_t {};
} // unnamed namespace

template <> constexpr inline auto AcquisitionEngine::qt_create_metaobjectdata<qt_meta_tag_ZN17AcquisitionEngineE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AcquisitionEngine",
        "rawPacketReady",
        "",
        "RawPacket",
        "pkt",
        "warningMessage",
        "msg",
        "statusMessage",
        "start",
        "portName",
        "baudRate",
        "stop",
        "onRawInt16",
        "raw",
        "onSignalQuality",
        "v",
        "onUvReady",
        "uv"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'rawPacketReady'
        QtMocHelpers::SignalData<void(const RawPacket &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'warningMessage'
        QtMocHelpers::SignalData<void(const QString &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Signal 'statusMessage'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'start'
        QtMocHelpers::SlotData<void(const QString &, qint32)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::Int, 10 },
        }}),
        // Slot 'stop'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onRawInt16'
        QtMocHelpers::SlotData<void(qint16)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Short, 13 },
        }}),
        // Slot 'onSignalQuality'
        QtMocHelpers::SlotData<void(quint8)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::UChar, 15 },
        }}),
        // Slot 'onUvReady'
        QtMocHelpers::SlotData<void(double)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 17 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AcquisitionEngine, qt_meta_tag_ZN17AcquisitionEngineE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AcquisitionEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.metaTypes,
    nullptr
} };

void AcquisitionEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AcquisitionEngine *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->rawPacketReady((*reinterpret_cast<std::add_pointer_t<RawPacket>>(_a[1]))); break;
        case 1: _t->warningMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->statusMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->start((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint32>>(_a[2]))); break;
        case 4: _t->stop(); break;
        case 5: _t->onRawInt16((*reinterpret_cast<std::add_pointer_t<qint16>>(_a[1]))); break;
        case 6: _t->onSignalQuality((*reinterpret_cast<std::add_pointer_t<quint8>>(_a[1]))); break;
        case 7: _t->onUvReady((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< RawPacket >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const RawPacket & )>(_a, &AcquisitionEngine::rawPacketReady, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const QString & )>(_a, &AcquisitionEngine::warningMessage, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AcquisitionEngine::*)(const QString & )>(_a, &AcquisitionEngine::statusMessage, 2))
            return;
    }
}

const QMetaObject *AcquisitionEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AcquisitionEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17AcquisitionEngineE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AcquisitionEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AcquisitionEngine::rawPacketReady(const RawPacket & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void AcquisitionEngine::warningMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AcquisitionEngine::statusMessage(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
